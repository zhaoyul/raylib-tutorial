#include "level_04_remote.h"
#include <raylib.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <set>

namespace fs = std::filesystem;

Level04_Remote::Level04_Remote()
    : Level(4, "远程协作", "学习 remote, push, pull")
    , currentStage(Stage::INTRO)
    , timer(0)
    , stageComplete(false)
    , remoteAdded(false)
    , pushed(false)
    , fetched(false)
    , pulled(false) {
}

Level04_Remote::~Level04_Remote() = default;

void Level04_Remote::Initialize() {
    currentStage = Stage::INTRO;
    timer = 0;
    stageComplete = false;
    remoteAdded = false;
    pushed = false;
    fetched = false;
    pulled = false;
    remoteHasNewCommits = false;
    remoteCommits.clear();

    repoPath = "/tmp/gitfighter_level4_" + std::to_string((int)GetTime());
    remotePath = "/tmp/gitfighter_level4_remote_" + std::to_string((int)GetTime());
    fs::create_directories(repoPath);
    fs::create_directories(remotePath);

    git = std::make_unique<GitWrapper>();

    splitView = std::make_unique<GitVis::SplitGitView>();
    splitView->Initialize(320, 100, 940, 520);
    splitView->SetSplitRatio(0.5f);
    splitView->SetGitWrapper(git.get());

    auto* commitPanel = splitView->GetCommitPanel();
    commitPanel->onNodeSelected = [this](const GitVis::CommitNode& node) {
        splitView->OnCommitSelected(node.hash);
    };
    splitView->SetRepoPath(repoPath);

    // 先创建 remote 仓库（模拟已存在的远程仓库）
    CreateRemoteRepo();
    // 再创建本地仓库
    CreateLocalRepo();

    std::cout << "Level 4 initialized at: " << repoPath << std::endl;
    std::cout << "Remote repo at: " << remotePath << std::endl;
}

void Level04_Remote::CreateRemoteRepo() {
    // 创建 bare 仓库作为 remote
    std::string cmd = "cd " + remotePath + " && git init --bare remote.git 2>&1";
    std::system(cmd.c_str());
    
    // 创建一个临时工作目录来初始化 remote 仓库的内容
    std::string tempPath = remotePath + "/temp_init";
    fs::create_directories(tempPath);
    
    // 在临时目录创建一些提交（模拟其他人已经 push 的内容）
    cmd = "cd " + tempPath + " && git init && git config user.email 'remote@git.com' && git config user.name 'Remote' 2>&1";
    std::system(cmd.c_str());
    
    {
        std::ofstream file(tempPath + "/README.md");
        file << "# Remote Project\n\nShared repository\n";
    }
    cmd = "cd " + tempPath + " && git add . && git commit -m 'Initial remote commit' 2>&1";
    std::system(cmd.c_str());
    
    {
        std::ofstream file(tempPath + "/shared.cpp");
        file << "// Shared code\nvoid shared() {}\n";
    }
    cmd = "cd " + tempPath + " && git add . && git commit -m 'Add shared module' 2>&1";
    std::system(cmd.c_str());
    
    // Push 到 bare 仓库
    cmd = "cd " + tempPath + " && git push " + remotePath + "/remote.git master 2>&1";
    std::system(cmd.c_str());
    
    // 记录 remote 的提交
    remoteCommits.push_back({"abc1234", "Initial remote commit"});
    remoteCommits.push_back({"def5678", "Add shared module"});
    
    // 清理临时目录
    fs::remove_all(tempPath);
    
    std::cout << "[Level4] Remote repo created at: " << remotePath << "/remote.git" << std::endl;
}

void Level04_Remote::CreateLocalRepo() {
    git->Init(repoPath);
    git->OpenRepo(repoPath);

    {
        std::ofstream file(repoPath + "/README.md");
        file << "# Local Project\n\nLocal development\n";
    }
    git->Add(".");
    git->Commit("Initial local commit");

    {
        std::ofstream file(repoPath + "/feature.cpp");
        file << "// New feature\nvoid feature() {}\n";
    }
    git->Add(".");
    git->Commit("Add feature");

    SyncGraphWithRepo();
    splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
}

