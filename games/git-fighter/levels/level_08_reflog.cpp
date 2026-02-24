#include "level_08_reflog.h"
#include <raylib.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <set>

namespace fs = std::filesystem;

Level08_Reflog::Level08_Reflog()
    : Level(8, "时光回溯", "用 reflog 拯救丢失的代码")
    , currentStage(Stage::INTRO)
    , timer(0)
    , stageComplete(false)
    , selectedReflogIndex(-1)
    , accidentTriggered(false)
    , reflogVisible(false)
    , codeRecovered(false)
    , panicPulse(0)
    , currentReflogPage(0) {
    lostCommits[0] = lostCommits[1] = lostCommits[2] = "";
}

Level08_Reflog::~Level08_Reflog() = default;

void Level08_Reflog::Initialize() {
    currentStage = Stage::INTRO;
    timer = 0;
    stageComplete = false;
    selectedReflogIndex = -1;
    accidentTriggered = false;
    reflogVisible = false;
    codeRecovered = false;
    panicPulse = 0;
    currentReflogPage = 0;
    recoveryMethod = "";
    
    repoPath = "/tmp/gitfighter_level8_" + std::to_string((int)GetTime());
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
    
    // 创建正常开发历史的仓库
    CreateNormalRepo();
    
    std::cout << "Level 8 initialized at: " << repoPath << std::endl;
}

void Level08_Reflog::CreateNormalRepo() {
    git->Init(repoPath);
    git->OpenRepo(repoPath);
    
    // Commit 1: 初始项目结构
    {
        std::ofstream file(repoPath + "/main.cpp");
        file << "#include <iostream>\n"
             << "\n"
             << "int main() {\n"
             << "    std::cout << \"Hello World\" << std::endl;\n"
             << "    return 0;\n"
             << "}\n";
    }
    git->Add(".");
    git->Commit("Initial project setup");
    
    // Commit 2: 添加核心功能
    {
        std::ofstream file(repoPath + "/core.cpp");
        file << "// 核心算法实现\n"
             << "int calculate(int a, int b) {\n"
             << "    return a + b;\n"
             << "}\n";
    }
    git->Add(".");
    git->Commit("Add core calculation module");
    
    // Commit 3: 添加数据库支持（重要功能！）
    {
        std::ofstream file(repoPath + "/database.cpp");
        file << "// 数据库连接模块\n"
             << "class Database {\n"
             << "public:\n"
             << "    bool connect() { return true; }\n"
             << "    void query(const char* sql) {}\n"
             << "};\n";
    }
    {
        std::ofstream file(repoPath + "/config.h");
        file << "// 配置文件\n"
             << "#define DB_HOST \"localhost\"\n"
             << "#define DB_PORT 5432\n";
    }
    git->Add(".");
    git->Commit("Add database support - CRITICAL FEATURE");
    
    // Commit 4: 添加用户认证
    {
        std::ofstream file(repoPath + "/auth.cpp");
        file << "// 用户认证模块\n"
             << "class Auth {\n"
             << "public:\n"
             << "    bool login(const char* user, const char* pass) {\n"
             << "        return true; // TODO: implement\n"
             << "    }\n"
             << "};\n";
    }
    git->Add(".");
    git->Commit("Add user authentication");
    
    // Commit 5: 最新功能 - API 接口
    {
        std::ofstream file(repoPath + "/api.cpp");
        file << "// REST API 接口\n"
             << "void handleRequest(const char* path) {\n"
             << "    // API implementation\n"
             << "}\n";
    }
    git->Add(".");
    git->Commit("Add REST API endpoints");
    
    // 记录这3个重要的 commit（会被丢失的）
    auto commits = git->GetCommitGraph(10);
    if (commits.size() >= 4) {
        lostCommits[0] = commits[0].hash;  // API
        lostCommits[1] = commits[1].hash;  // Auth
        lostCommits[2] = commits[2].hash;  // Database
    }
    
    SyncGraphWithRepo();
    splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
}

