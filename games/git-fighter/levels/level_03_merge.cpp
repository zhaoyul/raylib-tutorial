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

    std::cout << "[Level3] Creating repo at: " << repoPath << std::endl;

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
    std::cout << "[Level3] Commit 1: Initial config, HEAD=" << git->GetHEAD().substr(0,7) << std::endl;

    // 关键：先在 master 上创建一个提交，建立分叉的基础
    {
        std::ofstream file(repoPath + "/main.cpp");
        file << "// 主程序 v1.0\nint main() { return 0; }\n";
    }
    git->Add(".");
    auto result = git->Commit("Add main.cpp");
    std::string mainCommit2 = git->GetHEAD();
    std::cout << "[Level3] Commit 2: Add main.cpp, HEAD=" << mainCommit2.substr(0,7) << std::endl;
    std::cout << "[Level3] Current branch: " << git->GetCurrentBranch() << std::endl;

    // 从 master 分叉创建 feature 分支
    git->CreateBranch("feature");
    std::cout << "[Level3] Created feature branch from " << mainCommit2.substr(0,7) << std::endl;

    // 切换到 feature 并提交
    git->Checkout("feature");
    std::cout << "[Level3] Switched to feature, HEAD=" << git->GetHEAD().substr(0,7) << std::endl;
    std::cout << "[Level3] Current branch: " << git->GetCurrentBranch() << std::endl;

    {
        std::ofstream file(repoPath + "/config.txt");
        file << "# 配置文件\n"
             << "debug=true\n"
             << "timeout=60\n"
             << "retries=5\n"
             << "# Feature changes\n";
    }
    git->Add(".");
    git->Commit("Feature: Update config");
    std::string featureCommit = git->GetHEAD();
    std::cout << "[Level3] Commit 3 (feature): Update config, HEAD=" << featureCommit.substr(0,7) << std::endl;

    // 切换回 master 并提交 - 这会创建分叉！
    result = git->Checkout("master");
    if (!result.success) {
        std::cout << "[Level3] ERROR: Failed to checkout master: " << result.error << std::endl;
    }
    std::cout << "[Level3] Switched back to master, HEAD=" << git->GetHEAD().substr(0,7) << std::endl;
    std::cout << "[Level3] Current branch: " << git->GetCurrentBranch() << std::endl;

    {
        std::ofstream file(repoPath + "/config.txt");
        file << "# 配置文件\n"
             << "debug=false\n"
             << "timeout=45\n"
             << "retries=3\n"
             << "# Main changes\n";
    }
    git->Add(".");
    git->Commit("Main: Update config");
    std::string mainCommit3 = git->GetHEAD();
    std::cout << "[Level3] Commit 4 (master): Update config, HEAD=" << mainCommit3.substr(0,7) << std::endl;

    // 验证分支指向
    std::cout << "\n[Level3] Final state:" << std::endl;
    std::cout << "  master branch should be at: " << mainCommit3.substr(0,7) << std::endl;
    std::cout << "  feature branch should be at: " << featureCommit.substr(0,7) << std::endl;

    SyncGraphWithRepo();
    splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
}

void Level03_Merge::SyncGraphWithRepo() {
    if (!git || !git->IsRepoOpen()) return;

    auto* commitPanel = splitView->GetCommitPanel();
    commitPanel->Clear();

    auto commits = git->GetCommitGraph(50);

    // Debug: print commit graph structure
    std::cout << "=== Level3 Graph ===" << std::endl;
    for (int i = 0; i < (int)commits.size() && i < 6; i++) {
        const auto& c = commits[i];
        std::cout << i << ": " << c.shortHash();
        std::cout << " [";
        for (const auto& b : c.branches) std::cout << b << " ";
        std::cout << "] parent:";
        for (const auto& p : c.parents) std::cout << " " << p.substr(0, 7);
        std::cout << std::endl;
    }
    std::cout << "===================" << std::endl;

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
        // Use system git to perform real merge (creates actual conflict)
        std::string fullCmd = "cd " + repoPath + " && git merge feature 2>&1";
        
        FILE* pipe = popen(fullCmd.c_str(), "r");
        std::string output;
        if (pipe) {
            char buffer[4096];
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                output += buffer;
            }
            pclose(pipe);
        }
        
        mergeAttempted = true;
        currentStage = Stage::RESOLVE_CONFLICT;
        SyncGraphWithRepo();
        RefreshWorkingDirectory();
        
        if (output.find("CONFLICT") != std::string::npos || 
            output.find("conflict") != std::string::npos) {
            return "合并产生冲突! 需要手动解决。\n" + output;
        }
        return "Merging feature...\n" + output;
    }
    else if ((cmd == "add" || cmd == "add .") && currentStage == Stage::RESOLVE_CONFLICT) {
        // 使用系统 git 添加文件（保持 merge 状态）
        std::string fullCmd = "cd " + repoPath + " && git add . 2>&1";
        
        FILE* pipe = popen(fullCmd.c_str(), "r");
        std::string output;
        if (pipe) {
            char buffer[4096];
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                output += buffer;
            }
            pclose(pipe);
        }
        
        conflictResolved = true;
        currentStage = Stage::COMMIT_RESOLUTION;
        RefreshWorkingDirectory();
        
        // Trim trailing newline
        while (!output.empty() && (output.back() == '\n' || output.back() == '\r')) {
            output.pop_back();
        }
        
        return output.empty() ? "Added files to staging" : output;
    }
    else if (cmd.rfind("commit", 0) == 0 && currentStage == Stage::COMMIT_RESOLUTION) {
        // 使用系统 git 提交 merge（这会创建有两个父节点的 merge commit）
        std::string fullCmd = "cd " + repoPath + " && git commit -m \"Merge branch 'feature' - resolved conflicts\" 2>&1";
        
        FILE* pipe = popen(fullCmd.c_str(), "r");
        std::string output;
        if (pipe) {
            char buffer[4096];
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                output += buffer;
            }
            pclose(pipe);
        }
        
        currentStage = Stage::COMPLETE;
        stageComplete = true;
        SyncGraphWithRepo();
        RefreshWorkingDirectory();
        
        // Trim trailing newline
        while (!output.empty() && (output.back() == '\n' || output.back() == '\r')) {
            output.pop_back();
        }
        
        return output.empty() ? "Merge commit created successfully!" : output;
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
        DrawChinese("CTO: master 和 feature 分支各自提交了代码！", 120, dialogY + 20, 24, WHITE);
        DrawChinese("两个分支都修改了 config.txt，形成真正的分叉历史。", 120, dialogY + 50, 22, LIGHTGRAY);
        DrawChinese("尝试 merge feature 时必然产生冲突，需要手动解决！", 120, dialogY + 80, 22, RED);
        DrawChinese("按 [空格] 开始合并挑战", 120, dialogY + 110, 20, YELLOW);
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
