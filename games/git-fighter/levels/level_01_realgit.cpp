#include "level_01_realgit.h"
#include "raylib.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <set>

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
    
    // 设置仓库路径用于工作目录扫描
    splitView->SetRepoPath(repoPath);
    
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
        node.timestamp = c.timestamp;  // Use actual timestamp from git
        node.parents = c.parents;
        node.radius = 20;
        node.alpha = 1;
        node.scale = 1;
        node.position = {200, 100};  // 初始位置
        node.targetPos = {200, 100};
        node.position = {200, 100};
        
        for (const auto& branch : c.branches) {
            node.branches.push_back(branch);
        }
        
        std::cout << "  Adding commit: " << node.shortHash << " - " << node.message << std::endl;
        commitPanel->AddCommit(node);
    }
    
    // Add all branches to visualization
    std::set<std::string> addedBranches;
    Color branchColors[] = {
        {100, 200, 255, 255},   // Blue - main
        {255, 200, 100, 255},   // Orange
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
                // New branch - add it
                Color color = branchColors[colorIndex % 7];
                commitPanel->AddBranch(branchName, c.hash, color);
                colorIndex++;
            }
        }
    }
    
    // If no branches found, add main as fallback
    if (addedBranches.empty() && !head.empty()) {
        commitPanel->AddBranch("main", head, branchColors[0]);
    }
    
    if (!head.empty()) {
        commitPanel->SetHEAD(head);
        splitView->OnCommitSelected(head);
    }
    
    // Set current branch name
    std::string currentBranch = git->GetCurrentBranch();
    if (!currentBranch.empty()) {
        commitPanel->SetCurrentBranch(currentBranch);
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

std::string Level01_RealGit::ProcessLevelCommand(const std::string& cmd) {
    std::cout << "Processing level command: " << cmd << std::endl;
    
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
            return "Initialized Git repository";
        }
        return "Failed to init: " + result.error;
    }
    else if (cmd == "add" && currentStage == Stage::WAIT_ADD) {
        auto result = git->Add(".");
        if (result.success) {
            std::cout << "Git add successful" << std::endl;
            CheckGitStatus();
            // 推进到下一个阶段
            currentStage = Stage::WAIT_COMMIT;
            return "Added files to staging area";
        }
        return "Failed to add: " + result.error;
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
            return "Committed: 项目初始化";
        }
        return "Failed to commit: " + result.error;
    }
    
    // 调用基类处理通用命令
    return "";
}

