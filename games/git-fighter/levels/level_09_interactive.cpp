#include "level_09_interactive.h"
#include <raylib.h>
#include <iostream>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

Level09_Interactive::Level09_Interactive()
    : Level(9, "历史重写", "用交互式 rebase 整理提交")
    , currentStage(Stage::INTRO)
    , timer(0)
    , stageComplete(false)
    , selectedPlanIndex(0)
    , originalCount(0)
    , finalCount(0)
    , squashedCount(0)
    , droppedCount(0) {
}

Level09_Interactive::~Level09_Interactive() = default;

void Level09_Interactive::Initialize() {
    currentStage = Stage::INTRO;
    timer = 0;
    stageComplete = false;
    selectedPlanIndex = 0;
    originalCount = 0;
    finalCount = 0;
    squashedCount = 0;
    droppedCount = 0;
    commitPlans.clear();

    repoPath = "/tmp/gitfighter_level9_" + std::to_string((int)GetTime());
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

    CreateMessyHistory();
    std::cout << "Level 9 initialized at: " << repoPath << std::endl;
}

void Level09_Interactive::CreateMessyHistory() {
    git->Init(repoPath);
    git->OpenRepo(repoPath);

    // 创建混乱的提交历史
    std::vector<std::pair<std::string, std::string>> commits = {
        {"Initial commit", "main.cpp"},
        {"Add utils", "utils.cpp"},
        {"WIP", "temp.txt"},                    // 应该 squash 或 drop
        {"fix typo", "main.cpp"},               // 应该 squash
        {"Add feature A", "feature_a.cpp"},
        {"debug", "debug.log"},                 // 应该 drop
        {"fix bug", "bugfix.cpp"},              // 应该 squash 到 feature A
        {"oops", "fix.cpp"},                    // 应该 squash
        {"Final version", "final.cpp"},
        {"really final", "really_final.cpp"},   // 应该 squash
    };

    for (const auto& [msg, file] : commits) {
        std::ofstream f(repoPath + "/" + file);
        f << "// " << file << "\ncontent\n";
        f.close();
        git->Add(".");
        git->Commit(msg);
    }

    originalCount = (int)commits.size();

    SyncGraphWithRepo();
    splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
}

void Level09_Interactive::SyncGraphWithRepo() {
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

void Level09_Interactive::GenerateCommitPlan() {
    commitPlans.clear();
    auto commits = git->GetCommitGraph(20);

    bool first = true;
    for (const auto& c : commits) {
        CommitPlan plan;
        plan.hash = c.hash;
        plan.message = c.message;
        plan.action = Action::PICK;
        plan.isCurrent = first;
        commitPlans.push_back(plan);
        first = false;
    }

    // 智能建议
    for (size_t i = 0; i < commitPlans.size(); i++) {
        std::string msg = commitPlans[i].message;
        // 建议 squash 一些提交
        if (msg.find("WIP") != std::string::npos ||
            msg.find("fix typo") != std::string::npos ||
            msg.find("oops") != std::string::npos ||
            msg.find("really") != std::string::npos) {
            if (i > 0) {
                commitPlans[i].action = Action::SQUASH;
            }
        }
        // 建议 drop debug
        if (msg.find("debug") != std::string::npos) {
            commitPlans[i].action = Action::DROP;
        }
    }
}

void Level09_Interactive::CycleAction(int index) {
    if (index < 0 || index >= (int)commitPlans.size()) return;
    if (commitPlans[index].isCurrent) {
        // 第一个不能 squash
        switch (commitPlans[index].action) {
            case Action::PICK: commitPlans[index].action = Action::REWORD; break;
            case Action::REWORD: commitPlans[index].action = Action::DROP; break;
            case Action::DROP: commitPlans[index].action = Action::PICK; break;
            default: commitPlans[index].action = Action::PICK; break;
        }
    } else {
        switch (commitPlans[index].action) {
            case Action::PICK: commitPlans[index].action = Action::REWORD; break;
            case Action::REWORD: commitPlans[index].action = Action::SQUASH; break;
            case Action::SQUASH: commitPlans[index].action = Action::DROP; break;
            case Action::DROP: commitPlans[index].action = Action::PICK; break;
        }
    }
}

void Level09_Interactive::ExecuteRebase() {
    // 统计
    finalCount = 0;
    squashedCount = 0;
    droppedCount = 0;

    for (const auto& plan : commitPlans) {
        switch (plan.action) {
            case Action::PICK:
            case Action::REWORD:
                finalCount++;
                break;
            case Action::SQUASH:
                squashedCount++;
                break;
            case Action::DROP:
                droppedCount++;
                break;
        }
    }

    // 模拟执行结果
    currentStage = Stage::VERIFY_RESULT;
}

const char* Level09_Interactive::ActionToString(Action a) {
    switch (a) {
        case Action::PICK: return "pick";
        case Action::REWORD: return "reword";
        case Action::SQUASH: return "squash";
        case Action::DROP: return "drop";
    }
    return "?";
}

Color Level09_Interactive::ActionToColor(Action a) {
    switch (a) {
        case Action::PICK: return {100, 200, 255, 255};
        case Action::REWORD: return {255, 200, 100, 255};
        case Action::SQUASH: return {100, 255, 150, 255};
        case Action::DROP: return {255, 100, 100, 255};
    }
    return WHITE;
}

std::string Level09_Interactive::ProcessLevelCommand(const std::string& cmd) {
    RecordGitCommand(cmd);
    // Return empty for unknown commands to let base class handle them
    return "";
}

void Level09_Interactive::Update(float deltaTime) {
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
                currentStage = Stage::SHOW_HISTORY;
                break;
            case Stage::VERIFY_RESULT:
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

    if (IsKeyPressed(KEY_I) && currentStage == Stage::SHOW_HISTORY) {
        GenerateCommitPlan();
        currentStage = Stage::PLAN_REBASE;
    }

    if (currentStage == Stage::PLAN_REBASE) {
        // 上下选择
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_K)) {
            selectedPlanIndex = std::max(0, selectedPlanIndex - 1);
        }
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_J)) {
            selectedPlanIndex = std::min((int)commitPlans.size() - 1, selectedPlanIndex + 1);
        }

        // 空格切换 action
        if (IsKeyPressed(KEY_SPACE)) {
            CycleAction(selectedPlanIndex);
        }

        // 回车执行
        if (IsKeyPressed(KEY_ENTER)) {
            ExecuteRebase();
        }
    }

    // Dev tools (1/2/3 keys)
    HandleDevToolKeys();
}

