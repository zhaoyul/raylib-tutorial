#include "raylib.h"

enum GameState {
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER
};

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "第6章: 游戏状态");
    SetTargetFPS(60);
    
    GameState currentState = MENU;
    int score = 0;
    
    while (!WindowShouldClose()) {
        // 状态更新
        switch (currentState) {
            case MENU:
                if (IsKeyPressed(KEY_ENTER)) {
                    currentState = PLAYING;
                    score = 0;
                }
                break;
                
            case PLAYING:
                score++;
                if (IsKeyPressed(KEY_P)) currentState = PAUSED;
                if (IsKeyPressed(KEY_ESCAPE)) currentState = GAME_OVER;
                break;
                
            case PAUSED:
                if (IsKeyPressed(KEY_P)) currentState = PLAYING;
                if (IsKeyPressed(KEY_ESCAPE)) currentState = MENU;
                break;
                
            case GAME_OVER:
                if (IsKeyPressed(KEY_ENTER)) currentState = MENU;
                break;
        }
        
        // 渲染
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        switch (currentState) {
            case MENU:
                DrawText("主菜单", screenWidth/2 - 80, screenHeight/2 - 100, 40, DARKGRAY);
                DrawText("按 ENTER 开始游戏", screenWidth/2 - 130, screenHeight/2, 20, GRAY);
                break;
                
            case PLAYING:
                DrawText("游戏进行中", 10, 10, 30, DARKGRAY);
                DrawText(TextFormat("分数: %d", score), 10, 50, 25, BLUE);
                DrawText("按 P 暂停 | ESC 结束", 10, 85, 20, GRAY);
                DrawCircle(screenWidth/2, screenHeight/2, 30, GREEN);
                break;
                
            case PAUSED:
                DrawText("暂停", screenWidth/2 - 60, screenHeight/2 - 50, 40, ORANGE);
                DrawText("按 P 继续 | ESC 返回菜单", screenWidth/2 - 140, screenHeight/2 + 20, 20, GRAY);
                break;
                
            case GAME_OVER:
                DrawText("游戏结束", screenWidth/2 - 100, screenHeight/2 - 100, 40, RED);
                DrawText(TextFormat("最终分数: %d", score), screenWidth/2 - 80, screenHeight/2 - 20, 25, DARKGRAY);
                DrawText("按 ENTER 返回菜单", screenWidth/2 - 120, screenHeight/2 + 40, 20, GRAY);
                break;
        }
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
