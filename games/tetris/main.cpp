#include "raylib.h"
#include <vector>

const int gridWidth = 10;
const int gridHeight = 20;
const int blockSize = 30;
const int offsetX = 50;
const int offsetY = 50;
const int screenWidth = offsetX*2 + gridWidth*blockSize;
const int screenHeight = offsetY*2 + gridHeight*blockSize;

enum GameState { MENU, PLAYING, GAME_OVER };

// 方块形状定义 (7种)
const int shapes[7][4][4] = {
    // I
    {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}},
    // O
    {{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}},
    // T
    {{0,0,0,0},{0,1,0,0},{1,1,1,0},{0,0,0,0}},
    // S
    {{0,0,0,0},{0,1,1,0},{1,1,0,0},{0,0,0,0}},
    // Z
    {{0,0,0,0},{1,1,0,0},{0,1,1,0},{0,0,0,0}},
    // J
    {{0,0,0,0},{1,0,0,0},{1,1,1,0},{0,0,0,0}},
    // L
    {{0,0,0,0},{0,0,1,0},{1,1,1,0},{0,0,0,0}}
};

Color shapeColors[7] = {SKYBLUE, YELLOW, PURPLE, GREEN, RED, BLUE, ORANGE};

struct Piece {
    int type;
    int rotation;
    int x, y;
};

bool grid[gridHeight][gridWidth] = {};
Color gridColors[gridHeight][gridWidth];

static inline int shapeCell(int type, int rotation, int i, int j) {
    // `shapes[type]` stores rotation 0. Rotate on-the-fly.
    rotation &= 3;
    switch (rotation) {
        case 0: return shapes[type][i][j];
        case 1: return shapes[type][3 - j][i];       // 90 deg clockwise
        case 2: return shapes[type][3 - i][3 - j];   // 180 deg
        case 3: return shapes[type][j][3 - i];       // 270 deg clockwise
    }
    return 0;
}

bool checkCollision(const Piece& piece) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (shapeCell(piece.type, piece.rotation, i, j)) {
                int newX = piece.x + j;
                int newY = piece.y + i;
                if (newX < 0 || newX >= gridWidth || newY >= gridHeight) return true;
                if (newY >= 0 && grid[newY][newX]) return true;
            }
        }
    }
    return false;
}

void placePiece(const Piece& piece) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (shapeCell(piece.type, piece.rotation, i, j)) {
                int newX = piece.x + j;
                int newY = piece.y + i;
                if (newY >= 0 && newY < gridHeight && newX >= 0 && newX < gridWidth) {
                    grid[newY][newX] = true;
                    gridColors[newY][newX] = shapeColors[piece.type];
                }
            }
        }
    }
}

int clearLines() {
    int linesCleared = 0;
    for (int i = gridHeight - 1; i >= 0; i--) {
        bool fullLine = true;
        for (int j = 0; j < gridWidth; j++) {
            if (!grid[i][j]) {
                fullLine = false;
                break;
            }
        }
        if (fullLine) {
            linesCleared++;
            // 移动所有行下移
            for (int k = i; k > 0; k--) {
                for (int j = 0; j < gridWidth; j++) {
                    grid[k][j] = grid[k-1][j];
                    gridColors[k][j] = gridColors[k-1][j];
                }
            }
            for (int j = 0; j < gridWidth; j++) {
                grid[0][j] = false;
            }
            i++; // 重新检查这一行
        }
    }
    return linesCleared;
}

