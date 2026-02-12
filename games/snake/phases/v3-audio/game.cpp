#include "game.h"
#include <cmath>

// 颜色插值辅助函数
Color LerpColor(Color a, Color b, float t) {
    Color result;
    result.r = static_cast<unsigned char>(a.r + (b.r - a.r) * t);
    result.g = static_cast<unsigned char>(a.g + (b.g - a.g) * t);
    result.b = static_cast<unsigned char>(a.b + (b.b - a.b) * t);
    result.a = static_cast<unsigned char>(a.a + (b.a - a.a) * t);
    return result;
}

Game::Game()
    : obstacles(GRID_WIDTH, GRID_HEIGHT),
      state(GameState::MENU),
      score(0), lives(MAX_LIVES), highScore(0),
      moveTimer(0), baseMoveInterval(0.15f),
      ownsFont(false), messageTimer(0),
      playerName(""), finalScore(0), finalLength(0),
      settingsSelection(0) {
    initWindow();
    initFont();

    // 初始化音频系统
    AudioSystem::getInstance().init();
    AudioSystem::getInstance().generateDefaultSounds();

    // 加载设置
    settingsManager.load();
    settingsManager.applyToAudio();

    // 获取历史最高分
    highScore = highScoreManager.getHighestScore();
}

Game::~Game() {
    // 保存设置
    settingsManager.save();

    if (ownsFont) {
        UnloadFont(uiFont);
    }

    // 关闭窗口（音频会在 AudioSystem 析构时自动关闭）
    CloseWindow();
}

void Game::initWindow() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "贪吃蛇 v3-audio");
    SetTargetFPS(60);
}

void Game::initFont() {
    uiFont = GetFontDefault();

    const char* fontPaths[] = {
        "/System/Library/Fonts/Supplemental/Arial Unicode.ttf",
        "/System/Library/Fonts/PingFang.ttc",
        "/usr/share/fonts/truetype/wqy/wqy-zenhei.ttc",
        "C:/Windows/Fonts/simhei.ttf",
        nullptr
    };

    const char* allText = "0123456789 -:,贪吃蛇v3audio按ENTER开始游戏暂停继续结束分数长度生命道具操作方向键选择确认WASD移动P暂停ESC返回菜单设置高分榜音量音效音乐难度简单普通困难MuteSoundMusicVolumeEasyNormalHardEnterNamePlayerNewRecord玩家输入你的名字删除保存并建议";  // 包含所有可能用到的字符

    for (int i = 0; fontPaths[i] != nullptr; i++) {
        if (!FileExists(fontPaths[i])) continue;

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
}

void Game::init() {
    snake = std::make_unique<Snake>(GRID_WIDTH / 2, GRID_HEIGHT / 2, GRID_WIDTH, GRID_HEIGHT);
    obstacles.generate(5, *snake);
    spawnItem();
    score = 0;
    lives = MAX_LIVES;
    moveTimer = 0;
    baseMoveInterval = 0.15f;
    speedEffect = SpeedEffect();
    message.clear();
    messageTimer = 0;
    particles.clear();
    screenShake = ScreenShake();
    playerName.clear();

    // 播放背景音乐
    AudioSystem::getInstance().playBackgroundMusic();
}

void Game::reset() {
    snake.reset();
    currentItem.reset();
    obstacles.clear();
    state = GameState::MENU;
    AudioSystem::getInstance().stopBackgroundMusic();
}

void Game::run() {
    while (isRunning()) {
        float deltaTime = GetFrameTime();

        // 更新音频系统
        AudioSystem::getInstance().update();

        update(deltaTime);
        draw();
    }
}

void Game::update(float deltaTime) {
    handleInput();

    particles.update(deltaTime);
    screenShake.update(deltaTime);

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
        case GameState::SETTINGS:
            updateSettings(deltaTime);
            break;
        case GameState::HIGH_SCORES:
            updateHighScores(deltaTime);
            break;
        case GameState::ENTER_NAME:
            updateEnterName(deltaTime);
            break;
    }

    if (messageTimer > 0) {
        messageTimer -= deltaTime;
        if (messageTimer < 0) messageTimer = 0;
    }
}

