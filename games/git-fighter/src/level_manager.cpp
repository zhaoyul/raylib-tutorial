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

// Shared implementation of ExecuteGitCommand for all levels
std::string Level::ExecuteGitCommand(const std::string& cmd) {
    RecordGitCommand(cmd);
    
    GitWrapper* git = GetGitWrapper();
    if (!git) {
        return "Error: Git not initialized";
    }
    
    // Parse and execute basic git commands
    if (cmd == "init") {
        auto result = git->Init(".");
        SyncGraphWithRepo();
        return result.success ? "Initialized empty Git repository" : result.error;
    }
    else if (cmd.rfind("add", 0) == 0) {
        std::string file = cmd.length() > 4 ? cmd.substr(4) : ".";
        auto result = git->Add(file);
        return result.success ? "Added " + file : result.error;
    }
    else if (cmd.rfind("commit", 0) == 0) {
        size_t spacePos = cmd.find(' ');
        std::string msg = (spacePos != std::string::npos) ? cmd.substr(spacePos + 1) : "commit";
        auto result = git->Commit(msg);
        if (result.success) {
            SyncGraphWithRepo();
            return "Committed: " + msg;
        }
        return result.error;
    }
    else if (cmd == "status") {
        auto result = git->Status();
        return result.success ? result.output : result.error;
    }
    else if (cmd.rfind("branch", 0) == 0) {
        if (cmd.length() > 7) {
            std::string branchName = cmd.substr(7);
            auto result = git->CreateBranch(branchName);
            SyncGraphWithRepo();
            return result.success ? "Created branch " + branchName : result.error;
        } else {
            auto branches = git->GetBranches();
            std::string result = "Branches:\n";
            for (const auto& b : branches) result += "  " + b + "\n";
            return result;
        }
    }
    else if (cmd.rfind("checkout", 0) == 0) {
        std::string branch = cmd.length() > 9 ? cmd.substr(9) : "main";
        auto result = git->Checkout(branch);
        SyncGraphWithRepo();
        return result.success ? "Switched to " + branch : result.error;
    }
    else if (cmd.rfind("merge", 0) == 0) {
        std::string branch = cmd.length() > 6 ? cmd.substr(6) : "";
        if (!branch.empty()) {
            auto result = git->Merge(branch);
            SyncGraphWithRepo();
            return result.success ? "Merged " + branch : result.error;
        }
        return "Usage: merge <branch>";
    }
    
    // Pass to level-specific handler
    return ProcessLevelCommand(cmd);
}

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
