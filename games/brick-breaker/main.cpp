#include "raylib.h"
#include <vector>

const int screenWidth = 800;
const int screenHeight = 600;

enum GameState { MENU, PLAYING, PAUSED, GAME_OVER, WIN };

struct Paddle {
    float x, y, width, height;
    float speed;
};

struct Ball {
    float x, y, radius;
    float vx, vy;
    bool active;
};

struct Brick {
    float x, y, width, height;
    bool active;
    Color color;
};

int main() {
    InitWindow(screenWidth, screenHeight, "打砖块 Brick Breaker");
    SetTargetFPS(60);
    
    GameState state = MENU;
    int score = 0;
    int lives = 3;
    
    // 初始化挡板
    Paddle paddle = {screenWidth/2 - 60, screenHeight - 40, 120, 20, 8.0f};
    
    // 初始化球
    Ball ball = {screenWidth/2, screenHeight - 60, 8, 0, -300, false};
    
    // 创建砖块
    std::vector<Brick> bricks;
    const int rows = 5;
    const int cols = 10;
    const float brickWidth = 70;
    const float brickHeight = 25;
    const float brickSpacing = 5;
    const float offsetX = 35;
    const float offsetY = 50;
    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            Brick brick;
            brick.x = offsetX + j * (brickWidth + brickSpacing);
            brick.y = offsetY + i * (brickHeight + brickSpacing);
            brick.width = brickWidth;
            brick.height = brickHeight;
            brick.active = true;
            brick.color = ColorFromHSV(i * 36, 0.7f, 0.9f);
            bricks.push_back(brick);
        }
    }
    
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        
        // 状态处理
        if (state == MENU) {
            if (IsKeyPressed(KEY_ENTER)) {
                state = PLAYING;
                score = 0;
                lives = 3;
                ball.active = false;
                ball.x = paddle.x + paddle.width/2;
                ball.y = paddle.y - ball.radius - 1;
                // 重置砖块
                for (auto& brick : bricks) brick.active = true;
            }
        }
        else if (state == PLAYING) {
            // 挡板控制
            if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) paddle.x -= paddle.speed;
            if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) paddle.x += paddle.speed;
            
            // 边界限制
            if (paddle.x < 0) paddle.x = 0;
            if (paddle.x > screenWidth - paddle.width) paddle.x = screenWidth - paddle.width;
            
            // 发射球
            if (!ball.active && IsKeyPressed(KEY_SPACE)) {
                ball.active = true;
            }
            
            // 球跟随挡板（未发射时）
            if (!ball.active) {
                ball.x = paddle.x + paddle.width/2;
                ball.y = paddle.y - ball.radius - 1;
            }
            
            // 更新球
            if (ball.active) {
                ball.x += ball.vx * deltaTime;
                ball.y += ball.vy * deltaTime;
                
                // 墙壁碰撞
                if (ball.x <= ball.radius || ball.x >= screenWidth - ball.radius) {
                    ball.vx = -ball.vx;
                }
                if (ball.y <= ball.radius) {
                    ball.vy = -ball.vy;
                }
                
                // 挡板碰撞
                if (ball.y + ball.radius >= paddle.y &&
                    ball.x >= paddle.x && ball.x <= paddle.x + paddle.width &&
                    ball.vy > 0) {
                    ball.vy = -ball.vy;
                    // 根据击中位置调整水平速度
                    float hitPos = (ball.x - paddle.x) / paddle.width;
                    ball.vx = (hitPos - 0.5f) * 400;
                }
                
                // 砖块碰撞
                for (auto& brick : bricks) {
                    if (brick.active) {
                        Rectangle brickRect = {brick.x, brick.y, brick.width, brick.height};
                        if (CheckCollisionCircleRec({ball.x, ball.y}, ball.radius, brickRect)) {
                            brick.active = false;
                            ball.vy = -ball.vy;
                            score += 10;
                        }
                    }
                }
                
                // 球掉落
                if (ball.y > screenHeight) {
                    lives--;
                    if (lives <= 0) {
                        state = GAME_OVER;
                    }
                    ball.active = false;
                }
            }
            
            // 检查胜利
            bool allBricksDestroyed = true;
            for (const auto& brick : bricks) {
                if (brick.active) {
                    allBricksDestroyed = false;
                    break;
                }
            }
            if (allBricksDestroyed) state = WIN;
            
            // 暂停
            if (IsKeyPressed(KEY_P)) state = PAUSED;
        }
        else if (state == PAUSED) {
            if (IsKeyPressed(KEY_P)) state = PLAYING;
            if (IsKeyPressed(KEY_ESCAPE)) state = MENU;
        }
        else if (state == GAME_OVER || state == WIN) {
            if (IsKeyPressed(KEY_ENTER)) state = MENU;
        }
        
        // 绘制
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        if (state == MENU) {
            DrawText("打砖块", screenWidth/2 - 100, 150, 60, DARKBLUE);
            DrawText("BRICK BREAKER", screenWidth/2 - 150, 220, 30, BLUE);
            DrawText("按 ENTER 开始游戏", screenWidth/2 - 130, 300, 20, DARKGRAY);
            DrawText("操作: 左右方向键或 A/D - 移动挡板", screenWidth/2 - 200, 350, 20, GRAY);
            DrawText("     空格键 - 发射球", screenWidth/2 - 200, 380, 20, GRAY);
        }
        else if (state == PLAYING || state == PAUSED) {
            // 绘制UI
            DrawText(TextFormat("分数: %d", score), 10, 10, 25, DARKGRAY);
            DrawText(TextFormat("生命: %d", lives), screenWidth - 120, 10, 25, RED);
            
            // 绘制砖块
            for (const auto& brick : bricks) {
                if (brick.active) {
                    DrawRectangle(brick.x, brick.y, brick.width, brick.height, brick.color);
                    DrawRectangleLines(brick.x, brick.y, brick.width, brick.height, DARKGRAY);
                }
            }
            
            // 绘制挡板
            DrawRectangle(paddle.x, paddle.y, paddle.width, paddle.height, BLUE);
            
            // 绘制球
            DrawCircle(ball.x, ball.y, ball.radius, RED);
            
            // 暂停提示
            if (state == PAUSED) {
                DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.5f));
                DrawText("暂停", screenWidth/2 - 60, screenHeight/2 - 40, 40, WHITE);
                DrawText("按 P 继续", screenWidth/2 - 70, screenHeight/2 + 20, 20, WHITE);
            }
        }
        else if (state == GAME_OVER) {
            DrawText("游戏结束", screenWidth/2 - 120, 200, 50, RED);
            DrawText(TextFormat("最终分数: %d", score), screenWidth/2 - 100, 280, 30, DARKGRAY);
            DrawText("按 ENTER 返回菜单", screenWidth/2 - 130, 350, 20, GRAY);
        }
        else if (state == WIN) {
            DrawText("恭喜胜利！", screenWidth/2 - 120, 200, 50, GREEN);
            DrawText(TextFormat("最终分数: %d", score), screenWidth/2 - 100, 280, 30, DARKGRAY);
            DrawText("按 ENTER 返回菜单", screenWidth/2 - 130, 350, 20, GRAY);
        }
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
