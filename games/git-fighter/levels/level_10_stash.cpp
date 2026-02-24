#include "level_10_stash.h"
#include <raylib.h>
#include <iostream>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

Level10_Stash::Level10_Stash()
    : Level(10, "Stash 战场", "用 stash 管理未提交的更改")
    , currentStage(Stage::INTRO)
    , timer(0)
    , stageComplete(false)
    , hasStash(false)
    , stashCount(0)
    , hasUncommittedChanges(false)
    , hasFeatureFile(false)
    , hasEmergencyFix(false) {
}

Level10_Stash::~Level10_Stash() = default;

void Level10_Stash::Initialize() {
    currentStage = Stage::INTRO;
    timer = 0;
    stageComplete = false;
    hasStash = false;
    stashCount = 0;
    hasUncommittedChanges = false;
    hasFeatureFile = false;
    hasEmergencyFix = false;
    stashMessage = "";
    
    repoPath = "/tmp/gitfighter_level10_" + std::to_string((int)GetTime());
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
    
    SetupRepo();
    std::cout << "Level 10 initialized at: " << repoPath << std::endl;
}

void Level10_Stash::SetupRepo() {
    git->Init(repoPath);
    git->OpenRepo(repoPath);
    
    // 基础提交
    {
        std::ofstream f(repoPath + "/main.cpp");
        f << "// 主程序 v1.0\nint main() { return 0; }\n";
    }
    git->Add(".");
    git->Commit("Initial version");
    
    // 创建 feature 分支
    git->CreateBranch("feature-login");
    git->Checkout("feature-login");
    
    // 开始开发功能（未提交）
    {
        std::ofstream f(repoPath + "/login.cpp");
        f << "// 登录功能开发中...\n" << "class Login {\n" << "    // TODO: implement\n" << "};\n";
    }
    hasUncommittedChanges = true;
    hasFeatureFile = true;
    
    // 创建 hotfix 分支
    git->Checkout("main");
    git->CreateBranch("hotfix-security");
    git->Checkout("feature-login");  // 回到 feature 分支
    
    SyncGraphWithRepo();
    splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
}

void Level10_Stash::SyncGraphWithRepo() {
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
    
    commitPanel->SetHEAD(git->GetHEAD());
    commitPanel->SetCurrentBranch(git->GetCurrentBranch());
    commitPanel->RecalculateLayout();
}

void Level10_Stash::CreateStash() {
    hasStash = true;
    stashCount++;
    stashMessage = "WIP on feature-login: login development";
    hasUncommittedChanges = false;
    // 模拟文件被暂存
    std::remove((repoPath + "/login.cpp").c_str());
    hasFeatureFile = false;
    splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
}

void Level10_Stash::PopStash() {
    if (hasStash) {
        hasStash = false;
        stashCount--;
        hasUncommittedChanges = true;
        hasFeatureFile = true;
        // 恢复文件
        {
            std::ofstream f(repoPath + "/login.cpp");
            f << "// 登录功能开发中...\n" << "class Login {\n" << "    // TODO: implement\n" << "};\n";
        }
        stashMessage = "";
        splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
    }
}

void Level10_Stash::SwitchToEmergency() {
    git->Checkout("hotfix-security");
    SyncGraphWithRepo();
    splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
}

void Level10_Stash::ApplyEmergencyFix() {
    {
        std::ofstream f(repoPath + "/security.patch");
        f << "// 安全补丁\nvoid applySecurityFix() {}\n";
    }
    git->Add(".");
    git->Commit("Critical security fix");
    hasEmergencyFix = true;
    SyncGraphWithRepo();
    splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
}

std::string Level10_Stash::ProcessLevelCommand(const std::string& cmd) {
    RecordGitCommand(cmd);
    if (cmd == "stash" && currentStage == Stage::EMERGENCY_CALL) {
        CreateStash();
        currentStage = Stage::STASH_CHANGES;
        return "Stashed changes";
    }
    else if (cmd == "checkout hotfix-security" && currentStage == Stage::STASH_CHANGES) {
        SwitchToEmergency();
        currentStage = Stage::SWITCH_BRANCH;
        return "Switched to hotfix-security";
    }
    else if (cmd == "fix" && currentStage == Stage::SWITCH_BRANCH) {
        ApplyEmergencyFix();
        currentStage = Stage::FIX_EMERGENCY;
        return "Applied emergency fix";
    }
    else if (cmd == "checkout feature-login" && currentStage == Stage::FIX_EMERGENCY) {
        git->Checkout("feature-login");
        SyncGraphWithRepo();
        splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
        currentStage = Stage::POP_STASH;
        return "Switched to feature-login";
    }
    else if (cmd == "stash pop" && currentStage == Stage::POP_STASH) {
        PopStash();
        currentStage = Stage::COMPLETE;
        stageComplete = true;
        return "Restored stashed changes";
    }
    return "Command executed: " + cmd;
}

