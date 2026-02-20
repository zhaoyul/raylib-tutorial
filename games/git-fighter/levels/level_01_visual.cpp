#include "level_01_visual.h"
#include "../src/git_wrapper.h"
#include "raylib.h"
#include <cstring>

Level01_Visual::Level01_Visual() 
    : Level(1, "周末加班", "建立基础架构，学习 Git 基础命令"),
      currentStage(Stage::INTRO),
      timer(0),
      isDragging(false),
      repoInitialized(false),
      firstCommitDone(false),
      commitCount(0) {
    dialogueCTO = "小王，项目还没初始化呢，先建立基础架构。记住，勤提交，少出事。";
    dialoguePlayer = "收到！我这就开始。";
}

Level01_Visual::~Level01_Visual() = default;

void Level01_Visual::Initialize() {
    currentStage = Stage::INTRO;
    timer = 0;
    repoInitialized = false;
    firstCommitDone = false;
    commitCount = 0;
    
    // Initialize graph renderer
    graphRenderer = std::make_unique<GitVis::GitGraphRenderer>();
    graphRenderer->Initialize();
    
    // Start with empty graph or tutorial commits
    // Initially show what will be created
}

void Level01_Visual::Update(float deltaTime) {
    timer += deltaTime;
    
    // Update graph animations
    if (graphRenderer) {
        graphRenderer->UpdateAnimations(deltaTime);
    }
    
    // Handle mouse input for graph interaction
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mousePos = GetMousePosition();
        
        // Check if clicking on graph area
        if (mousePos.x > 350 && mousePos.x < 1230 && mousePos.y > 80 && mousePos.y < 600) {
            auto* node = graphRenderer->GetNodeAtPosition(mousePos);
            if (node) {
                graphRenderer->SelectNode(node->hash);
            } else {
                dragStart = mousePos;
                isDragging = true;
            }
        }
    }
    
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        isDragging = false;
    }
    
    if (isDragging) {
        Vector2 currentPos = GetMousePosition();
        Vector2 delta = {currentPos.x - dragStart.x, currentPos.y - dragStart.y};
        graphRenderer->Pan(delta);
        dragStart = currentPos;
    }
    
    // Zoom
    float wheel = GetMouseWheelMove();
    if (wheel != 0 && graphRenderer) {
        graphRenderer->Zoom(1.0f + wheel * 0.1f);
    }
    
    // Keyboard shortcuts for commands
    if (IsKeyPressed(KEY_I) && currentStage == Stage::WAIT_INIT) {
        ProcessCommand("init");
    }
    if (IsKeyPressed(KEY_A) && currentStage == Stage::WAIT_FIRST_COMMIT) {
        ProcessCommand("add");
    }
    if (IsKeyPressed(KEY_C) && currentStage == Stage::WAIT_FIRST_COMMIT) {
        ProcessCommand("commit");
    }
    if (IsKeyPressed(KEY_SPACE) && currentStage == Stage::INTRO) {
        currentStage = Stage::SHOW_GRAPH;
        timer = 0;
    }
    
    // Stage logic
    switch (currentStage) {
        case Stage::INTRO:
            if (timer > 2.0f && IsKeyPressed(KEY_SPACE)) {
                currentStage = Stage::SHOW_GRAPH;
                timer = 0;
            }
            break;
            
        case Stage::SHOW_GRAPH:
            if (timer > 1.0f) {
                currentStage = Stage::WAIT_INIT;
                timer = 0;
            }
            break;
            
        case Stage::WAIT_INIT:
            // Waiting for user to init repo
            break;
            
        case Stage::WAIT_FIRST_COMMIT:
            // Waiting for first commit
            break;
            
        case Stage::COMPLETE:
            if (timer > 2.0f && IsKeyPressed(KEY_ENTER)) {
                // Level complete
            }
            break;
    }
}

void Level01_Visual::Draw() {
    ClearBackground((Color){245, 247, 250, 255});
    
    // Draw left panel - commands and file status
    DrawRectangle(0, 0, 340, 720, WHITE);
    DrawRectangleLines(0, 0, 340, 720, (Color){200, 200, 200, 255});
    
    DrawCommandPanel();
    
    // Draw graph visualization on the right
    if (graphRenderer && currentStage >= Stage::SHOW_GRAPH) {
        graphRenderer->Draw(350, 80, 880, 520);
    }
    
    // Draw dialogue if in intro stage
    if (currentStage == Stage::INTRO) {
        DrawDialoguePanel();
    }
    
    // Draw stage progress
    DrawRectangle(350, 20, 880, 50, (Color){240, 240, 245, 255});
    DrawText("Level 1: 周末加班", 360, 30, 24, DARKGRAY);
    
    const char* stageText = "";
    switch (currentStage) {
        case Stage::INTRO: stageText = "阅读对话..."; break;
        case Stage::SHOW_GRAPH: stageText = "查看工作区..."; break;
        case Stage::WAIT_INIT: stageText = "等待初始化..."; break;
        case Stage::WAIT_FIRST_COMMIT: stageText = "等待首次提交..."; break;
        case Stage::COMPLETE: stageText = "完成!"; break;
    }
    DrawText(stageText, 800, 32, 18, BLUE);
    
    // Draw hint at bottom
    DrawRectangle(350, 610, 880, 100, (Color){250, 250, 252, 255});
    DrawRectangleLines(350, 610, 880, 100, (Color){200, 200, 200, 255});
    
    if (currentStage == Stage::WAIT_INIT) {
        DrawText("💡 提示: 按 [I] 执行 git init 初始化仓库", 370, 640, 20, DARKGRAY);
        DrawText("   或在终端输入: git init", 370, 665, 16, GRAY);
    } else if (currentStage == Stage::WAIT_FIRST_COMMIT) {
        DrawText("💡 提示: 按 [A] 添加文件, [C] 提交更改", 370, 640, 20, DARKGRAY);
    }
}