void Level08_Reflog::TriggerAccident() {
    // 执行危险的 reset --hard HEAD~3
    std::cout << "ACCIDENT: Executing reset --hard HEAD~3" << std::endl;
    
    auto result = git->ResetHard("HEAD~3");
    if (result.success) {
        accidentTriggered = true;
        // 文件会被清空/恢复，需要检查工作目录
        splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
        SyncGraphWithRepo();
    }
}

void Level08_Reflog::LoadReflog() {
    reflogEntries = git->GetReflog("HEAD");
    std::cout << "Loaded " << reflogEntries.size() << " reflog entries" << std::endl;
}

void Level08_Reflog::SyncGraphWithRepo() {
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
        {100, 200, 255, 255},
        {255, 200, 100, 255},
        {100, 255, 150, 255},
        {255, 100, 200, 255},
    };
    int colorIndex = 0;
    
    for (const auto& c : commits) {
        for (const auto& branchName : c.branches) {
            if (addedBranches.insert(branchName).second) {
                Color color = branchColors[colorIndex % 4];
                commitPanel->AddBranch(branchName, c.hash, color);
                colorIndex++;
            }
        }
    }
    
    std::string head = git->GetHEAD();
    if (!head.empty()) {
        commitPanel->SetHEAD(head);
    }
    
    std::string currentBranch = git->GetCurrentBranch();
    if (!currentBranch.empty()) {
        commitPanel->SetCurrentBranch(currentBranch);
    }
    
    commitPanel->RecalculateLayout();
}

std::string Level08_Reflog::ProcessLevelCommand(const std::string& cmd) {
    std::cout << "Processing command: " << cmd << std::endl;
    RecordGitCommand(cmd);
    
    if (cmd == "reset --hard HEAD~3" && currentStage == Stage::SHOW_HISTORY) {
        TriggerAccident();
        currentStage = Stage::PANIC_MODE;
        LoadReflog();
        return "Executed reset --hard HEAD~3";
    }
    else if (cmd.rfind("reflog", 0) == 0 && currentStage == Stage::PANIC_MODE) {
        reflogVisible = true;
        currentStage = Stage::SHOW_REFLOG;
        return "Showing reflog";
    }
    else if (cmd.rfind("reset --hard HEAD@{1}", 0) == 0 && 
             (currentStage == Stage::SHOW_REFLOG || currentStage == Stage::RECOVERY_DECISION)) {
        // 恢复到事故前一刻
        auto result = git->ResetHard("HEAD@{1}");
        if (result.success) {
            recoveryMethod = "reset";
            codeRecovered = true;
            currentStage = Stage::VERIFY_RECOVERY;
            SyncGraphWithRepo();
            splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
            return "Recovered to HEAD@{1}";
        }
    }
    else if (cmd.rfind("reset --hard HEAD@{2}", 0) == 0 && 
             (currentStage == Stage::SHOW_REFLOG || currentStage == Stage::RECOVERY_DECISION)) {
        // 恢复到更早的状态（完全恢复）
        auto result = git->ResetHard("HEAD@{2}");
        if (result.success) {
            recoveryMethod = "reset_full";
            codeRecovered = true;
            currentStage = Stage::VERIFY_RECOVERY;
            SyncGraphWithRepo();
            splitView->GetStructurePanel()->ScanWorkingDirectory(repoPath);
            return "Recovered to HEAD@{2}";
        }
    }
    return "Command executed: " + cmd;
}

