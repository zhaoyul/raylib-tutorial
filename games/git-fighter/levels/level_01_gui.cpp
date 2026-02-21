#include "level_01_gui.h"
#include <raylib.h>
#include <iostream>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;
using namespace GitGUI;

Level01_GUI::Level01_GUI()
    : Level(1, "周末加班", "使用真实 Git 建立项目")
    , currentStage(Stage::INTRO)
    , timer(0)
    , stageComplete(false)
    , readmeCreated(false)
    , mainCppCreated(false)
    , filesAdded(false) {
}

Level01_GUI::~Level01_GUI() = default;

void Level01_GUI::Initialize() {
    currentStage = Stage::INTRO;
    timer = 0;
    stageComplete = false;
    readmeCreated = false;
    mainCppCreated = false;
    filesAdded = false;
    lastCommitHash.clear();
    
    // Create temp directory
    repoPath = "/tmp/gitfighter_level1_" + std::to_string((int)GetTime());
    fs::create_directories(repoPath);
    
    // Init Git
    git = std::make_unique<GitWrapper>();
    
    // Init visualization
    splitView = std::make_unique<GitVis::SplitGitView>();
    splitView->Initialize(320, 100, 940, 520);
    splitView->SetSplitRatio(0.5f);
    splitView->SetGitWrapper(git.get());
    
    auto* commitPanel = splitView->GetCommitPanel();
    commitPanel->onNodeSelected = [this](const GitVis::CommitNode& node) {
        splitView->OnCommitSelected(node.hash);
    };
    
    // Init GUI
    gui = std::make_unique<GUIManager>();
    gui->Initialize();
    
    // Set Chinese font if available
    if (font && font->hasChineseFont) {
        gui->SetChineseFont(const_cast<Font*>(&font->chineseFont));
    }
    
    // Step indicator
    stepIndicator = std::make_unique<StepIndicator>(50, 80, 250);
    stepIndicator->SetSteps({"初始化", "暂存", "提交", "完成"});
    
    // Info card for selected object
    infoCard = std::make_unique<InfoCard>(Rectangle{20, 400, 260, 200}, "对象详情");
    
    std::cout << "Level 1 (GUI) initialized at: " << repoPath << std::endl;
}

void Level01_GUI::CreateSampleFiles() {
    // Create README.md
    {
        std::ofstream readme(repoPath + "/README.md");
        readme << "# 福报科技核心项目\n\n";
        readme << "这是一个改变命运的项目。\n\n";
        readme << "## 功能\n";
        readme << "- 高性能架构\n";
        readme << "- 分布式设计\n";
        readme << "- 996 福报模式\n";
    }
    readmeCreated = true;
    
    // Create main.cpp
    {
        std::ofstream mainCpp(repoPath + "/main.cpp");
        mainCpp << "#include <iostream>\n\n";
        mainCpp << "int main() {\n";
        mainCpp << "    std::cout << \"Hello, Git Fighter!\" << std::endl;\n";
        mainCpp << "    return 0;\n";
        mainCpp << "}\n";
    }
    mainCppCreated = true;
    
    std::cout << "Sample files created" << std::endl;
}

