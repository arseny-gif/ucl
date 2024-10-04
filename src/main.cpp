#include "app.h"
#include <pvzclass.h>
#include <string>
#include <random>

template<typename T>
T clamp(T x, T min, T max) {
    if (x < min) x = min;
    if (x > max) x = max;
    return x;
}

void SetZombies(std::vector<ZombieType::ZombieType> zombieList) {
    auto board = PVZ::GetBoard();
    SPT<PVZ::Wave> wave;
    while(board->GetWave(0) == nullptr) continue;
    for(int w = 0; w < board->WaveCount; w++) {
        wave = board->GetWave(w);
        if(wave == nullptr) continue;
        wave->SetAll(zombieList.data(), zombieList.size());
    }
}

void Frame::OnGameStatePreparing() {
    auto board = PVZ::GetBoard();
    if(board == nullptr) return;

    std::vector<ZombieType::ZombieType> multipliedZombies;
    int am;
    for(int i = 0; i < zombieTypes.size(); i++) {
        am = clamp<int>(difficulty, 1, (int)(50 / zombieTypes.size()));
        for(int j = 1; j < am; j++) {
            multipliedZombies.push_back(zombieTypes[i]);
        }
    }
    SetZombies(multipliedZombies);
    board->Sun = startingAmountOfSuns;
    board->WaveCount = waves;
    board->LevelProcessBar = 0;
    board->HugeWaveCountdown = firstWaveDelay;
}

void Frame::OnGameStatePlaying() {
    SPT<PVZ::Board> board = PVZ::GetBoard();
    if(board == nullptr) return;

    if(disableLawnMowers) {
        std::vector<SPT<PVZ::Lawnmover>> lawnMowers = board->GetAllLawnmovers();
        for(int lmi = 0; lmi < lawnMowers.size(); lmi++) {
            lawnMowers[lmi]->NotExist = true;
        }
    }
}

void Frame::OnGameStateMenu() {
}

std::mt19937 rng(std::random_device{}());
void Frame::GameLoop() {
    if(PVZ::GetPVZApp()->GameState == PVZGameState::Playing) {
        SPT<PVZ::Board> board = PVZ::GetBoard();
        if(board == nullptr) return;
        //if(PVZ::Memory::ReadMemory<PVZLevel::PVZLevel>(PVZ::Memory::ReadMemory<int>(board->GetBaseAddress() + 0x8C) + 0x7F8) != level) return;
        if(board->GamePaused) return;

        PVZ::Memory::WaitPVZ();
        std::vector<SPT<PVZ::Coin>> coins = board->GetAllCoins();
        for(int c = 0; c < coins.size(); c++) {
            SPT<PVZ::Coin> coin = coins.at(c);
            if(zombiesDropSuns) {
                switch(coin->Type) {
                    case CoinType::SilverDollar:
                        coin->NotExist = true;
                        Creator::CreateCoin(CoinType::MiniSun, coin->X, coin->Y, CoinMotionType::Product);
                        coins.erase(coins.begin() + c);
                        break;
                    case CoinType::GoldDollar:
                        coin->NotExist = true;
                        Creator::CreateCoin(CoinType::NormalSun, coin->X, coin->Y, CoinMotionType::Product);
                        coins.erase(coins.begin() + c);
                        break;
                    case CoinType::Diamond:
                        coin->NotExist = true;
                        Creator::CreateCoin(CoinType::LargeSun, coin->X, coin->Y, CoinMotionType::Product);
                        coins.erase(coins.begin() + c);
                        break;
                }
            }
            if(coin->ExistedTime >= autoCollectDelay && autoCollect && !coin->NotExist) {
                switch(coin->Type) {
                    case CoinType::MiniSun:
                    case CoinType::NormalSun:
                    case CoinType::LargeSun:
                    case CoinType::SilverDollar:
                    case CoinType::GoldDollar:
                    case CoinType::Diamond:
                        coin->Collect();
                        break;
                }
            }
        }

        std::vector<SPT<PVZ::Zombie>> zombies = board->GetAllZombies();
        int prevRow = 0;
        if(disableTopRow) {
            for(int z = 0; z < zombies.size(); z++) {
                prevRow++;
                if(prevRow = 6) prevRow = 1;
                if(zombies.at(z)->Row == 0) zombies.at(z)->Row = prevRow;
            }
        }
        
        if(frozenZombiesDropSuns) {
            std::vector<SPT<PVZ::Plant>> plants = board->GetAllPlants();
            for(int p = 0; p < plants.size(); p++) {
                SPT<PVZ::Plant> plant = plants.at(p);
                if(plant->Type == PlantType::Iceshroom && plant->EffectiveCountdown <= 2 && plant->EffectiveCountdown > 0) {
                    for(int z = 0; z < zombies.size(); z++) {
                        if(std::uniform_int_distribution<>{0, 1}(rng)) {
                            Creator::CreateCoin(CoinType::MiniSun, zombies.at(z)->X, zombies.at(z)->Y, CoinMotionType::Product);
                        }
                    }
                }
            }
        }
        PVZ::Memory::ResumePVZ();
    }
}

void Frame::TryGameLoop() {
    __try {
        GameLoop();
    } __except(EXCEPTION_ACCESS_VIOLATION) {
        TryGameLoop();
    }
}

void Frame::TryGameStateEvent(int state) {
    __try {
        if(state == 2) OnGameStatePreparing();
        else if(state == 3) OnGameStatePlaying();
        else if(state == 1) OnGameStateMenu();
    } __except(EXCEPTION_ACCESS_VIOLATION) {
        TryGameStateEvent(state);
    }
}

wxIMPLEMENT_APP(App);