void Level09_Interactive::Draw() {
    ClearBackground({30, 35, 45, 255});
    if (splitView) splitView->Draw();

    if (currentStage == Stage::PLAN_REBASE ||
        currentStage == Stage::EXECUTE_REBASE ||
        currentStage == Stage::VERIFY_RESULT) {
        DrawInteractivePanel();
    }

    DrawStatusPanel();
    DrawDialogueIfNeeded();
}

void Level09_Interactive::DrawStatusPanel() {
    DrawRectangle(0, 0, 300, 720, {40, 44, 52, 255});
    DrawRectangleLines(0, 0, 300, 720, {100, 150, 200, 255});

    DrawChinese("Level 9: 历史重写", 20, 20, 28, WHITE);
    DrawChinese("交互式 rebase", 20, 55, 18, LIGHTGRAY);
    DrawText("9 / 10", 250, 30, 16, {100, 200, 255, 255});

    DrawChinese("当前任务:", 20, 100, 20, {100, 200, 255, 255});

    const char* stageText = "";
    Color stageColor = YELLOW;
    switch (currentStage) {
        case Stage::INTRO: stageText = "按空格开始"; break;
        case Stage::SHOW_HISTORY: stageText = "按 I 规划"; break;
        case Stage::PLAN_REBASE: stageText = "编辑计划"; break;
        case Stage::EXECUTE_REBASE: stageText = "执行中..."; break;
        case Stage::VERIFY_RESULT: stageText = "验证完成"; stageColor = GREEN; break;
        case Stage::COMPLETE: stageText = "完成!"; stageColor = GREEN; break;
    }
    DrawText(stageText, 20, 130, 18, stageColor);

    // 统计
    if (currentStage == Stage::PLAN_REBASE ||
        currentStage == Stage::EXECUTE_REBASE ||
        currentStage == Stage::VERIFY_RESULT) {
        DrawRectangle(10, 170, 280, 150, {40, 45, 55, 255});
        DrawChinese("整理统计:", 20, 180, 18, WHITE);
        DrawText(("原始: " + std::to_string(originalCount) + " commits").c_str(), 20, 210, 16, LIGHTGRAY);
        DrawText(("预计: " + std::to_string(finalCount) + " commits").c_str(), 20, 235, 16, GREEN);
        DrawText(("Squash: " + std::to_string(squashedCount)).c_str(), 20, 260, 14, {100, 255, 150, 255});
        DrawText(("Drop: " + std::to_string(droppedCount)).c_str(), 20, 280, 14, {255, 100, 100, 255});
    }

    // 提示
    DrawRectangle(10, 600, 280, 100, {50, 50, 60, 255});
    DrawChinese("提示:", 20, 610, 18, {100, 200, 255, 255});
    if (currentStage == Stage::PLAN_REBASE) {
        DrawChinese("↑↓: 选择", 20, 635, 16, LIGHTGRAY);
        DrawChinese("Space: 切换动作", 20, 655, 16, LIGHTGRAY);
        DrawChinese("Enter: 执行", 20, 675, 16, LIGHTGRAY);
    }
}