void Game::updateMenu(float /* deltaTime */) {
    // 菜单选择
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        settingsSelection = (settingsSelection + 1) % 3;
        AudioSystem::getInstance().play(SoundType::MENU_SELECT);
    }
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        settingsSelection = (settingsSelection + 2) % 3;
        AudioSystem::getInstance().play(SoundType::MENU_SELECT);
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        AudioSystem::getInstance().play(SoundType::PAUSE);
        switch (settingsSelection) {
            case 0:
                init();
                state = GameState::PLAYING;
                break;
            case 1:
                state = GameState::HIGH_SCORES;
                break;
            case 2:
                settingsSelection = 0;
                state = GameState::SETTINGS;
                break;
        }
    }
}

void Game::updatePlaying(float deltaTime) {
    updateSpeedEffect(deltaTime);

    if (currentItem) {
        currentItem->update(deltaTime);
        if (currentItem->isExpired()) {
            spawnItem();
        }
    }

    snake->updateDirection();

    moveTimer += deltaTime;
    float interval = getCurrentMoveInterval();

    if (moveTimer >= interval) {
        moveTimer = 0;

        bool alive = snake->move();
        Position newHead = snake->getHead();

        if (!alive) {
            lives--;
            screenShake.start(10.0f, 0.3f);
            AudioSystem::getInstance().play(SoundType::COLLISION);

            Vector2 hitPos = {
                newHead.x * GRID_SIZE + GRID_SIZE / 2.0f,
                newHead.y * GRID_SIZE + GRID_SIZE / 2.0f
            };
            particles.emitExplosion(hitPos, RED, 50);

            if (lives <= 0) {
                state = GameState::GAME_OVER;
                finalScore = score;
                finalLength = snake->getLength();
                AudioSystem::getInstance().stopBackgroundMusic();
                AudioSystem::getInstance().play(SoundType::GAME_OVER);

                // 检查是否是高分
                if (highScoreManager.isHighScore(score)) {
                    state = GameState::ENTER_NAME;
                }
            } else {
                snake->reset(GRID_WIDTH / 2, GRID_HEIGHT / 2);
                showMessage("失去一条生命!");
            }
            return;
        }

        if (obstacles.checkCollision(newHead.x, newHead.y)) {
            lives--;
            screenShake.start(10.0f, 0.3f);
            AudioSystem::getInstance().play(SoundType::COLLISION);

            if (lives <= 0) {
                state = GameState::GAME_OVER;
                finalScore = score;
                finalLength = snake->getLength();
                AudioSystem::getInstance().stopBackgroundMusic();
                AudioSystem::getInstance().play(SoundType::GAME_OVER);

                if (highScoreManager.isHighScore(score)) {
                    state = GameState::ENTER_NAME;
                }
            } else {
                snake->reset(GRID_WIDTH / 2, GRID_HEIGHT / 2);
                showMessage("撞墙了! 失去一条生命!");
            }
            return;
        }

        if (currentItem && newHead.x == currentItem->getX() && newHead.y == currentItem->getY()) {
            currentItem->onEat(*snake, *this);
            showMessage("吃到" + currentItem->getName() + "! +" + std::to_string(currentItem->getScore()) + "分");

            // 播放对应音效
            switch (currentItem->getType()) {
                case ItemType::NORMAL:
                    AudioSystem::getInstance().play(SoundType::EAT_NORMAL);
                    break;
                case ItemType::GOLDEN:
                    AudioSystem::getInstance().play(SoundType::EAT_GOLDEN);
                    break;
                case ItemType::SPEED_UP:
                    AudioSystem::getInstance().play(SoundType::EAT_SPEED);
                    break;
                case ItemType::SLOW_DOWN:
                    AudioSystem::getInstance().play(SoundType::EAT_SLOW);
                    break;
            }

            Vector2 particlePos = {
                currentItem->getX() * GRID_SIZE + GRID_SIZE / 2.0f,
                currentItem->getY() * GRID_SIZE + GRID_SIZE / 2.0f
            };
            particles.emitExplosion(particlePos, currentItem->getColor(), 30);
            screenShake.start(3.0f, 0.1f);

            spawnItem();
            checkExtraLife();
        }

        Vector2 trailPos = {
            newHead.x * GRID_SIZE + GRID_SIZE / 2.0f,
            newHead.y * GRID_SIZE + GRID_SIZE / 2.0f
        };
        particles.emitTrail(trailPos, Fade(GREEN, 0.5f));
    }
}