void Level01_Visual::Shutdown() {
    if (graphRenderer) {
        graphRenderer->Shutdown();
        graphRenderer.reset();
    }
}

bool Level01_Visual::IsComplete() const {
    return currentStage == Stage::COMPLETE;
}

void Level01_Visual::DrawCommandPanel() {
    int y = 20;
    
    // Title
    DrawText("Git 命令面板", 20, y, 24, DARKGRAY);
    y += 50;
    
    // Available commands based on stage
    struct CmdInfo {
        const char* key;
        const char* name;
        const char* desc;
        bool available;
    };
    
    CmdInfo commands[] = {
        {"[I]", "git init", "初始化仓库", currentStage >= Stage::WAIT_INIT},
        {"[A]", "git add", "添加文件", currentStage >= Stage::WAIT_FIRST_COMMIT && !firstCommitDone},
        {"[C]", "git commit", "提交更改", currentStage >= Stage::WAIT_FIRST_COMMIT && !firstCommitDone},
        {"[R]", "reset view", "重置视图", currentStage >= Stage::SHOW_GRAPH},
    };
    
    for (const auto& cmd : commands) {
        Color bgColor = cmd.available ? (Color){100, 150, 255, 255} : (Color){200, 200, 200, 255};
        Color textColor = cmd.available ? WHITE : GRAY;
        
        DrawRectangle(20, y, 300, 60, bgColor);
        DrawRectangleLines(20, y, 300, 60, (Color){150, 150, 150, 255});
        
        DrawText(cmd.key, 35, y + 10, 20, textColor);
        DrawText(cmd.name, 100, y + 10, 20, textColor);
        DrawText(cmd.desc, 35, y + 35, 14, textColor);
        
        y += 70;
    }
    
    // Status section
    y += 20;
    DrawLine(20, y, 320, y, (Color){200, 200, 200, 255});
    y += 20;
    
    DrawText("当前状态:", 20, y, 18, DARKGRAY);
    y += 30;
    
    if (repoInitialized) {
        DrawText("✓ 仓库已初始化", 20, y, 16, GREEN);
    } else {
        DrawText("○ 未初始化", 20, y, 16, GRAY);
    }
    y += 25;
    
    if (firstCommitDone) {
        DrawText("✓ 首次提交完成", 20, y, 16, GREEN);
    } else if (repoInitialized) {
        DrawText("○ 等待提交", 20, y, 16, ORANGE);
    }
    
    // File status
    y += 50;
    DrawText("文件状态:", 20, y, 18, DARKGRAY);
    y += 30;
    
    if (currentStage >= Stage::WAIT_FIRST_COMMIT) {
        DrawText("??  README.md", 20, y, 16, RED);
        y += 22;
        DrawText("??  main.cpp", 20, y, 16, RED);
    }
}

void Level01_Visual::DrawDialoguePanel() {
    // Draw dialogue box at the bottom
    DrawRectangle(0, 580, 1280, 140, (Color){30, 30, 40, 240});
    DrawRectangleLines(0, 580, 1280, 140, (Color){100, 150, 200, 255});
    
    // CTO avatar
    DrawCircle(80, 640, 40, (Color){100, 150, 200, 255});
    DrawText("CTO", 65, 635, 18, WHITE);
    
    // Dialogue text
    DrawText(dialogueCTO, 150, 610, 24, WHITE);
    
    // Continue hint
    if (timer > 1.0f) {
        DrawText("按 [空格] 继续...", 1100, 690, 16, LIGHTGRAY);
    }
}

void Level01_Visual::ProcessCommand(const std::string& cmd) {
    if (cmd == "init" && currentStage == Stage::WAIT_INIT) {
        repoInitialized = true;
        
        // Add initial commit to graph
        graphRenderer->AddCommit("a1b2c3d", "Initial commit", {}, "小王", 1700000000);
        graphRenderer->AddBranch("main", "a1b2c3d");
        graphRenderer->SetHEAD("a1b2c3d");
        graphRenderer->RecalculateLayout();
        graphRenderer->ZoomToFit();
        
        currentStage = Stage::WAIT_FIRST_COMMIT;
        timer = 0;
    }
    else if (cmd == "add" && currentStage == Stage::WAIT_FIRST_COMMIT) {
        // File staged - visual feedback
    }
    else if (cmd == "commit" && currentStage == Stage::WAIT_FIRST_COMMIT) {
        firstCommitDone = true;
        commitCount++;
        
        // Add new commit to graph
        std::string newHash = "e4f5g6h";
        std::string parentHash = "a1b2c3d";
        graphRenderer->AddCommit(newHash, "Add main.cpp", {parentHash}, "小王", 1699996400);
        graphRenderer->SetHEAD(newHash);
        graphRenderer->RecalculateLayout();
        graphRenderer->StartCommitAnimation(newHash);
        
        currentStage = Stage::COMPLETE;
        timer = 0;
    }
    else if (cmd == "reset") {
        graphRenderer->ZoomToFit();
    }
}