void Level04_Remote::SyncGraphWithRepo() {
    if (!git || !git->IsRepoOpen()) return;

    auto* commitPanel = splitView->GetCommitPanel();
    commitPanel->Clear();

    auto commits = git->GetCommitGraph(50);
    for (const auto& c : commits) {
        GitVis::CommitNode node;
        node.hash = c.hash;
        node.shortHash = c.shortHash();
        node.message = c.message;
        node.author = c.author;
        node.parents = c.parents;
        node.radius = 20;
        node.alpha = 1;
        node.scale = 1;
        node.position = {200, 100};
        node.targetPos = {200, 100};
        node.position = {200, 100};
        // Copy branch info from git
        for (const auto& branch : c.branches) {
            node.branches.push_back(branch);
        }
        commitPanel->AddCommit(node);
    }

    // Add all branches to visualization
    std::set<std::string> addedBranches;
    Color branchColors[] = {
        {100, 200, 255, 255},   // Blue - main
        {255, 200, 100, 255},   // Orange - feature
        {100, 255, 150, 255},   // Green
        {255, 100, 200, 255},   // Pink
        {200, 255, 100, 255},   // Yellow-green
        {255, 150, 100, 255},   // Coral - origin/main
        {150, 100, 255, 255},   // Purple
    };
    int colorIndex = 0;

    for (const auto& c : commits) {
        for (const auto& branchName : c.branches) {
            if (addedBranches.insert(branchName).second) {
                Color color = branchColors[colorIndex % 7];
                commitPanel->AddBranch(branchName, c.hash, color);
                colorIndex++;
            }
        }
    }

    std::string head = git->GetHEAD();
    if (!head.empty()) {
        commitPanel->SetHEAD(head);
    }

    // Set current branch name
    std::string currentBranch = git->GetCurrentBranch();
    if (!currentBranch.empty()) {
        commitPanel->SetCurrentBranch(currentBranch);
    }

    commitPanel->RecalculateLayout();
}

void Level04_Remote::SyncRemoteBranches() {
    // 在提交图中添加 origin/main 分支标记
    if (!remoteAdded) return;
    
    auto* commitPanel = splitView->GetCommitPanel();
    
    // 获取本地仓库的远程分支信息
    std::string cmd = "cd " + repoPath + " && git branch -r 2>&1";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (pipe) {
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            std::string line(buffer);
            // Trim whitespace
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);
            
            if (line.find("origin/") != std::string::npos) {
                // 获取 remote 分支指向的 commit
                std::string revParseCmd = "cd " + repoPath + " && git rev-parse " + line + " 2>&1";
                FILE* revPipe = popen(revParseCmd.c_str(), "r");
                if (revPipe) {
                    char hashBuffer[41];
                    if (fgets(hashBuffer, sizeof(hashBuffer), revPipe) != nullptr) {
                        std::string hash(hashBuffer);
                        hash = hash.substr(0, 7);  // short hash
                        // Add origin/main branch to visualization
                        commitPanel->AddBranch(line, hash, {255, 150, 100, 255});
                    }
                    pclose(revPipe);
                }
            }
        }
        pclose(pipe);
    }
}