void Game::updatePaused(float /* deltaTime */) {
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_SPACE)) {
        state = GameState::PLAYING;
        AudioSystem::getInstance().resumeBackgroundMusic();
    }
}

void Game::updateGameOver(float /* deltaTime */) {
    if (IsKeyPressed(KEY_ENTER)) {
        reset();
        state = GameState::MENU;
        settingsSelection = 0;
    }
}

void Game::updateSettings(float /* deltaTime */) {
    handleSettingsInput();
}

void Game::updateHighScores(float /* deltaTime */) {
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) {
        state = GameState::MENU;
        settingsSelection = 0;
    }
}

void Game::updateEnterName(float /* deltaTime */) {
    // 简单的名字输入
    int key = GetCharPressed();
    if (key >= 32 && key <= 125 && playerName.length() < 10) {
        playerName += static_cast<char>(key);
    }

    if (IsKeyPressed(KEY_BACKSPACE) && !playerName.empty()) {
        playerName.pop_back();
    }

    if (IsKeyPressed(KEY_ENTER) && !playerName.empty()) {
        HighScoreEntry entry(playerName, finalScore, finalLength);
        highScoreManager.addEntry(entry);
        highScore = highScoreManager.getHighestScore();
        state = GameState::GAME_OVER;
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
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_SPACE)) {
        if (state == GameState::PLAYING) {
            state = GameState::PAUSED;
            AudioSystem::getInstance().pauseBackgroundMusic();
            AudioSystem::getInstance().play(SoundType::PAUSE);
        } else if (state == GameState::PAUSED) {
            state = GameState::PLAYING;
            AudioSystem::getInstance().resumeBackgroundMusic();
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        if (state == GameState::PLAYING || state == GameState::PAUSED) {
            reset();
            state = GameState::MENU;
            settingsSelection = 0;
        } else if (state == GameState::SETTINGS || state == GameState::HIGH_SCORES) {
            state = GameState::MENU;
            settingsSelection = 0;
        }
    }
}

void Game::handleSettingsInput() {
    // 5个选项：主音量、音效、音乐、难度、返回
    const int numOptions = 5;

    if (IsKeyPressed(KEY_DOWN)) {
        settingsSelection = (settingsSelection + 1) % numOptions;
        AudioSystem::getInstance().play(SoundType::MENU_SELECT);
    }
    if (IsKeyPressed(KEY_UP)) {
        settingsSelection = (settingsSelection + numOptions - 1) % numOptions;
        AudioSystem::getInstance().play(SoundType::MENU_SELECT);
    }

    Settings& s = settingsManager.get();
    AudioSystem& audio = AudioSystem::getInstance();

    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
        switch (settingsSelection) {
            case 0: // 主音量
                s.masterVolume = fmaxf(0.0f, s.masterVolume - 0.1f);
                audio.setMasterVolume(s.masterVolume);
                break;
            case 1: // 音效
                s.sfxVolume = fmaxf(0.0f, s.sfxVolume - 0.1f);
                audio.setSfxVolume(s.sfxVolume);
                break;
            case 2: // 音乐
                s.musicVolume = fmaxf(0.0f, s.musicVolume - 0.1f);
                audio.setMusicVolume(s.musicVolume);
                break;
            case 3: // 难度
                if (s.difficulty == Settings::Difficulty::NORMAL)
                    s.difficulty = Settings::Difficulty::EASY;
                else if (s.difficulty == Settings::Difficulty::HARD)
                    s.difficulty = Settings::Difficulty::NORMAL;
                break;
        }
    }

    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
        switch (settingsSelection) {
            case 0:
                s.masterVolume = fminf(1.0f, s.masterVolume + 0.1f);
                audio.setMasterVolume(s.masterVolume);
                break;
            case 1:
                s.sfxVolume = fminf(1.0f, s.sfxVolume + 0.1f);
                audio.setSfxVolume(s.sfxVolume);
                break;
            case 2:
                s.musicVolume = fminf(1.0f, s.musicVolume + 0.1f);
                audio.setMusicVolume(s.musicVolume);
                break;
            case 3:
                if (s.difficulty == Settings::Difficulty::EASY)
                    s.difficulty = Settings::Difficulty::NORMAL;
                else if (s.difficulty == Settings::Difficulty::NORMAL)
                    s.difficulty = Settings::Difficulty::HARD;
                break;
        }
    }

    if (IsKeyPressed(KEY_ENTER) && settingsSelection == 4) {
        settingsManager.save();
        state = GameState::MENU;
        settingsSelection = 0;
    }

    if (IsKeyPressed(KEY_M)) {
        s.muted = !s.muted;
        audio.mute(s.muted);
    }
}

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
        case GameState::SETTINGS:
            drawSettings();
            break;
        case GameState::HIGH_SCORES:
            drawHighScores();
            break;
        case GameState::ENTER_NAME:
            drawEnterName();
            break;
    }

    EndDrawing();
}

