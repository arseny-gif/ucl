#include "app.h"
#include <pvzclass.h>
#include <wx/spinbutt.h>
#include <wx/choice.h>
#include <wx/menu.h>
#include <string>
#include <sstream>
#include <vector>
#include <Const.h>
#include <iostream>

#define GetGrandparentStatusBar reinterpret_cast<wxFrame*>(GetGrandParent())->GetStatusBar
#define SetupHelpLabel(obj, cls) obj->Bind(wxEVT_ENTER_WINDOW, &cls::ShowHelp, this);obj->Bind(wxEVT_LEAVE_WINDOW, &cls::HideHelp, this);

template<typename T>
T clamp(T x, T min, T max) {
    if (x < min) x = min;
    if (x > max) x = max;
    return x;
}

std::vector<std::string> modifiers = {
    "Disable lawn mowers",
    "Disable top row",
    "Zombies drop suns",
    "Frozen zombies drop suns"
};

std::vector<std::string> options = {
    "Autocollect"
};

std::vector<std::string> valueOptions = {
    "Autocollect delay",
    "Starting amount of suns",
    "Waves",
    "First wave delay"
};

bool App::OnInit() {
    auto frame = new Frame();
    frame->Show();

    // logging
    //AllocConsole();
    //freopen("CONOUT$", "w", stdout);

    return true;
}

int App::OnExit() {
    PVZ::QuitPVZ();
    //FreeConsole();

    return 0;
}

Frame::Frame() : wxFrame(nullptr, wxID_ANY, "Ultimate Custom Level", wxDefaultPosition, wxSize(750, 295), wxDEFAULT_FRAME_STYLE ^ wxRESIZE_BORDER) {
    SetIcon(wxICON(APP_ICON));

    auto book = new wxNotebook(this, 1);

    auto levelPage = new LevelPage(book);
    auto spawnPage = new SpawnPage(book);
    auto modifiersPage = new ModifiersPage(book);
    auto optionsPage = new OptionsPage(book);

    wxColour col = book->GetThemeBackgroundColour();
    levelPage->SetBackgroundColour(col);
    spawnPage->SetBackgroundColour(col);
    modifiersPage->SetBackgroundColour(col);
    optionsPage->SetBackgroundColour(col);

    book->AddPage(levelPage, "Level");
    book->AddPage(spawnPage, "Spawn");
    book->AddPage(modifiersPage, "Modifiers");
    book->AddPage(optionsPage, "Options");

    auto bar = CreateStatusBar(2);
    int widths[2] = {-1, -2};
    bar->SetStatusWidths(2, widths);

    if(DWORD pid = ProcessOpener::Open()) {
        PVZ::InitPVZ(pid);
        bar->SetStatusText("PID: " + std::to_string(pid), 0);
        loaded = true;
    } else {
        wxMessageBox("Failed to open the process.", "Error", wxOK | wxICON_ERROR);
        bar->SetStatusText("Not loaded", 0);
        loaded = false;
    }

    auto timer = new wxTimer(this);
    this->Bind(wxEVT_TIMER, &Frame::MainLoop, this);
    timer->Start(1);
}

void Frame::MainLoop(wxTimerEvent &event) {
    if(!loaded) return;

    curState = PVZ::GetPVZApp()->GameState;
    if(prevState == -1) {
        prevState = curState;
    } else if(prevState != curState) {
        prevState = curState;
        TryGameStateEvent(curState);
    }

    TryGameLoop();
}

LevelPage::LevelPage(wxWindow* parent) : wxNotebookPage(parent, wxID_ANY) {
    auto levelNameL = new wxStaticText(this, wxID_ANY, "Level name", wxPoint(10, 10));
    auto levelName = new wxTextCtrl(this, 1, "zombari", wxPoint(80, 7));
    SetupHelpLabel(levelName, LevelPage);

    auto levelTypeL = new wxStaticText(this, wxID_ANY, "Level type", wxPoint(10, 39));
    auto levelArray = wxArrayString();
    for(int i = 0; i <= 72; i++) {
        levelArray.push_back(PVZLevel::ToString(static_cast<PVZLevel::PVZLevel>(i)));
    }
    auto levelType = new wxChoice(this, 2, wxPoint(80, 37), wxSize(175, 21), levelArray);
    levelType->SetSelection(PVZLevel::ZomBotany);
    SetupHelpLabel(levelType, LevelPage);

    auto levelSceneL = new wxStaticText(this, wxID_ANY, "Level scene", wxPoint(10, 69));
    auto sceneArray = wxArrayString();
    for(int i = 0; i <= 9; i++) {
        sceneArray.push_back(SceneType::ToString(static_cast<SceneType::SceneType>(i)));
    }
    auto levelScene = new wxChoice(this, 3, wxPoint(80, 66), wxSize(125, 21), sceneArray);
    levelScene->SetSelection(SceneType::Day);
    SetupHelpLabel(levelScene, LevelPage);

    auto patchButton = new wxButton(this, 4, "Patch", wxPoint(630, 150));
    patchButton->Bind(wxEVT_BUTTON, &LevelPage::Patch, this);
    SetupHelpLabel(patchButton, LevelPage);
}

