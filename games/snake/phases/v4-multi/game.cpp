#include "game.h"
#include <cmath>

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
      gameMode(GameMode::SINGLE),
      state(GameState::MENU),
      score(0), score2(0), lives(MAX_LIVES), lives2(MAX_LIVES),
      highScore(0), targetScore(100),
      moveTimer(0), baseMoveInterval(0.15f),
      ownsFont(false), messageTimer(0),
      playerName(""), finalScore(0), finalLength(0),
      settingsSelection(0) {
    initWindow();
    initFont();
    
    AudioSystem::getInstance().init();
    AudioSystem::getInstance().generateDefaultSounds();
    
    settingsManager.load();
    settingsManager.applyToAudio();
    
    highScore = highScoreManager.getHighestScore();
    
    // 初始化关卡管理器
    levelManager = std::make_unique<LevelManager>();
    levelEditor = std::make_unique<LevelEditor>(GRID_SIZE);
}

Game::~Game() {
    settingsManager.save();
    
    if (ownsFont) {
        UnloadFont(uiFont);
    }
    
    CloseWindow();
}

void Game::initWindow() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "贪吃蛇 v4-multi - 双人模式");
    SetExitKey(KEY_ESCAPE);  // ESC 退出程序
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
    
    const char* allText =
        "0123456789 -:,.!?[]()%+*/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
        "贪吃蛇按开始游戏暂停继续结束分数长度生命道具操作方向键选择确认移动返回菜单设置高分榜音量音效音乐难度简单普通困难玩家输入你的名字删除保存并建议双人单人编辑对战模式关卡工具墙壁橡皮橡皮擦出生点未尺寸新随机生成关卡已撞失去一条耗尽吃到普通食物金色加速减速奖励目标静音暂无记录纪录最终平局获胜主当前切换使用自定义地图左右上下退出程序"
        "WASDENTERESCP1P2VSv4multiMuteSoundMusicVolumeEasyNormalHardEnterNamePlayerNewRecord";  // 包含所有可能用到的字符
    
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
    // 根据当前关卡数据初始化
    if (currentLevelData.spawnPoints.size() >= 1) {
        snake = std::make_unique<Snake>(
            static_cast<int>(currentLevelData.spawnPoints[0].x),
            static_cast<int>(currentLevelData.spawnPoints[0].y),
            GRID_WIDTH, GRID_HEIGHT);
    } else {
        snake = std::make_unique<Snake>(GRID_WIDTH / 2, GRID_HEIGHT / 2, GRID_WIDTH, GRID_HEIGHT);
    }
    
    // 双人模式初始化第二条蛇
    if (gameMode == GameMode::VERSUS) {
        int spawnX = GRID_WIDTH / 3;
        int spawnY = GRID_HEIGHT / 3;
        if (currentLevelData.spawnPoints.size() >= 2) {
            spawnX = static_cast<int>(currentLevelData.spawnPoints[1].x);
            spawnY = static_cast<int>(currentLevelData.spawnPoints[1].y);
        }

        snake2 = std::make_unique<Snake>(spawnX, spawnY, GRID_WIDTH, GRID_HEIGHT);
    } else {
        snake2.reset();
    }
    
    // 加载关卡墙壁
    obstacles.clear();

    for (const auto& wall : currentLevelData.walls) {
        obstacles.addObstacle(static_cast<int>(wall.x), static_cast<int>(wall.y));
    }

    if (obstacles.getCount() == 0) {
        obstacles.generate(5, *snake);
    }
    
    spawnItem();
    score = 0;
    score2 = 0;
    lives = MAX_LIVES;
    lives2 = MAX_LIVES;
    targetScore = currentLevelData.targetScore > 0 ? currentLevelData.targetScore : 100;
    moveTimer = 0;
    baseMoveInterval = 0.15f;
    speedEffect = SpeedEffect();
    message.clear();
    messageTimer = 0;
    particles.clear();
    screenShake = ScreenShake();
    playerName.clear();
    
    AudioSystem::getInstance().playBackgroundMusic();
}

