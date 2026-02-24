#include "level_05_rebase.h"
#include <raylib.h>
#include <iostream>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

Level05_Rebase::Level05_Rebase()
    : Level(5, "变基危机", "学习 rebase 和解决变基冲突")
    , currentStage(Stage::INTRO)
    , timer(0)
    , stageComplete(false)
    , rebaseStarted(false)
    , conflictOccurred(false)
    , conflictResolved(false)
    , conflictCount(2)
    , resolvedCount(0)
    , rebaseProgress(0) {
}

Level05_Rebase::~Level05_Rebase() = default;

void Level05_Rebase::Initialize() {
    currentStage = Stage::INTRO;
    timer = 0;
    stageComplete = false;
    rebaseStarted = false;
    conflictOccurred = false;
    conflictResolved = false;
    resolvedCount = 0;
    rebaseProgress = 0;
    
    repoPath = "/tmp/gitfighter_level5_" + std::to_string((int)GetTime());
    fs::create_directories(repoPath);
    
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
    
    CreateRebaseScenario();
    std::cout << "Level 5 initialized at: " << repoPath << std::endl;
}

void Level05_Rebase::CreateRebaseScenario() {
    git->Init(repoPath);
    git->OpenRepo(repoPath);
    
    // Main 分支提交 1: 基础
    {
        std::ofstream f(repoPath + "/app.cpp");
        f << "// 主应用\n" << "void main() {\n" << "    init();\n" << "}\n";
    }
    git->Add(".");
    git->Commit("Initial app");
    
    // Main 分支提交 2: 添加配置
    {
        std::ofstream f(repoPath + "/config.cpp");
        f << "// 配置模块 v1.0\n" << "int timeout = 30;\n";
    }
    git->Add(".");
    git->Commit("Add config module");
    
    // Main 分支提交 3: 配置更新（会导致冲突）
    {
        std::ofstream f(repoPath + "/config.cpp");
        f << "// 配置模块 v2.0\n" << "int timeout = 60;\n" << "int retries = 3;\n";
    }
    git->Add(".");
    git->Commit("Update config with retries");
    
    // 创建 feature 分支
    git->CreateBranch("feature");
    git->Checkout("feature");
    
    // Feature 提交 1: 修改同一文件（制造冲突）
    {
        std::ofstream f(repoPath + "/config.cpp");
        f << "// 配置模块 feature版\n" << "int timeout = 45;\n" << "bool debug = true;\n";
    }
    git->Add(".");
    git->Commit("Feature: adjust timeout and add debug");
    
    // Feature 提交 2: 另一个文件
    {
        std::ofstream f(repoPath + "/feature.cpp");
        f << "// 新功能\n" << "void newFeature() {}\n";
    }
    git->Add(".");
    git->Commit("Add new feature module");
    
    // 回到 main 继续开发
    git->Checkout("main");
    {
        std::ofstream f(repoPath + "/main.cpp");
        f << "// 主程序\n" << "int main() { run(); }\n";
    }
    git->Add(".");
    git->Commit("Update main program");
    
    SyncGraphWithRepo();
    splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
}

void Level05_Rebase::SyncGraphWithRepo() {
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
        for (const auto& branch : c.branches) {
            node.branches.push_back(branch);
        }
        commitPanel->AddCommit(node);
    }
    
    std::set<std::string> addedBranches;
    Color branchColors[] = {
        {100, 200, 255, 255},   // main - blue
        {255, 200, 100, 255},   // feature - orange
        {100, 255, 150, 255},
    };
    int colorIndex = 0;
    
    for (const auto& c : commits) {
        for (const auto& branchName : c.branches) {
            if (addedBranches.insert(branchName).second) {
                Color color = branchColors[colorIndex % 3];
                commitPanel->AddBranch(branchName, c.hash, color);
                colorIndex++;
            }
        }
    }
    
    std::string head = git->GetHEAD();
    if (!head.empty()) commitPanel->SetHEAD(head);
    
    std::string currentBranch = git->GetCurrentBranch();
    if (!currentBranch.empty()) commitPanel->SetCurrentBranch(currentBranch);
    
    commitPanel->RecalculateLayout();
}

