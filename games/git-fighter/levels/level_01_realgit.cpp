#include "level_01_realgit.h"
#include "raylib.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstdlib>

namespace fs = std::filesystem;

Level01_RealGit::Level01_RealGit()
    : Level(1, "周末加班", "使用真实 Git 建立项目"),
      currentStage(Stage::INTRO),
      timer(0),
      stageComplete(false),
      readmeCreated(false),
      mainCppCreated(false),
      filesAdded(false) {
    dialogueCTO = "小王，项目还没初始化呢。使用 git init 建立仓库，然后添加文件并提交。";
}

Level01_RealGit::~Level01_RealGit() = default;

void Level01_RealGit::Initialize() {
    currentStage = Stage::INTRO;
    timer = 0;
    stageComplete = false;
    readmeCreated = false;
    mainCppCreated = false;
    filesAdded = false;
    lastCommitHash.clear();
    
    // 创建临时目录
    repoPath = "/tmp/gitfighter_level1_" + std::to_string((int)GetTime());
    fs::create_directories(repoPath);
    
    // 初始化 Git 包装器
    git = std::make_unique<GitWrapper>();
    
    // 初始化分屏视图
    splitView = std::make_unique<GitVis::SplitGitView>();
    splitView->Initialize(320, 100, 940, 520);
    splitView->SetSplitRatio(0.5f);  // 50/50 分割
    splitView->SetGitWrapper(git.get());  // 设置 git wrapper 用于显示内部结构
    
    // 设置回调
    auto* commitPanel = splitView->GetCommitPanel();
    commitPanel->onNodeSelected = [this](const GitVis::CommitNode& node) {
        splitView->OnCommitSelected(node.hash);
    };
    
    std::cout << "Level 1 initialized at: " << repoPath << std::endl;
    
    // Auto-start for testing (skip intro and execute commands)
    // currentStage = Stage::WAIT_INIT;
    // ProcessGitCommand("init");
}

void Level01_RealGit::CreateSampleFiles() {
    // 创建 README.md
    std::ofstream readme(repoPath + "/README.md");
    readme << "# 福报科技核心项目\n\n";
    readme << "这是一个改变命运的项目。\n\n";
    readme << "## 功能\n";
    readme << "- 高性能架构\n";
    readme << "- 分布式设计\n";
    readme << "- 996 福报模式\n";
    readme.close();
    readmeCreated = true;
    
    // 创建 main.cpp
    std::ofstream mainCpp(repoPath + "/main.cpp");
    mainCpp << "#include <iostream>\n\n";
    mainCpp << "int main() {\n";
    mainCpp << "    std::cout << \"Hello, 福报科技!\" << std::endl;\n";
    mainCpp << "    return 0;\n";
    mainCpp << "}\n";
    mainCpp.close();
    mainCppCreated = true;
    
    std::cout << "Sample files created" << std::endl;
}

void Level01_RealGit::SyncGraphWithRepo() {
    if (!git || !git->IsRepoOpen()) {
        std::cout << "SyncGraphWithRepo: Git not initialized" << std::endl;
        return;
    }
    
    std::string head = git->GetHEAD();
    std::cout << "SyncGraphWithRepo: HEAD = " << head << std::endl;
    
    auto* commitPanel = splitView->GetCommitPanel();
    commitPanel->Clear();
    
    // 从真实仓库读取提交历史
    auto commits = git->GetCommitGraph(50);
    std::cout << "SyncGraphWithRepo: Found " << commits.size() << " commits" << std::endl;
    
    for (const auto& c : commits) {
        GitVis::CommitNode node;
        node.hash = c.hash;
        node.shortHash = c.hash.substr(0, 7);
        node.message = c.message;
        node.author = c.author;
        node.timestamp = 0;
        node.parents = c.parents;
        node.radius = 20;
        node.alpha = 1;
        node.scale = 1;
        node.position = {200, 100};  // 初始位置
        node.targetPos = {200, 100};
        node.springX.position = node.springX.target = 200;
        node.springY.position = node.springY.target = 100;
        
        for (const auto& branch : c.branches) {
            node.branches.push_back(branch);
        }
        
        std::cout << "  Adding commit: " << node.shortHash << " - " << node.message << std::endl;
        commitPanel->AddCommit(node);
    }
    
    if (!head.empty()) {
        commitPanel->AddBranch("main", head, {100, 200, 255, 255});
        commitPanel->SetHEAD(head);
        splitView->OnCommitSelected(head);
    }
    
    commitPanel->RecalculateLayout();
    std::cout << "SyncGraphWithRepo: Layout recalculated" << std::endl;
}

