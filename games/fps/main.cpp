#include "raylib.h"
#include "raymath.h"
#include <vector>

const int screenWidth = 800;
const int screenHeight = 600;

enum GameState { MENU, PLAYING, GAME_OVER };

struct Target {
    Vector3 position;
    float size;
    bool active;
    Color color;
};

int main() {
    InitWindow(screenWidth, screenHeight, "第一人称射击 FPS");
    SetTargetFPS(60);
    
    GameState state = MENU;
    
    // 设置相机
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 2.0f, 0.0f };
    camera.target = (Vector3){ 0.0f, 2.0f, 1.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
    std::vector<Target> targets;
    int score = 0;
    float gameTimer = 60.0f;
    
    auto initGame = [&]() {
        targets.clear();
        // 创建目标
        for (int i = 0; i < 10; i++) {
            Target t;
            t.position = (Vector3){
                (float)GetRandomValue(-20, 20),
                (float)GetRandomValue(1, 4),
                (float)GetRandomValue(5, 30)
            };
            t.size = 1.0f;
            t.active = true;
            t.color = RED;
            targets.push_back(t);
        }
        score = 0;
        gameTimer = 60.0f;
        camera.position = (Vector3){ 0.0f, 2.0f, 0.0f };
        camera.target = (Vector3){ 0.0f, 2.0f, 1.0f };
    };
    
    initGame();
    
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        
        if (state == MENU) {
            if (IsKeyPressed(KEY_ENTER)) {
                initGame();
                state = PLAYING;
                DisableCursor();
            }
        }
        else if (state == PLAYING) {
            // 更新相机
            UpdateCamera(&camera, CAMERA_FIRST_PERSON);
            
            // 射击检测
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Ray ray = GetScreenToWorldRay({(float)screenWidth/2, (float)screenHeight/2}, camera);
                
                for (auto& target : targets) {
                    if (target.active) {
                        BoundingBox box = {
                            {target.position.x - target.size/2, target.position.y - target.size/2, target.position.z - target.size/2},
                            {target.position.x + target.size/2, target.position.y + target.size/2, target.position.z + target.size/2}
                        };
                        
                        RayCollision collision = GetRayCollisionBox(ray, box);
                        if (collision.hit) {
                            target.active = false;
                            score += 10;
                            break;
                        }
                    }
                }
            }
            
            // 更新计时器
            gameTimer -= deltaTime;
            if (gameTimer <= 0) {
                state = GAME_OVER;
                EnableCursor();
            }
            
            // 检查是否所有目标被击中
            bool allHit = true;
            for (const auto& target : targets) {
                if (target.active) {
                    allHit = false;
                    break;
                }
            }
            if (allHit) {
                state = GAME_OVER;
                EnableCursor();
            }
            
            if (IsKeyPressed(KEY_ESCAPE)) {
                state = MENU;
                EnableCursor();
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
            DrawText("第一人称射击", screenWidth/2 - 150, 150, 50, DARKBLUE);
            DrawText("FPS GAME", screenWidth/2 - 80, 210, 30, BLUE);
            DrawText("按 ENTER 开始", screenWidth/2 - 100, 280, 20, DARKGRAY);
            DrawText("WASD: 移动 | 鼠标: 瞄准", screenWidth/2 - 140, 330, 20, GRAY);
            DrawText("左键: 射击 | ESC: 暂停", screenWidth/2 - 140, 360, 20, GRAY);
        }
        else if (state == PLAYING) {
            BeginMode3D(camera);
            
            // 绘制地面
            DrawPlane((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector2){ 50.0f, 50.0f }, Fade(GREEN, 0.5f));
            DrawGrid(50, 1.0f);
            
            // 绘制目标
            for (const auto& target : targets) {
                if (target.active) {
                    DrawCube(target.position, target.size, target.size, target.size, target.color);
                    DrawCubeWires(target.position, target.size, target.size, target.size, MAROON);
                }
            }
            
            // 绘制一些环境物体
            DrawCube((Vector3){-5, 2, 10}, 2, 4, 2, GRAY);
            DrawCube((Vector3){5, 2, 15}, 2, 4, 2, GRAY);
            DrawCube((Vector3){0, 1, 25}, 3, 2, 3, DARKGRAY);
            
            EndMode3D();
            
            // 绘制准星
            DrawLine(screenWidth/2 - 10, screenHeight/2, screenWidth/2 + 10, screenHeight/2, RED);
            DrawLine(screenWidth/2, screenHeight/2 - 10, screenWidth/2, screenHeight/2 + 10, RED);
            
            // UI
            DrawRectangle(0, 0, screenWidth, 40, Fade(BLACK, 0.7f));
            DrawText(TextFormat("分数: %d", score), 10, 10, 20, WHITE);
            DrawText(TextFormat("时间: %.1f", gameTimer), screenWidth/2 - 50, 10, 20, YELLOW);
            
            int activeTargets = 0;
            for (const auto& t : targets) if (t.active) activeTargets++;
            DrawText(TextFormat("目标: %d", activeTargets), screenWidth - 130, 10, 20, RED);
        }
        else if (state == GAME_OVER) {
            bool won = gameTimer > 0;
            if (won) {
                DrawText("完成！", screenWidth/2 - 80, 200, 50, GREEN);
                DrawText(TextFormat("剩余时间: %.1f 秒", gameTimer), screenWidth/2 - 120, 270, 22, DARKGRAY);
            } else {
                DrawText("时间到！", screenWidth/2 - 90, 200, 50, RED);
            }
            DrawText(TextFormat("最终分数: %d", score), screenWidth/2 - 90, 310, 25, DARKGRAY);
            DrawText("按 ENTER 返回菜单", screenWidth/2 - 120, 360, 20, GRAY);
        }
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