void Level10_Stash::Update(float deltaTime) {
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
                currentStage = Stage::DEVELOPING;
                break;
            case Stage::DEVELOPING:
                currentStage = Stage::EMERGENCY_CALL;
                break;
            case Stage::COMPLETE:
                stageComplete = true;
                break;
            default:
                break;
        }
    }
    
    if (IsKeyPressed(KEY_S) && currentStage == Stage::EMERGENCY_CALL) {
        ProcessLevelCommand("stash");
    }
    
    if (IsKeyPressed(KEY_C) && currentStage == Stage::STASH_CHANGES) {
        ProcessLevelCommand("checkout hotfix-security");
    }
    
    if (IsKeyPressed(KEY_F) && currentStage == Stage::SWITCH_BRANCH) {
        ProcessLevelCommand("fix");
    }
    
    if (IsKeyPressed(KEY_R) && currentStage == Stage::FIX_EMERGENCY) {
        ProcessLevelCommand("checkout feature-login");
    }
    
    if (IsKeyPressed(KEY_P) && currentStage == Stage::POP_STASH) {
        ProcessLevelCommand("stash pop");
    }
}

void Level10_Stash::Draw() {
    ClearBackground({30, 35, 45, 255});
    if (splitView) splitView->Draw();
    
    if (hasStash || currentStage == Stage::STASH_CHANGES ||
        currentStage == Stage::SWITCH_BRANCH || currentStage == Stage::FIX_EMERGENCY) {
        DrawStashPanel();
    }
    
    DrawStatusPanel();
    DrawDialogueIfNeeded();
}

void Level10_Stash::DrawStatusPanel() {
    DrawRectangle(0, 0, 300, 720, {40, 44, 52, 255});
    DrawRectangleLines(0, 0, 300, 720, {100, 150, 200, 255});
    
    DrawChinese("Level 10: Stash 战场", 20, 20, 28, WHITE);
    DrawChinese("暂存未完成的更改", 20, 55, 18, LIGHTGRAY);
    
    DrawChinese("当前步骤:", 20, 100, 20, {100, 200, 255, 255});
    
    const char* stageText = "";
    Color stageColor = YELLOW;
    switch (currentStage) {
        case Stage::INTRO: stageText = "按空格开始"; break;
        case Stage::DEVELOPING: stageText = "开发中..."; break;
        case Stage::EMERGENCY_CALL: stageText = "按 S 暂存"; break;
        case Stage::STASH_CHANGES: stageText = "按 C 切换"; break;
        case Stage::SWITCH_BRANCH: stageText = "按 F 修复"; break;
        case Stage::FIX_EMERGENCY: stageText = "按 R 返回"; break;
        case Stage::POP_STASH: stageText = "按 P 恢复"; break;
        case Stage::COMPLETE: stageText = "完成!"; stageColor = GREEN; break;
    }
    DrawText(stageText, 20, 130, 18, stageColor);
    
    // 工作区状态
    DrawRectangle(10, 170, 280, 180, {40, 45, 55, 255});
    DrawChinese("工作区状态:", 20, 180, 18, WHITE);
    
    DrawChinese("当前分支:", 20, 210, 14, LIGHTGRAY);
    std::string branch = git ? git->GetCurrentBranch() : "unknown";
    DrawText(branch.c_str(), 130, 210, 14, YELLOW);
    
    DrawChinese("未提交更改:", 20, 240, 14, LIGHTGRAY);
    DrawChinese(hasUncommittedChanges ? "[有]" : "[无]", 
                130, 240, 14, hasUncommittedChanges ? RED : GREEN);
    
    DrawChinese("Stash 数量:", 20, 270, 14, LIGHTGRAY);
    DrawText(std::to_string(stashCount).c_str(), 130, 270, 14, 
             stashCount > 0 ? YELLOW : LIGHTGRAY);
    
    if (!stashMessage.empty()) {
        DrawText(stashMessage.substr(0, 25).c_str(), 20, 300, 12, {200, 200, 100, 255});
    }
    
    // 流程图
    DrawRectangle(10, 600, 280, 100, {50, 50, 60, 255});
    DrawChinese("工作流程:", 20, 610, 18, {100, 200, 255, 255});
    DrawChinese("1. stash 保存", 20, 635, 14, LIGHTGRAY);
    DrawChinese("2. 切换分支修复", 20, 655, 14, LIGHTGRAY);
    DrawChinese("3. 返回 + pop", 20, 675, 14, LIGHTGRAY);
}

