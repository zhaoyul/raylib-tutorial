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

    Font uiFont = GetFontDefault();
    bool ownsUIFont = false;
    {
        const char* fontPath = "/System/Library/Fonts/Supplemental/Arial Unicode.ttf";
        const char* allText =
            "0123456789 第一人称射击 按 ENTER 开始 WASD: 移动 | 鼠标: 瞄准 左键: 射击 | ESC: 暂停 分数: 时间: 0.0 目标: 完成！ 剩余时间: 0.0 秒 时间到！ 最终分数: 按 ENTER 返回菜单";

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
            DrawTextCentered("第一人称射击", 150, 50, DARKBLUE);
            DrawText("FPS GAME", screenWidth/2 - 80, 210, 30, BLUE);
            DrawTextCentered("按 ENTER 开始", 280, 20, DARKGRAY);
            DrawTextEx(uiFont, "WASD: 移动 | 鼠标: 瞄准",
                       {(float)screenWidth/2 - 140.0f, 330.0f}, 20, 1.0f, GRAY);
            DrawTextEx(uiFont, "左键: 射击 | ESC: 暂停",
                       {(float)screenWidth/2 - 140.0f, 360.0f}, 20, 1.0f, GRAY);
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
            DrawTextEx(uiFont, TextFormat("分数: %d", score), {10.0f, 10.0f}, 20, 1.0f, WHITE);
            DrawTextEx(uiFont, TextFormat("时间: %.1f", gameTimer),
                       {(float)screenWidth/2 - 50.0f, 10.0f}, 20, 1.0f, YELLOW);

            int activeTargets = 0;
            for (const auto& t : targets) if (t.active) activeTargets++;
            const char* tgtText = TextFormat("目标: %d", activeTargets);
            Vector2 tgtSz = MeasureTextEx(uiFont, tgtText, 20, 1.0f);
            DrawTextEx(uiFont, tgtText, {(float)screenWidth - 10.0f - tgtSz.x, 10.0f}, 20, 1.0f, RED);
        }
        else if (state == GAME_OVER) {
            bool won = gameTimer > 0;
            if (won) {
                DrawTextCentered("完成！", 200, 50, GREEN);
                DrawTextCentered(TextFormat("剩余时间: %.1f 秒", gameTimer), 270, 22, DARKGRAY);
            } else {
                DrawTextCentered("时间到！", 200, 50, RED);
            }
            DrawTextCentered(TextFormat("最终分数: %d", score), 310, 25, DARKGRAY);
            DrawTextCentered("按 ENTER 返回菜单", 360, 20, GRAY);
        }

        EndDrawing();
    }

    if (ownsUIFont) UnloadFont(uiFont);
    CloseWindow();
    return 0;
}