void Level08_Reflog::Update(float deltaTime) {
    timer += deltaTime;
    panicPulse += deltaTime * 5;  // 脉冲动画
    
    // 自适应布局
    if (splitView) {
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        splitView->SetBounds(320, 100, screenWidth - 320, screenHeight - 170);
        splitView->Update(deltaTime);
    }
    
    // 空格键推进剧情
    if (IsKeyPressed(KEY_SPACE)) {
        switch (currentStage) {
            case Stage::INTRO:
                currentStage = Stage::SHOW_HISTORY;
                break;
            case Stage::ACCIDENT_HAPPENS:
                ProcessLevelCommand("reset --hard HEAD~3");
                break;
            case Stage::PANIC_MODE:
                ProcessLevelCommand("reflog");
                break;
            case Stage::VERIFY_RECOVERY:
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
    
    // 数字键选择恢复方式
    if (currentStage == Stage::SHOW_REFLOG || currentStage == Stage::RECOVERY_DECISION) {
        if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_KP_1)) {
            // HEAD@{1} - 回到 reset 之前
            ProcessLevelCommand("reset --hard HEAD@{1}");
        }
        else if (IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2)) {
            // HEAD@{2} - 回到最新 commit
            ProcessLevelCommand("reset --hard HEAD@{2}");
        }
    }
    
    // R 键触发事故（在 SHOW_HISTORY 阶段）
    if (IsKeyPressed(KEY_R) && currentStage == Stage::SHOW_HISTORY) {
        currentStage = Stage::ACCIDENT_HAPPENS;
    }
}

void Level08_Reflog::Draw() {
    ClearBackground({30, 35, 45, 255});
    
    if (splitView) {
        splitView->Draw();
    }
    
    // 紧急模式覆盖层
    if (currentStage == Stage::PANIC_MODE || currentStage == Stage::SHOW_REFLOG) {
        DrawPanicOverlay();
    }
    
    DrawStatusPanel();
    
    // Reflog 面板
    if (reflogVisible && (currentStage == Stage::SHOW_REFLOG || 
                          currentStage == Stage::RECOVERY_DECISION ||
                          currentStage == Stage::RECOVER_CODE)) {
        DrawReflogPanel();
    }
    
    DrawDialogueIfNeeded();
}

void Level08_Reflog::DrawStatusPanel() {
    DrawRectangle(0, 0, 300, 720, {40, 44, 52, 255});
    DrawRectangleLines(0, 0, 300, 720, {100, 150, 200, 255});
    
    DrawChinese("Level 8: 时光回溯", 20, 20, 28, WHITE);
    DrawChinese("用 reflog 拯救代码", 20, 55, 18, LIGHTGRAY);
    DrawText("8 / 10", 250, 30, 16, {100, 200, 255, 255});
    
    DrawChinese("当前任务:", 20, 100, 20, {100, 200, 255, 255});
    
    const char* stageText = "";
    Color stageColor = YELLOW;
    switch (currentStage) {
        case Stage::INTRO: stageText = "按空格开始"; break;
        case Stage::SHOW_HISTORY: stageText = "按 R 触发事故"; break;
        case Stage::ACCIDENT_HAPPENS: stageText = "执行 reset --hard..."; break;
        case Stage::PANIC_MODE: stageText = "代码丢失！按空格"; stageColor = RED; break;
        case Stage::SHOW_REFLOG: stageText = "查看 reflog"; break;
        case Stage::RECOVERY_DECISION: stageText = "选择恢复方式"; break;
        case Stage::RECOVER_CODE: stageText = "恢复中..."; break;
        case Stage::VERIFY_RECOVERY: stageText = "验证恢复结果"; break;
        case Stage::COMPLETE: stageText = "完成！"; stageColor = GREEN; break;
    }
    DrawText(stageText, 20, 130, 18, stageColor);
    
    // 事故信息
    if (accidentTriggered) {
        DrawRectangle(10, 170, 280, 120, {60, 30, 30, 255});
        DrawRectangleLines(10, 170, 280, 120, RED);
        DrawChinese("! 事故报告 !", 20, 180, 22, RED);
        DrawChinese("执行了 reset --hard", 20, 210, 16, WHITE);
        DrawChinese("丢失了 3 个 commit:", 20, 235, 16, LIGHTGRAY);
        DrawChinese("- Database 模块", 30, 255, 14, {255, 150, 150, 255});
        DrawChinese("- Auth 认证", 30, 272, 14, {255, 150, 150, 255});
        DrawChinese("- API 接口", 30, 289, 14, {255, 150, 150, 255});
    }
    
    // 恢复状态
    if (codeRecovered) {
        DrawRectangle(10, 310, 280, 80, {30, 60, 30, 255});
        DrawRectangleLines(10, 310, 280, 80, GREEN);
        DrawChinese("[OK] 代码已恢复", 20, 320, 20, GREEN);
        DrawChinese(("方法: " + recoveryMethod).c_str(), 20, 350, 16, WHITE);
    }
    
    // 操作提示
    DrawRectangle(10, 600, 280, 100, {50, 50, 60, 255});
    DrawChinese("提示:", 20, 610, 18, {100, 200, 255, 255});
    if (currentStage == Stage::SHOW_HISTORY) {
        DrawChinese("按 R 模拟误操作", 20, 635, 16, LIGHTGRAY);
    } else if (currentStage == Stage::SHOW_REFLOG) {
        DrawChinese("按 1: 恢复到 reset前", 20, 635, 16, LIGHTGRAY);
        DrawChinese("按 2: 完全恢复", 20, 655, 16, LIGHTGRAY);
    }
}