void Level01_RealGit::CheckGitStatus() {
    if (!git || !git->IsRepoOpen()) return;
    
    auto status = git->GetWorkingDirectoryStatus();
    
    // 检查是否有暂存文件
    bool hasStaged = false;
    for (const auto& file : status) {
        if (file.status == FileStatus::STAGED) {
            hasStaged = true;
            break;
        }
    }
    
    // 状态转换逻辑
    if (currentStage == Stage::WAIT_ADD && hasStaged) {
        std::cout << "Files staged, moving to WAIT_COMMIT" << std::endl;
        currentStage = Stage::WAIT_COMMIT;
        filesAdded = true;
    }
}

void Level01_RealGit::ProcessGitCommand(const std::string& cmd) {
    std::cout << "Processing command: " << cmd << std::endl;
    
    if (cmd == "init" && currentStage == Stage::WAIT_INIT) {
        auto result = git->Init(repoPath);
        if (result.success) {
            std::cout << "Git init successful at: " << repoPath << std::endl;
            
            // Open the repo to start using it
            git->OpenRepo(repoPath);
            
            lastCommitHash = git->GetHEAD();
            std::cout << "Initial HEAD: " << lastCommitHash << std::endl;
            
            // Move to next stage and create files
            currentStage = Stage::WAIT_ADD;
            CreateSampleFiles();
            
            // Force a sync (will show empty repo)
            SyncGraphWithRepo();
            
            // Show working directory in structure panel
            if (splitView) {
                // Create some initial random files for testing
                std::cout << "[INIT] Creating initial random files..." << std::endl;
                git->CreateRandomFile();
                git->CreateRandomFile();
                git->CreateRandomDirectory();
                std::cout << "[INIT] Scanning working directory: " << repoPath << std::endl;
                splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
                std::cout << "[INIT] Initial scan complete." << std::endl;
            }
        }
    }
    else if (cmd == "add" && currentStage == Stage::WAIT_ADD) {
        auto result = git->Add(".");
        if (result.success) {
            std::cout << "Git add successful" << std::endl;
            CheckGitStatus();
        }
    }
    else if (cmd == "commit" && currentStage == Stage::WAIT_COMMIT) {
        auto result = git->Commit("Initial commit: 项目初始化");
        if (result.success) {
            std::cout << "Git commit successful" << std::endl;
            // 立即同步
            SyncGraphWithRepo();
            // Check if we should complete
            std::string head = git->GetHEAD();
            if (!head.empty() && head != lastCommitHash) {
                lastCommitHash = head;
                currentStage = Stage::COMPLETE;
                stageComplete = true;
            }
        }
    }
}

