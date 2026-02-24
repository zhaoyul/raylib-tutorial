#include "level_03_merge.h"
#include <raylib.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <set>

namespace fs = std::filesystem;

Level03_Merge::Level03_Merge()
    : Level(3, "合并危机", "学习处理合并冲突")
    , currentStage(Stage::INTRO)
    , timer(0)
    , stageComplete(false)
    , conflictCreated(false)
    , mergeAttempted(false)
    , conflictResolved(false) {
}

Level03_Merge::~Level03_Merge() = default;

void Level03_Merge::Initialize() {
    currentStage = Stage::INTRO;
    timer = 0;
    stageComplete = false;
    conflictCreated = false;
    mergeAttempted = false;
    conflictResolved = false;
    
    // 创建临时目录
    repoPath = "/tmp/gitfighter_level3_" + std::to_string((int)GetTime());
    fs::create_directories(repoPath);
    
    // 初始化 Git
    git = std::make_unique<GitWrapper>();
    
    // 初始化分屏视图
    splitView = std::make_unique<GitVis::SplitGitView>();
    splitView->Initialize(320, 100, 940, 520);
    splitView->SetSplitRatio(0.5f);
    splitView->SetGitWrapper(git.get());
    
    // 设置回调
    auto* commitPanel = splitView->GetCommitPanel();
    commitPanel->onNodeSelected = [this](const GitVis::CommitNode& node) {
        splitView->OnCommitSelected(node.hash);
    };
    splitView->SetRepoPath(repoPath);
    
    // 创建有冲突的仓库
    CreateRepoWithConflict();
    
    std::cout << "Level 3 initialized at: " << repoPath << std::endl;
}

void Level03_Merge::CreateRepoWithConflict() {
    // 初始化仓库
    git->Init(repoPath);
    git->OpenRepo(repoPath);
    
    // 创建初始文件
    {
        std::ofstream file(repoPath + "/config.txt");
        file << "# 配置文件\n"
             << "debug=false\n"
             << "timeout=30\n"
             << "retries=3\n";
    }
    git->Add(".");
    git->Commit("Initial config");
    
    // 创建 feature 分支并修改
    git->CreateBranch("feature");
    git->Checkout("feature");
    {
        std::ofstream file(repoPath + "/config.txt");
        file << "# 配置文件\n"
             << "debug=true\n"
             << "timeout=60\n"
             << "retries=5\n"
             << "# Feature branch changes\n";
    }
    git->Add(".");
    git->Commit("Update config in feature");
    
    // 回到 main 分支并修改（制造冲突）
    git->Checkout("main");
    {
        std::ofstream file(repoPath + "/config.txt");
        file << "# 配置文件\n"
             << "debug=false\n"
             << "timeout=45\n"
             << "retries=3\n"
             << "# Main branch changes\n";
    }
    git->Add(".");
    git->Commit("Update config in main");
    
    SyncGraphWithRepo();
    splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
}