void Game::drawMenu() {
    auto drawTextCentered = [&](const char* text, float y, float size, Color color) {
        Vector2 sz = MeasureTextEx(uiFont, text, size, 1.0f);
        DrawTextEx(uiFont, text, {(SCREEN_WIDTH - sz.x) * 0.5f, y}, size, 1.0f, color);
    };

    drawTextCentered("贪吃蛇", 80, 60, DARKGREEN);
    drawTextCentered("v3-audio", 150, 30, GREEN);

    const char* options[] = {"开始游戏", "高分榜", "设置"};
    float startY = 250;
    float gap = 50;

    for (int i = 0; i < 3; i++) {
        Color color = (i == settingsSelection) ? DARKGREEN : GRAY;
        float size = (i == settingsSelection) ? 30 : 25;
        drawTextCentered(options[i], startY + i * gap, size, color);
    }

    if (highScore > 0) {
        drawTextCentered(TextFormat("最高分: %d", highScore), 450, 20, GOLD);
    }

    drawTextCentered("操作: 方向键选择, ENTER 确认", 520, 16, DARKGRAY);
}

void Game::drawPlaying() {
    if (screenShake.isActive()) {
        Vector2 offset = screenShake.getOffset();
        BeginScissorMode(static_cast<int>(offset.x), static_cast<int>(offset.y), SCREEN_WIDTH, SCREEN_HEIGHT);
    }

    drawGrid();
    particles.draw();
    obstacles.draw(GRID_SIZE);
    if (currentItem) currentItem->draw(GRID_SIZE);
    snake->draw(GRID_SIZE);
    drawUI();
    drawMessage();

    if (screenShake.isActive()) {
        EndScissorMode();
    }

    // 显示 FPS
    if (settingsManager.get().showFPS) {
        DrawFPS(SCREEN_WIDTH - 80, SCREEN_HEIGHT - 30);
    }
}

void Game::drawPaused() {
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
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.7f));

    auto drawTextCentered = [&](const char* text, float y, float size, Color color) {
        Vector2 sz = MeasureTextEx(uiFont, text, size, 1.0f);
        DrawTextEx(uiFont, text, {(SCREEN_WIDTH - sz.x) * 0.5f, y}, size, 1.0f, color);
    };

    drawTextCentered("游戏结束", 160, 50, RED);
    drawTextCentered(TextFormat("最终分数: %d", finalScore), 240, 30, WHITE);
    drawTextCentered(TextFormat("蛇的长度: %d", finalLength), 280, 25, LIGHTGRAY);

    if (finalScore == highScore && finalScore > 0) {
        drawTextCentered("新纪录!", 330, 30, GOLD);
    }

    drawTextCentered("按 ENTER 返回菜单", 400, 20, LIGHTGRAY);
}

