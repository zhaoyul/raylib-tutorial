#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

// Game States
enum GameState {
    MENU,
    SETTINGS,
    PLAYING,
    GAME_OVER
};

// Font and text management
struct UIText {
    Font chineseFont;
    bool hasChineseFont;
    const char* fontPath;
    
    // Text strings
    const char* title;
    const char* btnStart;
    const char* btnSettings;
    const char* btnExit;
    const char* btnBack;
    const char* lblPlayer;
    const char* lblPlayerName;
    const char* lblDifficulty;
    const char* lblVolume;
    const char* lblBrightness;
    const char* chkFullscreen;
    const char* chkShowFps;
    const char* settingsTitle;
    const char* diffEasy;
    const char* txtMove;
    const char* txtScore;
    const char* txtEscMenu;
    const char* txtGameOver;
    const char* txtFinalScore;
    const char* txtEnterReturn;
};

// Try to load font with Chinese support
bool TryLoadFont(UIText& ui, const char* path) {
    if (!FileExists(path)) {
        return false;
    }
    
    TraceLog(LOG_INFO, "Loading font: %s", path);
    
    // Load font with larger size for better quality
    ui.chineseFont = LoadFontEx(path, 32, NULL, 0);
    
    if (ui.chineseFont.texture.id != 0) {
        TraceLog(LOG_INFO, "Font loaded: %s (glyphs: %d)", path, ui.chineseFont.glyphCount);
        
        // Check if it has enough glyphs for Chinese (should be > 1000)
        if (ui.chineseFont.glyphCount > 1000) {
            ui.hasChineseFont = true;
            ui.fontPath = path;
            GuiSetFont(ui.chineseFont);
            TraceLog(LOG_INFO, "Chinese font active: %s", path);
            return true;
        } else {
            TraceLog(LOG_WARNING, "Font has only %d glyphs, Chinese may not display correctly", 
                     ui.chineseFont.glyphCount);
            UnloadFont(ui.chineseFont);
            ui.chineseFont = (Font){0};
        }
    }
    
    return false;
}

void LoadChineseFont(UIText& ui) {
    ui.hasChineseFont = false;
    ui.fontPath = NULL;
    
    // Try downloaded fonts first (these work correctly)
    if (TryLoadFont(ui, "data/fonts/AlibabaPuHuiTi-Regular.otf")) goto set_chinese;
    if (TryLoadFont(ui, "data/fonts/NotoSansSC-Regular.otf")) goto set_chinese;
    if (TryLoadFont(ui, "data/fonts/NotoSansCJKsc-Regular.otf")) goto set_chinese;
    
    // System fonts usually don't work well (limited glyphs)
    TraceLog(LOG_WARNING, "No Chinese font found in data/fonts/");
    TraceLog(LOG_WARNING, "Please download a Chinese font to data/fonts/ directory");
    TraceLog(LOG_WARNING, "See data/fonts/README.md for instructions");
    
    goto set_english;
    
set_chinese:
    ui.title = "RAYGUI 演示";
    ui.btnStart = "开始游戏";
    ui.btnSettings = "设置";
    ui.btnExit = "退出";
    ui.btnBack = "返回";
    ui.lblPlayer = "当前玩家: %s";
    ui.lblPlayerName = "玩家名称:";
    ui.lblDifficulty = "游戏难度:";
    ui.lblVolume = "音量:";
    ui.lblBrightness = "亮度:";
    ui.chkFullscreen = "全屏模式";
    ui.chkShowFps = "显示 FPS";
    ui.settingsTitle = "游戏设置";
    ui.diffEasy = "简单;中等;困难";
    ui.txtMove = "使用方向键移动";
    ui.txtScore = "分数: %d";
    ui.txtEscMenu = "按 ESC 返回菜单";
    ui.txtGameOver = "游戏结束!";
    ui.txtFinalScore = "最终分数: %d";
    ui.txtEnterReturn = "按 ENTER 返回菜单";
    return;
    
set_english:
    ui.title = "RAYGUI DEMO";
    ui.btnStart = "Start Game";
    ui.btnSettings = "Settings";
    ui.btnExit = "Exit";
    ui.btnBack = "Back";
    ui.lblPlayer = "Player: %s";
    ui.lblPlayerName = "Player Name:";
    ui.lblDifficulty = "Difficulty:";
    ui.lblVolume = "Volume:";
    ui.lblBrightness = "Brightness:";
    ui.chkFullscreen = "Fullscreen";
    ui.chkShowFps = "Show FPS";
    ui.settingsTitle = "Game Settings";
    ui.diffEasy = "Easy;Medium;Hard";
    ui.txtMove = "Use arrow keys to move";
    ui.txtScore = "Score: %d";
    ui.txtEscMenu = "Press ESC to return to menu";
    ui.txtGameOver = "Game Over!";
    ui.txtFinalScore = "Final Score: %d";
    ui.txtEnterReturn = "Press ENTER to return to menu";
}

