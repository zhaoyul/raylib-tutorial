#pragma once
#include "raylib.h"
#include <string>
#include <vector>
#include <functional>

// Forward declare LevelFont
struct LevelFont;

// Git command console for direct git command input
class GitConsole {
public:
    GitConsole();
    ~GitConsole();
    
    void Initialize(int x, int y, int width, int height);
    void Update(float deltaTime);
    void Draw();
    
    // Set callback for command execution
    void SetCommandCallback(std::function<void(const std::string&)> callback);
    
    // Set font for Chinese text rendering
    void SetFont(const LevelFont* font);
    
    // Console visibility
    void Show();
    void Hide();
    void Toggle();
    bool IsVisible() const { return visible; }
    
    // Add output message
    void AddOutput(const std::string& message, Color color = WHITE);
    
    // Clear console
    void Clear();
    
private:
    Rectangle bounds;
    bool visible;
    
    // Input
    std::string inputBuffer;
    float cursorBlinkTimer;
    bool cursorVisible;
    
    // Output history
    struct Message {
        std::string text;
        Color color;
    };
    std::vector<Message> outputHistory;
    int scrollOffset;
    int maxHistoryLines;
    
    // Callback
    std::function<void(const std::string&)> commandCallback;
    
    // Font for Chinese text rendering
    const LevelFont* chineseFont;
    
    // Command history
    std::vector<std::string> commandHistory;
    int historyIndex;
    
    void ProcessInput();
    void ExecuteCommand(const std::string& cmd);
    void DrawInputLine();
    void DrawOutput();
    
    // Helper to draw text with Chinese support
    void DrawTextChinese(const char* text, int x, int y, int fontSize, Color color);
};