void Level05_Rebase::StartRebase() {
    rebaseStarted = true;
    // 模拟 rebase 开始
    std::cout << "Starting rebase..." << std::endl;
}

void Level05_Rebase::ResolveConflict() {
    resolvedCount++;
    if (resolvedCount >= conflictCount) {
        conflictResolved = true;
    }
}

void Level05_Rebase::ContinueRebase() {
    if (conflictResolved) {
        currentStage = Stage::REBASE_COMPLETE;
        SyncGraphWithRepo();
    }
}

std::string Level05_Rebase::ProcessLevelCommand(const std::string& cmd) {
    RecordGitCommand(cmd);
    if (cmd == "rebase main" && currentStage == Stage::SHOW_BRANCHES) {
        StartRebase();
        currentStage = Stage::REBASE_CONFLICT;
        return "Started rebase onto main";
    }
    else if (cmd == "resolve" && currentStage == Stage::REBASE_CONFLICT) {
        ResolveConflict();
        if (conflictResolved) {
            currentStage = Stage::CONTINUE_REBASE;
        }
        return "Resolved conflict";
    }
    else if (cmd == "rebase --continue" && currentStage == Stage::CONTINUE_REBASE) {
        ContinueRebase();
        return "Continued rebase";
    }
    return "Command executed: " + cmd;
}

void Level05_Rebase::Update(float deltaTime) {
    timer += deltaTime;
    
    if (splitView) {
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        splitView->SetBounds(320, 100, screenWidth - 320, screenHeight - 170);
        splitView->Update(deltaTime);
    }
    
    if (IsKeyPressed(KEY_SPACE)) {
        switch (currentStage) {
            case Stage::INTRO:
                currentStage = Stage::SHOW_BRANCHES;
                break;
            case Stage::REBASE_COMPLETE:
                currentStage = Stage::COMPLETE;
                stageComplete = true;
                break;
            case Stage::COMPLETE:
                stageComplete = true;
                break;
            default:
                break;
        }
    }
    
    if (IsKeyPressed(KEY_R) && currentStage == Stage::SHOW_BRANCHES) {
        ProcessLevelCommand("rebase main");
        currentStage = Stage::START_REBASE;
    }
    
    if (IsKeyPressed(KEY_C) && currentStage == Stage::START_REBASE) {
        currentStage = Stage::REBASE_CONFLICT;
    }
    
    if (IsKeyPressed(KEY_F) && currentStage == Stage::REBASE_CONFLICT) {
        ProcessLevelCommand("resolve");
    }
    
    if (IsKeyPressed(KEY_ENTER) && currentStage == Stage::CONTINUE_REBASE) {
        ProcessLevelCommand("rebase --continue");
    }
}

void Level05_Rebase::Draw() {
    ClearBackground({30, 35, 45, 255});
    if (splitView) splitView->Draw();
    DrawStatusPanel();
    DrawDialogueIfNeeded();
}