void Level01_GUI::SyncGraphWithRepo() {
    if (!git || !git->IsRepoOpen()) return;
    
    std::string head = git->GetHEAD();
    std::cout << "SyncGraphWithRepo: HEAD = " << head << std::endl;
    
    auto* commitPanel = splitView->GetCommitPanel();
    commitPanel->Clear();
    
    auto commits = git->GetCommitGraph(50);
    std::cout << "SyncGraphWithRepo: Found " << commits.size() << " commits" << std::endl;
    
    for (const auto& c : commits) {
        GitVis::CommitNode node;
        node.hash = c.hash;
        node.shortHash = c.hash.substr(0, 7);
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
    
    if (!head.empty()) {
        commitPanel->AddBranch("main", head, {100, 200, 255, 255});
        commitPanel->SetHEAD(head);
        splitView->OnCommitSelected(head);
    }
    
    commitPanel->RecalculateLayout();
    std::cout << "SyncGraphWithRepo: Layout recalculated" << std::endl;
}

void Level01_GUI::ProcessGitCommand(const std::string& cmd) {
    std::cout << "Processing command: " << cmd << std::endl;
    
    if (cmd == "init" && currentStage == Stage::WAIT_INIT) {
        auto result = git->Init(repoPath);
        if (result.success) {
            std::cout << "Git init successful at: " << repoPath << std::endl;
            git->OpenRepo(repoPath);
            lastCommitHash = git->GetHEAD();
            std::cout << "Initial HEAD: " << lastCommitHash << std::endl;
            currentStage = Stage::WAIT_ADD;
            CreateSampleFiles();
            SyncGraphWithRepo();
            splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
            
            // Create initial random files for better visualization
            git->CreateRandomFile();
            git->CreateRandomFile();
            git->CreateRandomDirectory();
            splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
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
            SyncGraphWithRepo();
            std::string head = git->GetHEAD();
            if (!head.empty() && head != lastCommitHash) {
                lastCommitHash = head;
                currentStage = Stage::COMPLETE;
                stageComplete = true;
            }
        }
    }
}

void Level01_GUI::CheckGitStatus() {
    if (!git || !git->IsRepoOpen()) return;
    
    auto status = git->GetWorkingDirectoryStatus();
    
    bool hasStaged = false;
    bool hasUntracked = false;
    
    for (const auto& file : status) {
        if (file.status == FileStatus::STAGED) {
            hasStaged = true;
        }
        if (file.status == FileStatus::UNTRACKED) {
            hasUntracked = true;
        }
    }
    
    if (currentStage == Stage::WAIT_ADD && hasStaged) {
        std::cout << "Files staged, moving to WAIT_COMMIT" << std::endl;
        currentStage = Stage::WAIT_COMMIT;
        filesAdded = true;
    }
}

void Level01_GUI::Update(float deltaTime) {
    timer += deltaTime;
    
    // Update visualization
    if (splitView) {
        splitView->Update(deltaTime);
    }
    
    // Update GUI
    if (gui) {
        gui->Update(deltaTime);
    }
    
    // Update step indicator
    switch (currentStage) {
        case Stage::INTRO: stepIndicator->currentStep = 0; break;
        case Stage::WAIT_INIT: stepIndicator->currentStep = 0; break;
        case Stage::WAIT_ADD: stepIndicator->currentStep = 1; break;
        case Stage::WAIT_COMMIT: stepIndicator->currentStep = 2; break;
        case Stage::COMPLETE: stepIndicator->currentStep = 3; break;
    }
    
    // Keyboard shortcuts
    if (IsKeyPressed(KEY_SPACE) && currentStage == Stage::INTRO) {
        currentStage = Stage::WAIT_INIT;
    }
    
    // GUI buttons
    if (currentStage == Stage::WAIT_INIT) {
        if (gui->Button("初始化仓库 (git init)", 50, 450, 250, 50)) {
            ProcessGitCommand("init");
        }
    }
    else if (currentStage == Stage::WAIT_ADD) {
        if (gui->Button("添加文件 (git add)", 50, 450, 250, 50)) {
            ProcessGitCommand("add");
        }
    }
    else if (currentStage == Stage::WAIT_COMMIT) {
        if (gui->Button("提交更改 (git commit)", 50, 450, 250, 50)) {
            ProcessGitCommand("commit");
        }
    }
    
    // Debug keys
    if (git && git->IsRepoOpen()) {
        if (IsKeyPressed(KEY_ONE)) {
            std::string filename = git->GenerateRandomFilename();
            if (git->CreateRandomFile()) {
                std::cout << "[KEY_ONE] Created: " << filename << std::endl;
                splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
            }
        }
        if (IsKeyPressed(KEY_TWO)) {
            std::string dirname = git->GenerateRandomDirname();
            if (git->CreateRandomDirectory()) {
                std::cout << "[KEY_TWO] Created: " << dirname << std::endl;
                splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
            }
        }
    }
    
    // Check status periodically
    static float checkTimer = 0;
    checkTimer += deltaTime;
    if (checkTimer > 0.5f) {
        checkTimer = 0;
        CheckGitStatus();
    }
}

void Level01_GUI::Draw() {
    ClearBackground(gui->GetTheme().background);
    
    // Left panel - Controls and info
    DrawLeftPanel();
    
    // Right panel - Visualization
    DrawRightPanel();
    
    // Bottom bar - Commands
    DrawBottomBar();
}

void Level01_GUI::DrawLeftPanel() {
    const Theme& theme = gui->GetTheme();
    
    // Panel background
    DrawRectangle(0, 0, 320, 720, theme.panelBg);
    DrawLine(320, 0, 320, 720, theme.panelBorder);
    
    // Title
    gui->Title("Level 1: 周末加班", 20, 20);
    gui->Subtitle("学习 git init/add/commit", 20, 55);
    
    // Step indicator
    stepIndicator->Draw(theme);
    
    // Current stage info
    int y = 180;
    Color panelBgLight = {static_cast<unsigned char>(theme.panelBg.r + 10), static_cast<unsigned char>(theme.panelBg.g + 10), static_cast<unsigned char>(theme.panelBg.b + 10), 255};
    DrawRectangle(20, y, 280, 100, panelBgLight);
    DrawRectangleLines(20, y, 280, 100, theme.panelBorder);
    
    const char* stageDesc = "";
    switch (currentStage) {
        case Stage::INTRO: stageDesc = "准备开始\n按空格键开始游戏"; break;
        case Stage::WAIT_INIT: stageDesc = "需要初始化 Git 仓库\n点击按钮执行 git init"; break;
        case Stage::WAIT_ADD: stageDesc = "文件已创建\n执行 git add 暂存文件"; break;
        case Stage::WAIT_COMMIT: stageDesc = "文件已暂存\n执行 git commit 提交"; break;
        case Stage::COMPLETE: stageDesc = "恭喜！\n关卡完成！"; break;
    }
    DrawChinese(stageDesc, 30, y + 15, 16, theme.textPrimary);
    
    // Action button area (buttons drawn in Update, just draw background here)
    Color panelBgDark = {static_cast<unsigned char>(theme.panelBg.r - 10), static_cast<unsigned char>(theme.panelBg.g - 10), static_cast<unsigned char>(theme.panelBg.b - 10), 255};
    DrawRectangle(20, 440, 280, 70, panelBgDark);
    
    // File status list
    y = 520;
    DrawChinese("工作区文件:", 20, y, 16, theme.textSecondary);
    y += 25;
    
    if (git && git->IsRepoOpen()) {
        auto files = git->GetWorkingDirectoryStatus();
        for (const auto& file : files) {
            const char* statusStr = "";
            Color statusColor;
            switch (file.status) {
                case FileStatus::UNTRACKED: statusStr = "??"; statusColor = theme.error; break;
                case FileStatus::MODIFIED: statusStr = "M"; statusColor = theme.warning; break;
                case FileStatus::STAGED: statusStr = "A"; statusColor = theme.success; break;
                default: statusStr = ""; statusColor = theme.textSecondary;
            }
            
            DrawText(file.path.c_str(), 30, y, 14, theme.textPrimary);
            DrawText(statusStr, 280, y, 14, statusColor);
            y += 20;
        }
    }
}

void Level01_GUI::DrawRightPanel() {
    // Draw split view visualization
    if (splitView) {
        splitView->Draw();
    }
}

void Level01_GUI::DrawBottomBar() {
    const Theme& theme = gui->GetTheme();
    
    // Bottom bar background
    DrawRectangle(0, 650, 1280, 70, theme.panelBg);
    DrawLine(0, 650, 1280, 650, theme.panelBorder);
    
    // Current command info
    int x = 340;
    if (currentStage == Stage::WAIT_INIT) {
        DrawChinese("当前命令: git init", x, 665, 20, theme.primary);
        DrawChinese("初始化一个新的 Git 仓库", x, 690, 14, theme.textSecondary);
    }
    else if (currentStage == Stage::WAIT_ADD) {
        DrawChinese("当前命令: git add .", x, 665, 20, theme.primary);
        DrawChinese("将所有修改添加到暂存区", x, 690, 14, theme.textSecondary);
    }
    else if (currentStage == Stage::WAIT_COMMIT) {
        DrawChinese("当前命令: git commit", x, 665, 20, theme.primary);
        DrawChinese("创建一个新的提交", x, 690, 14, theme.textSecondary);
    }
    else if (currentStage == Stage::COMPLETE) {
        DrawChinese("✓ 关卡完成！", x, 665, 20, theme.success);
    }
    
    // Keyboard hints
    DrawChinese("[1] 随机文件  [2] 随机目录  [ESC] 暂停", 900, 675, 14, theme.textSecondary);
}

void Level01_GUI::Shutdown() {
    splitView.reset();
    git.reset();
    gui.reset();
    stepIndicator.reset();
    infoCard.reset();
    
    try {
        fs::remove_all(repoPath);
    } catch (...) {}
}

bool Level01_GUI::IsComplete() const {
    return stageComplete;
}
