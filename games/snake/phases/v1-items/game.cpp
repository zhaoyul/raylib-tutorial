#include "game.h"
#include <cmath>

// ============================================================
// 构造函数和析构函数
// ============================================================
Game::Game()
    : obstacles(GRID_WIDTH, GRID_HEIGHT),
      state(GameState::MENU),
      score(0), lives(MAX_LIVES), highScore(0),
      moveTimer(0), baseMoveInterval(0.15f),
      ownsFont(false), messageTimer(0) {
    initWindow();
    initFont();
}

Game::~Game() {
    if (ownsFont) {
        UnloadFont(uiFont);
    }
    CloseWindow();
}

// ============================================================
// 初始化
// ============================================================
void Game::initWindow() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "贪吃蛇 v1-items");
    SetTargetFPS(60);
}

void Game::initFont() {
    uiFont = GetFontDefault();
    
    // 尝试加载系统字体（支持中文）
    const char* fontPaths[] = {
        // macOS
        "/System/Library/Fonts/Supplemental/Arial Unicode.ttf",
        "/System/Library/Fonts/Supplemental/Arial Unicode.ttc",
        "/System/Library/Fonts/PingFang.ttc",
        "/System/Library/Fonts/STHeiti Light.ttc",
        "/System/Library/Fonts/STHeiti Medium.ttc",
        "/Library/Fonts/Arial Unicode.ttf",
        // Linux
        "/usr/share/fonts/truetype/wqy/wqy-zenhei.ttc",
        "/usr/share/fonts/truetype/wqy/wqy-microhei.ttc",
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        // Windows
        "C:/Windows/Fonts/simhei.ttf",
        "C:/Windows/Fonts/msyh.ttc",
        "C:/Windows/Fonts/msgothic.ttc",
        nullptr
    };
    
    // 需要显示的所有字符（必须包含所有可能用到的字符）
    const char* allText = 
        // 英文和数字
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 .,:!-"
        // 游戏界面
        "贪吃蛇v1items道具与障碍按ENTER或空格开始游戏暂停继续结束分数长度生命"
        // 操作提示
        "操作方向键WASD移动P暂停ESC返回菜单"
        // 道具名称
        "普通食物金色加速减速"
        // 消息提示
        "吃到失去一条撞墙奖励生命ExtraLifeGotNormalGoldenSpeedUpSlowDownHitobstacleLostlifepointsSpeed";
    
    for (int i = 0; fontPaths[i] != nullptr; i++) {
        // 检查文件是否存在
        if (!FileExists(fontPaths[i])) {
            continue;
        }
        
        int cpCount = 0;
        int* cps = LoadCodepoints(allText, &cpCount);
        Font f = LoadFontEx(fontPaths[i], 64, cps, cpCount);
        UnloadCodepoints(cps);
        
        if (f.texture.id != 0) {
            uiFont = f;
            ownsFont = true;
            SetTextureFilter(uiFont.texture, TEXTURE_FILTER_BILINEAR);
            TraceLog(LOG_INFO, "Font loaded: %s", fontPaths[i]);
            return;
        }
    }
    
    // 如果没有找到合适的字体，使用默认字体（英文界面）
    TraceLog(LOG_WARNING, "No Chinese font found, using default font");
    uiFont = GetFontDefault();
    ownsFont = false;
}

void Game::init() {
    snake = std::make_unique<Snake>(GRID_WIDTH / 2, GRID_HEIGHT / 2, GRID_WIDTH, GRID_HEIGHT);
    obstacles.generate(5, *snake);  // 生成5个障碍物
    spawnItem();
    score = 0;
    lives = MAX_LIVES;
    moveTimer = 0;
    baseMoveInterval = 0.15f;
    speedEffect = SpeedEffect();
    message.clear();
    messageTimer = 0;
}

void Game::reset() {
    snake.reset();
    currentItem.reset();
    obstacles.clear();
    state = GameState::MENU;
}