void Level08_Reflog::DrawDialogueIfNeeded() {
    int screenHeight = GetScreenHeight();
    int dialogY = screenHeight - 180;
    if (currentStage == Stage::INTRO) {
        DrawRectangle(100, dialogY, 1080, 150, {40, 44, 52, 240});
        DrawRectangleLines(100, dialogY, 1080, 150, {100, 150, 200, 255});
        DrawChinese("CTO: 今天教你们 Git 的救命神器 - reflog！", 120, dialogY + 20, 24, WHITE);
        DrawChinese("即使执行了 reset --hard，代码也能找回来。", 120, dialogY + 50, 22, LIGHTGRAY);
        DrawChinese("按 [空格] 开始演示", 120, dialogY + 100, 20, YELLOW);
    }
    else if (currentStage == Stage::SHOW_HISTORY) {
        DrawRectangle(100, dialogY, 1080, 100, {40, 60, 40, 240});
        DrawRectangleLines(100, dialogY, 1080, 100, {100, 200, 100, 255});
        DrawChinese("这是一个正常开发的仓库，有 5 个 commit，包含数据库、认证、API 等重要功能。", 120, dialogY + 30, 22, WHITE);
        DrawChinese("按 [R] 键模拟误执行 git reset --hard HEAD~3（危险操作！）", 120, dialogY + 60, 20, YELLOW);
    }
    else if (currentStage == Stage::PANIC_MODE) {
        float pulse = (sinf(panicPulse) + 1.0f) * 0.5f;
        Color alertColor = ColorLerp(RED, WHITE, pulse);
        DrawRectangle(100, dialogY, 1080, 150, {60, 20, 20, 240});
        DrawRectangleLines(100, dialogY, 1080, 150, alertColor);
        DrawChinese("CTO: 糟糕！3 个重要的 commit 不见了！", 120, dialogY + 20, 26, RED);
        DrawChinese("Database、Auth、API 代码全丢了！怎么办？！", 120, dialogY + 55, 24, WHITE);
        DrawChinese("按 [空格] 查看 reflog", 120, dialogY + 100, 22, YELLOW);
    }
    else if (currentStage == Stage::VERIFY_RECOVERY) {
        DrawRectangle(100, dialogY, 1080, 100, {40, 60, 40, 240});
        DrawRectangleLines(100, dialogY, 1080, 100, GREEN);
        DrawChinese("CTO: 看到了吗？reflog 记录了 Git 的所有操作历史！", 120, dialogY + 20, 24, GREEN);
        DrawChinese("只要找到事故前的 reflog 条目，用 reset --hard 就能恢复！", 120, dialogY + 50, 22, WHITE);
        DrawChinese("按 [空格] 完成关卡", 120, dialogY + 80, 20, YELLOW);
    }
}

