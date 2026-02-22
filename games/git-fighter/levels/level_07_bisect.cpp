#include "level_07_bisect.h"
#include <raylib.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <algorithm>

namespace fs = std::filesystem;

Level07_Bisect::Level07_Bisect()
    : Level(7, "故障定位", "用 bisect 二分查找问题 commit")
    , currentStage(Stage::INTRO)
    , timer(0)
    , stageComplete(false)
    , goodIndex(-1)
    , badIndex(-1)
    , currentTestIndex(-1)
    , bugCommitIndex(-1)
    , stepsTaken(0)
    , maxSteps(0)
    , currentTestResult(false) {
}

Level07_Bisect::~Level07_Bisect() = default;

void Level07_Bisect::Initialize() {
    currentStage = Stage::INTRO;
    timer = 0;
    stageComplete = false;
    goodIndex = -1;
    badIndex = -1;
    currentTestIndex = -1;
    bugCommitIndex = -1;
    stepsTaken = 0;
    maxSteps = 0;
    allCommits.clear();
    commitStatus.clear();
    
    repoPath = "/tmp/gitfighter_level7_" + std::to_string((int)GetTime());
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
    
    CreateBuggyHistory();
    std::cout << "Level 7 initialized at: " << repoPath << std::endl;
    std::cout << "Bug introduced at commit index: " << bugCommitIndex << std::endl;
}

void Level07_Bisect::CreateBuggyHistory() {
    git->Init(repoPath);
    git->OpenRepo(repoPath);
    
    // 创建 10 个 commit，其中第 6 个引入 bug
    const int totalCommits = 10;
    bugCommitIndex = 6;  // 第 7 个 commit（0-indexed 是 6）引入 bug
    
    for (int i = 0; i < totalCommits; i++) {
        std::string filename = repoPath + "/version.txt";
        std::ofstream f(filename);
        
        if (i < bugCommitIndex) {
            // Bug 之前 - 正常工作
            f << "Version " << (i + 1) << ".0\n";
            f << "Status: WORKING\n";
            f << "Feature: " << i << "\n";
        } else {
            // Bug 之后 - 有问题
            f << "Version " << (i + 1) << ".0\n";
            f << "Status: BROKEN\n";
            f << "Feature: " << i << "\n";
            if (i == bugCommitIndex) {
                f << "BUG: Null pointer introduced!\n";
            }
        }
        f.close();
        
        git->Add(".");
        std::string msg = (i == bugCommitIndex) ? 
            "Add feature " + std::to_string(i) + " [BUG INTRODUCED]" :
            "Add feature " + std::to_string(i);
        git->Commit(msg);
    }
    
    // 获取所有 commits（从新到旧）
    auto commits = git->GetCommitGraph(totalCommits);
    for (const auto& c : commits) {
        allCommits.push_back(c.hash);
        commitStatus.push_back(false);  // unknown
    }
    
    // 计算最优步数：log2(n)
    maxSteps = 0;
    int n = totalCommits;
    while (n > 0) {
        n /= 2;
        maxSteps++;
    }
    
    SyncGraphWithRepo();
    splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
}