void LevelPage::Patch(wxCommandEvent &event) {
    if(!reinterpret_cast<Frame*>(GetGrandParent())->loaded) {
        wxMessageBox("PVZ process is not found.", "Warning", wxOK | wxICON_WARNING);
        return;
    }

    std::string levelName = reinterpret_cast<wxTextCtrl*>(this->GetWindowChild(1))->GetValue();
    int levelType = reinterpret_cast<wxChoice*>(this->GetWindowChild(2))->GetSelection();
    int levelScene = reinterpret_cast<wxChoice*>(this->GetWindowChild(3))->GetSelection();

    reinterpret_cast<Frame*>(GetGrandParent())->level = static_cast<PVZLevel::PVZLevel>(levelType);

    // LEVEL NAME PATCH
    int ptr = PVZ::GetChallengeDefinition(static_cast<PVZLevel::PVZLevel>(levelType))->NamePTR;
    size_t size = NULL;
    for(int i = 0;;i++) {
        if(PVZ::Memory::ReadMemory<char>(ptr + i) != 0x00) {
            size++;
        } else {
            break;
        }
    }
    for(int i = 0; i < size; i++) {
        DWORD oldprotect;
        char u = 0x1f;
        VirtualProtectEx(PVZ::Memory::hProcess, (LPVOID)(ptr + i), sizeof(char), PAGE_EXECUTE_READWRITE, &oldprotect);
        if(i < levelName.length()) {
            WriteProcessMemory(PVZ::Memory::hProcess, (LPVOID)(ptr + i), &levelName[i], sizeof(char), NULL);
        } else {
            WriteProcessMemory(PVZ::Memory::hProcess, (LPVOID)(ptr + i), &u, sizeof(char), NULL);
        }
        VirtualProtectEx(PVZ::Memory::hProcess, (LPVOID)(ptr + i), sizeof(char), oldprotect, &oldprotect);
    }

    // LEVEL SCENE PATCH
    Const::SetLevelScene(static_cast<PVZLevel::PVZLevel>(levelType), static_cast<SceneType::SceneType>(levelScene));

    wxMessageBox("Successfully patched.", "Level patch", wxOK | wxICON_INFORMATION);
}

void LevelPage::ShowHelp(wxMouseEvent &event) {
    std::string text = "";
    switch(event.GetId()) {
        case 1:
            text = "Your own level name";
            break;
        case 2:
            text = "Level mode (type) to replace (recommended to choose Zombotany)";
            break;
        case 3:
            text = "Level scene type";
            break;
        case 4:
            text = "Patch the level";
            break;
    }
    if(text.length() > 0) GetGrandparentStatusBar()->SetStatusText(text, 1);
}

void LevelPage::HideHelp(wxMouseEvent &event) {
    GetGrandparentStatusBar()->SetStatusText("", 1);
}

SpawnPage::SpawnPage(wxWindow* parent) : wxNotebookPage(parent, wxID_ANY) {
    int x = 10, y = 10;
    for(int i = 1; i <= 33; i++) {
        std::string zombie = ZombieType::ToString(static_cast<ZombieType::ZombieType>(i - 1));
        std::stringstream zss;
        for(int c = 0; c < zombie.length(); c++) {
            if(std::isupper(zombie[c]) && c != 0) {
                zss << " " << zombie[c];
            } else {
                zss << zombie[c];
            }
        }
        auto checkBox = new wxCheckBox(this, 100 + i, zss.str().c_str(), wxPoint(x, y));
        SetupHelpLabel(checkBox, SpawnPage);
        zss.clear();

        if(i % 7 == 0 && i != 0) {
            x += 145;
            y = 10;
        } else {
            y += 23;
        }
    }

    auto applyButton = new wxButton(this, 2, "Apply", wxPoint(630, 150));
    applyButton->Bind(wxEVT_BUTTON, &SpawnPage::Apply, this);

    SetupHelpLabel(applyButton, SpawnPage);

    auto difficultyInput = new wxTextCtrl(this, 1, "1", wxPoint(585, 150), wxSize(25, 22), NULL, wxTextValidator(wxFILTER_NUMERIC));

    auto difficultyCtrl = new wxSpinButton(this, wxID_ANY, wxPoint(610, 150), wxSize(20, 22));
    difficultyCtrl->Bind(wxEVT_SPIN_UP, &SpawnPage::OnSpinUp, this);
    difficultyCtrl->Bind(wxEVT_SPIN_DOWN, &SpawnPage::OnSpinDown, this);

    difficultyInput->Bind(wxEVT_AFTER_CHAR, &SpawnPage::OnInput, this);
    difficultyInput->SetMaxLength(2);
    SetupHelpLabel(difficultyInput, SpawnPage);
}