std::string Level04_Remote::ProcessLevelCommand(const std::string& cmd) {
    std::cout << "Processing command: " << cmd << std::endl;
    RecordGitCommand(cmd);

    if (cmd.rfind("remote add", 0) == 0 && currentStage == Stage::ADD_REMOTE) {
        // 使用系统 git 添加 remote
        std::string fullCmd = "cd " + repoPath + " && git remote add origin " + remotePath + "/remote.git 2>&1";
        FILE* pipe = popen(fullCmd.c_str(), "r");
        std::string output;
        if (pipe) {
            char buffer[4096];
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                output += buffer;
            }
            pclose(pipe);
        }
        
        // 执行 fetch 来获取远程分支信息
        fullCmd = "cd " + repoPath + " && git fetch origin 2>&1";
        pipe = popen(fullCmd.c_str(), "r");
        if (pipe) {
            char buffer[4096];
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                output += buffer;
            }
            pclose(pipe);
        }
        
        remoteAdded = true;
        currentStage = Stage::PUSH_MAIN;
        SyncGraphWithRepo();
        SyncRemoteBranches();
        
        // Trim output
        while (!output.empty() && (output.back() == '\n' || output.back() == '\r')) {
            output.pop_back();
        }
        return output.empty() ? "Added remote origin and fetched" : output;
    }
    else if (cmd.rfind("push", 0) == 0 && currentStage == Stage::PUSH_MAIN) {
        // 使用系统 git push
        std::string fullCmd = "cd " + repoPath + " && git push -u origin master 2>&1";
        FILE* pipe = popen(fullCmd.c_str(), "r");
        std::string output;
        if (pipe) {
            char buffer[4096];
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                output += buffer;
            }
            pclose(pipe);
        }
        
        pushed = true;
        currentStage = Stage::FETCH_REMOTE;
        
        // 模拟其他人 push 了新提交到 remote
        remoteHasNewCommits = true;
        
        SyncGraphWithRepo();
        SyncRemoteBranches();
        
        while (!output.empty() && (output.back() == '\n' || output.back() == '\r')) {
            output.pop_back();
        }
        return output.empty() ? "Pushed to origin master" : output;
    }
    else if (cmd == "fetch" && currentStage == Stage::FETCH_REMOTE) {
        // 在 fetch 前，先模拟其他人向 remote 添加了新提交
        if (remoteHasNewCommits) {
            std::string tempPath = remotePath + "/temp_simulate";
            fs::create_directories(tempPath);
            
            // Clone remote, add commit, push back
            std::string cmd = "cd " + tempPath + " && git clone " + remotePath + "/remote.git . 2>&1";
            std::system(cmd.c_str());
            cmd = "cd " + tempPath + " && git config user.email 'other@git.com' && git config user.name 'Other' 2>&1";
            std::system(cmd.c_str());
            
            {
                std::ofstream file(tempPath + "/other_update.cpp");
                file << "// Other developer's update\nvoid other_func() {}\n";
            }
            cmd = "cd " + tempPath + " && git add . && git commit -m 'Other: Add new function' 2>&1";
            std::system(cmd.c_str());
            cmd = "cd " + tempPath + " && git push 2>&1";
            std::system(cmd.c_str());
            
            fs::remove_all(tempPath);
            remoteHasNewCommits = false;  // 已添加
        }
        
        // 执行 fetch
        std::string fullCmd = "cd " + repoPath + " && git fetch origin 2>&1";
        FILE* pipe = popen(fullCmd.c_str(), "r");
        std::string output;
        if (pipe) {
            char buffer[4096];
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                output += buffer;
            }
            pclose(pipe);
        }
        
        fetched = true;
        currentStage = Stage::PULL_CHANGES;
        SyncGraphWithRepo();
        SyncRemoteBranches();
        
        while (!output.empty() && (output.back() == '\n' || output.back() == '\r')) {
            output.pop_back();
        }
        return output.empty() ? "Fetched from origin" : output;
    }
    else if (cmd == "pull" && currentStage == Stage::PULL_CHANGES) {
        // 使用系统 git pull
        std::string fullCmd = "cd " + repoPath + " && git pull origin master 2>&1";
        FILE* pipe = popen(fullCmd.c_str(), "r");
        std::string output;
        if (pipe) {
            char buffer[4096];
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                output += buffer;
            }
            pclose(pipe);
        }
        
        pulled = true;
        currentStage = Stage::COMPLETE;
        stageComplete = true;
        SyncGraphWithRepo();
        SyncRemoteBranches();
        RefreshWorkingDirectory();
        
        while (!output.empty() && (output.back() == '\n' || output.back() == '\r')) {
            output.pop_back();
        }
        return output.empty() ? "Pulled from origin" : output;
    }
    // Return empty for unknown commands to let base class handle them
    return "";
}

void Level04_Remote::Update(float deltaTime) {
    timer += deltaTime;

    // 自适应布局
    if (splitView) {
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        splitView->SetBounds(320, 100, screenWidth - 320, screenHeight - 170);
        splitView->Update(deltaTime);
    }

    if (IsKeyPressed(KEY_SPACE) && currentStage == Stage::INTRO) {
        currentStage = Stage::ADD_REMOTE;
    }

    if (IsKeyPressed(KEY_ENTER)) {
        switch (currentStage) {
            case Stage::ADD_REMOTE:
                ProcessLevelCommand("remote add origin https://github.com/company/project.git");
                break;
            case Stage::PUSH_MAIN:
                ProcessLevelCommand("push origin main");
                break;
            case Stage::FETCH_REMOTE:
                ProcessLevelCommand("fetch");
                break;
            case Stage::PULL_CHANGES:
                ProcessLevelCommand("pull");
                break;
            case Stage::COMPLETE:
                stageComplete = true;
                break;
            default:
                break;
        }
    }

    // Dev tools (1/2/3 keys)
    HandleDevToolKeys();
}