// ============================================================
// 主循环
// ============================================================
void Game::run() {
    while (isRunning()) {
        float deltaTime = GetFrameTime();
        update(deltaTime);
        draw();
    }
}

// ============================================================
// 更新
// ============================================================
void Game::update(float deltaTime) {
    handleInput();
    
    switch (state) {
        case GameState::MENU:
            updateMenu(deltaTime);
            break;
        case GameState::PLAYING:
            updatePlaying(deltaTime);
            break;
        case GameState::PAUSED:
            updatePaused(deltaTime);
            break;
        case GameState::GAME_OVER:
            updateGameOver(deltaTime);
            break;
    }
    
    // 更新消息提示
    if (messageTimer > 0) {
        messageTimer -= deltaTime;
        if (messageTimer < 0) messageTimer = 0;
    }
}

void Game::updateMenu(float /* deltaTime */) {
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        init();
        state = GameState::PLAYING;
    }
}

void Game::updatePlaying(float deltaTime) {
    // 更新速度效果
    updateSpeedEffect(deltaTime);
    
    // 更新道具生命周期
    if (currentItem) {
        currentItem->update(deltaTime);
        if (currentItem->isExpired()) {
            spawnItem();
        }
    }
    
    // 更新蛇的方向
    snake->updateDirection();
    
    // 移动计时
    moveTimer += deltaTime;
    float interval = getCurrentMoveInterval();
    
    if (moveTimer >= interval) {
        moveTimer = 0;
        
        // 尝试移动
        bool alive = snake->move();
        Position newHead = snake->getHead();
        
        if (!alive) {
            // 撞到墙壁或自身
            lives--;
            if (lives <= 0) {
                state = GameState::GAME_OVER;
                if (score > highScore) {
                    highScore = score;
                }
            } else {
                // 重置蛇的位置
                snake->reset(GRID_WIDTH / 2, GRID_HEIGHT / 2);
                showMessage("失去一条生命!");
            }
            return;
        }
        
        // 检查障碍物碰撞
        if (obstacles.checkCollision(newHead.x, newHead.y)) {
            lives--;
            if (lives <= 0) {
                state = GameState::GAME_OVER;
                if (score > highScore) {
                    highScore = score;
                }
            } else {
                snake->reset(GRID_WIDTH / 2, GRID_HEIGHT / 2);
                showMessage("撞墙了! 失去一条生命!");
            }
            return;
        }
        
        // 检查是否吃到道具
        if (currentItem && newHead.x == currentItem->getX() && newHead.y == currentItem->getY()) {
            // 应用道具效果
            currentItem->onEat(*snake, *this);
            showMessage("吃到" + currentItem->getName() + "! +" + std::to_string(currentItem->getScore()) + "分");
            spawnItem();
            
            // 检查额外生命奖励
            checkExtraLife();
        }
    }
}

void Game::updatePaused(float /* deltaTime */) {
    // 暂停时只等待继续
}

void Game::updateGameOver(float /* deltaTime */) {
    if (IsKeyPressed(KEY_ENTER)) {
        reset();
        state = GameState::MENU;
    }
}

void Game::updateSpeedEffect(float deltaTime) {
    if (speedEffect.active) {
        speedEffect.remaining -= deltaTime;
        if (speedEffect.remaining <= 0) {
            speedEffect.active = false;
            speedEffect.multiplier = 1.0f;
        }
    }
}

void Game::handleInput() {
    // 暂停键
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_SPACE)) {
        if (state == GameState::PLAYING) {
            state = GameState::PAUSED;
        } else if (state == GameState::PAUSED) {
            state = GameState::PLAYING;
        }
    }
    
    // ESC 返回菜单
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (state == GameState::PLAYING || state == GameState::PAUSED) {
            reset();
            state = GameState::MENU;
        }
    }
}

