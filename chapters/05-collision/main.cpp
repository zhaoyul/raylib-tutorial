#include "raylib.h"

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "第5章: 碰撞检测");
    SetTargetFPS(60);

    Rectangle player = {350, 250, 50, 50};
    Rectangle obstacle = {200, 200, 100, 100};
    Vector2 ball = {600, 300};
    float ballRadius = 25;

    while (!WindowShouldClose()) {
        // 移动玩家
        if (IsKeyDown(KEY_RIGHT)) player.x += 3;
        if (IsKeyDown(KEY_LEFT)) player.x -= 3;
        if (IsKeyDown(KEY_DOWN)) player.y += 3;
        if (IsKeyDown(KEY_UP)) player.y -= 3;

        // 检测碰撞
        bool playerObstacleCollision = CheckCollisionRecs(player, obstacle);
        bool playerBallCollision = CheckCollisionCircleRec(ball, ballRadius, player);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("碰撞检测示例 - 使用方向键移动", 10, 10, 20, DARKGRAY);

        // 绘制障碍物
        DrawRectangleRec(obstacle, playerObstacleCollision ? RED : GRAY);
        DrawText("障碍物", (int)obstacle.x + 10, (int)obstacle.y + 40, 20, WHITE);

        // 绘制球
        DrawCircleV(ball, ballRadius, playerBallCollision ? RED : BLUE);
        DrawText("球", (int)ball.x - 10, (int)ball.y - 10, 20, WHITE);

        // 绘制玩家
        DrawRectangleRec(player, GREEN);
        DrawText("玩家", (int)player.x, (int)player.y + 15, 15, WHITE);

        // 显示碰撞状态
        if (playerObstacleCollision) {
            DrawText("碰撞: 玩家 <-> 障碍物", 10, 50, 20, RED);
        }
        if (playerBallCollision) {
            DrawText("碰撞: 玩家 <-> 球", 10, 80, 20, RED);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
