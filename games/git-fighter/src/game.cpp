#include "game.h"
#include "level_manager.h"
#include <iostream>
#include <cstdlib>

// GameFont implementation
void GameFont::Load() {
    hasChineseFont = false;

    // Try multiple possible paths
    const char* paths[] = {
        "data/fonts/NotoSansCJKsc-Regular.otf",
        "../data/fonts/NotoSansCJKsc-Regular.otf",
        "../../data/fonts/NotoSansCJKsc-Regular.otf",
    };

    const char* fontPath = nullptr;
    for (const auto& p : paths) {
        if (FileExists(p)) {
            fontPath = p;
            break;
        }
    }

    if (!fontPath) {
        TraceLog(LOG_WARNING, "Chinese font not found in any standard path");
        return;
    }

    // Generate codepoints for Chinese (ASCII + CJK)
    int codepoints[20000];
    int idx = 0;
    for (int cp = 0x0020; cp <= 0x007E; cp++) codepoints[idx++] = cp;
    for (int cp = 0x4E00; cp <= 0x8FFF; cp++) codepoints[idx++] = cp;

    chineseFont = LoadFontEx(fontPath, 48, codepoints, idx);

    if (chineseFont.texture.id != 0 && chineseFont.glyphCount > 1000) {
        hasChineseFont = true;
        TraceLog(LOG_INFO, "Chinese font loaded: %d glyphs", chineseFont.glyphCount);
    } else {
        TraceLog(LOG_WARNING, "Failed to load Chinese font properly");
    }
}

void GameFont::Unload() {
    if (hasChineseFont) {
        UnloadFont(chineseFont);
        hasChineseFont = false;
    }
}

GitGame::GitGame()
    : currentState(GameState::MENU),
      currentLevel(1),
      shouldQuit(false),
      levelCompleteShown(false),
      levelManager(std::make_unique<LevelManager>()),
      uiManager(nullptr),
      renderTarget({0}),
      useRenderTarget(false) {
}

GitGame::~GitGame() = default;

bool GitGame::Initialize() {
    // Create resizable window, render target handles the scaling
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, TITLE);
    SetTargetFPS(60);

    // Create render target for consistent rendering at fixed resolution
    renderTarget = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    useRenderTarget = true;

    // Load Chinese font
    gameFont.Load();

    if (!levelManager->Initialize()) {
        std::cerr << "Failed to initialize level manager" << std::endl;
        return false;
    }

    return true;
}

void GitGame::Run() {
    while (!WindowShouldClose() && !shouldQuit) {
        float deltaTime = GetFrameTime();
        Update(deltaTime);
        Draw();
    }
}

void GitGame::Shutdown() {
    if (useRenderTarget) {
        UnloadRenderTexture(renderTarget);
    }
    gameFont.Unload();
    levelManager.reset();
    CloseWindow();
}

void GitGame::ChangeState(GameState newState) {
    currentState = newState;
}

void GitGame::LoadLevel(int levelNum) {
    currentLevel = levelNum;
    levelCompleteShown = false;
    levelManager->LoadLevel(levelNum);
    ChangeState(GameState::PLAYING);
}

void GitGame::CompleteCurrentLevel() {
    ChangeState(GameState::LEVEL_COMPLETE);
}

void GitGame::Update(float deltaTime) {
    switch (currentState) {
        case GameState::MENU:
            if (IsKeyPressed(KEY_ENTER)) {
                LoadLevel(1);
            }
            if (IsKeyPressed(KEY_ESCAPE)) {
                shouldQuit = true;
            }
            break;

        case GameState::PLAYING:
            levelManager->Update(deltaTime);

            // Check if level is complete, but don't auto-transition
            if (levelManager->IsCurrentLevelComplete() && !levelCompleteShown) {
                levelCompleteShown = true;
                std::cout << "Level " << currentLevel << " complete! Click NEXT to continue." << std::endl;
            }
            
            // Handle Next button click
            if (levelCompleteShown && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mousePos = GetMousePosition();
                // Convert to render target coordinates
                int screenWidth = GetScreenWidth();
                int screenHeight = GetScreenHeight();
                float scaleX = (float)screenWidth / SCREEN_WIDTH;
                float scaleY = (float)screenHeight / SCREEN_HEIGHT;
                float scale = (scaleX < scaleY) ? scaleX : scaleY;
                int drawWidth = (int)(SCREEN_WIDTH * scale);
                int drawHeight = (int)(SCREEN_HEIGHT * scale);
                int drawX = (screenWidth - drawWidth) / 2;
                int drawY = (screenHeight - drawHeight) / 2;
                
                // Convert mouse to virtual screen coordinates
                int virtualMouseX = (int)((mousePos.x - drawX) / scale);
                int virtualMouseY = (int)((mousePos.y - drawY) / scale);
                
                Rectangle nextButton = {
                    (float)(SCREEN_WIDTH/2 - 150),
                    10,
                    300,
                    50
                };
                
                if (virtualMouseX >= nextButton.x && 
                    virtualMouseX <= nextButton.x + nextButton.width &&
                    virtualMouseY >= nextButton.y && 
                    virtualMouseY <= nextButton.y + nextButton.height) {
                    levelCompleteShown = false;
                    LoadLevel(currentLevel + 1);
                }
            }

            if (IsKeyPressed(KEY_ESCAPE)) {
                ChangeState(GameState::PAUSED);
            }
            break;

        case GameState::PAUSED:
            if (IsKeyPressed(KEY_ENTER)) {
                ChangeState(GameState::PLAYING);
            }
            if (IsKeyPressed(KEY_ESCAPE)) {
                ChangeState(GameState::MENU);
            }
            break;

        case GameState::LEVEL_COMPLETE:
            // Kept for compatibility, but now we use levelCompleteShown flag in PLAYING
            ChangeState(GameState::PLAYING);
            break;

        default:
            break;
    }
}

