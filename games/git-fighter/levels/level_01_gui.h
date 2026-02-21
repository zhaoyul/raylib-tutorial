#pragma once
#include "level_manager.h"
#include "../src/git_visualization.h"
#include "../src/git_wrapper.h"
#include "../src/gui_manager.h"
#include <memory>
#include <string>

// Level 1: Modern GUI version
class Level01_GUI : public Level {
public:
    Level01_GUI();
    ~Level01_GUI();
    
    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Shutdown() override;
    
    bool IsComplete() const override;
    
private:
    enum class Stage {
        INTRO,
        WAIT_INIT,
        WAIT_ADD,
        WAIT_COMMIT,
        COMPLETE
    };
    
    Stage currentStage;
    float timer;
    bool stageComplete;
    
    // Git
    std::unique_ptr<GitWrapper> git;
    std::string repoPath;
    std::string lastCommitHash;
    
    // Visualization
    std::unique_ptr<GitVis::SplitGitView> splitView;
    
    // GUI
    std::unique_ptr<GitGUI::GUIManager> gui;
    std::unique_ptr<GitGUI::StepIndicator> stepIndicator;
    std::unique_ptr<GitGUI::InfoCard> infoCard;
    
    // File creation flags
    bool readmeCreated;
    bool mainCppCreated;
    bool filesAdded;
    
    // Methods
    void CreateSampleFiles();
    void SyncGraphWithRepo();
    void ProcessGitCommand(const std::string& cmd);
    void DrawLeftPanel();
    void DrawRightPanel();
    void DrawBottomBar();
    void CheckGitStatus();
};
