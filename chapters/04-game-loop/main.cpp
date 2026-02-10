#include "raylib.h"
#include <vector>

struct Ball {
    float x, y;
    float vx, vy;
    float radius;
    Color color;
};

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "第4章: 游戏循环示例");
    SetTargetFPS(60);

    std::vector<Ball> balls;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        // 输入处理
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            Ball newBall;
            newBall.x = mousePos.x;
            newBall.y = mousePos.y;
            newBall.vx = (GetRandomValue(-200, 200));
            newBall.vy = (GetRandomValue(-200, 200));
            newBall.radius = 15.0f;
            newBall.color = ColorFromHSV(GetRandomValue(0, 360), 0.8f, 0.9f);
            balls.push_back(newBall);
        }

        // 更新
        for (auto& ball : balls) {
            ball.x += ball.vx * deltaTime;
            ball.y += ball.vy * deltaTime;

            // 边界反弹
            if (ball.x < ball.radius || ball.x > screenWidth - ball.radius) {
                ball.vx = -ball.vx;
                ball.x = (ball.x < ball.radius) ? ball.radius : screenWidth - ball.radius;
            }
            if (ball.y < ball.radius || ball.y > screenHeight - ball.radius) {
                ball.vy = -ball.vy;
                ball.y = (ball.y < ball.radius) ? ball.radius : screenHeight - ball.radius;
            }
        }

        // 渲染
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("游戏循环示例 - 点击创建弹跳球", 10, 10, 20, DARKGRAY);
        DrawText(TextFormat("球数量: %d | FPS: %d", (int)balls.size(), GetFPS()), 10, 40, 20, DARKGRAY);

        for (const auto& ball : balls) {
            DrawCircle((int)ball.x, (int)ball.y, ball.radius, ball.color);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
