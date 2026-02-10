#include "raylib.h"
#include <vector>
#include <deque>

const int screenWidth = 800;
const int screenHeight = 600;
const int gridSize = 20;
const int gridWidth = screenWidth / gridSize;
const int gridHeight = screenHeight / gridSize;

enum GameState { MENU, PLAYING, GAME_OVER };
enum Direction { UP, DOWN, LEFT, RIGHT };

struct Position {
    int x, y;
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

int main() {
    InitWindow(screenWidth, screenHeight, "贪吃蛇 Snake");
    SetTargetFPS(60);

    Font uiFont = GetFontDefault();
    bool ownsUIFont = false;
    {
        const char* fontPath = "/System/Library/Fonts/Supplemental/Arial Unicode.ttf";
        const char* allText =
            "0123456789 贪吃蛇 按 ENTER 或 空格 开始游戏 操作: 方向键或 WASD 分数: 长度: 游戏结束 最终分数: 蛇的长度: 按 ENTER 返回菜单";

        int cpCount = 0;
        int* cps = LoadCodepoints(allText, &cpCount);
        Font f = LoadFontEx(fontPath, 64, cps, cpCount);
        UnloadCodepoints(cps);

        if (f.texture.id != 0) {
            uiFont = f;
            ownsUIFont = true;
            SetTextureFilter(uiFont.texture, TEXTURE_FILTER_BILINEAR);
        }
    }

    auto DrawTextCentered = [&](const char* text, float y, float fontSize, Color color) {
        Vector2 sz = MeasureTextEx(uiFont, text, fontSize, 1.0f);
        DrawTextEx(uiFont, text, {(screenWidth - sz.x) * 0.5f, y}, fontSize, 1.0f, color);
    };

    GameState state = MENU;
    Direction direction = RIGHT;
    Direction nextDirection = RIGHT;

    std::deque<Position> snake;
    Position food;
    int score = 0;
    float moveTimer = 0;
    float moveInterval = 0.15f;

    // 初始化游戏
    auto initGame = [&]() {
        snake.clear();
        snake.push_back({gridWidth/2, gridHeight/2});
        snake.push_back({gridWidth/2 - 1, gridHeight/2});
        snake.push_back({gridWidth/2 - 2, gridHeight/2});

        food = {GetRandomValue(0, gridWidth-1), GetRandomValue(0, gridHeight-1)};
        direction = RIGHT;
        nextDirection = RIGHT;
        score = 0;
        moveTimer = 0;
        moveInterval = 0.15f;
    };

    initGame();

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        // 状态处理
        if (state == MENU) {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                initGame();
                state = PLAYING;
            }
        }
        else if (state == PLAYING) {
            // 输入处理（方向改变）
            if ((IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) && direction != DOWN) {
                nextDirection = UP;
            }
            if ((IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) && direction != UP) {
                nextDirection = DOWN;
            }
            if ((IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) && direction != RIGHT) {
                nextDirection = LEFT;
            }
            if ((IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) && direction != LEFT) {
                nextDirection = RIGHT;
            }

            // 更新游戏
            moveTimer += deltaTime;
            if (moveTimer >= moveInterval) {
                moveTimer = 0;
                direction = nextDirection;

                // 计算新头部位置
                Position newHead = snake.front();
                switch (direction) {
                    case UP: newHead.y--; break;
                    case DOWN: newHead.y++; break;
                    case LEFT: newHead.x--; break;
                    case RIGHT: newHead.x++; break;
                }

                // 检查墙壁碰撞
                if (newHead.x < 0 || newHead.x >= gridWidth ||
                    newHead.y < 0 || newHead.y >= gridHeight) {
                    state = GAME_OVER;
                }

                // 检查自身碰撞
                for (size_t i = 0; i < snake.size(); i++) {
                    if (snake[i] == newHead) {
                        state = GAME_OVER;
                    }
                }

                if (state == PLAYING) {
                    snake.push_front(newHead);

                    // 检查食物
                    if (newHead == food) {
                        score += 10;
                        // 加速
                        if (moveInterval > 0.05f) {
                            moveInterval *= 0.95f;
                        }
                        // 生成新食物
                        bool validPosition = false;
                        while (!validPosition) {
                            food = {GetRandomValue(0, gridWidth-1), GetRandomValue(0, gridHeight-1)};
                            validPosition = true;
                            for (const auto& segment : snake) {
                                if (segment == food) {
                                    validPosition = false;
                                    break;
                                }
                            }
                        }
                    } else {
                        snake.pop_back();
                    }
                }
            }
        }
        else if (state == GAME_OVER) {
            if (IsKeyPressed(KEY_ENTER)) {
                state = MENU;
            }
        }

        // 绘制
        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (state == MENU) {
            DrawTextCentered("贪吃蛇", 150, 60, DARKGREEN);
            DrawText("SNAKE", screenWidth/2 - 80, 220, 40, GREEN);
            DrawTextCentered("按 ENTER 或 空格 开始游戏", 300, 20, DARKGRAY);
            DrawTextCentered("操作: 方向键或 WASD", 350, 20, GRAY);
        }
        else if (state == PLAYING) {
            // 绘制网格
            for (int i = 0; i < gridWidth; i++) {
                for (int j = 0; j < gridHeight; j++) {
                    Color color = ((i + j) % 2 == 0) ? Fade(GREEN, 0.1f) : Fade(GREEN, 0.05f);
                    DrawRectangle(i * gridSize, j * gridSize, gridSize, gridSize, color);
                }
            }

            // 绘制食物
            DrawRectangle(food.x * gridSize + 2, food.y * gridSize + 2,
                         gridSize - 4, gridSize - 4, RED);

            // 绘制蛇
            for (size_t i = 0; i < snake.size(); i++) {
                Color color = (i == 0) ? DARKGREEN : GREEN;
                DrawRectangle(snake[i].x * gridSize + 1, snake[i].y * gridSize + 1,
                            gridSize - 2, gridSize - 2, color);
            }

            // 绘制分数
            const char* scoreText = TextFormat("分数: %d", score);
            DrawTextEx(uiFont, scoreText, {10.0f, 10.0f}, 25, 1.0f, DARKGRAY);

            const char* lenText = TextFormat("长度: %d", (int)snake.size());
            Vector2 lenSz = MeasureTextEx(uiFont, lenText, 25, 1.0f);
            DrawTextEx(uiFont, lenText, {(float)screenWidth - 10.0f - lenSz.x, 10.0f}, 25, 1.0f, DARKGRAY);
        }
        else if (state == GAME_OVER) {
            DrawTextCentered("游戏结束", 200, 50, RED);
            DrawTextCentered(TextFormat("最终分数: %d", score), 280, 30, DARKGRAY);
            DrawTextCentered(TextFormat("蛇的长度: %d", (int)snake.size()), 320, 25, DARKGRAY);
            DrawTextCentered("按 ENTER 返回菜单", 380, 20, GRAY);
        }

        EndDrawing();
    }

    if (ownsUIFont) UnloadFont(uiFont);
    CloseWindow();
    return 0;
}