void Level09_Interactive::DrawInteractivePanel() {
    int screenWidth = GetScreenWidth();
    int panelX = screenWidth - 500;
    int panelY = 100;
    int panelW = 480;
    int panelH = 520;

    DrawRectangle(panelX, panelY, panelW, panelH, {20, 25, 35, 240});
    DrawRectangleLines(panelX, panelY, panelW, panelH, {100, 200, 255, 255});

    DrawChinese("交互式 Rebase 计划", panelX + 10, panelY + 10, 22, {100, 200, 255, 255});

    // 操作说明
    int legendY = panelY + 45;
    DrawText("pick", panelX + 20, legendY, 12, ActionToColor(Action::PICK));
    DrawText("reword", panelX + 80, legendY, 12, ActionToColor(Action::REWORD));
    DrawText("squash", panelX + 150, legendY, 12, ActionToColor(Action::SQUASH));
    DrawText("drop", panelX + 220, legendY, 12, ActionToColor(Action::DROP));

    // 提交列表
    int listY = panelY + 80;
    int itemHeight = 40;
    int visibleItems = (panelH - 150) / itemHeight;

    int startIdx = std::max(0, selectedPlanIndex - visibleItems / 2);
    int endIdx = std::min((int)commitPlans.size(), startIdx + visibleItems);

    for (int i = startIdx; i < endIdx; i++) {
        const auto& plan = commitPlans[i];
        int y = listY + (i - startIdx) * itemHeight;

        // 选中高亮
        if (i == selectedPlanIndex && currentStage == Stage::PLAN_REBASE) {
            DrawRectangle(panelX + 5, y - 2, panelW - 10, itemHeight - 4, {60, 70, 90, 200});
        }

        // Action 标签
        const char* actionStr = ActionToString(plan.action);
        Color actionColor = ActionToColor(plan.action);

        // 如果是第一个且试图 squash，显示警告
        if (plan.isCurrent && plan.action == Action::SQUASH) {
            actionColor = {150, 150, 150, 255};
        }

        DrawRectangle(panelX + 15, y, 70, 25, actionColor);
        DrawText(actionStr, panelX + 20, y + 5, 14, BLACK);

        // Commit 信息
        std::string shortMsg = plan.message;
        if (shortMsg.length() > 30) shortMsg = shortMsg.substr(0, 30) + "...";
        DrawText(shortMsg.c_str(), panelX + 95, y + 5, 14, WHITE);

        // 简短 hash
        DrawText(plan.hash.substr(0, 7).c_str(), panelX + panelW - 80, y + 5, 12, {150, 150, 150, 255});
    }

    // 底部状态
    DrawRectangle(panelX, panelY + panelH - 50, panelW, 50, {30, 35, 45, 255});
    if (currentStage == Stage::VERIFY_RESULT) {
        DrawChinese("[完成] 历史已整理!", panelX + 20, panelY + panelH - 35, 18, GREEN);
    } else {
        DrawChinese("按 Space 切换操作, Enter 执行", panelX + 20, panelY + panelH - 35, 16, LIGHTGRAY);
    }
}

void Level09_Interactive::DrawDialogueIfNeeded() {
    int screenHeight = GetScreenHeight();
    int dialogY = screenHeight - 180;
    if (currentStage == Stage::INTRO) {
        DrawRectangle(100, dialogY, 1080, 150, {40, 44, 52, 240});
        DrawRectangleLines(100, dialogY, 1080, 150, {100, 150, 200, 255});
        DrawChinese("CTO: 功能开发完成了，但提交历史太乱了！", 120, dialogY + 20, 24, WHITE);
        DrawChinese("有 'WIP', 'fix typo', 'oops' 这样的临时提交，不能这样提交到 main。", 120, dialogY + 50, 22, LIGHTGRAY);
        DrawChinese("按 [空格] 开始整理历史", 120, dialogY + 100, 20, YELLOW);
    }
    else if (currentStage == Stage::SHOW_HISTORY) {
        DrawRectangle(100, dialogY, 1080, 100, {40, 40, 60, 240});
        DrawRectangleLines(100, dialogY, 1080, 100, {100, 150, 200, 255});
        DrawChinese("看看这些提交：WIP、debug、oops... 这些不应该出现在正式历史中。", 120, dialogY + 30, 22, WHITE);
        DrawChinese("用 git rebase -i 来 squash 合并、drop 删除、reword 修改消息。", 120, dialogY + 60, 20, YELLOW);
    }
    else if (currentStage == Stage::VERIFY_RESULT) {
        DrawRectangle(100, dialogY, 1080, 120, {40, 60, 40, 240});
        DrawRectangleLines(100, dialogY, 1080, 120, GREEN);
        DrawChinese("CTO: 完美！从 10 个杂乱的提交整理成了干净的历史。", 120, dialogY + 20, 24, GREEN);
        DrawChinese(("Squash 合并了 " + std::to_string(squashedCount) + " 个，删除了 " + std::to_string(droppedCount) + " 个临时提交。").c_str(),
                    120, dialogY + 55, 20, WHITE);
        DrawChinese("现在可以干净地 merge 到 main 了。按 [空格] 完成。", 120, dialogY + 90, 18, YELLOW);
    }
}

void Level09_Interactive::RefreshWorkingDirectory() {
    if (splitView && !repoPath.empty()) {
        splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
    }
}

void Level09_Interactive::Shutdown() {
    splitView.reset();
    git.reset();
    try { fs::remove_all(repoPath); } catch (...) {}
}

bool Level09_Interactive::IsComplete() const {
    return stageComplete;
}
