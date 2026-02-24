#include "level_manager.h"
#include "git_wrapper.h"
#include "../levels/level_01_realgit.h"
#include "../levels/level_02_branch.h"
#include "../levels/level_03_merge.h"
#include "../levels/level_04_remote.h"
#include "../levels/level_05_rebase.h"
#include "../levels/level_06_cherrypick.h"
#include "../levels/level_07_bisect.h"
#include "../levels/level_08_reflog.h"
#include "../levels/level_09_interactive.h"
#include "../levels/level_10_stash.h"
#include <iostream>
#include <algorithm>

Level::Level(int id, const std::string& name, const std::string& desc)
    : levelId(id), levelName(name), description(desc) {}

LevelManager::LevelManager() = default;

LevelManager::~LevelManager() {
    // Ensure current level is properly shut down
    if (currentLevel) {
        currentLevel->Shutdown();
        currentLevel.reset();
    }
}

bool LevelManager::Initialize() {
    git = std::make_unique<GitWrapper>();
    
    // Load Chinese font
    font.Load();
    
    // Register all levels - store them in a separate registry
    auto level1 = std::make_unique<Level01_RealGit>();
    levels.push_back(std::move(level1));
    
    auto level2 = std::make_unique<Level02_Branch>();
    levels.push_back(std::move(level2));
    
    auto level3 = std::make_unique<Level03_Merge>();
    levels.push_back(std::move(level3));
    
    auto level4 = std::make_unique<Level04_Remote>();
    levels.push_back(std::move(level4));
    
    // Level 5: Rebase 变基危机
    auto level5 = std::make_unique<Level05_Rebase>();
    levels.push_back(std::move(level5));
    
    // Level 6: Cherry-pick 紧急修复
    auto level6 = std::make_unique<Level06_CherryPick>();
    levels.push_back(std::move(level6));
    
    // Level 7: Bisect 故障定位
    auto level7 = std::make_unique<Level07_Bisect>();
    levels.push_back(std::move(level7));
    
    // Level 8: Reflog 时光回溯
    auto level8 = std::make_unique<Level08_Reflog>();
    levels.push_back(std::move(level8));
    
    // Level 9: Interactive Rebase 历史重写
    auto level9 = std::make_unique<Level09_Interactive>();
    levels.push_back(std::move(level9));
    
    // Level 10: Stash 战场
    auto level10 = std::make_unique<Level10_Stash>();
    levels.push_back(std::move(level10));
    
    return true;
}

void LevelManager::RegisterLevel(std::unique_ptr<Level> level) {
    levels.push_back(std::move(level));
}

void LevelManager::LoadLevel(int levelId) {
    // Find level by ID
    auto it = std::find_if(levels.begin(), levels.end(),
        [levelId](const auto& level) { return level->GetId() == levelId; });
    
    if (it != levels.end()) {
        UnloadCurrentLevel();
        // Move ownership to currentLevel
        currentLevel = std::move(*it);
        // Remove from available levels
        levels.erase(it);
        
        // Set font for level
        currentLevel->SetFont(&font);
        
        // Set command history callback
        currentLevel->onGitCommand = [this](const std::string& cmd) {
            this->RecordCommand(cmd);
        };
        
        currentLevel->Initialize();
        std::cout << "Loaded Level " << levelId << ": " << currentLevel->GetName() << std::endl;
    }
}

void LevelManager::UnloadCurrentLevel() {
    if (currentLevel) {
        currentLevel->Shutdown();
        // Return to available levels
        levels.push_back(std::move(currentLevel));
        currentLevel.reset();
    }
}

void LevelManager::Update(float deltaTime) {
    if (currentLevel) {
        currentLevel->Update(deltaTime);
    }
}

void LevelManager::Draw() {
    if (currentLevel) {
        currentLevel->Draw();
    }
}

bool LevelManager::IsCurrentLevelComplete() const {
    return currentLevel && currentLevel->IsComplete();
}