void Game::drawSettings() {
    auto drawTextCentered = [&](const char* text, float y, float size, Color color) {
        Vector2 sz = MeasureTextEx(uiFont, text, size, 1.0f);
        DrawTextEx(uiFont, text, {(SCREEN_WIDTH - sz.x) * 0.5f, y}, size, 1.0f, color);
    };

    drawTextCentered("设 置", 60, 50, DARKGREEN);

    Settings& s = settingsManager.get();
    float y = 150;
    float gap = 70;

    // 主音量
    drawVolumeBar("主音量", 100, y, 200, s.masterVolume, settingsSelection == 0);
    y += gap;

    // 音效音量
    drawVolumeBar("音效", 100, y, 200, s.sfxVolume, settingsSelection == 1);
    y += gap;

    // 音乐音量
    drawVolumeBar("音乐", 100, y, 200, s.musicVolume, settingsSelection == 2);
    y += gap;

    // 难度
    const char* diffStr = (s.difficulty == Settings::Difficulty::EASY) ? "简单" :
                          (s.difficulty == Settings::Difficulty::NORMAL) ? "普通" : "困难";
    Color diffColor = (settingsSelection == 3) ? DARKGREEN : BLACK;
    DrawTextEx(uiFont, "难度", {100, y}, 25, 1.0f, diffColor);
    Vector2 valSize = MeasureTextEx(uiFont, diffStr, 25, 1.0f);
    DrawTextEx(uiFont, diffStr, {350 - valSize.x * 0.5f, y}, 25, 1.0f, diffColor);
    y += gap;

    // 返回
    Color backColor = (settingsSelection == 4) ? DARKGREEN : GRAY;
    drawTextCentered("保存并返回", 480, (settingsSelection == 4) ? 30 : 25, backColor);

    // 静音提示
    if (s.muted) {
        drawTextCentered("[M] 静音", 530, 20, RED);
    } else {
        drawTextCentered("按 M 静音", 530, 16, DARKGRAY);
    }

    drawTextCentered("方向键调整, ENTER 确认", 560, 16, GRAY);
}

void Game::drawHighScores() {
    auto drawTextCentered = [&](const char* text, float y, float size, Color color) {
        Vector2 sz = MeasureTextEx(uiFont, text, size, 1.0f);
        DrawTextEx(uiFont, text, {(SCREEN_WIDTH - sz.x) * 0.5f, y}, size, 1.0f, color);
    };

    drawTextCentered("高分榜", 60, 50, GOLD);

    const auto& entries = highScoreManager.getEntries();
    float y = 130;

    if (entries.empty()) {
        drawTextCentered("暂无记录", 250, 25, GRAY);
    } else {
        for (size_t i = 0; i < entries.size() && i < 10; i++) {
            const auto& e = entries[i];
            Color color = (i < 3) ? GOLD : DARKGRAY;

            // 排名
            const char* rankText = TextFormat("%d.", static_cast<int>(i) + 1);
            DrawTextEx(uiFont, rankText, {150, y}, 25, 1.0f, color);

            // 名字
            DrawTextEx(uiFont, e.name.c_str(), {200, y}, 25, 1.0f, BLACK);

            // 分数
            const char* scoreText = TextFormat("%d", e.score);
            Vector2 scoreSize = MeasureTextEx(uiFont, scoreText, 25, 1.0f);
            DrawTextEx(uiFont, scoreText, {500 - scoreSize.x, y}, 25, 1.0f, color);

            // 日期
            DrawTextEx(uiFont, e.date.c_str(), {520, y}, 20, 1.0f, GRAY);

            y += 40;
        }
    }

    drawTextCentered("按 ENTER 或 ESC 返回", 550, 18, DARKGRAY);
}

void Game::drawEnterName() {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.8f));

    auto drawTextCentered = [&](const char* text, float y, float size, Color color) {
        Vector2 sz = MeasureTextEx(uiFont, text, size, 1.0f);
        DrawTextEx(uiFont, text, {(SCREEN_WIDTH - sz.x) * 0.5f, y}, size, 1.0f, color);
    };

    drawTextCentered("新纪录!", 150, 40, GOLD);
    drawTextCentered(TextFormat("分数: %d", finalScore), 210, 30, WHITE);
    drawTextCentered("输入你的名字:", 280, 25, LIGHTGRAY);

    // 显示输入的名字
    const char* nameText = playerName.empty() ? "_" : playerName.c_str();
    Vector2 nameSize = MeasureTextEx(uiFont, nameText, 40, 1.0f);
    DrawTextEx(uiFont, nameText, {(SCREEN_WIDTH - nameSize.x) * 0.5f, 330}, 40, 1.0f, WHITE);

    drawTextCentered("按 ENTER 确认", 420, 20, LIGHTGRAY);
    drawTextCentered("按 BACKSPACE 删除", 450, 16, GRAY);
}

