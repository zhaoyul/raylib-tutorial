#pragma once
#include "raylib.h"
#include "snake.h"
#include "item.h"
#include "obstacle.h"
#include "particle.h"
#include "screenshake.h"
#include "audio_system.h"
#include "highscore.h"
#include "settings.h"
#include <memory>
#include <string>

// 游戏状态
enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER,
    SETTINGS,       // 设置菜单
    HIGH_SCORES,    // 高分榜
    ENTER_NAME      // 输入名字
};

// 速度效果结构
struct SpeedEffect {
    float multiplier;   // 速度倍数
    float remaining;    // 剩余时间
    bool active;

    SpeedEffect() : multiplier(1.0f), remaining(0.0f), active(false) {}
};

// ============================================================
// Game 类 - 管理整个游戏状态和逻辑
// ============================================================
class Game {
private:
    // 游戏常量
    static constexpr int SCREEN_WIDTH = 800;
    static constexpr int SCREEN_HEIGHT = 600;
    static constexpr int GRID_SIZE = 20;
    static constexpr int GRID_WIDTH = SCREEN_WIDTH / GRID_SIZE;
    static constexpr int GRID_HEIGHT = SCREEN_HEIGHT / GRID_SIZE;
    static constexpr int MAX_LIVES = 3;
    static constexpr int LIVES_PER_EXTRA = 500;  // 每500分奖励生命

    // 游戏对象
    std::unique_ptr<Snake> snake;
    std::unique_ptr<Item> currentItem;
    ObstacleManager obstacles;
    ParticleSystem particles;    // 粒子系统
    ScreenShake screenShake;     // 屏幕震动

    // 游戏状态
    GameState state;
    int score;
    int lives;
    int highScore;

    // 移动控制
    float moveTimer;
    float baseMoveInterval;
    SpeedEffect speedEffect;

    // UI
    Font uiFont;
    bool ownsFont;

    // 消息提示
    std::string message;
    float messageTimer;

    // 高分榜和设置
    HighScoreManager highScoreManager;
    SettingsManager settingsManager;
    std::string playerName;
    int finalScore;
    int finalLength;

    // 设置菜单选项
    int settingsSelection;

public:
    Game();
    ~Game();

    // 主循环
    void run();

    // 游戏控制
    void init();
    void reset();
    void update(float deltaTime);
    void draw();

    // 状态查询
    bool isRunning() const { return !WindowShouldClose(); }

    // 道具效果
    void addScore(int points);
    void applySpeedEffect(float multiplier, float duration);
    void showMessage(const std::string& msg);

    // 获取常量
    static int getScreenWidth() { return SCREEN_WIDTH; }
    static int getScreenHeight() { return SCREEN_HEIGHT; }
    static int getGridSize() { return GRID_SIZE; }

private:
    // 初始化
    void initWindow();
    void initFont();
    void spawnItem();

    // 更新
    void updateMenu(float deltaTime);
    void updatePlaying(float deltaTime);
    void updatePaused(float deltaTime);
    void updateGameOver(float deltaTime);
    void updateSettings(float deltaTime);
    void updateHighScores(float deltaTime);
    void updateEnterName(float deltaTime);
    void updateSpeedEffect(float deltaTime);

    // 绘制
    void drawMenu();
    void drawPlaying();
    void drawPaused();
    void drawGameOver();
    void drawSettings();
    void drawHighScores();
    void drawEnterName();
    void drawGrid();
    void drawUI();
    void drawMessage();
    void drawLives();
    void drawVolumeBar(const char* label, float x, float y, float width, float value, bool selected);

    // 工具函数
    void checkExtraLife();
    float getCurrentMoveInterval() const;
    void saveHighScore();

    // 输入处理
    void handleInput();
    void handleSettingsInput();
};