void SpawnPage::Apply(wxCommandEvent &event) {
    if(!reinterpret_cast<Frame*>(GetGrandParent())->loaded) {
        wxMessageBox("PVZ process is not found.", "Warning", wxOK | wxICON_WARNING);
        return;
    }
    reinterpret_cast<Frame*>(GetGrandParent())->zombieTypes.clear();
    for(int i = 101; i <= 133; i++) {
        auto checkBox = reinterpret_cast<wxCheckBox*>(this->GetWindowChild(i));
        if(!checkBox) return;
        if(checkBox->GetValue()) {
            reinterpret_cast<Frame*>(GetGrandParent())->zombieTypes.push_back(static_cast<ZombieType::ZombieType>(i - 101));
        }
    }
    reinterpret_cast<Frame*>(GetGrandParent())->difficulty = std::atoi(reinterpret_cast<wxTextCtrl*>(this->GetWindowChild(1))->GetValue());
}

void SpawnPage::OnInput(wxKeyEvent &event) {
    wxString val = reinterpret_cast<wxTextCtrl*>(this->GetWindowChild(1))->GetValue();
    if(std::atoi(val) > 50 || std::atoi(val) < 1) {
        reinterpret_cast<wxTextCtrl*>(this->GetWindowChild(1))->SetValue(std::to_string(clamp<int>(std::atoi(val), 1, 50)));
    }
}

void SpawnPage::OnSpinUp(wxSpinEvent &event) {
    auto difficultyInput = reinterpret_cast<wxTextCtrl*>(this->GetWindowChild(1));
    int value = std::atoi(difficultyInput->GetValue());
    if(value < 50) {
        difficultyInput->SetValue(std::to_string(value + 1));
    }
}

void SpawnPage::OnSpinDown(wxSpinEvent &event) {
    auto difficultyInput = reinterpret_cast<wxTextCtrl*>(this->GetWindowChild(1));
    int value = std::atoi(difficultyInput->GetValue());
    if(value > 1) {
        difficultyInput->SetValue(std::to_string(value - 1));
    }
}

void SpawnPage::ShowHelp(wxMouseEvent &event) {
    std::string text = "";
    switch(event.GetId()) {
        case 1:
            text = "Level difficulty (zombie count multiplier)";
            break;
        case 2:
            text = "Apply";
            break;
        case 100:
            text = "Zombie type";
            break;
    }
    if(text.length() > 0) GetGrandparentStatusBar()->SetStatusText(text, 1);
}

void SpawnPage::HideHelp(wxMouseEvent &event) {
    GetGrandparentStatusBar()->SetStatusText("", 1);
}

ModifiersPage::ModifiersPage(wxWindow* parent) : wxNotebookPage(parent, wxID_ANY) {
    auto apply = new wxButton(this, 1, "Apply", wxPoint(630, 150));
    SetupHelpLabel(apply, ModifiersPage);
    apply->Bind(wxEVT_BUTTON, &ModifiersPage::Apply, this);

    int x = 10, y = 10;
    for(int i = 0; i < modifiers.size(); i++) {
        auto checkBox = new wxCheckBox(this, 100 + i, modifiers[i], wxPoint(x, y));
        SetupHelpLabel(checkBox, ModifiersPage);

        if(i % 7 == 0 && i != 0) {
            x += 145;
            y = 10;
        } else {
            y += 23;
        }
    }
}

void ModifiersPage::Apply(wxCommandEvent &event) {\
    if(!reinterpret_cast<Frame*>(GetGrandParent())->loaded) {
        wxMessageBox("PVZ process is not found.", "Warning", wxOK | wxICON_WARNING);
        return;
    }
    for(int i = 0; i < modifiers.size(); i++) {
        reinterpret_cast<Frame*>(GetGrandParent())->SetBooleanSettingByIndex(i, reinterpret_cast<wxCheckBox*>(GetWindowChild(100 + i))->GetValue());
    }
}