void Level10_Stash::DrawStashPanel() {
    int screenWidth = GetScreenWidth();
    int panelX = screenWidth - 350;
    int panelY = 100;
    int panelW = 330;
    int panelH = 200;
    
    DrawRectangle(panelX, panelY, panelW, panelH, {30, 35, 45, 240});
    DrawRectangleLines(panelX, panelY, panelW, panelH, {200, 200, 100, 255});
    
    DrawChinese("Stash 储藏室", panelX + 10, panelY + 10, 22, {200, 200, 100, 255});
    
    if (hasStash) {
        // 显示 stash 内容
        DrawRectangle(panelX + 10, panelY + 50, panelW - 20, 80, {50, 50, 40, 200});
        DrawChinese("[保存的更改]", panelX + 20, panelY + 60, 16, YELLOW);
        DrawText(stashMessage.c_str(), panelX + 20, panelY + 90, 14, WHITE);
        
        // stash pop 提示
        if (currentStage == Stage::POP_STASH) {
            DrawRectangle(panelX + 10, panelY + 140, panelW - 20, 40, {40, 80, 40, 200});
            DrawChinese("按 P 恢复工作", panelX + 80, panelY + 155, 16, GREEN);
        }
    } else {
        DrawChinese("(空)", panelX + 140, panelY + 100, 16, {100, 100, 100, 255});
    }
}

void Level10_Stash::DrawDialogueIfNeeded() {
    int screenHeight = GetScreenHeight();
    int dialogY = screenHeight - 180;
    if (currentStage == Stage::INTRO) {
        DrawRectangle(100, dialogY, 1080, 150, {40, 44, 52, 240});
        DrawRectangleLines(100, dialogY, 1080, 150, {100, 150, 200, 255});
        DrawChinese("CTO: 今天要学习一个超实用的技能 - git stash！", 120, dialogY + 20, 24, WHITE);
        DrawChinese("当你开发到一半，突然需要切换分支修复紧急 bug 时，stash 就是你的救星。", 120, dialogY + 50, 22, LIGHTGRAY);
        DrawChinese("按 [空格] 开始场景演示", 120, dialogY + 100, 20, YELLOW);
    }
    else if (currentStage == Stage::DEVELOPING) {
        DrawRectangle(100, dialogY, 1080, 100, {40, 60, 40, 240});
        DrawRectangleLines(100, dialogY, 1080, 100, {100, 200, 100, 255});
        DrawChinese("你正在 feature-login 分支开发登录功能，代码写了一半还没提交。", 120, dialogY + 30, 22, WHITE);
        DrawChinese("突然！生产环境出现安全漏洞，需要立即修复！", 120, dialogY + 60, 20, RED);
    }
    else if (currentStage == Stage::EMERGENCY_CALL) {
        DrawRectangle(100, dialogY, 1080, 120, {60, 40, 40, 240});
        DrawRectangleLines(100, dialogY, 1080, 120, RED);
        DrawChinese("CTO: 你不能直接切换分支！未提交的更改会被破坏！", 120, dialogY + 20, 26, RED);
        DrawChinese("需要先用 git stash 暂存当前工作，然后再切换分支。", 120, dialogY + 60, 22, WHITE);
        DrawChinese("按 [S] 执行 stash", 120, dialogY + 90, 20, YELLOW);
    }
    else if (currentStage == Stage::POP_STASH) {
        DrawRectangle(100, dialogY, 1080, 120, {40, 60, 40, 240});
        DrawRectangleLines(100, dialogY, 1080, 120, GREEN);
        DrawChinese("CTO: 安全修复完成！现在回到 feature-login 分支恢复之前的工作。", 120, dialogY + 20, 24, GREEN);
        DrawChinese("按 [P] 执行 stash pop 恢复暂存的更改。", 120, dialogY + 60, 22, WHITE);
    }
    else if (currentStage == Stage::COMPLETE) {
        DrawRectangle(100, dialogY, 1080, 100, {40, 60, 40, 240});
        DrawRectangleLines(100, dialogY, 1080, 100, GREEN);
        DrawChinese("CTO: 完美！你学会了 git stash save/pop/list 的完整工作流。", 120, dialogY + 20, 24, GREEN);
        DrawChinese("记住：临时切换分支时，stash 是最安全的做法！", 120, dialogY + 50, 20, WHITE);
    }
}

void Level10_Stash::Shutdown() {
    splitView.reset();
    git.reset();
    try { fs::remove_all(repoPath); } catch (...) {}
}

bool Level10_Stash::IsComplete() const {
    return stageComplete;
}


