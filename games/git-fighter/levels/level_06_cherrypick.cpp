#include "level_06_cherrypick.h"
#include <raylib.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <unordered_set>

namespace fs = std::filesystem;

Level06_CherryPick::Level06_CherryPick()
    : Level(6, "紧急修复", "用 cherry-pick 挑选修复到生产环境")
    , currentStage(Stage::INTRO)
    , timer(0)
    , stageComplete(false)
    , currentPickIndex(0)
    , hasConflict(false) {
}

Level06_CherryPick::~Level06_CherryPick() = default;

void Level06_CherryPick::Initialize() {
    currentStage = Stage::INTRO;
    timer = 0;
    stageComplete = false;
    currentPickIndex = 0;
    hasConflict = false;
    pendingConflictHash.clear();
    availableFixes.clear();
    selectedFixes.clear();
    pickedFixes.clear();

    repoPath = "/tmp/gitfighter_level6_" + std::to_string((int)GetTime());
    fs::create_directories(repoPath);

    git = std::make_unique<GitWrapper>();

    splitView = std::make_unique<GitVis::SplitGitView>();
    splitView->Initialize(320, 100, 940, 520);
    splitView->SetSplitRatio(0.5f);
    splitView->SetGitWrapper(git.get());

    auto* commitPanel = splitView->GetCommitPanel();
    commitPanel->onNodeSelected = [this](const GitVis::CommitNode& node) {
        hoveredCommit = node.hash;
        splitView->OnCommitSelected(node.hash);
    };
    splitView->SetRepoPath(repoPath);

    CreateProductionScenario();
    std::cout << "Level 6 initialized at: " << repoPath << std::endl;
}

void Level06_CherryPick::CreateProductionScenario() {
    git->Init(repoPath);
    git->OpenRepo(repoPath);

    // 模拟 release 分支 - 稳定的生产版本
    {
        std::ofstream f(repoPath + "/server.cpp");
        f << "// 服务器 v1.0 (stable)\n"
          << "void startServer() {\n"
          << "    listen(8080);\n"
          << "    // BUG: 内存泄漏在这里\n"
          << "}\n";
    }
    git->Add(".");
    git->Commit("Release v1.0");

    // 创建 dev 分支继续开发
    git->CreateBranch("dev");
    git->Checkout("dev");

    // Dev commit 1: 新功能（不需要到 release）
    {
        std::ofstream f(repoPath + "/feature.cpp");
        f << "// 实验性功能\nvoid beta() {}\n";
    }
    git->Add(".");
    git->Commit("Add experimental feature");

    // Dev commit 2: Bug 修复 1（需要 cherry-pick！）
    {
        std::ofstream f(repoPath + "/server.cpp");
        f << "// 服务器 v1.0 (stable)\n"
          << "void startServer() {\n"
          << "    listen(8080);\n"
          << "    // FIX: 修复内存泄漏\n"
          << "    initMemoryPool();\n"
          << "}\n";
    }
    git->Add(".");
    git->Commit("CRITICAL: Fix memory leak in server");

    // Dev commit 3: 更多功能
    {
        std::ofstream f(repoPath + "/api.cpp");
        f << "// API 模块\nvoid handleRequest() {}\n";
    }
    git->Add(".");
    git->Commit("Add API module");

    // Dev commit 4: Bug 修复 2（也需要 cherry-pick！）
    {
        std::ofstream f(repoPath + "/server.cpp");
        f << "// 服务器 v1.0 (stable)\n"
          << "void startServer() {\n"
          << "    listen(8080);\n"
          << "    // FIX: 修复内存泄漏\n"
          << "    initMemoryPool();\n"
          << "    // FIX: 添加超时控制\n"
          << "    setTimeout(30);\n"
          << "}\n";
    }
    git->Add(".");
    git->Commit("CRITICAL: Add timeout to prevent hanging");

    // Dev commit 5: 重构（不需要到 release）
    {
        std::ofstream f(repoPath + "/utils.cpp");
        f << "// 工具函数\nvoid helper() {}\n";
    }
    git->Add(".");
    git->Commit("Refactor utilities");

    // 回到 release 分支
    git->Checkout("main");

    // 收集可用的修复 commits
    auto commits = git->GetCommitGraph(20);
    for (const auto& c : commits) {
        if (c.message.find("CRITICAL") != std::string::npos ||
            c.message.find("Fix") != std::string::npos ||
            c.message.find("BUG") != std::string::npos) {
            availableFixes.push_back(c.hash);
        }
    }
    // revwalk 默认是新到旧；关卡里按 1/2 时希望先应用较早修复，降低冲突概率
    std::reverse(availableFixes.begin(), availableFixes.end());
    std::unordered_set<std::string> seenFixes;
    std::vector<std::string> dedupedFixes;
    dedupedFixes.reserve(availableFixes.size());
    for (const auto& h : availableFixes) {
        if (seenFixes.insert(h).second) {
            dedupedFixes.push_back(h);
        }
    }
    availableFixes.swap(dedupedFixes);

    SyncGraphWithRepo();
    splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
}