void Level04_Remote::Draw() {
    ClearBackground({30, 35, 45, 255});

    if (splitView) {
        splitView->Draw();
    }

    DrawStatusPanel();
    DrawDialogueIfNeeded();
}

void Level04_Remote::DrawStatusPanel() {
    DrawRectangle(0, 0, 300, 720, {40, 44, 52, 255});
    DrawRectangleLines(0, 0, 300, 720, {100, 150, 200, 255});

    DrawChinese("Level 4: 远程协作", 20, 20, 28, WHITE);
    DrawChinese("学习 remote/push/pull", 20, 55, 18, LIGHTGRAY);
    DrawText("4 / 10", 250, 30, 16, {100, 200, 255, 255});

    DrawChinese("当前任务:", 20, 100, 20, {100, 200, 255, 255});

    const char* stageText = "";
    switch (currentStage) {
        case Stage::INTRO: stageText = "按空格开始"; break;
        case Stage::ADD_REMOTE: stageText = "1. remote add origin"; break;
        case Stage::PUSH_MAIN: stageText = "2. push origin main"; break;
        case Stage::FETCH_REMOTE: stageText = "3. fetch"; break;
        case Stage::PULL_CHANGES: stageText = "4. pull"; break;
        case Stage::COMPLETE: stageText = "完成!"; break;
    }
    DrawChinese(stageText, 20, 130, 18, YELLOW);

    // 进度指示
    DrawChinese("进度:", 20, 200, 20, GREEN);
    DrawText(remoteAdded ? "[X] remote add" : "[ ] remote add", 20, 230, 16, remoteAdded ? GREEN : GRAY);
    DrawText(pushed ? "[X] push" : "[ ] push", 20, 255, 16, pushed ? GREEN : GRAY);
    DrawText(fetched ? "[X] fetch" : "[ ] fetch", 20, 280, 16, fetched ? GREEN : GRAY);
    DrawText(pulled ? "[X] pull" : "[ ] pull", 20, 305, 16, pulled ? GREEN : GRAY);

    // Local / Remote 标注说明
    DrawRectangle(10, 340, 280, 90, {40, 44, 52, 255});
    DrawRectangleLines(10, 340, 280, 90, {100, 150, 200, 255});
    DrawChinese("图例说明:", 20, 350, 18, WHITE);
    
    // Local branch indicator (blue)
    DrawCircle(35, 380, 8, {100, 200, 255, 255});
    DrawText("master", 50, 375, 14, WHITE);
    DrawChinese("- 本地分支", 110, 375, 14, LIGHTGRAY);
    
    // Remote branch indicator (orange)
    DrawCircle(35, 405, 8, {255, 150, 100, 255});
    DrawText("origin/master", 50, 400, 14, WHITE);
    DrawChinese("- 远程分支", 160, 400, 14, LIGHTGRAY);

    DrawRectangle(10, 600, 280, 100, {50, 50, 60, 255});
    DrawChinese("提示:", 20, 610, 18, {100, 200, 255, 255});
    DrawChinese("remote 管理远程仓库", 20, 635, 16, LIGHTGRAY);
    DrawChinese("push 推送本地提交", 20, 655, 16, LIGHTGRAY);
    DrawChinese("pull 拉取远程更新", 20, 675, 16, LIGHTGRAY);
}

void Level04_Remote::DrawDialogueIfNeeded() {
    if (currentStage == Stage::INTRO) {
        int screenHeight = GetScreenHeight();
        int dialogY = screenHeight - 180;
        DrawRectangle(100, dialogY, 1080, 150, {40, 44, 52, 240});
        DrawRectangleLines(100, dialogY, 1080, 150, {100, 150, 200, 255});
        DrawChinese("CTO: 多人开发需要远程仓库。", 120, dialogY + 20, 24, WHITE);
        DrawChinese("push 上传代码, pull 下载更新。", 120, dialogY + 50, 22, LIGHTGRAY);
        DrawChinese("按 [空格] 开始", 120, dialogY + 100, 20, YELLOW);
    }
}

void Level04_Remote::RefreshWorkingDirectory() {
    if (splitView && !repoPath.empty()) {
        splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
    }
}

void Level04_Remote::Shutdown() {
    splitView.reset();
    git.reset();

    try {
        fs::remove_all(repoPath);
        fs::remove_all(remotePath);
    } catch (...) {}
}

bool Level04_Remote::IsComplete() const {
    return stageComplete;
}