void ModifiersPage::ShowHelp(wxMouseEvent &event) {
    std::string text = "";
    switch(event.GetId()) {
        case 1:
            text = "Apply";
            break;
        case 100:
            text = "Disable lawn mowers";
            break;
        case 101:
            text = "Disable top (the first) row of zombies for free space for plants";
            break;
        case 102:
            text = "Whether zombies drop suns on death";
            break;
        case 103:
            text = "Whether zombies drop suns when freezing with an iceshroom";
            break;
    }
    if(text.length() > 0) GetGrandparentStatusBar()->SetStatusText(text, 1);
}

void ModifiersPage::HideHelp(wxMouseEvent &event) {
    GetGrandparentStatusBar()->SetStatusText("", 1);
}

OptionsPage::OptionsPage(wxWindow* parent) : wxNotebookPage(parent, wxID_ANY) {
    auto apply = new wxButton(this, 1, "Apply", wxPoint(630, 150));
    apply->Bind(wxEVT_BUTTON, &OptionsPage::Apply, this);
    SetupHelpLabel(apply, OptionsPage);

    int x = 10, y = 10;
    for(int i = 0; i < options.size(); i++) {
        auto checkBox = new wxCheckBox(this, 100 + i, options[i], wxPoint(x, y));
        SetupHelpLabel(checkBox, OptionsPage);

        if(i % 7 == 0 && i != 0) {
            x += 145;
            y = 10;
        } else {
            y += 23;
        }
    }
    for(int i = 0; i < valueOptions.size(); i++) {
        auto label = new wxStaticText(this, wxID_ANY, valueOptions[i], wxPoint(x, y));
        auto input = new wxTextCtrl(this, 200 + i, "0", wxPoint(x + 150, y - 4), wxSize(75, 22), NULL, wxTextValidator(wxFILTER_NUMERIC));
        auto ctrl = new wxSpinButton(this, 300 + i, wxPoint(x + 225, y - 4), wxSize(20, 22));
        ctrl->Bind(wxEVT_SPIN_UP, &OptionsPage::OnSpinUp, this);
        ctrl->Bind(wxEVT_SPIN_DOWN, &OptionsPage::OnSpinDown, this);
        ctrl->SetMin(0);

        input->Bind(wxEVT_AFTER_CHAR, &OptionsPage::OnInput, this);

        SetupHelpLabel(input, OptionsPage);

        if(i % 7 == 0 && i != 0) {
            x += 145;
            y = 10;
        } else {
            y += 23;
        }
    }
}

void OptionsPage::Apply(wxCommandEvent &event) {
    if(!reinterpret_cast<Frame*>(GetGrandParent())->loaded) {
        wxMessageBox("PVZ process is not found.", "Warning", wxOK | wxICON_WARNING);
        return;
    }

    reinterpret_cast<Frame*>(GetGrandParent())->SetBooleanSettingByIndex(4, reinterpret_cast<wxCheckBox*>(GetWindowChild(100))->GetValue());
    for(int i = 0; i < valueOptions.size(); i++) {
        reinterpret_cast<Frame*>(GetGrandParent())->SetIntSettingByIndex(i, std::atoi(reinterpret_cast<wxTextCtrl*>(GetWindowChild(200 + i))->GetValue()));
    }
}

void OptionsPage::OnInput(wxKeyEvent& event) {
    wxString val = reinterpret_cast<wxTextCtrl*>(this->GetWindowChild(event.GetId()))->GetValue();
    if(std::atoi(val) < 0) {
        event.Skip();
    }
}

void OptionsPage::OnSpinUp(wxSpinEvent& event) {
    auto input = reinterpret_cast<wxTextCtrl*>(this->GetWindowChild(event.GetId() - 100));
    int value = std::atoi(input->GetValue());
    input->SetValue(std::to_string(value + 1));
}

void OptionsPage::OnSpinDown(wxSpinEvent& event) {
    auto input = reinterpret_cast<wxTextCtrl*>(this->GetWindowChild(event.GetId() - 100));
    int value = std::atoi(input->GetValue());
    if(value > 0) {
        input->SetValue(std::to_string(value - 1));
    }
}

void OptionsPage::ShowHelp(wxMouseEvent &event) {
    std::string text = "";
    switch(event.GetId()) {
        case 1:
            text = "Apply";
            break;
        case 100:
            text = "Autocollect of suns and coins";
            break;
        case 200:
            text = "Autocollect delay in cs (centiseconds)";
            break;
        case 201:
            text = "Starting amount of suns";
            break;
        case 202:
            text = "Total number of waves of zombies (10 waves = 1 huge wave)";
            break;
        case 203:
            text = "Delay of the first wave of zombies";
            break;
    }
    if(text.length() > 0) GetGrandparentStatusBar()->SetStatusText(text, 1);
}

void OptionsPage::HideHelp(wxMouseEvent &event) {
    GetGrandparentStatusBar()->SetStatusText("", 1);
}