void Level01_RealGit::Update(float deltaTime) {
    timer += deltaTime;
    
    // 更新可视化
    if (splitView) {
        splitView->Update(deltaTime);
    }
    
    // 键盘快捷键 - 游戏流程
    if (IsKeyPressed(KEY_I) && currentStage == Stage::WAIT_INIT) {
        ProcessGitCommand("init");
    }
    if (IsKeyPressed(KEY_A) && currentStage == Stage::WAIT_ADD) {
        ProcessGitCommand("add");
    }
    if (IsKeyPressed(KEY_C) && currentStage == Stage::WAIT_COMMIT) {
        ProcessGitCommand("commit");
    }
    if (IsKeyPressed(KEY_SPACE) && currentStage == Stage::INTRO) {
        currentStage = Stage::WAIT_INIT;
    }
    
    // 随机生成内容按键 - 用于测试内部结构可视化
    // 使用数字键 1/2/3 避免 F 键被系统占用
    if (git && git->IsRepoOpen()) {
        if (IsKeyPressed(KEY_ONE)) {
            // 1: 创建随机文件
            std::string filename = git->GenerateRandomFilename();
            std::cout << "[KEY_ONE] Attempting to create random file..." << std::endl;
            if (git->CreateRandomFile()) {
                std::cout << "[KEY_ONE] Created random file: " << filename << std::endl;
                std::cout << "[KEY_ONE] Scanning working directory: " << repoPath << std::endl;
                splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
                std::cout << "[KEY_ONE] Scan complete." << std::endl;
            } else {
                std::cout << "[KEY_ONE] Failed to create random file!" << std::endl;
            }
        }
        if (IsKeyPressed(KEY_TWO)) {
            // 2: 创建随机目录
            std::string dirname = git->GenerateRandomDirname();
            std::cout << "[KEY_TWO] Attempting to create random directory..." << std::endl;
            if (git->CreateRandomDirectory()) {
                std::cout << "[KEY_TWO] Created random directory: " << dirname << std::endl;
                splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
            } else {
                std::cout << "[KEY_TWO] Failed to create random directory!" << std::endl;
            }
        }
        if (IsKeyPressed(KEY_THREE)) {
            // 3: 追加随机内容到现有文件
            auto files = git->GetWorkingDirectoryStatus();
            if (!files.empty()) {
                std::string targetFile = files[rand() % files.size()].path;
                std::cout << "[KEY_THREE] Appending to: " << targetFile << std::endl;
                if (git->AppendRandomContent(targetFile)) {
                    std::cout << "[KEY_THREE] Appended random content." << std::endl;
                    splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
                }
            }
        }
    }
    
    // 定期检查 Git 状态
    static float checkTimer = 0;
    checkTimer += deltaTime;
    if (checkTimer > 0.5f) {
        checkTimer = 0;
        CheckGitStatus();
    }
}

void Level01_RealGit::Draw() {
    ClearBackground((Color){245, 247, 250, 255});
    
    // 左侧状态面板
    DrawStatusPanel();
    
    // 右侧分屏可视化
    if (splitView) {
        splitView->Draw();
    }
    
    // 对话（仅在 intro 阶段）
    if (currentStage == Stage::INTRO) {
        DrawDialogueIfNeeded();
    }
    
    // 底部命令提示
    DrawCommandInput();
}

void Level01_RealGit::DrawStatusPanel() {
    // 左侧面板背景
    DrawRectangle(0, 0, 320, 720, WHITE);
    DrawRectangleLines(0, 0, 320, 720, (Color){200, 200, 200, 255});
    
    int y = 20;
    
    // 标题
    DrawChinese("任务目标", 20, y, 26, DARKGRAY);
    y += 45;
    
    // 阶段指示
    const char* stages[] = {"初始化仓库", "添加文件", "提交更改"};
    int currentIdx = 0;
    switch (currentStage) {
        case Stage::WAIT_INIT: currentIdx = 0; break;
        case Stage::WAIT_ADD: currentIdx = 1; break;
        case Stage::WAIT_COMMIT: currentIdx = 2; break;
        default: currentIdx = -1;
    }
    
    for (int i = 0; i < 3; i++) {
        Color c = (i <= currentIdx) ? GREEN : LIGHTGRAY;
        if (i == currentIdx) c = BLUE;
        
        DrawCircle(30, y + 10, 8, c);
        DrawChinese(stages[i], 50, y, 20, c);
        y += 35;
    }
    
    y += 30;
    DrawLine(20, y, 300, y, (Color){200, 200, 200, 255});
    y += 20;
    
    // 文件状态
    DrawChinese("工作区文件", 20, y, 24, DARKGRAY);
    y += 40;
    
    if (readmeCreated) {
        DrawChinese("README.md", 40, y, 18, DARKGRAY);
        y += 28;
    }
    if (mainCppCreated) {
        DrawChinese("main.cpp", 40, y, 18, DARKGRAY);
        y += 28;
    }
    
    // Git 状态
    y += 20;
    DrawLine(20, y, 300, y, (Color){200, 200, 200, 255});
    y += 20;
    
    DrawChinese("Git 状态", 20, y, 24, DARKGRAY);
    y += 40;
    
    if (git && git->IsRepoOpen()) {
        auto status = git->GetWorkingDirectoryStatus();
        if (status.empty()) {
            DrawChinese("工作区干净", 40, y, 18, GREEN);
        } else {
            for (const auto& file : status) {
                const char* statusText = "";
                Color statusColor = DARKGRAY;
                switch (file.status) {
                    case FileStatus::UNTRACKED: statusText = "??"; statusColor = RED; break;
                    case FileStatus::MODIFIED: statusText = "M"; statusColor = ORANGE; break;
                    case FileStatus::STAGED: statusText = "A"; statusColor = GREEN; break;
                }
                DrawText(TextFormat("%s %s", statusText, file.path.c_str()), 
                        40, y, 16, statusColor);
                y += 24;
            }
        }
    } else {
        DrawChinese("未初始化", 40, y, 18, GRAY);
    }
    
    // 仓库路径
    y += 30;
    DrawLine(20, y, 300, y, (Color){200, 200, 200, 255});
    y += 20;
    
    DrawChinese("仓库路径", 20, y, 20, DARKGRAY);
    y += 28;
    DrawText(repoPath.c_str(), 20, y, 12, GRAY);
}

