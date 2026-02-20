#include "level_01_init.h"
#include "../src/git_wrapper.h"
#include "raylib.h"
#include <cstring>

Level01_Init::Level01_Init() 
    : Level(1, "周末加班", "建立基础架构，学习 Git 基础命令"),
      currentStage(Stage::DIALOGUE_CTO),
      timer(0),
      commitCount(0),
      git(nullptr) {
    dialogueCTO = "小王，项目还没初始化呢，先建立基础架构。记住，勤提交，少出事。";
    dialoguePlayer = "收到！我这就开始。";
}

void Level01_Init::Initialize() {
    currentStage = Stage::DIALOGUE_CTO;
    timer = 0;
    commitCount = 0;
    
    // Create a temporary directory for this level
    std::string levelPath = "/tmp/gitfighter_level1_" + std::to_string(GetTime());
    git = new GitWrapper();
    
    // Don't init yet - let player do it
}

void Level01_Init::Update(float deltaTime) {
    timer += deltaTime;
    
    switch (currentStage) {
        case Stage::DIALOGUE_CTO:
            if (timer > 3.0f || IsKeyPressed(KEY_SPACE)) {
                AdvanceStage();
            }
            break;
            
        case Stage::SHOW_INSTRUCTIONS:
            if (timer > 2.0f || IsKeyPressed(KEY_SPACE)) {
                AdvanceStage();
            }
            break;
            
        case Stage::WAIT_INIT:
            // Check if player typed 'init' command
            if (CheckCommandInput()) {
                // In real game, parse command properly
                if (git && git->IsRepoOpen()) {
                    AdvanceStage();
                }
            }
            break;
            
        case Stage::CREATE_FILES:
            // Auto-create some tutorial files
            if (git && git->IsRepoOpen()) {
                git->CreateFile("README.md", "# 福报科技核心项目\n\n这是一个改变命运的项目。");
                git->CreateFile("main.cpp", "#include <iostream>\n\nint main() {\n    std::cout << \"Hello, World!\" << std::endl;\n    return 0;\n}");
                AdvanceStage();
            }
            break;
            
        case Stage::WAIT_ADD:
            // Wait for player to add files
            if (CheckCommandInput()) {
                AdvanceStage();
            }
            break;
            
        case Stage::WAIT_COMMIT:
            // Wait for player to commit
            if (CheckCommandInput()) {
                commitCount++;
                if (commitCount >= 1) {
                    AdvanceStage();
                }
            }
            break;
            
        case Stage::COMPLETE:
            // Level complete
            break;
    }
}

void Level01_Init::Draw() {
    ClearBackground((Color){245, 247, 250, 255});  // Light office background
    
    // Draw workspace (file explorer visualization)
    DrawWorkspace();
    
    // Draw command hint
    DrawCommandHint();
    
    // Draw stage indicator
    DrawStageIndicator();
    
    // Draw dialogue box if needed
    if (currentStage == Stage::DIALOGUE_CTO) {
        DrawDialogue();
    }
}

void Level01_Init::Shutdown() {
    if (git) {
        delete git;
        git = nullptr;
    }
}

bool Level01_Init::IsComplete() const {
    return currentStage == Stage::COMPLETE;
}

void Level01_Init::AdvanceStage() {
    timer = 0;
    switch (currentStage) {
        case Stage::DIALOGUE_CTO: currentStage = Stage::SHOW_INSTRUCTIONS; break;
        case Stage::SHOW_INSTRUCTIONS: currentStage = Stage::WAIT_INIT; break;
        case Stage::WAIT_INIT: currentStage = Stage::CREATE_FILES; break;
        case Stage::CREATE_FILES: currentStage = Stage::WAIT_ADD; break;
        case Stage::WAIT_ADD: currentStage = Stage::WAIT_COMMIT; break;
        case Stage::WAIT_COMMIT: currentStage = Stage::COMPLETE; break;
        default: break;
    }
}

bool Level01_Init::CheckCommandInput() {
    // In the full game, this would check a command input buffer
    // For now, use keyboard shortcuts for testing
    if (currentStage == Stage::WAIT_INIT && IsKeyPressed(KEY_I)) {
        git->Init("/tmp/gitfighter_level1_repo");
        return true;
    }
    if (currentStage == Stage::WAIT_ADD && IsKeyPressed(KEY_A)) {
        git->Add(".");
        return true;
    }
    if (currentStage == Stage::WAIT_COMMIT && IsKeyPressed(KEY_C)) {
        git->Commit("Initial commit: 项目初始化");
        return true;
    }
    return false;
}

