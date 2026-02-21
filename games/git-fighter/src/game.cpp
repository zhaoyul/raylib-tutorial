#include "game.h"
#include "level_manager.h"
#include <iostream>
#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;

// Clean up any leftover temp directories from previous runs
static void CleanupTempDirectories() {
    std::cout << "Cleaning up temp directories..." << std::endl;
    try {
        for (const auto& entry : fs::directory_iterator("/tmp")) {
            if (entry.is_directory()) {
                std::string name = entry.path().filename().string();
                if (name.find("gitfighter_level") == 0) {
                    std::cout << "  Removing: " << entry.path() << std::endl;
                    fs::remove_all(entry.path());
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error cleaning up temp directories: " << e.what() << std::endl;
    }
}

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
    // Clean up any leftover temp directories from previous runs
    CleanupTempDirectories();
    
    // Create resizable window
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, TITLE);
    SetTargetFPS(60);

    // Render target disabled - drawing directly to screen for fixed-size UI elements
    // renderTarget = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    useRenderTarget = false;

    // Load Chinese font
    gameFont.Load();

    // Initialize Git Console
    gitConsole.Initialize(100, 100, 800, 300);
    
    // Set font for Chinese text rendering
    gitConsole.SetFont(&levelManager->GetFont());
    
    // Setup command callback to forward to current level
    gitConsole.SetCommandCallback([this](const std::string& cmd) {
        if (auto* level = levelManager->GetCurrentLevel()) {
            // Execute command through level and get result
            std::string result = level->ExecuteGitCommand(cmd);
            
            // Add result to console output
            if (!result.empty()) {
                // Split result by lines and add each line
                std::string line;
                for (char c : result) {
                    if (c == '\n') {
                        if (!line.empty()) {
                            gitConsole.AddOutput(line, WHITE);
                            line.clear();
                        }
                    } else {
                        line += c;
                    }
                }
                if (!line.empty()) {
                    gitConsole.AddOutput(line, WHITE);
                }
            }
        } else {
            gitConsole.AddOutput("错误: 没有活动的关卡", RED);
        }
    });

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
    // Render target disabled
    // if (useRenderTarget) {
    //     UnloadRenderTexture(renderTarget);
    // }
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
            // Update git console first (handles TAB toggle)
            gitConsole.Update(deltaTime);
            
            // Only update level if console is not visible (to avoid interference)
            if (!gitConsole.IsVisible()) {
                levelManager->Update(deltaTime);
            }

            // Check if level is complete, but don't auto-transition
            if (levelManager->IsCurrentLevelComplete() && !levelCompleteShown) {
                levelCompleteShown = true;
                std::cout << "Level " << currentLevel << " complete! Click NEXT to continue." << std::endl;
            }
            
            // Handle Next button click
            if (levelCompleteShown && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mousePos = GetMousePosition();
                int windowWidth = GetScreenWidth();
                
                Rectangle nextButton = {
                    (float)(windowWidth/2 - 150),
                    10,
                    300,
                    50
                };
                
                if (CheckCollisionPointRec(mousePos, nextButton)) {
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
    BeginDrawing();
    
    // Get actual window size
    int windowWidth = GetScreenWidth();
    int windowHeight = GetScreenHeight();
    
    // Fill entire window with background color
    ClearBackground((Color){30, 30, 40, 255});

    switch (currentState) {
        case GameState::MENU:
            DrawMenu();
            break;

        case GameState::PLAYING:
            levelManager->Draw();
            DrawHUD();
            
            // Draw Git Console
            gitConsole.Draw();
            
            // Draw Next button when level is complete
            if (levelCompleteShown) {
                // Semi-transparent overlay at top
                DrawRectangle(windowWidth/2 - 150, 10, 300, 50, {0, 100, 0, 200});
                DrawRectangleLines(windowWidth/2 - 150, 10, 300, 50, {0, 255, 0, 255});
                DrawChineseText("✓ 关卡完成! 点击此处进入下一关", windowWidth/2 - 130, 22, 20, WHITE);
            }
            break;

        case GameState::PAUSED:
            levelManager->Draw();
            DrawRectangle(0, 0, windowWidth, windowHeight, (Color){0, 0, 0, 180});
            DrawChineseText("已暂停", windowWidth/2 - 80, windowHeight/2 - 40, 56, WHITE);
            DrawChineseText("按 [ENTER] 继续", windowWidth/2 - 140, windowHeight/2 + 40, 26, LIGHTGRAY);
            DrawChineseText("按 [ESC] 返回主菜单", windowWidth/2 - 160, windowHeight/2 + 75, 22, GRAY);
            break;

        case GameState::LEVEL_COMPLETE:
            levelManager->Draw();
            DrawRectangle(0, 0, windowWidth, windowHeight, (Color){0, 0, 0, 200});
            DrawChineseText("关卡完成!", windowWidth/2 - 150, windowHeight/2 - 50, 56, GREEN);
            DrawChineseText("按 [ENTER] 进入下一关", windowWidth/2 - 180, windowHeight/2 + 40, 26, WHITE);
            break;

        default:
            break;
    }

    EndDrawing();
}

Vector2 GitGame::GetGameMouseOffset() const {
    int windowWidth = GetScreenWidth();
    int windowHeight = GetScreenHeight();
    int offsetX = (windowWidth - SCREEN_WIDTH) / 2;
    int offsetY = (windowHeight - SCREEN_HEIGHT) / 2;
    if (offsetX < 0) offsetX = 0;
    if (offsetY < 0) offsetY = 0;
    return {(float)offsetX, (float)offsetY};
}

Vector2 GitGame::ScreenToGameMouse(Vector2 screenPos) const {
    Vector2 offset = GetGameMouseOffset();
    return {screenPos.x - offset.x, screenPos.y - offset.y};
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
    int windowWidth = GetScreenWidth();
    int windowHeight = GetScreenHeight();
    
    // Level info at top left
    if (auto* level = levelManager->GetCurrentLevel()) {
        DrawRectangle(0, 0, 400, 50, (Color){30, 30, 40, 200});
        DrawChineseText(TextFormat("Level %d: %s", level->GetId(), level->GetName().c_str()),
                 20, 12, 26, WHITE);
    }

    // Help at bottom - fixed to window bottom
    DrawRectangle(0, windowHeight - 40, windowWidth, 40, (Color){30, 30, 40, 200});
    DrawChineseText("[ESC] 暂停  |  [I] git init  |  [A] git add  |  [C] git commit",
             250, windowHeight - 32, 22, LIGHTGRAY);
}