void Game::reset() {
    snake.reset();
    snake2.reset();
    currentItem.reset();
    obstacles.clear();
    state = GameState::MENU;
    settingsSelection = 0;
    AudioSystem::getInstance().stopBackgroundMusic();
}

void Game::run() {
    while (isRunning()) {
        float deltaTime = GetFrameTime();
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
        case GameState::LEVEL_EDITOR:
            updateLevelEditor(deltaTime);
            break;
    }
    
    if (messageTimer > 0) {
        messageTimer -= deltaTime;
        if (messageTimer < 0) messageTimer = 0;
    }
}

void Game::updateMenu(float /* deltaTime */) {
    // 现在菜单有4个选项：单人、双人、编辑器、设置
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        settingsSelection = (settingsSelection + 1) % 5;
        AudioSystem::getInstance().play(SoundType::MENU_SELECT);
    }
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        settingsSelection = (settingsSelection + 4) % 5;
        AudioSystem::getInstance().play(SoundType::MENU_SELECT);
    }

    if (IsKeyPressed(KEY_LEFT)) {
        levelManager->prevLevel();
        AudioSystem::getInstance().play(SoundType::MENU_SELECT);
    }
    if (IsKeyPressed(KEY_RIGHT)) {
        levelManager->nextLevel();
        AudioSystem::getInstance().play(SoundType::MENU_SELECT);
    }
    
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        AudioSystem::getInstance().play(SoundType::PAUSE);
        switch (settingsSelection) {
            case 0:
                gameMode = GameMode::SINGLE;
                currentLevelData = levelManager->getCurrentLevel();
                init();
                state = GameState::PLAYING;
                break;
            case 1:
                gameMode = GameMode::VERSUS;
                currentLevelData = levelManager->getCurrentLevel();
                init();
                state = GameState::PLAYING;
                break;
            case 2:
                state = GameState::HIGH_SCORES;
                break;
            case 3:
                levelEditor->newLevel("新关卡", GRID_WIDTH, GRID_HEIGHT);
                state = GameState::LEVEL_EDITOR;
                break;
            case 4:
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
    
    // 玩家1输入
    snake->updateDirection();

    // 玩家2输入（对战模式）
    if (gameMode == GameMode::VERSUS && snake2) {
        snake2->updateDirectionPlayer2();
    }
    
    moveTimer += deltaTime;
    float interval = getCurrentMoveInterval();
    
    if (moveTimer >= interval) {
        moveTimer = 0;
        updateSnakeMovement(*snake, score, lives, 1);

        if (state != GameState::PLAYING) {
            return;
        }

        if (gameMode == GameMode::VERSUS && snake2) {
            updateSnakeMovement(*snake2, score2, lives2, 2);

            if (state != GameState::PLAYING) {
                return;
            }
            
            // 检查对战结束
            if (score >= targetScore || score2 >= targetScore) {
                state = GameState::GAME_OVER;
                finalScore = score;
                finalLength = snake->getLength();
                AudioSystem::getInstance().stopBackgroundMusic();
            }
        }
    }
}