void Level01_Init::DrawWorkspace() {
    // Draw file explorer panel on the left
    DrawRectangle(20, 80, 300, 500, WHITE);
    DrawRectangleLines(20, 80, 300, 500, (Color){200, 200, 200, 255});
    DrawChinese("文件资源管理器", 30, 90, 24, DARKGRAY);
    
    // Draw files based on stage
    int y = 130;
    if (currentStage >= Stage::CREATE_FILES) {
        DrawChinese("README.md", 60, y, 20, DARKGRAY);
        y += 35;
        DrawChinese("main.cpp", 60, y, 20, DARKGRAY);
        y += 35;
    }
    
    // Show git status visualization
    if (git && git->IsRepoOpen()) {
        auto status = git->GetWorkingDirectoryStatus();
        y = 250;
        DrawLine(20, y - 10, 320, y - 10, (Color){200, 200, 200, 255});
        DrawChinese("Git 状态", 30, y, 22, DARKGRAY);
        y += 35;
        
        for (const auto& file : status) {
            const char* statusText = "";
            Color statusColor = DARKGRAY;
            switch (file.status) {
                case FileStatus::UNTRACKED: statusText = "??"; statusColor = RED; break;
                case FileStatus::MODIFIED: statusText = "M"; statusColor = ORANGE; break;
                case FileStatus::STAGED: statusText = "A"; statusColor = GREEN; break;
                case FileStatus::COMMITTED: statusText = "✓"; statusColor = DARKGREEN; break;
            }
            DrawChinese(TextFormat("%s %s", statusText, file.path.c_str()), 40, y, 18, statusColor);
            y += 28;
        }
    }
}

void Level01_Init::DrawCommandHint() {
    // Draw command reference panel on the right
    DrawRectangle(940, 80, 320, 400, (Color){250, 250, 252, 255});
    DrawRectangleLines(940, 80, 320, 400, (Color){200, 200, 200, 255});
    DrawChinese("可用命令", 960, 90, 26, DARKGRAY);
    
    int y = 140;
    Color cmdColor = DARKGRAY;
    
    // Highlight available commands based on stage
    if (currentStage == Stage::WAIT_INIT) {
        cmdColor = BLUE;
        DrawChinese("[I] git init", 960, y, 22, cmdColor);
    } else {
        cmdColor = DARKGREEN;
        DrawChinese("git init ✓", 960, y, 22, cmdColor);
    }
    y += 45;
    
    if (currentStage == Stage::WAIT_ADD) {
        cmdColor = BLUE;
        DrawChinese("[A] git add .", 960, y, 22, cmdColor);
    } else if (currentStage > Stage::WAIT_ADD) {
        DrawChinese("git add . ✓", 960, y, 22, DARKGREEN);
    } else {
        DrawChinese("git add .", 960, y, 22, LIGHTGRAY);
    }
    y += 45;
    
    if (currentStage == Stage::WAIT_COMMIT) {
        cmdColor = BLUE;
        DrawChinese("[C] git commit", 960, y, 22, cmdColor);
        y += 32;
        DrawChinese("-m \"Initial commit\"", 980, y, 18, cmdColor);
    } else if (currentStage > Stage::WAIT_COMMIT) {
        DrawChinese("git commit ✓", 960, y, 22, DARKGREEN);
    } else {
        DrawChinese("git commit", 960, y, 22, LIGHTGRAY);
    }
}

void Level01_Init::DrawStageIndicator() {
    // Draw progress at the top
    const char* stages[] = {"初始化", "创建文件", "添加文件", "提交"};
    int currentIdx = 0;
    switch (currentStage) {
        case Stage::WAIT_INIT: currentIdx = 0; break;
        case Stage::CREATE_FILES: currentIdx = 1; break;
        case Stage::WAIT_ADD: currentIdx = 2; break;
        case Stage::WAIT_COMMIT: currentIdx = 3; break;
        default: currentIdx = 0;
    }
    
    int x = 400;
    for (int i = 0; i < 4; i++) {
        Color circleColor = (i <= currentIdx) ? BLUE : LIGHTGRAY;
        DrawCircle(x, 40, 14, circleColor);
        DrawChinese(stages[i], x - 35, 62, 20, (i <= currentIdx) ? DARKGRAY : LIGHTGRAY);
        
        if (i < 3) {
            DrawLine(x + 14, 40, x + 86, 40, (i < currentIdx) ? BLUE : LIGHTGRAY);
        }
        x += 100;
    }
}

void Level01_Init::DrawDialogue() {
    // Draw dialogue box at the bottom
    DrawRectangle(0, 580, 1280, 140, (Color){30, 30, 40, 240});
    DrawRectangleLines(0, 580, 1280, 140, (Color){100, 150, 200, 255});
    
    // CTO avatar placeholder
    DrawCircle(80, 640, 40, (Color){100, 150, 200, 255});
    DrawChinese("CTO", 60, 632, 22, WHITE);
    
    // Dialogue text
    DrawChinese(dialogueCTO.c_str(), 150, 605, 28, WHITE);
    
    // Continue hint
    if (timer > 1.0f) {
        DrawChinese("按 [空格] 继续...", 1050, 685, 20, LIGHTGRAY);
    }
}
