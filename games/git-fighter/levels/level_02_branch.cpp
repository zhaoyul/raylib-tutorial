#include "level_02_branch.h"
#include <raylib.h>
#include <iostream>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

Level02_Branch::Level02_Branch()
    : Level(2, "分支探险", "学习 git branch 和 git checkout")
    , currentStage(Stage::INTRO)
    , timer(0)
    , stageComplete(false)
    , currentBranch("main")
    , featureBranchName("feature/login")
    , featureBranchCreated(false)
    , changesCommitted(false)
    , merged(false) {
}

Level02_Branch::~Level02_Branch() = default;

void Level02_Branch::Initialize() {
    currentStage = Stage::INTRO;
    timer = 0;
    stageComplete = false;
    featureBranchCreated = false;
    changesCommitted = false;
    merged = false;
    currentBranch = "main";
    
    // 创建临时目录
    repoPath = "/tmp/gitfighter_level2_" + std::to_string((int)GetTime());
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
    
    // 创建初始仓库
    CreateInitialRepo();
    
    std::cout << "Level 2 initialized at: " << repoPath << std::endl;
}

void Level02_Branch::CreateInitialRepo() {
    // 初始化仓库
    git->Init(repoPath);
    git->OpenRepo(repoPath);
    
    // 创建初始提交
    {
        std::ofstream file(repoPath + "/README.md");
        file << "# 电商平台\n\n项目初始化\n";
    }
    git->Add(".");
    git->Commit("Initial commit: 项目初始化");
    
    // 创建第二个提交
    {
        std::ofstream file(repoPath + "/main.cpp");
        file << "#include <iostream>\nint main() { return 0; }\n";
    }
    git->Add(".");
    git->Commit("Add main.cpp");
    
    SyncGraphWithRepo();
    splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
}

void Level02_Branch::SyncGraphWithRepo() {
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
        node.springX.position = node.springX.target = 200;
        node.springY.position = node.springY.target = 100;
        commitPanel->AddCommit(node);
    }
    
    std::string head = git->GetHEAD();
    if (!head.empty()) {
        commitPanel->AddBranch(currentBranch, head, {100, 200, 255, 255});
        commitPanel->SetHEAD(head);
    }
    
    commitPanel->RecalculateLayout();
}

void Level02_Branch::ProcessGitCommand(const std::string& cmd) {
    std::cout << "Processing command: " << cmd << std::endl;
    
    if (cmd.rfind("branch", 0) == 0 && currentStage == Stage::CREATE_BRANCH) {
        // Parse: branch feature/login
        size_t spacePos = cmd.find(' ');
        if (spacePos != std::string::npos) {
            std::string branchName = cmd.substr(spacePos + 1);
            auto result = git->CreateBranch(branchName);
            if (result.success) {
                featureBranchName = branchName;
                featureBranchCreated = true;
                currentStage = Stage::SWITCH_BRANCH;
                SyncGraphWithRepo();
            }
        }
    }
    else if (cmd.rfind("checkout", 0) == 0 && currentStage == Stage::SWITCH_BRANCH) {
        size_t spacePos = cmd.find(' ');
        if (spacePos != std::string::npos) {
            std::string branchName = cmd.substr(spacePos + 1);
            auto result = git->Checkout(branchName);
            if (result.success) {
                currentBranch = branchName;
                currentStage = Stage::MAKE_CHANGES;
                // 创建新文件
                {
                    std::ofstream file(repoPath + "/login.cpp");
                    file << "// 登录模块\nvoid login() {}\n";
                }
                SyncGraphWithRepo();
                splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
            }
        }
    }
    else if (cmd == "add" && currentStage == Stage::MAKE_CHANGES) {
        git->Add(".");
        currentStage = Stage::COMMIT_FEATURE;
    }
    else if (cmd.rfind("commit", 0) == 0 && currentStage == Stage::COMMIT_FEATURE) {
        auto result = git->Commit("Add login module");
        if (result.success) {
            changesCommitted = true;
            currentStage = Stage::MERGE_BRANCH;
            SyncGraphWithRepo();
        }
    }
}

void Level02_Branch::CheckGitStatus() {
    // 检查各阶段状态
}