// ============================================================
// 绘制
// ============================================================
void Game::draw() {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    switch (state) {
        case GameState::MENU:
            drawMenu();
            break;
        case GameState::PLAYING:
            drawPlaying();
            break;
        case GameState::PAUSED:
            drawPlaying();
            drawPaused();
            break;
        case GameState::GAME_OVER:
            drawPlaying();
            drawGameOver();
            break;
    }
    
    EndDrawing();
}

void Game::drawMenu() {
    auto drawTextCentered = [&](const char* text, float y, float size, Color color) {
        Vector2 sz = MeasureTextEx(uiFont, text, size, 1.0f);
        DrawTextEx(uiFont, text, {(SCREEN_WIDTH - sz.x) * 0.5f, y}, size, 1.0f, color);
    };
    
    drawTextCentered("贪吃蛇", 120, 60, DARKGREEN);
    drawTextCentered("v1-items", 190, 30, GREEN);
    drawTextCentered("道具与障碍", 240, 20, DARKGRAY);
    drawTextCentered("按 ENTER 或 空格 开始游戏", 300, 20, DARKGRAY);
    drawTextCentered("操作: 方向键或 WASD 移动, P/空格 暂停", 340, 16, GRAY);
    
    if (highScore > 0) {
        drawTextCentered(TextFormat("最高分: %d", highScore), 400, 20, GOLD);
    }
}

void Game::drawPlaying() {
    // 绘制网格背景
    drawGrid();
    
    // 绘制障碍物
    obstacles.draw(GRID_SIZE);
    
    // 绘制道具
    if (currentItem) {
        currentItem->draw(GRID_SIZE);
    }
    
    // 绘制蛇
    snake->draw(GRID_SIZE);
    
    // 绘制UI
    drawUI();
    drawMessage();
}

void Game::drawPaused() {
    // 半透明遮罩
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.5f));
    
    auto drawTextCentered = [&](const char* text, float y, float size, Color color) {
        Vector2 sz = MeasureTextEx(uiFont, text, size, 1.0f);
        DrawTextEx(uiFont, text, {(SCREEN_WIDTH - sz.x) * 0.5f, y}, size, 1.0f, color);
    };
    
    drawTextCentered("暂 停", 250, 50, WHITE);
    drawTextCentered("按 P 或 空格 继续", 320, 20, LIGHTGRAY);
    drawTextCentered("按 ESC 返回菜单", 360, 16, GRAY);
}

void Game::drawGameOver() {
    // 半透明遮罩
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.7f));
    
    auto drawTextCentered = [&](const char* text, float y, float size, Color color) {
        Vector2 sz = MeasureTextEx(uiFont, text, size, 1.0f);
        DrawTextEx(uiFont, text, {(SCREEN_WIDTH - sz.x) * 0.5f, y}, size, 1.0f, color);
    };
    
    drawTextCentered("游戏结束", 180, 50, RED);
    drawTextCentered(TextFormat("最终分数: %d", score), 260, 30, WHITE);
    drawTextCentered(TextFormat("蛇的长度: %d", snake->getLength()), 300, 25, LIGHTGRAY);
    
    if (score == highScore && score > 0) {
        drawTextCentered("新纪录!", 350, 30, GOLD);
    }
    
    drawTextCentered("按 ENTER 返回菜单", 420, 20, LIGHTGRAY);
}

void Game::drawGrid() {
    for (int i = 0; i < GRID_WIDTH; i++) {
        for (int j = 0; j < GRID_HEIGHT; j++) {
            Color color = ((i + j) % 2 == 0) ? Fade(GREEN, 0.1f) : Fade(GREEN, 0.05f);
            DrawRectangle(i * GRID_SIZE, j * GRID_SIZE, GRID_SIZE, GRID_SIZE, color);
        }
    }
}

