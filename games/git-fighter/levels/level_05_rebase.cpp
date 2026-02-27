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

    // Main 分支提交 3: 配置更新（分叉点）
    {
        std::ofstream f(repoPath + "/config.cpp");
        f << "// 配置模块 v2.0\n" << "int timeout = 60;\n" << "int retries = 3;\n";
    }
    git->Add(".");
    git->Commit("Update config with retries");

    // 关键：在分叉点前，main 先做一个提交
    {
        std::ofstream f(repoPath + "/main.cpp");
        f << "// 主程序\n" << "int main() { init(); }\n";
    }
    git->Add(".");
    git->Commit("Add main.cpp");

    // 现在创建 feature 分支（从分叉点）
    git->CreateBranch("feature");
    git->Checkout("feature");

    // Feature 提交 1: 修改 config（制造冲突）
    {
        std::ofstream f(repoPath + "/config.cpp");
        f << "// 配置模块 feature版\n" << "int timeout = 45;\n" << "bool debug = true;\n";
    }
    git->Add(".");
    git->Commit("Feature: adjust timeout and add debug");

    // Feature 提交 2: 添加新功能
    {
        std::ofstream f(repoPath + "/feature.cpp");
        f << "// 新功能\n" << "void newFeature() {}\n";
    }
    git->Add(".");
    git->Commit("Add new feature module");

    // 回到 master 继续开发（形成分叉）
    git->Checkout("master");
    {
        std::ofstream f(repoPath + "/utils.cpp");
        f << "// 工具函数\n" << "void helper() {}\n";
    }
    git->Add(".");
    git->Commit("Add utils on master");

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
    // Return empty for unknown commands to let base class handle them
    return "";
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

    // Dev tools (1/2/3 keys)
    HandleDevToolKeys();
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
    DrawText("5 / 10", 250, 30, 16, {100, 200, 255, 255});

    DrawChinese("当前任务:", 20, 100, 20, {100, 200, 255, 255});

    const char* stageText = "";
    Color stageColor = YELLOW;
    switch (currentStage) {
        case Stage::INTRO: stageText = "按空格开始"; break;
        case Stage::SHOW_BRANCHES: stageText = "按 R 开始 rebase"; break;
        case Stage::START_REBASE: stageText = "已开始，按 C 进入冲突"; break;
        case Stage::REBASE_CONFLICT: stageText = "有冲突! 按 F 解决"; stageColor = RED; break;
        case Stage::RESOLVE_CONFLICT: stageText = "解决冲突中..."; break;
        case Stage::CONTINUE_REBASE: stageText = "冲突已解，按回车继续"; break;
        case Stage::REBASE_COMPLETE: stageText = "rebase 完成!"; stageColor = GREEN; break;
        case Stage::COMPLETE: stageText = "完成!"; stageColor = GREEN; break;
    }
    DrawChinese(stageText, 20, 130, 18, stageColor);

    // 冲突进度
    if (currentStage == Stage::REBASE_CONFLICT || currentStage == Stage::CONTINUE_REBASE) {
        DrawRectangle(10, 170, 280, 100, {60, 40, 40, 255});
        DrawChinese("冲突解决进度:", 20, 180, 18, WHITE);
        DrawChinese(("已解决: " + std::to_string(resolvedCount) + "/" + std::to_string(conflictCount)).c_str(),
                    20, 210, 16, YELLOW);

        // 进度条
        float progress = (float)resolvedCount / conflictCount;
        DrawRectangle(20, 240, 260 * progress, 20, GREEN);
        DrawRectangleLines(20, 240, 260, 20, WHITE);
    }

    // 步骤清单：降低操作路径歧义
    DrawRectangle(10, 350, 280, 190, {35, 40, 50, 255});
    DrawRectangleLines(10, 350, 280, 190, {100, 150, 200, 255});
    DrawChinese("通关步骤:", 20, 360, 18, {100, 200, 255, 255});

    bool stepIntroDone = (currentStage != Stage::INTRO);
    bool stepRebaseDone = (currentStage != Stage::INTRO && currentStage != Stage::SHOW_BRANCHES);
    bool stepConflictDone = conflictResolved || currentStage == Stage::REBASE_COMPLETE || currentStage == Stage::COMPLETE;
    bool stepContinueDone = (currentStage == Stage::REBASE_COMPLETE || currentStage == Stage::COMPLETE);
    bool stepCompleteDone = (currentStage == Stage::COMPLETE);

    bool stepIntroActive = !stepIntroDone;
    bool stepRebaseActive = (currentStage == Stage::SHOW_BRANCHES);
    bool stepConflictActive = (currentStage == Stage::START_REBASE || currentStage == Stage::REBASE_CONFLICT);
    bool stepContinueActive = (currentStage == Stage::CONTINUE_REBASE);
    bool stepCompleteActive = (currentStage == Stage::REBASE_COMPLETE);

    auto drawStep = [&](int y, const char* label, bool done, bool active) {
        const char* status = done ? "[x]" : "[ ]";
        Color color = done ? GREEN : (active ? YELLOW : LIGHTGRAY);
        std::string line = std::string(status) + " " + label;
        DrawChinese(line.c_str(), 20, y, 15, color);
    };

    drawStep(388, "开始挑战 (空格)", stepIntroDone, stepIntroActive);
    drawStep(416, "启动 rebase (R)", stepRebaseDone, stepRebaseActive);
    drawStep(444, "处理冲突 (C -> F)", stepConflictDone, stepConflictActive);
    drawStep(472, "继续 rebase (Enter)", stepContinueDone, stepContinueActive);
    drawStep(500, "通关 (空格)", stepCompleteDone, stepCompleteActive);

    // 说明
    DrawRectangle(10, 600, 280, 100, {50, 50, 60, 255});
    DrawChinese("提示:", 20, 610, 18, {100, 200, 255, 255});
    if (currentStage == Stage::SHOW_BRANCHES) {
        DrawChinese("rebase 重写历史", 20, 635, 16, LIGHTGRAY);
        DrawChinese("使提交记录更线性", 20, 655, 16, LIGHTGRAY);
    } else if (currentStage == Stage::START_REBASE) {
        DrawChinese("下一步: 按 C 触发冲突", 20, 635, 16, LIGHTGRAY);
        DrawChinese("然后按 F 解决冲突", 20, 655, 16, LIGHTGRAY);
    } else if (currentStage == Stage::REBASE_CONFLICT) {
        DrawChinese(("F: 标记冲突已解决 (" + std::to_string(conflictCount) + " 次)").c_str(), 20, 635, 16, LIGHTGRAY);
        DrawChinese("全部解决后按回车继续", 20, 655, 16, LIGHTGRAY);
    } else if (currentStage == Stage::CONTINUE_REBASE) {
        DrawChinese("冲突已全部解决", 20, 635, 16, LIGHTGRAY);
        DrawChinese("按 Enter 执行 rebase --continue", 20, 655, 16, LIGHTGRAY);
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
    else if (currentStage == Stage::START_REBASE) {
        DrawRectangle(100, dialogY, 1080, 120, {40, 50, 70, 240});
        DrawRectangleLines(100, dialogY, 1080, 120, {100, 170, 255, 255});
        DrawChinese("rebase 已启动。下一步会进入冲突处理演示。", 120, dialogY + 20, 22, WHITE);
        DrawChinese("按 [C] 进入冲突 -> 按 [F] 解决 -> 按 [Enter] 继续", 120, dialogY + 55, 20, YELLOW);
    }
    else if (currentStage == Stage::REBASE_CONFLICT) {
        DrawRectangle(100, dialogY, 1080, 120, {60, 30, 30, 240});
        DrawRectangleLines(100, dialogY, 1080, 120, RED);
        DrawChinese("CTO: 冲突了！config.cpp 需要手动解决。", 120, dialogY + 20, 26, RED);
        DrawChinese("rebase 比 merge 更容易冲突，因为逐个应用提交。", 120, dialogY + 55, 22, WHITE);
        DrawChinese(("按 [F] 解决冲突 (共 " + std::to_string(conflictCount) + " 次)").c_str(), 120, dialogY + 90, 20, YELLOW);
    }
    else if (currentStage == Stage::CONTINUE_REBASE) {
        DrawRectangle(100, dialogY, 1080, 100, {40, 60, 40, 240});
        DrawRectangleLines(100, dialogY, 1080, 100, {100, 200, 100, 255});
        DrawChinese("冲突已解决，准备继续 rebase。", 120, dialogY + 25, 24, WHITE);
        DrawChinese("按 [Enter] 执行 rebase --continue", 120, dialogY + 60, 20, YELLOW);
    }
    else if (currentStage == Stage::REBASE_COMPLETE) {
        DrawRectangle(100, dialogY, 1080, 100, {40, 60, 40, 240});
        DrawRectangleLines(100, dialogY, 1080, 100, GREEN);
        DrawChinese("CTO: rebase 完成！现在 feature 有了线性的历史。", 120, dialogY + 20, 24, GREEN);
        DrawChinese("注意：rebase 会改写 commit hash，不要在共享分支上使用！", 120, dialogY + 50, 20, YELLOW);
    }
}

void Level05_Rebase::RefreshWorkingDirectory() {
    if (splitView && !repoPath.empty()) {
        splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
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