void Level03_Merge::SyncGraphWithRepo() {
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
        {255, 150, 100, 255},   // Coral
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

std::string Level03_Merge::ProcessLevelCommand(const std::string& cmd) {
    std::cout << "Processing command: " << cmd << std::endl;
    RecordGitCommand(cmd);
    
    if (cmd == "merge feature" && currentStage == Stage::ATTEMPT_MERGE) {
        auto result = git->Merge("feature");
        mergeAttempted = true;
        currentStage = Stage::RESOLVE_CONFLICT;
        SyncGraphWithRepo();
        return "Merging feature...";
    }
    else if (cmd == "add" && currentStage == Stage::RESOLVE_CONFLICT) {
        // 模拟冲突已解决，添加文件
        git->Add(".");
        conflictResolved = true;
        currentStage = Stage::COMMIT_RESOLUTION;
        return "Added files to staging";
    }
    else if (cmd.rfind("commit", 0) == 0 && currentStage == Stage::COMMIT_RESOLUTION) {
        auto result = git->Commit("Merge branch 'feature' - resolved conflicts");
        if (result.success) {
            currentStage = Stage::COMPLETE;
            stageComplete = true;
            SyncGraphWithRepo();
            return "Committed merge resolution";
        }
    }
    // Return empty for unknown commands to let base class handle them
    return "";
}

void Level03_Merge::Update(float deltaTime) {
    timer += deltaTime;
    
    // 自适应布局
    if (splitView) {
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        splitView->SetBounds(320, 100, screenWidth - 320, screenHeight - 170);
        splitView->Update(deltaTime);
    }
    
    if (IsKeyPressed(KEY_SPACE) && currentStage == Stage::INTRO) {
        currentStage = Stage::ATTEMPT_MERGE;
    }
    
    if (IsKeyPressed(KEY_ENTER)) {
        switch (currentStage) {
            case Stage::ATTEMPT_MERGE:
                ProcessLevelCommand("merge feature");
                break;
            case Stage::RESOLVE_CONFLICT:
                ProcessLevelCommand("add");
                break;
            case Stage::COMMIT_RESOLUTION:
                ProcessLevelCommand("commit");
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

void Level03_Merge::Draw() {
    ClearBackground({30, 35, 45, 255});
    
    if (splitView) {
        splitView->Draw();
    }
    
    DrawStatusPanel();
    DrawDialogueIfNeeded();
}

void Level03_Merge::DrawStatusPanel() {
    DrawRectangle(0, 0, 300, 720, {40, 44, 52, 255});
    DrawRectangleLines(0, 0, 300, 720, {100, 150, 200, 255});
    
    DrawChinese("Level 3: 合并危机", 20, 20, 28, WHITE);
    DrawChinese("学习处理合并冲突", 20, 55, 18, LIGHTGRAY);
    DrawText("3 / 10", 250, 30, 16, {100, 200, 255, 255});
    
    DrawChinese("当前任务:", 20, 100, 20, {100, 200, 255, 255});
    
    const char* stageText = "";
    switch (currentStage) {
        case Stage::INTRO: stageText = "按空格开始"; break;
        case Stage::CREATE_CONFLICT: stageText = "创建冲突中..."; break;
        case Stage::ATTEMPT_MERGE: stageText = "执行: merge feature"; break;
        case Stage::RESOLVE_CONFLICT: stageText = "解决冲突后: add"; break;
        case Stage::COMMIT_RESOLUTION: stageText = "提交: commit"; break;
        case Stage::COMPLETE: stageText = "完成!"; break;
    }
    DrawText(stageText, 20, 130, 18, YELLOW);
    
    if (currentStage == Stage::RESOLVE_CONFLICT) {
        DrawRectangle(10, 300, 280, 200, {60, 30, 30, 255});
        DrawChinese("! 冲突提示", 20, 310, 20, RED);
        DrawChinese("config.txt 存在冲突", 20, 340, 16, WHITE);
        DrawChinese("需要手动编辑文件", 20, 365, 16, LIGHTGRAY);
        DrawChinese("保留需要的更改", 20, 390, 16, LIGHTGRAY);
        DrawChinese("然后 add + commit", 20, 415, 16, GREEN);
    }
    
    DrawRectangle(10, 600, 280, 100, {50, 50, 60, 255});
    DrawChinese("提示:", 20, 610, 18, {100, 200, 255, 255});
    DrawChinese("冲突发生在同一文件", 20, 635, 16, LIGHTGRAY);
    DrawChinese("被不同分支修改时", 20, 655, 16, LIGHTGRAY);
}

void Level03_Merge::DrawDialogueIfNeeded() {
    if (currentStage == Stage::INTRO) {
        int screenHeight = GetScreenHeight();
        int dialogY = screenHeight - 180;
        DrawRectangle(100, dialogY, 1080, 150, {40, 44, 52, 240});
        DrawRectangleLines(100, dialogY, 1080, 150, {100, 150, 200, 255});
        DrawChinese("CTO: main 与 feature 分支一起修改了 config.txt！", 120, dialogY + 20, 24, WHITE);
        DrawChinese("尝试合并时会产生冲突，需要手动解决。", 120, dialogY + 50, 22, LIGHTGRAY);
        DrawChinese("按 [空格] 开始合并挑战", 120, dialogY + 100, 20, YELLOW);
    }
}

void Level03_Merge::RefreshWorkingDirectory() {
    if (splitView && !repoPath.empty()) {
        splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
    }
}

void Level03_Merge::Shutdown() {
    splitView.reset();
    git.reset();
    
    try {
        fs::remove_all(repoPath);
    } catch (...) {}
}

bool Level03_Merge::IsComplete() const {
    return stageComplete;
}