void Game::drawUI() {
    // 分数
    const char* scoreText = TextFormat("分数: %d", score);
    DrawTextEx(uiFont, scoreText, {10.0f, 10.0f}, 25, 1.0f, DARKGRAY);
    
    // 长度
    const char* lenText = TextFormat("长度: %d", snake->getLength());
    Vector2 lenSz = MeasureTextEx(uiFont, lenText, 25, 1.0f);
    DrawTextEx(uiFont, lenText, {(float)SCREEN_WIDTH - 10.0f - lenSz.x, 10.0f}, 25, 1.0f, DARKGRAY);
    
    // 生命
    drawLives();
    
    // 速度效果指示
    if (speedEffect.active) {
        const char* speedText = TextFormat("%.1fx 速度", speedEffect.multiplier);
        Color speedColor = (speedEffect.multiplier < 1.0f) ? SKYBLUE : PURPLE;
        Vector2 speedSz = MeasureTextEx(uiFont, speedText, 20, 1.0f);
        DrawTextEx(uiFont, speedText, {(SCREEN_WIDTH - speedSz.x) * 0.5f, 10.0f}, 20, 1.0f, speedColor);
    }
}

void Game::drawLives() {
    float x = 10.0f;
    float y = 45.0f;
    float size = 15.0f;
    
    DrawTextEx(uiFont, "生命:", {x, y}, 20, 1.0f, DARKGRAY);
    x += 50;
    
    for (int i = 0; i < lives; i++) {
        // 绘制心形（简化为圆形）
        DrawCircle(x + i * (size + 5) + size/2, y + size/2 + 2, size/2, RED);
    }
}

void Game::drawMessage() {
    if (messageTimer <= 0 || message.empty()) return;
    
    float alpha = messageTimer / 2.0f; // 2秒淡出
    if (alpha > 1.0f) alpha = 1.0f;
    
    Vector2 sz = MeasureTextEx(uiFont, message.c_str(), 25, 1.0f);
    float x = (SCREEN_WIDTH - sz.x) * 0.5f;
    float y = SCREEN_HEIGHT * 0.7f;
    
    // 阴影
    DrawTextEx(uiFont, message.c_str(), {x + 2, y + 2}, 25, 1.0f, Fade(BLACK, alpha * 0.5f));
    // 文字
    DrawTextEx(uiFont, message.c_str(), {x, y}, 25, 1.0f, Fade(GOLD, alpha));
}

// ============================================================
// 工具函数
// ============================================================
void Game::spawnItem() {
    bool validPosition = false;
    int x, y;
    int attempts = 0;
    
    while (!validPosition && attempts < 100) {
        attempts++;
        x = GetRandomValue(0, GRID_WIDTH - 1);
        y = GetRandomValue(0, GRID_HEIGHT - 1);
        
        validPosition = true;
        
        // 检查是否与蛇重叠
        for (const auto& segment : snake->getBody()) {
            if (segment.x == x && segment.y == y) {
                validPosition = false;
                break;
            }
        }
        
        // 检查是否与障碍物重叠
        if (validPosition && obstacles.checkCollision(x, y)) {
            validPosition = false;
        }
    }
    
    if (validPosition) {
        currentItem = ItemFactory::createWeightedItem(x, y);
    }
}

void Game::addScore(int points) {
    score += points;
    
    // 加速效果
    if (baseMoveInterval > 0.05f) {
        baseMoveInterval *= 0.98f;
    }
}

void Game::applySpeedEffect(float multiplier, float duration) {
    speedEffect.multiplier = multiplier;
    speedEffect.remaining = duration;
    speedEffect.active = true;
    
    if (multiplier < 1.0f) {
        showMessage("加速!");
    } else {
        showMessage("减速!");
    }
}

void Game::showMessage(const std::string& msg) {
    message = msg;
    messageTimer = 2.0f; // 显示2秒
}

void Game::checkExtraLife() {
    static int lastLifeMilestone = 0;
    int currentMilestone = score / LIVES_PER_EXTRA;
    
    if (currentMilestone > lastLifeMilestone && lives < MAX_LIVES) {
        lives++;
        lastLifeMilestone = currentMilestone;
        showMessage("奖励生命!");
    }
}

float Game::getCurrentMoveInterval() const {
    return baseMoveInterval * speedEffect.multiplier;
}