void Game::drawVolumeBar(const char* label, float x, float y, float width, float value, bool selected) {
    Color labelColor = selected ? DARKGREEN : BLACK;
    Color barColor = selected ? GREEN : LIGHTGRAY;

    DrawTextEx(uiFont, label, {x, y}, 25, 1.0f, labelColor);

    // 绘制音量条背景
    float barX = x + 150;
    float barHeight = 20;
    DrawRectangle(static_cast<int>(barX), static_cast<int>(y + 5), static_cast<int>(width), static_cast<int>(barHeight), LIGHTGRAY);

    // 绘制音量值
    DrawRectangle(static_cast<int>(barX), static_cast<int>(y + 5), static_cast<int>(width * value), static_cast<int>(barHeight), barColor);

    // 绘制数值
    const char* valText = TextFormat("%d%%", static_cast<int>(value * 100));
    DrawTextEx(uiFont, valText, {barX + width + 10, y}, 20, 1.0f, labelColor);
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
    const char* scoreText = TextFormat("分数: %d", score);
    DrawTextEx(uiFont, scoreText, {10.0f, 10.0f}, 25, 1.0f, DARKGRAY);

    const char* lenText = TextFormat("长度: %d", snake->getLength());
    Vector2 lenSz = MeasureTextEx(uiFont, lenText, 25, 1.0f);
    DrawTextEx(uiFont, lenText, {(float)SCREEN_WIDTH - 10.0f - lenSz.x, 10.0f}, 25, 1.0f, DARKGRAY);

    drawLives();

    if (speedEffect.active) {
        const char* speedText = TextFormat("%.1fx 速度", speedEffect.multiplier);
        Color speedColor = (speedEffect.multiplier < 1.0f) ? SKYBLUE : PURPLE;
        Vector2 speedSz = MeasureTextEx(uiFont, speedText, 20, 1.0f);
        DrawTextEx(uiFont, speedText, {(SCREEN_WIDTH - speedSz.x) * 0.5f, 10.0f}, 20, 1.0f, speedColor);
    }

    // 静音指示
    if (settingsManager.get().muted) {
        DrawTextEx(uiFont, "[静音]", {SCREEN_WIDTH * 0.5f - 30.0f, 40.0f}, 20, 1.0f, RED);
    }
}

void Game::drawLives() {
    float x = 10.0f;
    float y = 45.0f;
    float size = 15.0f;

    DrawTextEx(uiFont, "生命:", {x, y}, 20, 1.0f, DARKGRAY);
    x += 50;

    for (int i = 0; i < lives; i++) {
        DrawCircle(static_cast<int>(x + i * (size + 5) + size/2), static_cast<int>(y + size/2 + 2), size/2, RED);
    }
}

void Game::drawMessage() {
    if (messageTimer <= 0 || message.empty()) return;

    float alpha = messageTimer / 2.0f;
    if (alpha > 1.0f) alpha = 1.0f;

    Vector2 sz = MeasureTextEx(uiFont, message.c_str(), 25, 1.0f);
    float x = (SCREEN_WIDTH - sz.x) * 0.5f;
    float y = SCREEN_HEIGHT * 0.7f;

    DrawTextEx(uiFont, message.c_str(), {x + 2, y + 2}, 25, 1.0f, Fade(BLACK, alpha * 0.5f));
    DrawTextEx(uiFont, message.c_str(), {x, y}, 25, 1.0f, Fade(GOLD, alpha));
}

void Game::spawnItem() {
    bool validPosition = false;
    int x, y;
    int attempts = 0;

    while (!validPosition && attempts < 100) {
        attempts++;
        x = GetRandomValue(0, GRID_WIDTH - 1);
        y = GetRandomValue(0, GRID_HEIGHT - 1);

        validPosition = true;

        for (const auto& segment : snake->getBody()) {
            if (segment.x == x && segment.y == y) {
                validPosition = false;
                break;
            }
        }

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
    messageTimer = 2.0f;
}

void Game::checkExtraLife() {
    static int lastLifeMilestone = 0;
    int currentMilestone = score / LIVES_PER_EXTRA;

    if (currentMilestone > lastLifeMilestone && lives < MAX_LIVES) {
        lives++;
        lastLifeMilestone = currentMilestone;
        showMessage("奖励生命!");
        AudioSystem::getInstance().play(SoundType::EXTRA_LIFE);
    }
}

float Game::getCurrentMoveInterval() const {
    return baseMoveInterval * speedEffect.multiplier;
}