void Level05_Rebase::DrawStatusPanel() {
    DrawRectangle(0, 0, 300, 720, {40, 44, 52, 255});
    DrawRectangleLines(0, 0, 300, 720, {100, 150, 200, 255});
    
    DrawChinese("Level 5: 变基危机", 20, 20, 28, WHITE);
    DrawChinese("学习 rebase", 20, 55, 18, LIGHTGRAY);
    
    DrawChinese("当前任务:", 20, 100, 20, {100, 200, 255, 255});
    
    const char* stageText = "";
    Color stageColor = YELLOW;
    switch (currentStage) {
        case Stage::INTRO: stageText = "按空格开始"; break;
        case Stage::SHOW_BRANCHES: stageText = "按 R 开始 rebase"; break;
        case Stage::START_REBASE: stageText = "rebase 开始"; break;
        case Stage::REBASE_CONFLICT: stageText = "有冲突! 按 F"; stageColor = RED; break;
        case Stage::RESOLVE_CONFLICT: stageText = "解决冲突中..."; break;
        case Stage::CONTINUE_REBASE: stageText = "按回车继续"; break;
        case Stage::REBASE_COMPLETE: stageText = "rebase 完成!"; stageColor = GREEN; break;
        case Stage::COMPLETE: stageText = "完成!"; stageColor = GREEN; break;
    }
    DrawText(stageText, 20, 130, 18, stageColor);
    
    // 冲突进度
    if (currentStage == Stage::REBASE_CONFLICT || currentStage == Stage::CONTINUE_REBASE) {
        DrawRectangle(10, 170, 280, 100, {60, 40, 40, 255});
        DrawChinese("冲突解决进度:", 20, 180, 18, WHITE);
        DrawText(("已解决: " + std::to_string(resolvedCount) + "/" + std::to_string(conflictCount)).c_str(), 
                 20, 210, 16, YELLOW);
        
        // 进度条
        float progress = (float)resolvedCount / conflictCount;
        DrawRectangle(20, 240, 260 * progress, 20, GREEN);
        DrawRectangleLines(20, 240, 260, 20, WHITE);
    }
    
    // 说明
    DrawRectangle(10, 600, 280, 100, {50, 50, 60, 255});
    DrawChinese("提示:", 20, 610, 18, {100, 200, 255, 255});
    if (currentStage == Stage::SHOW_BRANCHES) {
        DrawChinese("rebase 重写历史", 20, 635, 16, LIGHTGRAY);
        DrawChinese("使提交记录更线性", 20, 655, 16, LIGHTGRAY);
    } else if (currentStage == Stage::REBASE_CONFLICT) {
        DrawChinese("F: 标记冲突已解决", 20, 635, 16, LIGHTGRAY);
        DrawChinese("解决后继续 rebase", 20, 655, 16, LIGHTGRAY);
    }
}

void Level05_Rebase::DrawDialogueIfNeeded() {
    int screenHeight = GetScreenHeight();
    int dialogY = screenHeight - 180;
    if (currentStage == Stage::INTRO) {
        DrawRectangle(100, dialogY, 1080, 150, {40, 44, 52, 240});
        DrawRectangleLines(100, dialogY, 1080, 150, {100, 150, 200, 255});
        DrawChinese("CTO: feature 分支已经落后 main 很久了。", 120, dialogY + 20, 24, WHITE);
        DrawChinese("我们需要用 rebase 来保持线性历史，但可能会遇到冲突。", 120, dialogY + 50, 22, LIGHTGRAY);
        DrawChinese("按 [空格] 开始挑战", 120, dialogY + 100, 20, YELLOW);
    }
    else if (currentStage == Stage::SHOW_BRANCHES) {
        DrawRectangle(100, dialogY, 1080, 100, {40, 60, 40, 240});
        DrawRectangleLines(100, dialogY, 1080, 100, {100, 200, 100, 255});
        DrawChinese("main 和 feature 都修改了 config.cpp，rebase 会产生冲突。", 120, dialogY + 30, 22, WHITE);
        DrawChinese("按 [R] 开始 rebase feature 到 main", 120, dialogY + 60, 20, YELLOW);
    }
    else if (currentStage == Stage::REBASE_CONFLICT) {
        DrawRectangle(100, dialogY, 1080, 120, {60, 30, 30, 240});
        DrawRectangleLines(100, dialogY, 1080, 120, RED);
        DrawChinese("CTO: 冲突了！config.cpp 需要手动解决。", 120, dialogY + 20, 26, RED);
        DrawChinese("rebase 比 merge 更容易冲突，因为逐个应用提交。", 120, dialogY + 55, 22, WHITE);
        DrawChinese("按 [F] 解决冲突", 120, dialogY + 90, 20, YELLOW);
    }
    else if (currentStage == Stage::REBASE_COMPLETE) {
        DrawRectangle(100, dialogY, 1080, 100, {40, 60, 40, 240});
        DrawRectangleLines(100, dialogY, 1080, 100, GREEN);
        DrawChinese("CTO: rebase 完成！现在 feature 有了线性的历史。", 120, dialogY + 20, 24, GREEN);
        DrawChinese("注意：rebase 会改写 commit hash，不要在共享分支上使用！", 120, dialogY + 50, 20, YELLOW);
    }
}

void Level05_Rebase::Shutdown() {
    splitView.reset();
    git.reset();
    try { fs::remove_all(repoPath); } catch (...) {}
}

bool Level05_Rebase::IsComplete() const {
    return stageComplete;
}


