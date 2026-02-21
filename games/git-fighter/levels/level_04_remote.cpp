#include "level_04_remote.h"
#include <raylib.h>
#include <iostream>
#include <filesystem>
#include <fstream>

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
    
    repoPath = "/tmp/gitfighter_level4_" + std::to_string((int)GetTime());
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
    
    CreateLocalRepo();
    
    std::cout << "Level 4 initialized at: " << repoPath << std::endl;
}

void Level04_Remote::CreateLocalRepo() {
    git->Init(repoPath);
    git->OpenRepo(repoPath);
    
    {
        std::ofstream file(repoPath + "/README.md");
        file << "# 协作项目\n\n本地开发中\n";
    }
    git->Add(".");
    git->Commit("Initial local commit");
    
    {
        std::ofstream file(repoPath + "/feature.cpp");
        file << "// 新功能开发\nvoid feature() {}\n";
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
        commitPanel->AddCommit(node);
    }
    
    std::string head = git->GetHEAD();
    if (!head.empty()) {
        commitPanel->AddBranch("main", head, {100, 200, 255, 255});
        if (pushed) {
            commitPanel->AddBranch("origin/main", head, {255, 150, 100, 255});
        }
        commitPanel->SetHEAD(head);
    }
    
    commitPanel->RecalculateLayout();
}

void Level04_Remote::ProcessGitCommand(const std::string& cmd) {
    std::cout << "Processing command: " << cmd << std::endl;
    
    if (cmd.rfind("remote add", 0) == 0 && currentStage == Stage::ADD_REMOTE) {
        // 模拟添加远程仓库
        remoteAdded = true;
        currentStage = Stage::PUSH_MAIN;
        SyncGraphWithRepo();
    }
    else if (cmd.rfind("push", 0) == 0 && currentStage == Stage::PUSH_MAIN) {
        pushed = true;
        currentStage = Stage::FETCH_REMOTE;
        SyncGraphWithRepo();
    }
    else if (cmd == "fetch" && currentStage == Stage::FETCH_REMOTE) {
        fetched = true;
        currentStage = Stage::PULL_CHANGES;
    }
    else if (cmd == "pull" && currentStage == Stage::PULL_CHANGES) {
        pulled = true;
        currentStage = Stage::COMPLETE;
        stageComplete = true;
        // 模拟拉取了远程更新
        {
            std::ofstream file(repoPath + "/remote_update.txt");
            file << "# 远程更新\n来自 origin/main\n";
        }
        SyncGraphWithRepo();
    }
}

void Level04_Remote::Update(float deltaTime) {
    timer += deltaTime;
    
    if (splitView) {
        splitView->Update(deltaTime);
    }
    
    if (IsKeyPressed(KEY_SPACE) && currentStage == Stage::INTRO) {
        currentStage = Stage::ADD_REMOTE;
    }
    
    if (IsKeyPressed(KEY_ENTER)) {
        switch (currentStage) {
            case Stage::ADD_REMOTE:
                ProcessGitCommand("remote add origin https://github.com/company/project.git");
                break;
            case Stage::PUSH_MAIN:
                ProcessGitCommand("push origin main");
                break;
            case Stage::FETCH_REMOTE:
                ProcessGitCommand("fetch");
                break;
            case Stage::PULL_CHANGES:
                ProcessGitCommand("pull");
                break;
            case Stage::COMPLETE:
                stageComplete = true;
                break;
            default:
                break;
        }
    }
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
    
    DrawChinese("当前阶段:", 20, 100, 20, {100, 200, 255, 255});
    
    const char* stageText = "";
    switch (currentStage) {
        case Stage::INTRO: stageText = "按空格开始"; break;
        case Stage::ADD_REMOTE: stageText = "1. remote add origin"; break;
        case Stage::PUSH_MAIN: stageText = "2. push origin main"; break;
        case Stage::FETCH_REMOTE: stageText = "3. fetch"; break;
        case Stage::PULL_CHANGES: stageText = "4. pull"; break;
        case Stage::COMPLETE: stageText = "完成!"; break;
    }
    DrawText(stageText, 20, 130, 18, YELLOW);
    
    // 进度指示
    DrawChinese("进度:", 20, 200, 20, GREEN);
    DrawText(remoteAdded ? "[X] remote add" : "[ ] remote add", 20, 230, 16, remoteAdded ? GREEN : GRAY);
    DrawText(pushed ? "[X] push" : "[ ] push", 20, 255, 16, pushed ? GREEN : GRAY);
    DrawText(fetched ? "[X] fetch" : "[ ] fetch", 20, 280, 16, fetched ? GREEN : GRAY);
    DrawText(pulled ? "[X] pull" : "[ ] pull", 20, 305, 16, pulled ? GREEN : GRAY);
    
    DrawRectangle(10, 600, 280, 100, {50, 50, 60, 255});
    DrawChinese("提示:", 20, 610, 18, {100, 200, 255, 255});
    DrawChinese("remote 管理远程仓库", 20, 635, 16, LIGHTGRAY);
    DrawChinese("push 推送本地提交", 20, 655, 16, LIGHTGRAY);
    DrawChinese("pull 拉取远程更新", 20, 675, 16, LIGHTGRAY);
}

void Level04_Remote::DrawDialogueIfNeeded() {
    if (currentStage == Stage::INTRO) {
        DrawRectangle(100, 500, 1080, 150, {40, 44, 52, 240});
        DrawRectangleLines(100, 500, 1080, 150, {100, 150, 200, 255});
        DrawChinese("CTO: 项目需要团队协作，需要连接远程仓库。", 120, 520, 24, WHITE);
        DrawChinese("学会 push 分享你的代码，pull 获取他人更新。", 120, 550, 22, LIGHTGRAY);
        DrawChinese("按 [空格] 开始学习远程协作", 120, 600, 20, YELLOW);
    }
}

void Level04_Remote::Shutdown() {
    splitView.reset();
    git.reset();
    
    try {
        fs::remove_all(repoPath);
    } catch (...) {}
}

bool Level04_Remote::IsComplete() const {
    return stageComplete;
}