void Level08_Reflog::DrawPanicOverlay() {
    // 红色警告闪烁效果
    float pulse = (sinf(panicPulse) + 1.0f) * 0.5f;
    unsigned char alpha = static_cast<unsigned char>(pulse * 30);
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), {255, 0, 0, alpha});
    
    // 顶部警告条
    if (fmodf(timer, 1.0f) < 0.5f) {
        DrawRectangle(300, 0, GetScreenWidth() - 300, 40, {200, 50, 50, 200});
        DrawChinese("!!! 紧急状态: 代码丢失 !!!", 600, 10, 24, WHITE);
    }
}

void Level08_Reflog::DrawReflogPanel() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    
    // Reflog 面板 - 右侧覆盖层
    int panelX = screenWidth - 450;
    int panelY = 100;
    int panelW = 430;
    int panelH = screenHeight - 270;
    
    DrawRectangle(panelX, panelY, panelW, panelH, {20, 25, 35, 240});
    DrawRectangleLines(panelX, panelY, panelW, panelH, {100, 200, 255, 255});
    
    DrawChinese("Git Reflog - 操作历史", panelX + 10, panelY + 10, 22, {100, 200, 255, 255});
    DrawChinese("(按序号选择恢复)", panelX + 250, panelY + 15, 14, LIGHTGRAY);
    
    int y = panelY + 50;
    int entryHeight = 70;
    
    for (size_t i = 0; i < reflogEntries.size() && y + entryHeight < panelY + panelH; i++) {
        const auto& entry = reflogEntries[i];
        
        // 条目背景
        Color bgColor = (i == 0) ? Color{60, 80, 60, 200} : Color{40, 45, 55, 200};
        if (entry.action == "reset" && entry.message.find("moving") != std::string::npos) {
            // 标记事故点
            bgColor = {80, 40, 40, 200};
        }
        
        DrawRectangle(panelX + 10, y, panelW - 20, entryHeight - 5, bgColor);
        
        // 序号和动作
        Color actionColor = WHITE;
        if (entry.action == "commit") actionColor = GREEN;
        else if (entry.action == "reset") actionColor = RED;
        else if (entry.action == "checkout") actionColor = YELLOW;
        
        std::string idxStr = "[" + std::to_string(i) + "] ";
        DrawText((idxStr + entry.action).c_str(), panelX + 20, y + 8, 16, actionColor);
        
        // 简短 hash
        DrawText(entry.hash.substr(0, 7).c_str(), panelX + 150, y + 8, 14, {150, 150, 150, 255});
        
        // 消息
        std::string msg = entry.message;
        if (msg.length() > 35) msg = msg.substr(0, 35) + "...";
        DrawText(msg.c_str(), panelX + 20, y + 30, 13, LIGHTGRAY);
        
        // 提示可恢复
        if (i >= 1 && i <= 2 && !codeRecovered) {
            if (i == 1) {
                DrawChinese("按 [1] 恢复", panelX + panelW - 100, y + 25, 14, GREEN);
            } else if (i == 2) {
                DrawChinese("按 [2] 完全恢复", panelX + panelW - 110, y + 25, 14, {100, 255, 150, 255});
            }
        }
        
        y += entryHeight;
    }
    
    // 底部说明
    DrawRectangle(panelX, panelY + panelH - 60, panelW, 60, {30, 35, 45, 255});
    DrawLine(panelX + 10, panelY + panelH - 60, panelX + panelW - 10, panelY + panelH - 60, {100, 100, 120, 255});
    DrawChinese("Reflog 原理:", panelX + 15, panelY + panelH - 50, 14, {200, 200, 100, 255});
    DrawChinese("Git 会记录每次 HEAD 移动", panelX + 15, panelY + panelH - 32, 13, LIGHTGRAY);
    DrawChinese("即使 reset --hard 也能找回", panelX + 15, panelY + panelH - 15, 13, LIGHTGRAY);
}

void Level08_Reflog::Shutdown() {
    splitView.reset();
    git.reset();
    
    try {
        fs::remove_all(repoPath);
    } catch (...) {}
}

bool Level08_Reflog::IsComplete() const {
    return stageComplete;
}