void Level01_RealGit::Update(float deltaTime) {
    timer += deltaTime;
    
    // 更新可视化区域大小（自适应窗口）
    if (splitView) {
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        // Right panel: x=320, y=100, width=screenWidth-320, height=screenHeight-100-70 (leave space for command bar)
        splitView->SetBounds(320, 100, screenWidth - 320, screenHeight - 170);
        splitView->Update(deltaTime);
    }
    
    // 键盘快捷键 - 游戏流程（通过 ExecuteGitCommand 以记录到历史日志）
    if (IsKeyPressed(KEY_I) && currentStage == Stage::WAIT_INIT) {
        ExecuteGitCommand("init");
    }
    if (IsKeyPressed(KEY_A) && currentStage == Stage::WAIT_ADD) {
        ExecuteGitCommand("add");
    }
    if (IsKeyPressed(KEY_C) && currentStage == Stage::WAIT_COMMIT) {
        ExecuteGitCommand("commit");
    }
    if (IsKeyPressed(KEY_SPACE) && currentStage == Stage::INTRO) {
        currentStage = Stage::WAIT_INIT;
    }
    
    // Dev tools (1/2/3 keys) - Level 1 special handling
    // Allow file creation even before git init
    bool fileChanged = false;
    if (!repoPath.empty()) {
        if (IsKeyPressed(KEY_ONE)) {
            std::string filename = git->GenerateRandomFilename();
            std::cout << "[KEY_ONE] Creating random file: " << filename << std::endl;
            if (git->CreateRandomFile()) {
                std::cout << "[KEY_ONE] Created: " << filename << std::endl;
                fileChanged = true;
            }
        }
        if (IsKeyPressed(KEY_TWO)) {
            std::string dirname = git->GenerateRandomDirname();
            std::cout << "[KEY_TWO] Creating random directory: " << dirname << std::endl;
            if (git->CreateRandomDirectory()) {
                std::cout << "[KEY_TWO] Created: " << dirname << std::endl;
                fileChanged = true;
            }
        }
        if (IsKeyPressed(KEY_THREE)) {
            auto files = git->GetWorkingDirectoryStatus();
            if (!files.empty()) {
                std::string targetFile = files[rand() % files.size()].path;
                std::cout << "[KEY_THREE] Appending to: " << targetFile << std::endl;
                if (git->AppendRandomContent(targetFile)) {
                    fileChanged = true;
                }
            }
        }
    }
    
    // Refresh view if files changed
    if (fileChanged && splitView) {
        splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
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
    ClearBackground({30, 35, 45, 255});
    
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
    int screenHeight = GetScreenHeight();
    
    // 左侧面板背景 - 统一深色风格
    DrawRectangle(0, 0, 320, screenHeight, {40, 44, 52, 255});
    DrawRectangleLines(0, 0, 320, screenHeight, {100, 150, 200, 255});
    
    int y = 20;
    
    // 关卡信息（统一格式）
    DrawChinese("Level 1: Git 入门", 20, y, 28, WHITE);
    DrawChinese("学习 init/add/commit", 20, y + 35, 18, LIGHTGRAY);
    
    // 关卡进度（1/10）
    DrawText("1 / 10", 250, y + 10, 16, {100, 200, 255, 255});
    
    y += 80;
    
    // 标题
    DrawChinese("任务目标", 20, y, 24, {100, 200, 255, 255});
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
    DrawLine(20, y, 300, y, {100, 100, 120, 255});
    y += 20;
    
    // 文件状态 - 动态扫描目录
    DrawChinese("工作区文件", 20, y, 24, {100, 200, 255, 255});
    y += 40;
    
    // Scan directory dynamically
    if (!repoPath.empty() && fs::exists(repoPath)) {
        try {
            int fileCount = 0;
            for (const auto& entry : fs::directory_iterator(repoPath)) {
                std::string name = entry.path().filename().string();
                if (name[0] == '.' || name == ".git") continue;
                
                std::string displayName = name;
                if (entry.is_directory()) displayName += "/";
                
                // Check git status for this file
                Color fileColor = DARKGRAY;
                if (git && git->IsRepoOpen()) {
                    auto status = git->GetWorkingDirectoryStatus();
                    for (const auto& s : status) {
                        if (s.path == name) {
                            if (s.status == FileStatus::UNTRACKED) fileColor = RED;
                            else if (s.status == FileStatus::MODIFIED) fileColor = ORANGE;
                            else if (s.status == FileStatus::STAGED) fileColor = GREEN;
                            break;
                        }
                    }
                }
                
                // 统一颜色：未跟踪=红，已修改=黄，已暂存=绿，其他=浅灰
                Color displayColor = LIGHTGRAY;
                if (fileColor.r == RED.r && fileColor.g == RED.g && fileColor.b == RED.b) displayColor = RED;
                else if (fileColor.g == GREEN.g) displayColor = GREEN;
                else if (fileColor.r == ORANGE.r) displayColor = YELLOW;
                
                DrawChinese(displayName.c_str(), 40, y, 18, displayColor);
                y += 28;
                fileCount++;
                
                // Limit to 8 files to avoid overflow
                if (fileCount >= 8) {
                    DrawChinese("...", 40, y, 18, GRAY);
                    y += 28;
                    break;
                }
            }
            if (fileCount == 0) {
                DrawChinese("(空目录)", 40, y, 16, {150, 150, 150, 255});
                y += 28;
            }
        } catch (...) {
            DrawChinese("(无法读取)", 40, y, 16, RED);
            y += 28;
        }
    } else {
        DrawChinese("(未初始化)", 40, y, 16, GRAY);
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
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int barY = screenHeight - 70;
    
    // 底部命令栏 - at bottom of window
    DrawRectangle(0, barY, screenWidth, 70, (Color){40, 44, 52, 255});
    DrawRectangleLines(0, barY, screenWidth, 70, (Color){100, 150, 200, 255});
    
    int x = 20;
    
    // 根据阶段显示可用命令
    int textY = barY + 20;
    if (currentStage == Stage::WAIT_INIT) {
        DrawChinese("按 [I] 执行: git init", x, textY, 24, {100, 200, 100, 255});
    }
    else if (currentStage == Stage::WAIT_ADD) {
        DrawChinese("按 [A] 执行: git add .", x, textY, 24, {100, 200, 100, 255});
    }
    else if (currentStage == Stage::WAIT_COMMIT) {
        DrawChinese("按 [C] 执行: git commit -m \"Initial commit\"", x, textY, 24, {100, 200, 100, 255});
    }
    else if (currentStage == Stage::COMPLETE) {
        DrawChinese("[完成] 关卡完成! 按 [ENTER] 继续", x, textY, 24, GREEN);
    }
    else {
        DrawChinese("按 [空格] 开始", x, textY, 24, WHITE);
    }
    
    // 提示 - right aligned
    DrawChinese("提示: 拖拽视图 | 滚轮缩放 | 点击节点查看结构", screenWidth - 580, textY + 5, 18, GRAY);
    
    // 随机生成快捷键说明（仅在 init 后显示）
    if (git && git->IsRepoOpen()) {
        DrawChinese("[1]随机文件 [2]随机目录 [3]追加内容", 20, textY + 25, 16, {150, 150, 200, 255});
    }
}

void Level01_RealGit::DrawDialogueIfNeeded() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int panelHeight = 130;
    int panelY = screenHeight - panelHeight - 70; // Above command bar
    
    // 底部对话面板
    DrawRectangle(0, panelY, screenWidth, panelHeight, (Color){30, 30, 40, 240});
    DrawRectangleLines(0, panelY, screenWidth, panelHeight, (Color){100, 150, 200, 255});
    
    int centerY = panelY + panelHeight / 2;
    
    // CTO 头像
    DrawCircle(70, centerY, 35, (Color){100, 150, 200, 255});
    DrawChinese("CTO", 52, centerY - 7, 20, WHITE);
    
    // 对话文本
    DrawChinese(dialogueCTO.c_str(), 130, panelY + 25, 26, WHITE);
    
    // 继续提示 - right aligned
    if (timer > 1.0f) {
        DrawChinese("按 [空格] 继续...", screenWidth - 200, panelY + 105, 20, LIGHTGRAY);
    }
}

void Level01_RealGit::RefreshWorkingDirectory() {
    if (splitView && !repoPath.empty()) {
        splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
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

std::string Level01_RealGit::ExecuteGitCommand(const std::string& cmd) {
    // First record the command
    RecordGitCommand(cmd);
    
    // Try level-specific command handling first (for game progression)
    std::string levelResult = ProcessLevelCommand(cmd);
    if (!levelResult.empty()) {
        return levelResult;
    }
    
    if (!git || !git->IsRepoOpen()) {
        return "错误: Git 仓库未初始化";
    }
    
    // Execute command in the repo directory using system()
    // Add "git " prefix for simple commands (add, commit, status, etc.)
    std::string fullCmd;
    if (cmd.find(" ") == std::string::npos || 
        cmd.find("init") == 0 || cmd.find("add") == 0 || cmd.find("commit") == 0 ||
        cmd.find("status") == 0 || cmd.find("branch") == 0 || cmd.find("checkout") == 0 ||
        cmd.find("merge") == 0 || cmd.find("log") == 0 || cmd.find("reset") == 0) {
        // Simple git command - add git prefix
        fullCmd = "cd " + repoPath + " && git " + cmd + " 2>&1";
    } else {
        // Complex command - execute as is
        fullCmd = "cd " + repoPath + " && " + cmd + " 2>&1";
    }
    
    std::string result;
    char buffer[4096];
    
    FILE* pipe = popen(fullCmd.c_str(), "r");
    if (!pipe) {
        return "错误: 无法执行命令";
    }
    
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    
    int exitCode = pclose(pipe);
    
    // Trim trailing newline
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    
    // Update visualization after git commands
    SyncGraphWithRepo();
    splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
    
    // Check for commit
    if ((cmd.find("commit") == 0) && exitCode == 0) {
        std::string head = git->GetHEAD();
        if (!head.empty() && head != lastCommitHash) {
            lastCommitHash = head;
            if (currentStage == Stage::WAIT_COMMIT) {
                currentStage = Stage::COMPLETE;
                stageComplete = true;
            }
        }
    }
    
    if (result.empty()) {
        result = (exitCode == 0) ? "命令执行成功" : "命令执行失败 (exit code: " + std::to_string(exitCode) + ")";
    }
    
    return result;
}