void Level01_RealGit::DrawCommandInput() {
    // 底部命令栏
    DrawRectangle(0, 650, 1280, 70, (Color){40, 44, 52, 255});
    DrawRectangleLines(0, 650, 1280, 70, (Color){100, 150, 200, 255});
    
    int x = 20;
    
    // 根据阶段显示可用命令
    if (currentStage == Stage::WAIT_INIT) {
        DrawChinese("按 [I] 执行: git init", x, 670, 24, {100, 200, 100, 255});
    }
    else if (currentStage == Stage::WAIT_ADD) {
        DrawChinese("按 [A] 执行: git add .", x, 670, 24, {100, 200, 100, 255});
    }
    else if (currentStage == Stage::WAIT_COMMIT) {
        DrawChinese("按 [C] 执行: git commit -m \"Initial commit\"", x, 670, 24, {100, 200, 100, 255});
    }
    else if (currentStage == Stage::COMPLETE) {
        DrawChinese("✓ 关卡完成! 按 [ENTER] 继续", x, 670, 24, GREEN);
    }
    else {
        DrawChinese("按 [空格] 开始", x, 670, 24, WHITE);
    }
    
    // 提示
    DrawChinese("提示: 拖拽视图 | 滚轮缩放 | 点击节点查看结构", 700, 675, 18, GRAY);
    
    // 随机生成快捷键说明（仅在 init 后显示）
    if (git && git->IsRepoOpen()) {
        DrawChinese("[1]随机文件 [2]随机目录 [3]追加内容", 20, 695, 16, {150, 150, 200, 255});
    }
}

void Level01_RealGit::DrawDialogueIfNeeded() {
    // 底部对话面板
    DrawRectangle(0, 520, 1280, 130, (Color){30, 30, 40, 240});
    DrawRectangleLines(0, 520, 1280, 130, (Color){100, 150, 200, 255});
    
    // CTO 头像
    DrawCircle(70, 585, 35, (Color){100, 150, 200, 255});
    DrawChinese("CTO", 52, 578, 20, WHITE);
    
    // 对话文本
    DrawChinese(dialogueCTO.c_str(), 130, 545, 26, WHITE);
    
    // 继续提示
    if (timer > 1.0f) {
        DrawChinese("按 [空格] 继续...", 1080, 625, 20, LIGHTGRAY);
    }
}

void Level01_RealGit::Shutdown() {
    // 清理临时目录
    if (!repoPath.empty() && fs::exists(repoPath)) {
        fs::remove_all(repoPath);
        std::cout << "Cleaned up: " << repoPath << std::endl;
    }
}

bool Level01_RealGit::IsComplete() const {
    return currentStage == Stage::COMPLETE;
}