void Level07_Bisect::SyncGraphWithRepo() {
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

int Level07_Bisect::CalculateNextIndex() {
    // 二分查找：取中间
    if (goodIndex == -1) {
        // 还没有 good，取靠近 bad 的
        return badIndex - 1;
    }
    if (badIndex == -1) {
        return goodIndex + 1;
    }
    return (goodIndex + badIndex) / 2;
}

void Level07_Bisect::StartBisect() {
    // 已知最新是 bad，最旧是 good
    badIndex = 0;  // 最新的（index 0）
    goodIndex = (int)allCommits.size() - 1;  // 最旧的
    commitStatus[badIndex] = false;  // bad
    commitStatus[goodIndex] = true;  // good
    
    currentTestIndex = CalculateNextIndex();
    currentStage = Stage::TESTING_COMMIT;
}

void Level07_Bisect::TestCurrentCommit() {
    // 模拟测试当前 commit
    // 如果 currentTestIndex >= bugCommitIndex，则是 bad（有 bug）
    currentTestResult = (currentTestIndex >= bugCommitIndex);
    
    if (currentTestResult) {
        currentStage = Stage::MARK_BAD;
    } else {
        currentStage = Stage::MARK_GOOD;
    }
}

void Level07_Bisect::MarkGood() {
    commitStatus[currentTestIndex] = true;
    goodIndex = currentTestIndex;
    stepsTaken++;
    NextBisectStep();
}

void Level07_Bisect::MarkBad() {
    commitStatus[currentTestIndex] = false;
    badIndex = currentTestIndex;
    stepsTaken++;
    NextBisectStep();
}

void Level07_Bisect::NextBisectStep() {
    // 检查是否找到 culprit
    if (badIndex - goodIndex <= 1) {
        currentStage = Stage::FOUND_CULPRIT;
        return;
    }
    
    currentTestIndex = CalculateNextIndex();
    currentStage = Stage::TESTING_COMMIT;
}

void Level07_Bisect::Update(float deltaTime) {
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
                currentStage = Stage::START_BISECT;
                break;
            case Stage::FOUND_CULPRIT:
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
    
    if (IsKeyPressed(KEY_B) && currentStage == Stage::START_BISECT) {
        StartBisect();
    }
    
    if (IsKeyPressed(KEY_T) && currentStage == Stage::TESTING_COMMIT) {
        TestCurrentCommit();
    }
    
    if (IsKeyPressed(KEY_G) && currentStage == Stage::MARK_GOOD) {
        MarkGood();
    }
    
    if (IsKeyPressed(KEY_X) && currentStage == Stage::MARK_BAD) {
        MarkBad();
    }
}

void Level07_Bisect::Draw() {
    ClearBackground({30, 35, 45, 255});
    if (splitView) splitView->Draw();
    
    if (currentStage != Stage::INTRO && currentStage != Stage::START_BISECT) {
        DrawBisectPanel();
        DrawCommitRange();
    }
    
    DrawStatusPanel();
    DrawDialogueIfNeeded();
}

void Level07_Bisect::DrawStatusPanel() {
    DrawRectangle(0, 0, 300, 720, {40, 44, 52, 255});
    DrawRectangleLines(0, 0, 300, 720, {100, 150, 200, 255});
    
    DrawChinese("Level 7: 故障定位", 20, 20, 28, WHITE);
    DrawChinese("bisect 二分查找", 20, 55, 18, LIGHTGRAY);
    
    DrawChinese("当前步骤:", 20, 100, 20, {100, 200, 255, 255});
    
    const char* stageText = "";
    Color stageColor = YELLOW;
    switch (currentStage) {
        case Stage::INTRO: stageText = "按空格开始"; break;
        case Stage::START_BISECT: stageText = "按 B 开始"; break;
        case Stage::TESTING_COMMIT: stageText = "按 T 测试"; break;
        case Stage::MARK_GOOD: stageText = "按 G=好"; stageColor = GREEN; break;
        case Stage::MARK_BAD: stageText = "按 X=坏"; stageColor = RED; break;
        case Stage::FOUND_CULPRIT: stageText = "找到了!"; stageColor = {255, 100, 255, 255}; break;
        case Stage::COMPLETE: stageText = "完成!"; stageColor = GREEN; break;
    }
    DrawText(stageText, 20, 130, 18, stageColor);
    
    // 进度统计
    if (currentStage != Stage::INTRO && currentStage != Stage::START_BISECT) {
        DrawRectangle(10, 170, 280, 150, {40, 40, 50, 255});
        DrawChinese("搜索进度:", 20, 180, 18, WHITE);
        DrawText(("步数: " + std::to_string(stepsTaken) + "/" + std::to_string(maxSteps)).c_str(),
                 20, 210, 16, YELLOW);
        DrawText(("范围: " + std::to_string(goodIndex + 1) + " - " + std::to_string(badIndex + 1)).c_str(),
                 20, 240, 16, LIGHTGRAY);
        
        if (currentTestIndex >= 0) {
            DrawText(("当前测试: #" + std::to_string(currentTestIndex + 1)).c_str(),
                     20, 270, 16, {100, 200, 255, 255});
        }
    }
    
    // 图例
    DrawRectangle(10, 600, 280, 100, {50, 50, 60, 255});
    DrawChinese("图例:", 20, 610, 18, {100, 200, 255, 255});
    DrawRectangle(20, 635, 15, 15, GREEN);
    DrawChinese("= 正常 (Good)", 45, 635, 14, LIGHTGRAY);
    DrawRectangle(20, 655, 15, 15, RED);
    DrawChinese("= 有问题 (Bad)", 45, 655, 14, LIGHTGRAY);
    DrawRectangle(20, 675, 15, 15, YELLOW);
    DrawChinese("= 测试中", 45, 675, 14, LIGHTGRAY);
}

void Level07_Bisect::DrawBisectPanel() {
    int screenWidth = GetScreenWidth();
    int panelX = screenWidth - 350;
    int panelY = 100;
    int panelW = 330;
    int panelH = 300;
    
    DrawRectangle(panelX, panelY, panelW, panelH, {20, 25, 35, 240});
    DrawRectangleLines(panelX, panelY, panelW, panelH, {100, 200, 255, 255});
    
    DrawChinese("Bisect 状态", panelX + 10, panelY + 10, 20, {100, 200, 255, 255});
    
    // 绘制测试进度指示
    int barY = panelY + 50;
    int barHeight = 200;
    int commitCount = (int)allCommits.size();
    float commitHeight = (float)barHeight / commitCount;
    
    for (int i = 0; i < commitCount; i++) {
        float y = barY + barHeight - (i + 1) * commitHeight;
        
        Color c = {80, 80, 80, 255};  // unknown
        if (commitStatus[i]) c = GREEN;  // good
        else if (!commitStatus[i] && (i == 0 || i == badIndex || commitStatus[i] == false)) {
            // Check if it's been explicitly marked bad
            if (i == 0 || i == badIndex) c = RED;
        }
        
        // Override for known states
        if (i == goodIndex) c = GREEN;
        if (i == badIndex) c = RED;
        if (i == currentTestIndex) c = YELLOW;
        
        DrawRectangle(panelX + 20, (int)y, 40, (int)commitHeight - 2, c);
        
        // Commit 编号
        DrawText(std::to_string(i + 1).c_str(), panelX + 70, (int)y + 2, 12, WHITE);
    }
    
    // 分界线
    if (goodIndex >= 0 && badIndex >= 0) {
        float goodY = barY + barHeight - (goodIndex + 0.5f) * commitHeight;
        float badY = barY + barHeight - (badIndex + 0.5f) * commitHeight;
        DrawLine(panelX + 100, (int)goodY, panelX + 280, (int)goodY, GREEN);
        DrawLine(panelX + 100, (int)badY, panelX + 280, (int)badY, RED);
    }
}

void Level07_Bisect::DrawCommitRange() {
    int screenWidth = GetScreenWidth();
    int panelX = screenWidth - 350;
    int panelY = 420;
    int panelW = 330;
    int panelH = 150;
    
    DrawRectangle(panelX, panelY, panelW, panelH, {30, 35, 45, 240});
    
    if (currentStage == Stage::TESTING_COMMIT && currentTestIndex >= 0) {
        DrawChinese("正在测试 commit:", panelX + 10, panelY + 10, 18, WHITE);
        DrawText(allCommits[currentTestIndex].substr(0, 7).c_str(),
                 panelX + 10, panelY + 40, 20, YELLOW);
        
        // 模拟测试结果预览
        DrawChinese("运行测试中...", panelX + 10, panelY + 80, 16, LIGHTGRAY);
    }
    else if (currentStage == Stage::MARK_GOOD) {
        DrawChinese("测试结果:", panelX + 10, panelY + 10, 18, WHITE);
        DrawChinese("[正常] 没有发现 Bug", panelX + 10, panelY + 50, 20, GREEN);
        DrawChinese("按 [G] 标记为 Good", panelX + 10, panelY + 100, 16, YELLOW);
    }
    else if (currentStage == Stage::MARK_BAD) {
        DrawChinese("测试结果:", panelX + 10, panelY + 10, 18, WHITE);
        DrawChinese("[失败] 发现 Bug!", panelX + 10, panelY + 50, 20, RED);
        DrawChinese("按 [X] 标记为 Bad", panelX + 10, panelY + 100, 16, YELLOW);
    }
    else if (currentStage == Stage::FOUND_CULPRIT) {
        DrawChinese("罪魁祸首找到!", panelX + 10, panelY + 10, 20, {255, 100, 255, 255});
        DrawText(allCommits[badIndex].substr(0, 7).c_str(),
                 panelX + 10, panelY + 50, 24, RED);
        DrawChinese("引入 Bug 的提交", panelX + 10, panelY + 100, 16, LIGHTGRAY);
    }
}

void Level07_Bisect::DrawDialogueIfNeeded() {
    if (currentStage == Stage::INTRO) {
        DrawRectangle(100, 500, 1080, 150, {40, 44, 52, 240});
        DrawRectangleLines(100, 500, 1080, 150, {100, 150, 200, 255});
        DrawChinese("CTO: 用户报告了一个严重 bug，但我们不知道是哪个 commit 引入的！", 120, 520, 24, RED);
        DrawChinese("有 10 个 commit，手动检查太慢了。用 bisect 二分查找吧！", 120, 550, 22, LIGHTGRAY);
        DrawChinese("按 [空格] 开始故障定位", 120, 600, 20, YELLOW);
    }
    else if (currentStage == Stage::START_BISECT) {
        DrawRectangle(100, 500, 1080, 100, {40, 40, 60, 240});
        DrawRectangleLines(100, 500, 1080, 100, {100, 150, 200, 255});
        DrawChinese("最新版本有 bug（Bad），最旧版本正常（Good）。", 120, 530, 22, WHITE);
        DrawChinese("按 [B] 开始 bisect，Git 会自动选择中间的 commit 让你测试。", 120, 560, 20, YELLOW);
    }
    else if (currentStage == Stage::TESTING_COMMIT) {
        DrawRectangle(100, 500, 1080, 100, {60, 60, 40, 240});
        DrawRectangleLines(100, 500, 1080, 100, {200, 200, 100, 255});
        DrawChinese(("正在测试 commit #" + std::to_string(currentTestIndex + 1)).c_str(), 120, 530, 24, YELLOW);
        DrawChinese("按 [T] 运行测试程序，检查这个版本是否有 bug。", 120, 560, 20, WHITE);
    }
    else if (currentStage == Stage::FOUND_CULPRIT) {
        DrawRectangle(100, 500, 1080, 120, {60, 30, 60, 240});
        DrawRectangleLines(100, 500, 1080, 120, {255, 100, 255, 255});
        DrawChinese("CTO: 找到了！二分查找只用了几步就定位到了罪魁祸首！", 120, 520, 26, {255, 100, 255, 255});
        DrawChinese(("Bug 是在 commit #" + std::to_string(badIndex + 1) + " 引入的").c_str(), 120, 560, 22, WHITE);
        DrawChinese("现在可以针对性地修复这个提交了。按 [空格] 完成。", 120, 590, 18, YELLOW);
    }
}

void Level07_Bisect::Shutdown() {
    splitView.reset();
    git.reset();
    try { fs::remove_all(repoPath); } catch (...) {}
}

bool Level07_Bisect::IsComplete() const {
    return stageComplete;
}