void UnloadChineseFont(UIText& ui) {
    if (ui.hasChineseFont) {
        UnloadFont(ui.chineseFont);
        ui.hasChineseFont = false;
    }
}

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Chapter 7: Raygui Basics");
    SetTargetFPS(60);
    
    UIText uiText = {};
    LoadChineseFont(uiText);
    
    GameState currentState = MENU;
    float volume = 0.5f;
    float brightness = 1.0f;
    bool fullscreen = false;
    bool showFps = true;
    int difficulty = 0;
    char playerName[64] = "Player1";
    bool nameEditMode = false;
    bool difficultyEditMode = false;
    int score = 0;
    float playerX = screenWidth / 2.0f;
    float playerY = screenHeight / 2.0f;
    
    while (!WindowShouldClose()) {
        switch (currentState) {
            case PLAYING:
                if (IsKeyDown(KEY_LEFT)) playerX -= 5.0f;
                if (IsKeyDown(KEY_RIGHT)) playerX += 5.0f;
                if (IsKeyDown(KEY_UP)) playerY -= 5.0f;
                if (IsKeyDown(KEY_DOWN)) playerY += 5.0f;
                
                if (playerX < 20) playerX = 20;
                if (playerX > screenWidth - 20) playerX = screenWidth - 20;
                if (playerY < 20) playerY = 20;
                if (playerY > screenHeight - 20) playerY = screenHeight - 20;
                
                score++;
                
                if (IsKeyPressed(KEY_ESCAPE)) currentState = MENU;
                break;
                
            case GAME_OVER:
                if (IsKeyPressed(KEY_ENTER)) {
                    currentState = MENU;
                    score = 0;
                }
                break;
                
            default:
                break;
        }
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        switch (currentState) {
            case MENU: {
                int centerX = screenWidth / 2 - 100;
                int startY = 150;
                
                DrawText(uiText.title, centerX + 30, 50, 30, DARKGRAY);
                
                if (GuiButton((Rectangle){(float)centerX, (float)startY, 200, 40}, uiText.btnStart)) {
                    currentState = PLAYING;
                    score = 0;
                    playerX = screenWidth / 2.0f;
                    playerY = screenHeight / 2.0f;
                }
                
                if (GuiButton((Rectangle){(float)centerX, (float)(startY + 60), 200, 40}, uiText.btnSettings)) {
                    currentState = SETTINGS;
                }
                
                if (GuiButton((Rectangle){(float)centerX, (float)(startY + 120), 200, 40}, uiText.btnExit)) {
                    UnloadChineseFont(uiText);
                    CloseWindow();
                    return 0;
                }
                
                GuiLabel((Rectangle){(float)centerX, (float)(startY + 200), 200, 20}, 
                         TextFormat(uiText.lblPlayer, playerName));
                break;
            }
            
            case SETTINGS: {
                int panelX = 150, panelY = 50, panelWidth = 500, panelHeight = 500;
                
                GuiPanel((Rectangle){(float)panelX, (float)panelY, (float)panelWidth, (float)panelHeight}, 
                         uiText.settingsTitle);
                
                int controlX = panelX + 30, controlY = panelY + 50, labelWidth = 120;
                
                GuiLabel((Rectangle){(float)controlX, (float)controlY, (float)labelWidth, 24}, uiText.lblPlayerName);
                if (GuiTextBox((Rectangle){(float)(controlX + labelWidth), (float)controlY, 200, 24}, 
                               playerName, 64, nameEditMode)) {
                    nameEditMode = !nameEditMode;
                }
                controlY += 50;
                
                GuiLabel((Rectangle){(float)controlX, (float)controlY, (float)labelWidth, 24}, uiText.lblDifficulty);
                if (GuiDropdownBox((Rectangle){(float)(controlX + labelWidth), (float)controlY, 200, 30},
                                   uiText.diffEasy, &difficulty, difficultyEditMode)) {
                    difficultyEditMode = !difficultyEditMode;
                }
                controlY += 60;
                
                GuiLabel((Rectangle){(float)controlX, (float)controlY, (float)labelWidth, 24}, uiText.lblVolume);
                GuiSlider((Rectangle){(float)(controlX + labelWidth), (float)(controlY + 5), 200, 15},
                          NULL, TextFormat("%d%%", (int)(volume * 100)), &volume, 0.0f, 1.0f);
                controlY += 50;
                
                GuiLabel((Rectangle){(float)controlX, (float)controlY, (float)labelWidth, 24}, uiText.lblBrightness);
                GuiSlider((Rectangle){(float)(controlX + labelWidth), (float)(controlY + 5), 200, 15},
                          NULL, TextFormat("%d%%", (int)(brightness * 100)), &brightness, 0.0f, 2.0f);
                controlY += 50;
                
                GuiCheckBox((Rectangle){(float)controlX, (float)controlY, 20, 20}, uiText.chkFullscreen, &fullscreen);
                controlY += 40;
                
                GuiCheckBox((Rectangle){(float)controlX, (float)controlY, 20, 20}, uiText.chkShowFps, &showFps);
                controlY += 60;
                
                if (GuiButton((Rectangle){(float)(panelX + panelWidth/2 - 60), (float)(panelY + panelHeight - 60), 
                                          120, 40}, uiText.btnBack)) {
                    currentState = MENU;
                }
                break;
            }
            
            case PLAYING: {
                DrawText(uiText.txtMove, 10, 10, 20, DARKGRAY);
                DrawText(TextFormat(uiText.txtScore, score), 10, 40, 20, BLUE);
                DrawText(uiText.txtEscMenu, 10, 70, 20, GRAY);
                
                Color playerColor = (difficulty == 0) ? GREEN : (difficulty == 1) ? YELLOW : RED;
                DrawCircle((int)playerX, (int)playerY, 20, playerColor);
                break;
            }
            
            case GAME_OVER: {
                DrawText(uiText.txtGameOver, screenWidth/2 - 100, screenHeight/2 - 50, 40, RED);
                DrawText(TextFormat(uiText.txtFinalScore, score), screenWidth/2 - 80, screenHeight/2 + 20, 25, DARKGRAY);
                DrawText(uiText.txtEnterReturn, screenWidth/2 - 120, screenHeight/2 + 80, 20, GRAY);
                break;
            }
        }
        
        if (showFps) DrawFPS(screenWidth - 100, 10);
        
        EndDrawing();
    }
    
    UnloadChineseFont(uiText);
    CloseWindow();
    return 0;
}