void Game::updateSnakeMovement(Snake& snakeRef, int& playerScore, int& playerLives, int playerId) {
    (void)playerScore;

    auto resetSnakeForPlayer = [&]() {
        int respawnX = (playerId == 1) ? GRID_WIDTH / 2 : GRID_WIDTH / 3;
        int respawnY = (playerId == 1) ? GRID_HEIGHT / 2 : GRID_HEIGHT / 3;

        if (currentLevelData.spawnPoints.size() >= static_cast<size_t>(playerId)) {
            const Vector2& spawn = currentLevelData.spawnPoints[playerId - 1];
            respawnX = static_cast<int>(spawn.x);
            respawnY = static_cast<int>(spawn.y);
        }

        if (playerId == 1 && snake) {
            snake->reset(respawnX, respawnY);
        } else if (playerId == 2 && snake2) {
            snake2->reset(respawnX, respawnY);
        }
    };

    auto handleLifeLoss = [&](const char* lifeLossMsg) {
        if (playerLives > 0) {
            playerLives--;
        }

        if (playerLives <= 0) {
            playerLives = 0;
            state = GameState::GAME_OVER;
            finalScore = score;
            finalLength = snake ? snake->getLength() : 0;
            AudioSystem::getInstance().stopBackgroundMusic();
            AudioSystem::getInstance().play(SoundType::GAME_OVER);
            showMessage(playerId == 1 ? "P1 生命耗尽!" : "P2 生命耗尽!");
            return;
        }

        resetSnakeForPlayer();
        showMessage(lifeLossMsg);
    };

    bool alive = snakeRef.move();
    Position newHead = snakeRef.getHead();
    
    if (!alive) {
        screenShake.start(10.0f, 0.3f);
        AudioSystem::getInstance().play(SoundType::COLLISION);
        
        Vector2 hitPos = {newHead.x * GRID_SIZE + GRID_SIZE / 2.0f, newHead.y * GRID_SIZE + GRID_SIZE / 2.0f};
        particles.emitExplosion(hitPos, (playerId == 1) ? BLUE : RED, 50);
        handleLifeLoss(playerId == 1 ? "P1 失去一条生命!" : "P2 失去一条生命!");
        return;
    }
    
    if (obstacles.checkCollision(newHead.x, newHead.y)) {
        screenShake.start(10.0f, 0.3f);
        AudioSystem::getInstance().play(SoundType::COLLISION);
        handleLifeLoss(playerId == 1 ? "撞墙了! 失去一条生命!" : "P2 撞墙了!");
        return;
    }
    
    if (currentItem && newHead.x == currentItem->getX() && newHead.y == currentItem->getY()) {
        currentItem->onEat(snakeRef, *this);
        showMessage((playerId == 1 ? "P1 " : "P2 ") + std::string("吃到") + currentItem->getName() + "!");
        
        switch (currentItem->getType()) {
            case ItemType::NORMAL: AudioSystem::getInstance().play(SoundType::EAT_NORMAL); break;
            case ItemType::GOLDEN: AudioSystem::getInstance().play(SoundType::EAT_GOLDEN); break;
            case ItemType::SPEED_UP: AudioSystem::getInstance().play(SoundType::EAT_SPEED); break;
            case ItemType::SLOW_DOWN: AudioSystem::getInstance().play(SoundType::EAT_SLOW); break;
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
    particles.emitTrail(trailPos, Fade((playerId == 1) ? GREEN : ORANGE, 0.5f));
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

void Game::updateLevelEditor(float /* deltaTime */) {
    levelEditor->updateLayout(SCREEN_WIDTH, SCREEN_HEIGHT);
    levelEditor->update();
    
    // Ctrl+S 保存
    if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_LEFT_SUPER)) && IsKeyPressed(KEY_S)) {
        LevelData data = levelEditor->getLevel();
        std::string filename = "level_" + std::to_string(time(nullptr)) + ".json";
        if (levelManager->saveLevel(data, filename)) {
            levelEditor->markSaved();
            levelManager->loadAllLevels();
            levelManager->setCurrentLevel(levelManager->getLevelCount() - 1);
            showMessage("关卡已保存!");
        }
    }
    
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
        state = GameState::MENU;
        settingsSelection = 0;
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
        } else if (state == GameState::LEVEL_EDITOR) {
            state = GameState::MENU;
            settingsSelection = 0;
        }
    }
}

void Game::handleSettingsInput() {
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
            case 0:
                s.masterVolume = fmaxf(0.0f, s.masterVolume - 0.1f);
                audio.setMasterVolume(s.masterVolume);
                break;
            case 1:
                s.sfxVolume = fmaxf(0.0f, s.sfxVolume - 0.1f);
                audio.setSfxVolume(s.sfxVolume);
                break;
            case 2:
                s.musicVolume = fmaxf(0.0f, s.musicVolume - 0.1f);
                audio.setMusicVolume(s.musicVolume);
                break;
            case 3:
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
        case GameState::LEVEL_EDITOR:
            levelEditor->draw(SCREEN_WIDTH, SCREEN_HEIGHT, uiFont);
            break;
    }
    
    EndDrawing();
}