void Level06_CherryPick::SyncGraphWithRepo() {
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
        {255, 200, 100, 255},   // dev - orange
        {100, 255, 150, 255},   // feature
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

std::string Level06_CherryPick::ProcessLevelCommand(const std::string& cmd) {
    RecordGitCommand(cmd);
    if (cmd.rfind("cherry-pick", 0) == 0 &&
        (currentStage == Stage::SELECT_COMMITS || currentStage == Stage::PICKING)) {
        std::string hash = cmd.substr(12);
        if (!hash.empty()) {
            if (std::find(pickedFixes.begin(), pickedFixes.end(), hash) != pickedFixes.end()) {
                currentStage = Stage::PICKING;
                return "Already cherry-picked " + hash.substr(0, 7);
            }

            if (std::find(selectedFixes.begin(), selectedFixes.end(), hash) == selectedFixes.end()) {
                selectedFixes.push_back(hash);
            }
            currentStage = Stage::PICKING;

            auto result = git->CherryPick(hash);
            if (result.success) {
                pickedFixes.push_back(hash);
                pendingConflictHash.clear();
                SyncGraphWithRepo();
                return "Cherry-picked " + hash.substr(0, 7);
            } else {
                hasConflict = true;
                pendingConflictHash = hash;
                currentStage = Stage::HANDLE_CONFLICT;
                return "Cherry-pick conflict on " + hash.substr(0, 7);
            }
        }
    }
    else if (cmd == "resolve" && currentStage == Stage::HANDLE_CONFLICT) {
        hasConflict = false;
        // 引导玩家重新选择提交，避免停在“已应用 0”但无法继续的状态
        currentStage = Stage::SELECT_COMMITS;
        return "Conflict marked resolved, choose another fix (1/2)";
    }
    else if (cmd == "done" && currentStage == Stage::PICKING) {
        if (pickedFixes.empty()) {
            currentStage = Stage::SELECT_COMMITS;
            return "No fixes applied yet, choose one with 1/2 first";
        }
        currentStage = Stage::VERIFY_FIX;
        return "Done picking";
    }
    // Return empty for unknown commands to let base class handle them
    return "";
}

void Level06_CherryPick::Update(float deltaTime) {
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
                currentStage = Stage::SHOW_COMMITS;
                break;
            case Stage::VERIFY_FIX:
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

    if (IsKeyPressed(KEY_C) && currentStage == Stage::SHOW_COMMITS) {
        currentStage = Stage::SELECT_COMMITS;
    }

    // 数字键选择 commits：在选择阶段和应用阶段都可继续选择
    if (currentStage == Stage::SHOW_COMMITS || currentStage == Stage::SELECT_COMMITS || currentStage == Stage::PICKING) {
        if (IsKeyPressed(KEY_ONE) && availableFixes.size() > 0) {
            ProcessLevelCommand("cherry-pick " + availableFixes[0]);
        }
        if (IsKeyPressed(KEY_TWO) && availableFixes.size() > 1) {
            ProcessLevelCommand("cherry-pick " + availableFixes[1]);
        }
    }

    if (IsKeyPressed(KEY_F) && currentStage == Stage::HANDLE_CONFLICT) {
        ProcessLevelCommand("resolve");
    }

    if (IsKeyPressed(KEY_ENTER) && currentStage == Stage::PICKING) {
        ProcessLevelCommand("done");
    }

    // Dev tools (1/2/3 keys)
    HandleDevToolKeys();
}

void Level06_CherryPick::Draw() {
    ClearBackground({30, 35, 45, 255});
    if (splitView) splitView->Draw();
    DrawStatusPanel();
    if (currentStage == Stage::SELECT_COMMITS) {
        DrawCommitSelector();
    }
    DrawDialogueIfNeeded();
}

void Level06_CherryPick::DrawStatusPanel() {
    DrawRectangle(0, 0, 300, 720, {40, 44, 52, 255});
    DrawRectangleLines(0, 0, 300, 720, {100, 150, 200, 255});

    DrawChinese("Level 6: 紧急修复", 20, 20, 28, WHITE);
    DrawChinese("cherry-pick 精准修复", 20, 55, 18, LIGHTGRAY);
    DrawText("6 / 10", 250, 30, 16, {100, 200, 255, 255});

    DrawChinese("当前任务:", 20, 100, 20, {100, 200, 255, 255});

    const char* stageText = "";
    Color stageColor = YELLOW;
    switch (currentStage) {
        case Stage::INTRO: stageText = "按空格开始"; break;
        case Stage::SHOW_COMMITS: stageText = "按 C 或 1/2 选择修复"; break;
        case Stage::SELECT_COMMITS: stageText = "按 1/2 应用修复"; break;
        case Stage::PICKING: stageText = "可继续按 1/2，按回车完成"; break;
        case Stage::HANDLE_CONFLICT: stageText = "冲突! 按 F 返回选择"; stageColor = RED; break;
        case Stage::VERIFY_FIX: stageText = "修复完成! 按空格通关"; stageColor = GREEN; break;
        case Stage::COMPLETE: stageText = "完成!"; stageColor = GREEN; break;
    }
    DrawChinese(stageText, 20, 130, 18, stageColor);

    // 进度
    if (currentStage == Stage::PICKING || currentStage == Stage::VERIFY_FIX) {
        DrawRectangle(10, 170, 280, 120, {40, 60, 40, 255});
        DrawChinese("修复进度:", 20, 180, 18, WHITE);
        DrawChinese(("已应用: " + std::to_string(pickedFixes.size())).c_str(), 20, 210, 16, GREEN);

        for (size_t i = 0; i < pickedFixes.size() && i < 3; i++) {
            DrawText(("[OK] " + pickedFixes[i].substr(0, 7)).c_str(), 20, 235 + i*20, 14, {100, 255, 100, 255});
        }
    }

    // 提示
    DrawRectangle(10, 560, 280, 140, {50, 50, 60, 255});
    DrawChinese("提示:", 20, 610, 18, {100, 200, 255, 255});
    if (currentStage == Stage::SHOW_COMMITS) {
        DrawChinese("C: 打开修复列表", 20, 635, 16, LIGHTGRAY);
        DrawChinese("或直接按 1/2 选修复", 20, 655, 16, LIGHTGRAY);
    } else if (currentStage == Stage::SELECT_COMMITS) {
        DrawChinese("1/2: 选择修复", 20, 635, 16, LIGHTGRAY);
        DrawChinese("只挑 CRITICAL 提交", 20, 655, 16, LIGHTGRAY);
    } else if (currentStage == Stage::PICKING) {
        DrawChinese("1/2: 可继续应用", 20, 635, 16, LIGHTGRAY);
        DrawChinese("Enter: 完成并验证", 20, 655, 16, LIGHTGRAY);
    } else if (currentStage == Stage::HANDLE_CONFLICT) {
        DrawChinese("F: 标记冲突已解决", 20, 635, 16, LIGHTGRAY);
        DrawChinese("返回后按 1/2 选其他修复", 20, 655, 16, LIGHTGRAY);
    } else if (currentStage == Stage::VERIFY_FIX) {
        DrawChinese("空格: 进入完成状态", 20, 635, 16, LIGHTGRAY);
        DrawChinese("你可以继续查看图谱变化", 20, 655, 16, LIGHTGRAY);
    }

    // 步骤清单：明确当前关卡的最短通关路径
    DrawRectangle(10, 350, 280, 190, {35, 40, 50, 255});
    DrawRectangleLines(10, 350, 280, 190, {100, 150, 200, 255});
    DrawChinese("通关步骤:", 20, 360, 18, {100, 200, 255, 255});

    bool stepOpenListDone = (currentStage != Stage::INTRO && currentStage != Stage::SHOW_COMMITS) || !pickedFixes.empty();
    bool stepPickFixDone = !pickedFixes.empty();
    bool stepResolveDone = !hasConflict;
    bool stepVerifyDone = (currentStage == Stage::VERIFY_FIX || currentStage == Stage::COMPLETE);
    bool stepCompleteDone = (currentStage == Stage::COMPLETE);

    bool stepOpenListActive = !stepOpenListDone;
    bool stepPickFixActive = stepOpenListDone && !stepPickFixDone;
    bool stepResolveActive = stepPickFixDone && hasConflict;
    bool stepVerifyActive = stepPickFixDone && stepResolveDone && !stepVerifyDone;
    bool stepCompleteActive = stepVerifyDone && !stepCompleteDone;

    auto drawStep = [&](int y, const char* label, bool done, bool active) {
        const char* status = done ? "[x]" : "[ ]";
        Color color = done ? GREEN : (active ? YELLOW : LIGHTGRAY);
        std::string line = std::string(status) + " " + label;
        DrawChinese(line.c_str(), 20, y, 15, color);
    };

    drawStep(388, "打开列表 (C)", stepOpenListDone, stepOpenListActive);
    drawStep(416, "应用修复 (1/2)", stepPickFixDone, stepPickFixActive);
    drawStep(444, "处理冲突 (F, 如有)", stepResolveDone, stepResolveActive);
    drawStep(472, "完成验证 (Enter)", stepVerifyDone, stepVerifyActive);
    drawStep(500, "通关 (空格)", stepCompleteDone, stepCompleteActive);
}

void Level06_CherryPick::DrawCommitSelector() {
    int screenWidth = GetScreenWidth();
    int panelX = screenWidth - 450;
    int panelY = 100;
    int panelW = 430;
    int panelH = 400;

    DrawRectangle(panelX, panelY, panelW, panelH, {20, 25, 35, 240});
    DrawRectangleLines(panelX, panelY, panelW, panelH, {100, 200, 255, 255});

    DrawChinese("可选的修复提交:", panelX + 10, panelY + 10, 20, {100, 200, 255, 255});

    int y = panelY + 50;
    for (size_t i = 0; i < availableFixes.size() && y < panelY + panelH - 60; i++) {
        bool isPicked = std::find(pickedFixes.begin(), pickedFixes.end(), availableFixes[i]) != pickedFixes.end();

        Color bgColor = isPicked ? Color{40, 80, 40, 200} : Color{60, 60, 40, 200};
        DrawRectangle(panelX + 10, y, panelW - 20, 50, bgColor);

        std::string label = "[" + std::to_string(i + 1) + "] " + availableFixes[i].substr(0, 7);
        DrawText(label.c_str(), panelX + 20, y + 8, 16, isPicked ? GREEN : YELLOW);

        if (isPicked) {
            DrawChinese("[已应用]", panelX + panelW - 100, y + 15, 14, GREEN);
        }

        y += 60;
    }

    DrawRectangle(panelX, panelY + panelH - 50, panelW, 50, {30, 35, 45, 255});
    DrawChinese("按数字键选择要应用的修复", panelX + 20, panelY + panelH - 35, 16, LIGHTGRAY);
}

void Level06_CherryPick::DrawDialogueIfNeeded() {
    int screenHeight = GetScreenHeight();
    int dialogY = screenHeight - 180;
    if (currentStage == Stage::INTRO) {
        DrawRectangle(100, dialogY, 1080, 150, {40, 44, 52, 240});
        DrawRectangleLines(100, dialogY, 1080, 150, {100, 150, 200, 255});
        DrawChinese("CTO: 生产环境服务器崩溃了！需要紧急修复！", 120, dialogY + 20, 24, RED);
        DrawChinese("dev 分支上有修复，但也有未完成的新功能。", 120, dialogY + 50, 22, LIGHTGRAY);
        DrawChinese("按 [空格] 开始紧急修复", 120, dialogY + 100, 20, YELLOW);
    }
    else if (currentStage == Stage::SHOW_COMMITS) {
        DrawRectangle(100, dialogY, 1080, 100, {40, 40, 60, 240});
        DrawRectangleLines(100, dialogY, 1080, 100, {100, 150, 200, 255});
        DrawChinese("main 分支是稳定的生产版本，dev 分支上有新功能和修复。", 120, dialogY + 30, 22, WHITE);
        DrawChinese("按 [C] 后用 [1]/[2] 选择修复，再按 [Enter] 完成。", 120, dialogY + 60, 20, YELLOW);
    }
    else if (currentStage == Stage::SELECT_COMMITS) {
        DrawRectangle(100, dialogY, 1080, 100, {60, 60, 40, 240});
        DrawRectangleLines(100, dialogY, 1080, 100, {200, 200, 100, 255});
        DrawChinese("选择标记为 CRITICAL 的修复提交，不要选新功能！", 120, dialogY + 30, 24, YELLOW);
        DrawChinese("按 [1] 或 [2] 应用对应的修复", 120, dialogY + 60, 20, WHITE);
    }
    else if (currentStage == Stage::PICKING) {
        DrawRectangle(100, dialogY, 1080, 110, {40, 60, 40, 240});
        DrawRectangleLines(100, dialogY, 1080, 110, {100, 200, 100, 255});
        DrawChinese("修复已应用到 main。你可以继续按 [1]/[2] 追加修复。", 120, dialogY + 25, 22, WHITE);
        DrawChinese("完成后按 [Enter] 进入验证，再按 [空格] 通关。", 120, dialogY + 60, 20, YELLOW);
    }
    else if (currentStage == Stage::HANDLE_CONFLICT) {
        DrawRectangle(100, dialogY, 1080, 100, {70, 35, 35, 240});
        DrawRectangleLines(100, dialogY, 1080, 100, RED);
        std::string conflictTitle = "cherry-pick 冲突";
        if (!pendingConflictHash.empty()) {
            conflictTitle += ": " + pendingConflictHash.substr(0, 7);
        }
        DrawChinese(conflictTitle.c_str(), 120, dialogY + 30, 24, RED);
        DrawChinese("按 [F] 后回到选择界面，建议先试另一个修复。", 120, dialogY + 60, 20, WHITE);
    }
    else if (currentStage == Stage::VERIFY_FIX) {
        DrawRectangle(100, dialogY, 1080, 100, {40, 60, 40, 240});
        DrawRectangleLines(100, dialogY, 1080, 100, GREEN);
        DrawChinese("CTO: 完美！cherry-pick 让我们只应用了必要的修复。", 120, dialogY + 20, 24, GREEN);
        DrawChinese("生产环境已修复，新功能还在 dev 分支等待发布。按 [空格] 通关。", 120, dialogY + 50, 20, WHITE);
    }
}

void Level06_CherryPick::RefreshWorkingDirectory() {
    if (splitView && !repoPath.empty()) {
        splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
    }
}

void Level06_CherryPick::Shutdown() {
    splitView.reset();
    git.reset();
    try { fs::remove_all(repoPath); } catch (...) {}
}

bool Level06_CherryPick::IsComplete() const {
    return stageComplete;
}