void GitGame::Draw() {
    if (useRenderTarget) {
        // Render to fixed-size texture
        BeginTextureMode(renderTarget);
        ClearBackground(BLACK);
    } else {
        BeginDrawing();
    }

    switch (currentState) {
        case GameState::MENU:
            DrawMenu();
            break;

        case GameState::PLAYING:
            levelManager->Draw();
            DrawHUD();
            
            // Draw Next button when level is complete
            if (levelCompleteShown) {
                // Semi-transparent overlay at top
                DrawRectangle(SCREEN_WIDTH/2 - 150, 10, 300, 50, {0, 100, 0, 200});
                DrawRectangleLines(SCREEN_WIDTH/2 - 150, 10, 300, 50, {0, 255, 0, 255});
                DrawChineseText("✓ 关卡完成! 点击此处进入下一关", SCREEN_WIDTH/2 - 130, 22, 20, WHITE);
            }
            break;

        case GameState::PAUSED:
            levelManager->Draw();
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0, 0, 0, 180});
            DrawChineseText("已暂停", SCREEN_WIDTH/2 - 80, SCREEN_HEIGHT/2 - 40, 56, WHITE);
            DrawChineseText("按 [ENTER] 继续", SCREEN_WIDTH/2 - 140, SCREEN_HEIGHT/2 + 40, 26, LIGHTGRAY);
            DrawChineseText("按 [ESC] 返回主菜单", SCREEN_WIDTH/2 - 160, SCREEN_HEIGHT/2 + 75, 22, GRAY);
            break;

        case GameState::LEVEL_COMPLETE:
            levelManager->Draw();
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0, 0, 0, 200});
            DrawChineseText("关卡完成!", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 - 50, 56, GREEN);
            DrawChineseText("按 [ENTER] 进入下一关", SCREEN_WIDTH/2 - 180, SCREEN_HEIGHT/2 + 40, 26, WHITE);
            break;

        default:
            break;
    }

    if (useRenderTarget) {
        EndTextureMode();

        // Draw scaled render target to screen
        BeginDrawing();
        ClearBackground(BLACK);

        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();

        // Calculate scale to fit while maintaining aspect ratio
        float scaleX = (float)screenWidth / SCREEN_WIDTH;
        float scaleY = (float)screenHeight / SCREEN_HEIGHT;
        float scale = (scaleX < scaleY) ? scaleX : scaleY;

        // Calculate centered position
        int drawWidth = (int)(SCREEN_WIDTH * scale);
        int drawHeight = (int)(SCREEN_HEIGHT * scale);
        int drawX = (screenWidth - drawWidth) / 2;
        int drawY = (screenHeight - drawHeight) / 2;

        // Draw render texture (flipped Y because OpenGL)
        Rectangle sourceRec = {0, 0, (float)renderTarget.texture.width, -(float)renderTarget.texture.height};
        Rectangle destRec = {(float)drawX, (float)drawY, (float)drawWidth, (float)drawHeight};
        DrawTexturePro(renderTarget.texture, sourceRec, destRec, (Vector2){0, 0}, 0.0f, WHITE);

        EndDrawing();
    } else {
        EndDrawing();
    }
}

void GitGame::DrawChineseText(const char* text, int x, int y, int fontSize, Color color) {
    if (gameFont.hasChineseFont) {
        DrawTextEx(gameFont.chineseFont, text, (Vector2){(float)x, (float)y}, (float)fontSize, 2.0f, color);
    } else {
        DrawText(text, x, y, fontSize, color);
    }
}

void GitGame::DrawMenu() {
    ClearBackground((Color){30, 35, 45, 255});

    // Title
    DrawText("Git Fighter", SCREEN_WIDTH/2 - 180, 150, 80, (Color){100, 200, 255, 255});
    DrawChineseText("救火架构师", SCREEN_WIDTH/2 - 140, 240, 52, (Color){150, 220, 255, 200});

    // Subtitle
    DrawText("The Firefighter Architect", SCREEN_WIDTH/2 - 180, 310, 28, LIGHTGRAY);

    // Menu options
    int menuY = 450;
    DrawChineseText("[ENTER] 开始游戏", SCREEN_WIDTH/2 - 140, menuY, 30, WHITE);
    DrawChineseText("[ESC] 退出", SCREEN_WIDTH/2 - 80, menuY + 55, 26, GRAY);

    // Footer
    DrawChineseText("Level 1: 周末加班 - 学习 git init, add, commit",
             SCREEN_WIDTH/2 - 300, 600, 22, (Color){150, 150, 150, 255});
}

void GitGame::DrawHUD() {
    // Level info at top left
    if (auto* level = levelManager->GetCurrentLevel()) {
        DrawRectangle(0, 0, 400, 50, (Color){30, 30, 40, 200});
        DrawChineseText(TextFormat("Level %d: %s", level->GetId(), level->GetName().c_str()),
                 20, 12, 26, WHITE);
    }

    // Help at bottom
    DrawRectangle(0, SCREEN_HEIGHT - 40, SCREEN_WIDTH, 40, (Color){30, 30, 40, 200});
    DrawChineseText("[ESC] 暂停  |  [I] git init  |  [A] git add  |  [C] git commit",
             250, SCREEN_HEIGHT - 32, 22, LIGHTGRAY);
}