void Game::drawMenu() {
    auto drawTextCentered = [&](const char* text, float y, float size, Color color) {
        Vector2 sz = MeasureTextEx(uiFont, text, size, 1.0f);
        DrawTextEx(uiFont, text, {(SCREEN_WIDTH - sz.x) * 0.5f, y}, size, 1.0f, color);
    };
    
    drawTextCentered("贪吃蛇", 60, 60, DARKGREEN);
    drawTextCentered("v4-multi", 130, 30, GREEN);
    
    const char* options[] = {"单人模式", "双人对战", "高分榜", "关卡编辑器", "设置"};
    float startY = 200;
    float gap = 50;
    
    for (int i = 0; i < 5; i++) {
        Color color = (i == settingsSelection) ? DARKGREEN : GRAY;
        float size = (i == settingsSelection) ? 30 : 25;
        drawTextCentered(options[i], startY + i * gap, size, color);
    }

    const LevelData& selectedLevel = levelManager->getCurrentLevel();
    drawTextCentered(
        TextFormat("当前关卡: %s (%d/%d)", selectedLevel.name.c_str(),
                   levelManager->getCurrentIndex() + 1, levelManager->getLevelCount()),
        450, 20, DARKBLUE);
    
    if (highScore > 0) {
        drawTextCentered(TextFormat("最高分: %d", highScore), 480, 20, GOLD);
    }
    
    drawTextCentered("左右键切换关卡  |  上下键选择模式  |  ENTER 确认", 540, 16, DARKGRAY);
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
    
    // 绘制蛇（不同颜色）
    if (snake) snake->draw(GRID_SIZE);
    if (snake2) {
        // 临时修改颜色绘制第二条蛇
        for (size_t i = 0; i < snake2->getBody().size(); i++) {
            const auto& pos = snake2->getBody()[i];
            Color color = (i == 0) ? DARKBLUE : BLUE;
            int padding = (i == 0) ? 1 : 2;
            DrawRectangle(pos.x * GRID_SIZE + padding, pos.y * GRID_SIZE + padding,
                        GRID_SIZE - padding * 2, GRID_SIZE - padding * 2, color);
        }
    }
    
    drawUI();
    drawMessage();
    
    if (screenShake.isActive()) {
        EndScissorMode();
    }
    
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
    
    if (gameMode == GameMode::VERSUS) {
        drawTextCentered("对战结束", 140, 50, RED);
        drawTextCentered(TextFormat("P1 分数: %d", score), 210, 28, BLUE);
        drawTextCentered(TextFormat("P2 分数: %d", score2), 250, 28, RED);
        
        if (score > score2) {
            drawTextCentered("P1 获胜!", 310, 35, GOLD);
        } else if (score2 > score) {
            drawTextCentered("P2 获胜!", 310, 35, GOLD);
        } else {
            drawTextCentered("平局!", 310, 35, GOLD);
        }
    } else {
        drawTextCentered("游戏结束", 160, 50, RED);
        drawTextCentered(TextFormat("最终分数: %d", finalScore), 240, 30, WHITE);
        drawTextCentered(TextFormat("蛇的长度: %d", finalLength), 280, 25, LIGHTGRAY);
        
        if (finalScore == highScore && finalScore > 0) {
            drawTextCentered("新纪录!", 330, 30, GOLD);
        }
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
    
    drawVolumeBar("主音量", 100, y, 200, s.masterVolume, settingsSelection == 0);
    y += gap;
    drawVolumeBar("音效", 100, y, 200, s.sfxVolume, settingsSelection == 1);
    y += gap;
    drawVolumeBar("音乐", 100, y, 200, s.musicVolume, settingsSelection == 2);
    y += gap;
    
    const char* diffStr = (s.difficulty == Settings::Difficulty::EASY) ? "简单" :
                          (s.difficulty == Settings::Difficulty::NORMAL) ? "普通" : "困难";
    Color diffColor = (settingsSelection == 3) ? DARKGREEN : BLACK;
    DrawTextEx(uiFont, "难度", {100, y}, 25, 1.0f, diffColor);
    Vector2 valSize = MeasureTextEx(uiFont, diffStr, 25, 1.0f);
    DrawTextEx(uiFont, diffStr, {350 - valSize.x * 0.5f, y}, 25, 1.0f, diffColor);
    y += gap;
    
    Color backColor = (settingsSelection == 4) ? DARKGREEN : GRAY;
    drawTextCentered("保存并返回", 480, (settingsSelection == 4) ? 30 : 25, backColor);
    
    if (s.muted) {
        drawTextCentered("[静音]", 530, 20, RED);
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
            
            const char* rankText = TextFormat("%d.", static_cast<int>(i) + 1);
            DrawTextEx(uiFont, rankText, {150, y}, 25, 1.0f, color);
            DrawTextEx(uiFont, e.name.c_str(), {200, y}, 25, 1.0f, BLACK);
            
            const char* scoreText = TextFormat("%d", e.score);
            Vector2 scoreSize = MeasureTextEx(uiFont, scoreText, 25, 1.0f);
            DrawTextEx(uiFont, scoreText, {500 - scoreSize.x, y}, 25, 1.0f, color);
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
    
    float barX = x + 150;
    float barHeight = 20;
    DrawRectangle(static_cast<int>(barX), static_cast<int>(y + 5), static_cast<int>(width), static_cast<int>(barHeight), LIGHTGRAY);
    DrawRectangle(static_cast<int>(barX), static_cast<int>(y + 5), static_cast<int>(width * value), static_cast<int>(barHeight), barColor);
    
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
    if (gameMode == GameMode::VERSUS) {
        // 双人模式UI
        const char* p1Text = TextFormat("P1 分数: %d", score);
        DrawTextEx(uiFont, p1Text, {10.0f, 10.0f}, 22, 1.0f, BLUE);
        DrawTextEx(uiFont, TextFormat("生命: %d", lives), {10.0f, 40.0f}, 18, 1.0f, BLUE);
        
        const char* p2Text = TextFormat("P2 分数: %d", score2);
        Vector2 p2Size = MeasureTextEx(uiFont, p2Text, 22, 1.0f);
        DrawTextEx(uiFont, p2Text, {SCREEN_WIDTH - 10.0f - p2Size.x, 10.0f}, 22, 1.0f, RED);
        DrawTextEx(uiFont, TextFormat("生命: %d", lives2), {SCREEN_WIDTH - 80.0f, 40.0f}, 18, 1.0f, RED);
        
        // 目标分数
        const char* targetText = TextFormat("目标: %d", targetScore);
        Vector2 targetSize = MeasureTextEx(uiFont, targetText, 20, 1.0f);
        DrawTextEx(uiFont, targetText, {(SCREEN_WIDTH - targetSize.x) * 0.5f, 10.0f}, 20, 1.0f, GOLD);
    } else {
        // 单人模式UI
        const char* scoreText = TextFormat("分数: %d", score);
        DrawTextEx(uiFont, scoreText, {10.0f, 10.0f}, 25, 1.0f, DARKGRAY);
        
        const char* lenText = TextFormat("长度: %d", snake->getLength());
        Vector2 lenSz = MeasureTextEx(uiFont, lenText, 25, 1.0f);
        DrawTextEx(uiFont, lenText, {SCREEN_WIDTH - 10.0f - lenSz.x, 10.0f}, 25, 1.0f, DARKGRAY);
        
        drawLives();
    }
    
    if (speedEffect.active) {
        const char* speedText = TextFormat("%.1fx 速度", speedEffect.multiplier);
        Color speedColor = (speedEffect.multiplier < 1.0f) ? SKYBLUE : PURPLE;
        Vector2 speedSz = MeasureTextEx(uiFont, speedText, 20, 1.0f);
        DrawTextEx(uiFont, speedText, {(SCREEN_WIDTH - speedSz.x) * 0.5f, 40.0f}, 20, 1.0f, speedColor);
    }
    
    if (settingsManager.get().muted) {
        DrawTextEx(uiFont, "[静音]", {SCREEN_WIDTH * 0.5f - 30.0f, 70.0f}, 20, 1.0f, RED);
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
        
        if (snake2) {
            for (const auto& segment : snake2->getBody()) {
                if (segment.x == x && segment.y == y) {
                    validPosition = false;
                    break;
                }
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