void Level02_Branch::Update(float deltaTime) {
    timer += deltaTime;
    
    if (splitView) {
        splitView->Update(deltaTime);
    }
    
    // 键盘快捷键
    if (IsKeyPressed(KEY_SPACE) && currentStage == Stage::INTRO) {
        currentStage = Stage::CREATE_BRANCH;
    }
    
    // 简单的命令输入处理
    if (IsKeyPressed(KEY_ENTER)) {
        // 模拟命令输入（实际应该有输入框）
        switch (currentStage) {
            case Stage::CREATE_BRANCH:
                ProcessGitCommand("branch feature/login");
                break;
            case Stage::SWITCH_BRANCH:
                ProcessGitCommand("checkout feature/login");
                break;
            case Stage::MAKE_CHANGES:
                ProcessGitCommand("add");
                break;
            case Stage::COMMIT_FEATURE:
                ProcessGitCommand("commit");
                break;
            case Stage::MERGE_BRANCH:
                stageComplete = true;
                currentStage = Stage::COMPLETE;
                break;
            default:
                break;
        }
    }
}

void Level02_Branch::Draw() {
    // 背景
    ClearBackground({30, 35, 45, 255});
    
    // 绘制可视化
    if (splitView) {
        splitView->Draw();
    }
    
    // 绘制状态面板
    DrawStatusPanel();
    
    // 绘制对话框
    DrawDialogueIfNeeded();
}

void Level02_Branch::DrawStatusPanel() {
    // 左侧状态面板
    DrawRectangle(0, 0, 300, 720, {40, 44, 52, 255});
    DrawRectangleLines(0, 0, 300, 720, {100, 150, 200, 255});
    
    DrawChinese("Level 2: 分支探险", 20, 20, 28, WHITE);
    DrawChinese("学习 branch 和 checkout", 20, 55, 18, LIGHTGRAY);
    
    // 当前阶段
    DrawChinese("当前阶段:", 20, 100, 20, {100, 200, 255, 255});
    
    const char* stageText = "";
    switch (currentStage) {
        case Stage::INTRO: stageText = "按空格开始"; break;
        case Stage::CREATE_BRANCH: stageText = "创建分支: branch feature/login"; break;
        case Stage::SWITCH_BRANCH: stageText = "切换分支: checkout feature/login"; break;
        case Stage::MAKE_CHANGES: stageText = "添加文件后: add"; break;
        case Stage::COMMIT_FEATURE: stageText = "提交: commit"; break;
        case Stage::MERGE_BRANCH: stageText = "完成! 按 ENTER 结束"; break;
        case Stage::COMPLETE: stageText = "关卡完成!"; break;
    }
    DrawChinese(stageText, 20, 130, 18, YELLOW);
    
    // 当前分支
    DrawChinese(TextFormat("当前分支: %s", currentBranch.c_str()), 20, 200, 20, GREEN);
    
    // 提示
    DrawRectangle(10, 600, 280, 100, {50, 50, 60, 255});
    DrawChinese("提示:", 20, 610, 18, {100, 200, 255, 255});
    DrawChinese("分支让你可以并行开发", 20, 635, 16, LIGHTGRAY);
    DrawChinese("不影响主分支", 20, 655, 16, LIGHTGRAY);
}

void Level02_Branch::DrawDialogueIfNeeded() {
    if (currentStage == Stage::INTRO) {
        DrawRectangle(100, 500, 1080, 150, {40, 44, 52, 240});
        DrawRectangleLines(100, 500, 1080, 150, {100, 150, 200, 255});
        DrawChinese("CTO: 我们需要开发一个新功能，但不想影响主分支。", 120, 520, 24, WHITE);
        DrawChinese("使用 git branch 创建新分支，然后用 git checkout 切换过去。", 120, 550, 22, LIGHTGRAY);
        DrawChinese("按 [空格] 开始", 120, 600, 20, YELLOW);
    }
}

void Level02_Branch::DrawCommandInput() {
    // 底部命令输入区
    DrawRectangle(0, 650, 1280, 70, {35, 35, 45, 255});
    DrawRectangleLines(0, 650, 1280, 70, {100, 150, 200, 255});
    DrawChinese("命令输入区 - 按 ENTER 执行当前阶段命令", 20, 675, 20, WHITE);
}

void Level02_Branch::Shutdown() {
    splitView.reset();
    git.reset();
    
    // 清理临时目录
    try {
        fs::remove_all(repoPath);
    } catch (...) {}
}

bool Level02_Branch::IsComplete() const {
    return stageComplete;
}
