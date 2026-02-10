#include "raylib.h"
#include <vector>

const int screenWidth = 800;
const int screenHeight = 600;

enum GameState { MENU, PLAYING, GAME_OVER, WIN };
enum Direction { UP, DOWN, LEFT, RIGHT };

struct Tank {
    float x, y;
    float size;
    Direction dir;
    Color color;
    int health;
};

struct Bullet {
    float x, y;
    Direction dir;
    float speed;
    bool active;
    bool fromPlayer;
};

struct Wall {
    float x, y, width, height;
};

int main() {
    InitWindow(screenWidth, screenHeight, "坦克大战 Tank Battle");
    SetTargetFPS(60);

    Font uiFont = GetFontDefault();
    bool ownsUIFont = false;
    {
        const char* fontPath = "/System/Library/Fonts/Supplemental/Arial Unicode.ttf";
        const char* allText =
            "0123456789 坦克大战 按 ENTER 开始 WASD: 移动 | 空格: 射击 分数: 生命: 游戏结束 按 ENTER 返回 胜利！";

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
    Tank player = {400, 500, 40, UP, BLUE, 3};
    std::vector<Tank> enemies;
    std::vector<Bullet> bullets;
    std::vector<Wall> walls;
    int score = 0;

    // 创建墙壁
    for (int i = 0; i < 10; i++) {
        walls.push_back({(float)GetRandomValue(100, 700),
                        (float)GetRandomValue(100, 400), 60, 60});
    }

    // 创建敌人
    for (int i = 0; i < 3; i++) {
        enemies.push_back({(float)(100 + i * 250), 50, 40, DOWN, RED, 1});
    }

    float enemyShootTimer = 0;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        if (state == MENU) {
            if (IsKeyPressed(KEY_ENTER)) {
                state = PLAYING;
                player = {400, 500, 40, UP, BLUE, 3};
                enemies.clear();
                bullets.clear();
                score = 0;
                for (int i = 0; i < 3; i++) {
                    enemies.push_back({(float)(100 + i * 250), 50, 40, DOWN, RED, 1});
                }
            }
        }
        else if (state == PLAYING) {
            // 玩家控制
            float speed = 200 * deltaTime;
            Tank oldPlayer = player;

            if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
                player.y -= speed;
                player.dir = UP;
            }
            if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
                player.y += speed;
                player.dir = DOWN;
            }
            if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
                player.x -= speed;
                player.dir = LEFT;
            }
            if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
                player.x += speed;
                player.dir = RIGHT;
            }

            // 边界检查
            if (player.x < player.size/2) player.x = player.size/2;
            if (player.x > screenWidth - player.size/2) player.x = screenWidth - player.size/2;
            if (player.y < player.size/2) player.y = player.size/2;
            if (player.y > screenHeight - player.size/2) player.y = screenHeight - player.size/2;

            // 发射子弹
            if (IsKeyPressed(KEY_SPACE)) {
                Bullet b = {player.x, player.y, player.dir, 400, true, true};
                bullets.push_back(b);
            }

            // 敌人射击
            enemyShootTimer += deltaTime;
            if (enemyShootTimer > 2.0f) {
                enemyShootTimer = 0;
                for (auto& enemy : enemies) {
                    if (GetRandomValue(0, 1) == 0) {
                        Bullet b = {enemy.x, enemy.y, DOWN, 300, true, false};
                        bullets.push_back(b);
                    }
                }
            }

            // 更新子弹
            for (auto& bullet : bullets) {
                if (bullet.active) {
                    switch (bullet.dir) {
                        case UP: bullet.y -= bullet.speed * deltaTime; break;
                        case DOWN: bullet.y += bullet.speed * deltaTime; break;
                        case LEFT: bullet.x -= bullet.speed * deltaTime; break;
                        case RIGHT: bullet.x += bullet.speed * deltaTime; break;
                    }

                    if (bullet.x < 0 || bullet.x > screenWidth ||
                        bullet.y < 0 || bullet.y > screenHeight) {
                        bullet.active = false;
                    }

                    // 子弹击中敌人
                    if (bullet.fromPlayer) {
                        for (auto& enemy : enemies) {
                            if (CheckCollisionCircles({bullet.x, bullet.y}, 5,
                                                     {enemy.x, enemy.y}, enemy.size/2)) {
                                bullet.active = false;
                                enemy.health--;
                                if (enemy.health <= 0) {
                                    enemy.x = -1000;
                                    score += 100;
                                }
                            }
                        }
                    }
                    // 子弹击中玩家
                    else {
                        if (CheckCollisionCircles({bullet.x, bullet.y}, 5,
                                                 {player.x, player.y}, player.size/2)) {
                            bullet.active = false;
                            player.health--;
                            if (player.health <= 0) {
                                state = GAME_OVER;
                            }
                        }
                    }
                }
            }

            // 清除无效子弹
            bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
                         [](const Bullet& b) { return !b.active; }), bullets.end());

            // 检查胜利
            bool allEnemiesDead = true;
            for (const auto& enemy : enemies) {
                if (enemy.health > 0) {
                    allEnemiesDead = false;
                    break;
                }
            }
            if (allEnemiesDead) state = WIN;
        }
        else if (state == GAME_OVER || state == WIN) {
            if (IsKeyPressed(KEY_ENTER)) {
                state = MENU;
            }
        }

        // 绘制
        BeginDrawing();
        ClearBackground(Fade(GREEN, 0.2f));

        if (state == MENU) {
            DrawTextCentered("坦克大战", 150, 50, DARKBLUE);
            DrawText("TANK BATTLE", screenWidth/2 - 100, 210, 25, BLUE);
            DrawTextCentered("按 ENTER 开始", 280, 20, DARKGRAY);
            DrawTextEx(uiFont, "WASD: 移动 | 空格: 射击",
                       {(float)screenWidth/2 - 140.0f, 330.0f}, 20, 1.0f, GRAY);
        }
        else if (state == PLAYING) {
            // 绘制UI
            const char* scoreText = TextFormat("分数: %d", score);
            DrawTextEx(uiFont, scoreText, {10.0f, 10.0f}, 25, 1.0f, DARKGRAY);

            const char* hpText = TextFormat("生命: %d", player.health);
            Vector2 hpSz = MeasureTextEx(uiFont, hpText, 25, 1.0f);
            DrawTextEx(uiFont, hpText, {(float)screenWidth - 10.0f - hpSz.x, 10.0f}, 25, 1.0f, RED);

            // 绘制墙壁
            for (const auto& wall : walls) {
                DrawRectangle(wall.x, wall.y, wall.width, wall.height, BROWN);
            }

            // 绘制敌人
            for (const auto& enemy : enemies) {
                if (enemy.health > 0) {
                    DrawRectangle(enemy.x - enemy.size/2, enemy.y - enemy.size/2,
                                enemy.size, enemy.size, enemy.color);
                    DrawRectangleLines(enemy.x - enemy.size/2, enemy.y - enemy.size/2,
                                     enemy.size, enemy.size, DARKGRAY);
                }
            }

            // 绘制玩家
            DrawRectangle(player.x - player.size/2, player.y - player.size/2,
                        player.size, player.size, player.color);
            DrawRectangleLines(player.x - player.size/2, player.y - player.size/2,
                             player.size, player.size, DARKBLUE);

            // 绘制子弹
            for (const auto& bullet : bullets) {
                if (bullet.active) {
                    DrawCircle(bullet.x, bullet.y, 5, bullet.fromPlayer ? YELLOW : ORANGE);
                }
            }
        }
        else if (state == GAME_OVER) {
            DrawTextCentered("游戏结束", 200, 45, RED);
            DrawTextCentered(TextFormat("分数: %d", score), 270, 28, DARKGRAY);
            DrawTextCentered("按 ENTER 返回", 330, 20, GRAY);
        }
        else if (state == WIN) {
            DrawTextCentered("胜利！", 200, 50, GREEN);
            DrawTextCentered(TextFormat("分数: %d", score), 270, 28, DARKGRAY);
            DrawTextCentered("按 ENTER 返回", 330, 20, GRAY);
        }

        EndDrawing();
    }

    if (ownsUIFont) UnloadFont(uiFont);
    CloseWindow();
    return 0;
}
