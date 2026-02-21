#pragma once
#include "level_manager.h"
#include <memory>

// Level 1: 初出茅庐 - 周末加班
// Teach: init, add, commit
class Level01_Init : public Level {
public:
    Level01_Init();
    
    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Shutdown() override;
    
    bool IsComplete() const override;
    
private:
    // Tutorial stages
    enum class Stage {
        DIALOGUE_CTO,        // CTO introduction
        SHOW_INSTRUCTIONS,   // Show commands to learn
        WAIT_INIT,          // Wait for git init
        CREATE_FILES,       // Auto-create some files
        WAIT_ADD,           // Wait for git add
        WAIT_COMMIT,        // Wait for git commit
        COMPLETE            // Level complete
    };
    
    Stage currentStage;
    float timer;
    int commitCount;
    std::string repoPath;  // Temp directory for this level
    
    // Visual elements
    void DrawWorkspace();
    void DrawCommandHint();
    void DrawStageIndicator();
    void DrawDialogue();
    
    // Helper
    void AdvanceStage();
    bool CheckCommandInput();
    
    // Git wrapper (passed from LevelManager)
    class GitWrapper* git;
};
