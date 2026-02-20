#pragma once
#include "raylib.h"
#include <memory>
#include <vector>
#include <string>

// Forward declarations
class GitWrapper;

// Chinese font support for levels
struct LevelFont {
    Font chineseFont;
    bool hasChineseFont = false;
    
    void Load();
    void Unload();
    void DrawChinese(const char* text, int x, int y, int fontSize, Color color) const;
};

// Base class for all levels
class Level {
public:
    Level(int id, const std::string& name, const std::string& desc);
    virtual ~Level() = default;
    
    // Lifecycle
    virtual void Initialize() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Draw() = 0;
    virtual void Shutdown() = 0;
    
    // Check if level objectives are complete
    virtual bool IsComplete() const = 0;
    
    // Font setter
    void SetFont(const LevelFont* f) { font = f; }
    
    // Getters
    int GetId() const { return levelId; }
    const std::string& GetName() const { return levelName; }
    const std::string& GetDescription() const { return description; }
    
protected:
    void DrawChinese(const char* text, int x, int y, int fontSize, Color color) const {
        if (font) font->DrawChinese(text, x, y, fontSize, color);
        else DrawText(text, x, y, fontSize, color);
    }
    
    int levelId;
    std::string levelName;
    std::string description;
    std::string dialogueCTO;
    std::string dialoguePlayer;
    const LevelFont* font = nullptr;
};

// Level Manager
class LevelManager {
public:
    LevelManager();
    ~LevelManager();
    
    bool Initialize();
    void RegisterLevel(std::unique_ptr<Level> level);
    
    void LoadLevel(int levelId);
    void UnloadCurrentLevel();
    
    void Update(float deltaTime);
    void Draw();
    
    Level* GetCurrentLevel() const { return currentLevel.get(); }
    bool IsCurrentLevelComplete() const;
    
    int GetTotalLevels() const { return levels.size(); }
    
    // Font access for levels
    const LevelFont& GetFont() const { return font; }
    
private:
    std::vector<std::unique_ptr<Level>> levels;
    std::unique_ptr<Level> currentLevel;
    std::unique_ptr<GitWrapper> git;
    LevelFont font;
};