int main() {
    InitWindow(screenWidth, screenHeight, "俄罗斯方块 Tetris");
    SetTargetFPS(60);

    Font uiFont = GetFontDefault();
    bool ownsUIFont = false;
    {
        const char* fontPath = "/System/Library/Fonts/Supplemental/Arial Unicode.ttf";
        // Only load glyphs we actually render (plus ASCII in the strings below).
        const char* allText =
            "0123456789 俄罗斯方块 TETRIS 按 ENTER 开始 左右: 移动 上: 旋转 下: 加速 游戏结束 分数: 0 按 ENTER 返回";

        int cpCount = 0;
        int* cps = LoadCodepoints(allText, &cpCount);
        Font f = LoadFontEx(fontPath, 32, cps, cpCount);
        UnloadCodepoints(cps);

        if (f.texture.id != 0) {
            uiFont = f;
            ownsUIFont = true;
            SetTextureFilter(uiFont.texture, TEXTURE_FILTER_BILINEAR);
        }
    }
    
    GameState state = MENU;
    Piece currentPiece;
    int score = 0;
    float fallTimer = 0;
    float fallSpeed = 0.5f;
    
    auto initGame = [&]() {
        for (int i = 0; i < gridHeight; i++) {
            for (int j = 0; j < gridWidth; j++) {
                grid[i][j] = false;
            }
        }
        currentPiece = {GetRandomValue(0, 6), 0, gridWidth/2 - 2, 0};
        score = 0;
        fallTimer = 0;
        fallSpeed = 0.5f;
    };
    
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        
        if (state == MENU) {
            if (IsKeyPressed(KEY_ENTER)) {
                initGame();
                state = PLAYING;
            }
        }
        else if (state == PLAYING) {
            // 输入处理
            if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
                Piece temp = currentPiece;
                temp.x--;
                if (!checkCollision(temp)) currentPiece = temp;
            }
            if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
                Piece temp = currentPiece;
                temp.x++;
                if (!checkCollision(temp)) currentPiece = temp;
            }
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
                Piece temp = currentPiece;
                temp.rotation++;
                if (!checkCollision(temp)) currentPiece = temp;
            }
            if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
                fallSpeed = 0.05f;
            } else {
                fallSpeed = 0.5f;
            }
            
            // 下落
            fallTimer += deltaTime;
            if (fallTimer >= fallSpeed) {
                fallTimer = 0;
                Piece temp = currentPiece;
                temp.y++;
                if (!checkCollision(temp)) {
                    currentPiece = temp;
                } else {
                    placePiece(currentPiece);
                    int lines = clearLines();
                    score += lines * 100;
                    
                    currentPiece = {GetRandomValue(0, 6), 0, gridWidth/2 - 2, 0};
                    if (checkCollision(currentPiece)) {
                        state = GAME_OVER;
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
            DrawTextEx(uiFont, "俄罗斯方块", {(float)screenWidth/2 - 100.0f, 150.0f}, 30, 1, DARKBLUE);
            DrawTextEx(uiFont, "TETRIS", {(float)screenWidth/2 - 60.0f, 190.0f}, 25, 1, BLUE);
            DrawTextEx(uiFont, "按 ENTER 开始", {(float)screenWidth/2 - 90.0f, 250.0f}, 20, 1, DARKGRAY);
            DrawTextEx(uiFont, "左右: 移动", {20.0f, 320.0f}, 18, 1, GRAY);
            DrawTextEx(uiFont, "上: 旋转", {20.0f, 350.0f}, 18, 1, GRAY);
            DrawTextEx(uiFont, "下: 加速", {20.0f, 380.0f}, 18, 1, GRAY);
        }
        else if (state == PLAYING) {
            // 绘制网格
            DrawRectangleLines(offsetX - 2, offsetY - 2, 
                             gridWidth * blockSize + 4, gridHeight * blockSize + 4, BLACK);
            
            // 绘制已放置的方块
            for (int i = 0; i < gridHeight; i++) {
                for (int j = 0; j < gridWidth; j++) {
                    if (grid[i][j]) {
                        DrawRectangle(offsetX + j * blockSize + 1, offsetY + i * blockSize + 1,
                                    blockSize - 2, blockSize - 2, gridColors[i][j]);
                    } else {
                        DrawRectangle(offsetX + j * blockSize + 1, offsetY + i * blockSize + 1,
                                    blockSize - 2, blockSize - 2, Fade(GRAY, 0.1f));
                    }
                }
            }
            
            // 绘制当前方块
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    if (shapeCell(currentPiece.type, currentPiece.rotation, i, j)) {
                        int drawX = offsetX + (currentPiece.x + j) * blockSize;
                        int drawY = offsetY + (currentPiece.y + i) * blockSize;
                        DrawRectangle(drawX + 1, drawY + 1, blockSize - 2, blockSize - 2,
                                    shapeColors[currentPiece.type]);
                    }
                }
            }
            
            // 显示分数
            DrawTextEx(uiFont, TextFormat("分数: %d", score), {10.0f, 10.0f}, 20, 1, DARKGRAY);
        }
        else if (state == GAME_OVER) {
            DrawTextEx(uiFont, "游戏结束", {(float)screenWidth/2 - 80.0f, 200.0f}, 35, 1, RED);
            DrawTextEx(uiFont, TextFormat("分数: %d", score), {(float)screenWidth/2 - 60.0f, 260.0f}, 25, 1, DARKGRAY);
            DrawTextEx(uiFont, "按 ENTER 返回", {(float)screenWidth/2 - 90.0f, 320.0f}, 20, 1, GRAY);
        }
        
        EndDrawing();
    }
    
    if (ownsUIFont) UnloadFont(uiFont);
    CloseWindow();
    return 0;
}
