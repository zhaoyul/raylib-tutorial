#include "raylib.h"

int main() {
    // 初始化窗口
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "第3章: Raylib 基础示例");
    SetTargetFPS(60);

    // 玩家位置
    float playerX = screenWidth / 2;
    float playerY = screenHeight / 2;
    float playerSpeed = 5.0f;
    float playerRadius = 20.0f;

    // 游戏循环
    while (!WindowShouldClose()) {
        // 更新
        // 键盘控制移动
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) playerX += playerSpeed;
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) playerX -= playerSpeed;
        if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) playerY += playerSpeed;
        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) playerY -= playerSpeed;

        // 边界检查
        if (playerX < playerRadius) playerX = playerRadius;
        if (playerX > screenWidth - playerRadius) playerX = screenWidth - playerRadius;
        if (playerY < playerRadius) playerY = playerRadius;
        if (playerY > screenHeight - playerRadius) playerY = screenHeight - playerRadius;

        // 绘制
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // 绘制标题
        DrawText("Raylib 基础示例", 10, 10, 30, DARKGRAY);
        DrawText("使用 WASD 或方向键移动圆形", 10, 50, 20, GRAY);

        // 绘制一些静态图形作为背景
        DrawRectangle(50, 150, 150, 100, LIGHTGRAY);
        DrawRectangleLines(50, 150, 150, 100, DARKGRAY);
        DrawText("矩形", 100, 190, 20, DARKGRAY);

        DrawCircle(650, 200, 40, LIGHTGRAY);
        DrawCircleLines(650, 200, 40, DARKGRAY);
        DrawText("圆形", 620, 260, 20, DARKGRAY);

        DrawLine(50, 400, 750, 400, DARKGRAY);
        DrawText("线条", 350, 410, 20, DARKGRAY);

        // 绘制玩家（可移动的圆形）
        DrawCircle((int)playerX, (int)playerY, playerRadius, BLUE);
        DrawCircleLines((int)playerX, (int)playerY, playerRadius, DARKBLUE);

        // 显示位置信息
        DrawText(TextFormat("位置: (%.0f, %.0f)", playerX, playerY), 10, screenHeight - 30, 20, DARKGRAY);

        EndDrawing();
    }

    // 关闭窗口
    CloseWindow();
    return 0;
}
