#ifndef __APP_H__
#define __APP_H__

#include <wx/wx.h>
#include <wx/notebook.h>
#include <pvzclass.h>
#include <vector>

#define BOOLEAN_SETTING(s) bool s = false
#define INT_SETTING(s) int s = 0;

class App : public wxApp {
public:
    virtual bool OnInit();

    virtual int OnExit();
};

class Frame : public wxFrame {
public:
    bool loaded;

    BOOLEAN_SETTING(disableLawnMowers);
    BOOLEAN_SETTING(disableTopRow);
    BOOLEAN_SETTING(zombiesDropSuns);
    BOOLEAN_SETTING(frozenZombiesDropSuns);
    BOOLEAN_SETTING(autoCollect);

    INT_SETTING(autoCollectDelay);
    INT_SETTING(startingAmountOfSuns);
    INT_SETTING(waves);
    INT_SETTING(firstWaveDelay);

    std::vector<ZombieType::ZombieType> zombieTypes = {};
    int difficulty = 1;

    PVZLevel::PVZLevel level;
    int prevState = -1;
    int curState;

    Frame();

    void MainLoop(wxTimerEvent &event);
    void GameLoop();
    void OnGameStatePreparing();
    void OnGameStatePlaying();
    void OnGameStateMenu();

    void TryGameLoop();
    void TryGameStateEvent(int state);
    
    void SetBooleanSettingByIndex(int i, bool val) {
        switch(i) {
            case 0:
                disableLawnMowers = val;
                break;
            case 1:
                disableTopRow = val;
                break;
            case 2:
                zombiesDropSuns = val;
                break;
            case 3:
                frozenZombiesDropSuns = val;
                break;
            case 4:
                autoCollect = val;
                break;
        }
    }

    void SetIntSettingByIndex(int i, int val) {
        switch(i) {
            case 0:
                autoCollectDelay = val;
                break;
            case 1:
                startingAmountOfSuns = val;
                break;
            case 2:
                waves = val;
                break;
            case 3:
                firstWaveDelay = val;
                break;
        }
    }
};

class LevelPage : public wxNotebookPage {
public:
    LevelPage(wxWindow* parent);

    void Patch(wxCommandEvent &event);

    void ShowHelp(wxMouseEvent &event);
    void HideHelp(wxMouseEvent &event);
};

class SpawnPage : public wxNotebookPage {
public:
    SpawnPage(wxWindow* parent);

    void Apply(wxCommandEvent &event);

    void OnInput(wxKeyEvent& event);
    void OnSpinUp(wxSpinEvent& event);
    void OnSpinDown(wxSpinEvent& event);

    void ShowHelp(wxMouseEvent &event);
    void HideHelp(wxMouseEvent &event);
};

class ModifiersPage : public wxNotebookPage {
public:
    ModifiersPage(wxWindow* parent);

    void Apply(wxCommandEvent &event);

    void ShowHelp(wxMouseEvent &event);
    void HideHelp(wxMouseEvent &event);
};

class OptionsPage : public wxNotebookPage {
public:
    OptionsPage(wxWindow* parent);

    void Apply(wxCommandEvent &event);
    
    void OnInput(wxKeyEvent& event);
    void OnSpinUp(wxSpinEvent &event);
    void OnSpinDown(wxSpinEvent &event);

    void ShowHelp(wxMouseEvent &event);
    void HideHelp(wxMouseEvent &event);
};

#endif