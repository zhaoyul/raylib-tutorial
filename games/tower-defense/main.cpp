#include "raylib.h"
#include <vector>
#include <cmath>

const int screenWidth = 800;
const int screenHeight = 600;

enum GameState { MENU, PLAYING, GAME_OVER, WIN };

struct Enemy {
    float x, y;
    float speed;
    int health;
    int maxHealth;
    int pathIndex;
    bool active;
};

struct Tower {
    float x, y;
    float range;
    float fireRate;
    float fireTimer;
    Color color;
};

struct Bullet {
    float x, y;
    float targetX, targetY;
    float speed;
    int damage;
    bool active;
};

// 定义路径点
std::vector<Vector2> pathPoints = {
    {50, 50}, {750, 50}, {750, 300}, {200, 300}, {200, 550}, {750, 550}
};

int main() {
    InitWindow(screenWidth, screenHeight, "塔防游戏 Tower Defense");
    SetTargetFPS(60);
    
    GameState state = MENU;
    std::vector<Enemy> enemies;
    std::vector<Tower> towers;
    std::vector<Bullet> bullets;
    
    int money = 500;
    int lives = 10;
    int wave = 0;
    float waveTimer = 0;
    int score = 0;
    
    auto spawnWave = [&]() {
        wave++;
        for (int i = 0; i < 5 + wave * 2; i++) {
            Enemy e;
            e.x = pathPoints[0].x;
            e.y = pathPoints[0].y;
            e.speed = 50 + wave * 5;
            e.health = 50 + wave * 20;
            e.maxHealth = e.health;
            e.pathIndex = 0;
            e.active = true;
            enemies.push_back(e);
        }
    };
    
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        
        if (state == MENU) {
            if (IsKeyPressed(KEY_ENTER)) {
                state = PLAYING;
                money = 500;
                lives = 10;
                wave = 0;
                score = 0;
                enemies.clear();
                towers.clear();
                bullets.clear();
                spawnWave();
            }
        }
        else if (state == PLAYING) {
            // 放置塔
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && money >= 100) {
                Vector2 mousePos = GetMousePosition();
                // 检查是否在路径上
                bool onPath = false;
                for (size_t i = 0; i < pathPoints.size() - 1; i++) {
                    if (CheckCollisionPointLine(mousePos, pathPoints[i], 
                                               pathPoints[i+1], 30)) {
                        onPath = true;
                        break;
                    }
                }
                if (!onPath) {
                    Tower t;
                    t.x = mousePos.x;
                    t.y = mousePos.y;
                    t.range = 100;
                    t.fireRate = 1.0f;
                    t.fireTimer = 0;
                    t.color = BLUE;
                    towers.push_back(t);
                    money -= 100;
                }
            }
            
            // 更新敌人
            for (auto& enemy : enemies) {
                if (enemy.active) {
                    if (enemy.pathIndex < (int)pathPoints.size() - 1) {
                        Vector2 target = pathPoints[enemy.pathIndex + 1];
                        Vector2 dir = {target.x - enemy.x, target.y - enemy.y};
                        float dist = sqrtf(dir.x * dir.x + dir.y * dir.y);
                        
                        if (dist < 5) {
                            enemy.pathIndex++;
                        } else {
                            dir.x /= dist;
                            dir.y /= dist;
                            enemy.x += dir.x * enemy.speed * deltaTime;
                            enemy.y += dir.y * enemy.speed * deltaTime;
                        }
                    } else {
                        enemy.active = false;
                        lives--;
                        if (lives <= 0) state = GAME_OVER;
                    }
                }
            }
            
            // 更新塔
            for (auto& tower : towers) {
                tower.fireTimer += deltaTime;
                if (tower.fireTimer >= tower.fireRate) {
                    // 寻找范围内的敌人
                    for (auto& enemy : enemies) {
                        if (enemy.active) {
                            float dist = sqrtf(pow(enemy.x - tower.x, 2) + 
                                             pow(enemy.y - tower.y, 2));
                            if (dist <= tower.range) {
                                Bullet b;
                                b.x = tower.x;
                                b.y = tower.y;
                                b.targetX = enemy.x;
                                b.targetY = enemy.y;
                                b.speed = 300;
                                b.damage = 25;
                                b.active = true;
                                bullets.push_back(b);
                                tower.fireTimer = 0;
                                break;
                            }
                        }
                    }
                }
            }
            
            // 更新子弹
            for (auto& bullet : bullets) {
                if (bullet.active) {
                    Vector2 dir = {bullet.targetX - bullet.x, bullet.targetY - bullet.y};
                    float dist = sqrtf(dir.x * dir.x + dir.y * dir.y);
                    
                    if (dist < 10) {
                        bullet.active = false;
                        // 击中敌人
                        for (auto& enemy : enemies) {
                            if (enemy.active) {
                                float enemyDist = sqrtf(pow(enemy.x - bullet.targetX, 2) + 
                                                       pow(enemy.y - bullet.targetY, 2));
                                if (enemyDist < 20) {
                                    enemy.health -= bullet.damage;
                                    if (enemy.health <= 0) {
                                        enemy.active = false;
                                        money += 25;
                                        score += 10;
                                    }
                                    break;
                                }
                            }
                        }
                    } else {
                        dir.x /= dist;
                        dir.y /= dist;
                        bullet.x += dir.x * bullet.speed * deltaTime;
                        bullet.y += dir.y * bullet.speed * deltaTime;
                    }
                }
            }
            
            // 清除无效实体
            enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
                         [](const Enemy& e) { return !e.active; }), enemies.end());
            bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
                         [](const Bullet& b) { return !b.active; }), bullets.end());
            
            // 波次管理
            if (enemies.empty()) {
                waveTimer += deltaTime;
                if (waveTimer >= 3.0f) {
                    waveTimer = 0;
                    if (wave < 10) {
                        spawnWave();
                    } else {
                        state = WIN;
                    }
                }
            }
        }
        else if (state == GAME_OVER || state == WIN) {
            if (IsKeyPressed(KEY_ENTER)) {
                state = MENU;
            }
        }
        
        // 绘制
        BeginDrawing();
        ClearBackground(Fade(GREEN, 0.3f));
        
        if (state == MENU) {
            DrawText("塔防游戏", screenWidth/2 - 100, 150, 50, DARKBLUE);
            DrawText("TOWER DEFENSE", screenWidth/2 - 130, 210, 25, BLUE);
            DrawText("按 ENTER 开始", screenWidth/2 - 100, 280, 20, DARKGRAY);
            DrawText("鼠标左键: 放置塔 (100金币)", screenWidth/2 - 150, 340, 18, GRAY);
        }
        else if (state == PLAYING) {
            // 绘制路径
            for (size_t i = 0; i < pathPoints.size() - 1; i++) {
                DrawLineEx(pathPoints[i], pathPoints[i+1], 40, Fade(BROWN, 0.5f));
            }
            
            // 绘制塔
            for (const auto& tower : towers) {
                DrawCircle(tower.x, tower.y, tower.range, Fade(BLUE, 0.1f));
                DrawCircle(tower.x, tower.y, 15, tower.color);
                DrawCircleLines(tower.x, tower.y, 15, DARKBLUE);
            }
            
            // 绘制敌人
            for (const auto& enemy : enemies) {
                if (enemy.active) {
                    DrawCircle(enemy.x, enemy.y, 12, RED);
                    // 血条
                    float healthPercent = (float)enemy.health / enemy.maxHealth;
                    DrawRectangle(enemy.x - 15, enemy.y - 20, 30, 5, BLACK);
                    DrawRectangle(enemy.x - 15, enemy.y - 20, 30 * healthPercent, 5, GREEN);
                }
            }
            
            // 绘制子弹
            for (const auto& bullet : bullets) {
                if (bullet.active) {
                    DrawCircle(bullet.x, bullet.y, 4, YELLOW);
                }
            }
            
            // UI
            DrawRectangle(0, 0, screenWidth, 35, Fade(BLACK, 0.7f));
            DrawText(TextFormat("金币: %d", money), 10, 8, 20, YELLOW);
            DrawText(TextFormat("生命: %d", lives), 150, 8, 20, RED);
            DrawText(TextFormat("波次: %d/10", wave), 300, 8, 20, WHITE);
            DrawText(TextFormat("分数: %d", score), 500, 8, 20, GREEN);
            
            if (enemies.empty()) {
                DrawText("下一波即将到来...", screenWidth/2 - 100, screenHeight - 40, 20, WHITE);
            }
        }
        else if (state == GAME_OVER) {
            DrawText("游戏结束", screenWidth/2 - 100, 200, 45, RED);
            DrawText(TextFormat("波次: %d | 分数: %d", wave, score), 
                    screenWidth/2 - 120, 270, 22, DARKGRAY);
            DrawText("按 ENTER 返回", screenWidth/2 - 100, 330, 20, GRAY);
        }
        else if (state == WIN) {
            DrawText("胜利！", screenWidth/2 - 70, 200, 50, GREEN);
            DrawText(TextFormat("最终分数: %d", score), screenWidth/2 - 90, 270, 25, DARKGRAY);
            DrawText("按 ENTER 返回", screenWidth/2 - 100, 330, 20, GRAY);
        }
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
