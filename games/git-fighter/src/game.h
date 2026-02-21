#pragma once
#include "raylib.h"
#include "ui_manager.h"
#include "git_console.h"
#include <memory>
#include <string>

class LevelManager;

// Chinese font support
struct GameFont {
    Font chineseFont;
    bool hasChineseFont;

    void Load();
    void Unload();
};

// Game states
enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    LEVEL_COMPLETE,
    GAME_OVER
};

// Main game class
class GitGame {
public:
    GitGame();
    ~GitGame();

    bool Initialize();
    void Run();
    void Shutdown();

    // State management
    void ChangeState(GameState newState);
    GameState GetState() const { return currentState; }

    // Level management
    void LoadLevel(int levelNum);
    void CompleteCurrentLevel();
    int GetCurrentLevel() const { return currentLevel; }

private:
    void Update(float deltaTime);
    void Draw();
    void DrawMenu();
    void DrawHUD();

    void DrawChineseText(const char* text, int x, int y, int fontSize, Color color);
    
    // Get mouse offset for game area centering
    Vector2 GetGameMouseOffset() const;
    Vector2 ScreenToGameMouse(Vector2 screenPos) const;

    GameState currentState;
    int currentLevel;
    bool shouldQuit;
    bool levelCompleteShown;  // Track if level is complete but waiting for Next click

    std::unique_ptr<LevelManager> levelManager;
    std::unique_ptr<UIManager> uiManager;
    GitConsole gitConsole;
    GameFont gameFont;

    // Render target for scaling (disabled)
    RenderTexture2D renderTarget;
    bool useRenderTarget;

    // Window settings
    static constexpr int SCREEN_WIDTH = 1280;
    static constexpr int SCREEN_HEIGHT = 720;
    static constexpr const char* TITLE = "Git Fighter - 救火架构师";
};
