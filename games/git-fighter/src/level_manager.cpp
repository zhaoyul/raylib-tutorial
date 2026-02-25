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
#include <set>

Level::Level(int id, const std::string& name, const std::string& desc)
    : levelId(id), levelName(name), description(desc) {}

// Common dev tool keys (1/2/3) for all levels
void Level::HandleDevToolKeys() {
    GitWrapper* git = GetGitWrapper();
    if (!git || !git->IsRepoOpen() || repoPath.empty()) return;
    
    bool fileChanged = false;
    
    if (IsKeyPressed(KEY_ONE)) {
        // 1: 创建随机文件
        std::string filename = git->GenerateRandomFilename();
        std::cout << "[KEY_ONE] Creating random file: " << filename << std::endl;
        if (git->CreateRandomFile()) {
            std::cout << "[KEY_ONE] Created: " << filename << std::endl;
            fileChanged = true;
        }
    }
    if (IsKeyPressed(KEY_TWO)) {
        // 2: 创建随机目录
        std::string dirname = git->GenerateRandomDirname();
        std::cout << "[KEY_TWO] Creating random directory: " << dirname << std::endl;
        if (git->CreateRandomDirectory()) {
            std::cout << "[KEY_TWO] Created: " << dirname << std::endl;
            fileChanged = true;
        }
    }
    if (IsKeyPressed(KEY_THREE)) {
        // 3: 追加随机内容到现有文件
        auto files = git->GetWorkingDirectoryStatus();
        if (!files.empty()) {
            std::string targetFile = files[rand() % files.size()].path;
            std::cout << "[KEY_THREE] Appending to: " << targetFile << std::endl;
            if (git->AppendRandomContent(targetFile)) {
                fileChanged = true;
            }
        }
    }
    
    // Refresh view if files changed
    if (fileChanged) {
        RefreshWorkingDirectory();
    }
}

// Shared implementation of ExecuteGitCommand for all levels
std::string Level::ExecuteGitCommand(const std::string& cmd) {
    RecordGitCommand(cmd);
    
    GitWrapper* git = GetGitWrapper();
    if (!git) {
        return "Error: Git not initialized";
    }
    
    // First, try level-specific command handling (for game progression)
    std::string levelResult = ProcessLevelCommand(cmd);
    if (!levelResult.empty()) {
        // Refresh both graph and working directory after level-specific command
        SyncGraphWithRepo();
        RefreshWorkingDirectory();
        return levelResult;
    }
    
    // If level didn't handle it, parse and execute basic git commands
    // Use repoPath for operations that need path
    if (cmd == "init") {
        std::string targetPath = repoPath.empty() ? "." : repoPath;
        auto result = git->Init(targetPath);
        if (result.success) {
            git->OpenRepo(targetPath);
        }
        SyncGraphWithRepo();
        return result.success ? "Initialized empty Git repository" : result.error;
    }
    else if (cmd.rfind("add", 0) == 0) {
        // Parse add command: "add", "add .", "add --all", "add file.cpp"
        std::string file = ".";  // default
        if (cmd.length() > 4) {
            std::string arg = cmd.substr(4);
            // Handle "add --all" or "add -A"
            if (arg == "--all" || arg == "-A" || arg == "." || arg == "*") {
                file = ".";
            } else {
                file = arg;  // specific file
            }
        }
        auto result = git->Add(file);
        // Refresh both graph (staging area) and working directory
        SyncGraphWithRepo();
        RefreshWorkingDirectory();
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
        // Refresh working directory to show current status
        RefreshWorkingDirectory();
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
        std::string branch = cmd.length() > 9 ? cmd.substr(9) : "master";
        auto result = git->Checkout(branch);
        SyncGraphWithRepo();
        RefreshWorkingDirectory();
        return result.success ? "Switched to " + branch : result.error;
    }
    else if (cmd.rfind("merge", 0) == 0) {
        std::string branch = cmd.length() > 6 ? cmd.substr(6) : "";
        if (!branch.empty()) {
            auto result = git->Merge(branch);
            SyncGraphWithRepo();
            RefreshWorkingDirectory();
            return result.success ? "Merged " + branch : result.error;
        }
        return "Usage: merge <branch>";
    }
    else if (cmd.rfind("reset", 0) == 0) {
        // Parse: reset --hard HEAD~1, reset --hard <commit>, reset <commit>
        size_t spacePos = cmd.find(' ');
        if (spacePos != std::string::npos) {
            std::string args = cmd.substr(spacePos + 1);
            // Remove --hard if present
            if (args.rfind("--hard ", 0) == 0) {
                args = args.substr(7);
            } else if (args == "--hard") {
                args = "HEAD";
            }
            auto result = git->ResetHard(args);
            SyncGraphWithRepo();
            RefreshWorkingDirectory();
            return result.success ? "Reset to " + args : result.error;
        }
        return "Usage: reset --hard <target>";
    }
    
    // Fallback: Execute any other git command using system git
    // This allows all levels to support all git commands (log, diff, stash, rebase, etc.)
    std::string fullCmd = "cd " + repoPath + " && git " + cmd + " 2>&1";
    
    FILE* pipe = popen(fullCmd.c_str(), "r");
    if (!pipe) {
        return "Error: Failed to execute command";
    }
    
    std::string result;
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    
    int exitCode = pclose(pipe);
    
    // Refresh UI after any git command that might modify state
    // Commands that modify state typically include: commit, merge, rebase, cherry-pick, revert, etc.
    static const std::set<std::string> modifyingCommands = {
        "commit", "merge", "rebase", "cherry-pick", "cherry", "revert", "stash", 
        "rm", "mv", "clean", "pull", "fetch", "push", "apply", "am"
    };
    
    // Check if command starts with any modifying command
    bool shouldRefresh = false;
    for (const auto& modCmd : modifyingCommands) {
        if (cmd.rfind(modCmd, 0) == 0) {
            shouldRefresh = true;
            break;
        }
    }
    
    if (shouldRefresh) {
        SyncGraphWithRepo();
        RefreshWorkingDirectory();
    }
    
    // Return output or error message
    if (result.empty()) {
        return (exitCode == 0) ? "Command executed successfully" : "Command failed";
    }
    
    // Trim trailing newline
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) {
        result.pop_back();
    }
    
    return result;
}

// Execute shell command - supports compound commands (&&, |, ;) and pipes
// This works for all levels including Level 1
std::string Level::ExecuteShellCommand(const std::string& cmd) {
    RecordGitCommand(cmd);
    
    // Handle special case for Level 1 when repo is not yet initialized
    std::string targetPath = repoPath;
    if (targetPath.empty()) {
        // Try to get from GitWrapper
        GitWrapper* git = GetGitWrapper();
        if (git && git->IsRepoOpen()) {
            targetPath = git->GetRepoPath();
        }
    }
    
    // Default to current directory if no repo path available
    if (targetPath.empty()) {
        targetPath = ".";
    }
    
    // Execute command in shell
    std::string fullCmd = "cd " + targetPath + " && " + cmd + " 2>&1";
    
    FILE* pipe = popen(fullCmd.c_str(), "r");
    if (!pipe) {
        return "Error: Failed to execute command";
    }
    
    std::string result;
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    
    int exitCode = pclose(pipe);
    
    // Always refresh UI after compound commands since they likely modify state
    SyncGraphWithRepo();
    RefreshWorkingDirectory();
    
    // Return output or error message
    if (result.empty()) {
        return (exitCode == 0) ? "Command executed successfully" : "Command failed";
    }
    
    // Trim trailing newline
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) {
        result.pop_back();
    }
    
    return result;
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
