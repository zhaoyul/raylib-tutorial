#pragma once
#include "raylib.h"
#include <string>
#include <vector>
#include <functional>
#include <memory>

// Modern GUI styling for Git Fighter
namespace GitGUI {

// Color scheme
struct Theme {
    Color background = {30, 35, 45, 255};
    Color panelBg = {40, 44, 52, 255};
    Color panelBorder = {100, 150, 200, 255};
    Color primary = {100, 200, 255, 255};
    Color primaryActive = {80, 160, 200, 255};
    Color secondary = {150, 220, 255, 200};
    Color secondaryHover = {170, 230, 255, 230};
    Color success = {100, 255, 150, 255};
    Color warning = {255, 220, 100, 255};
    Color error = {255, 100, 100, 255};
    Color textPrimary = {255, 255, 255, 255};
    Color textSecondary = {180, 180, 180, 255};
    Color buttonBg = {60, 70, 90, 255};
    Color buttonHover = {80, 100, 130, 255};
    Color buttonActive = {100, 150, 200, 255};
    Color inputBg = {50, 55, 65, 255};
    Color highlight = {255, 220, 100, 100};
};

// Button component
class GuiButton {
public:
    Rectangle bounds;
    std::string text;
    bool enabled = true;
    bool visible = true;
    
    GuiButton(const Rectangle& rect, const std::string& txt);
    void Draw(const Theme& theme);
    bool IsHovered() const;
    bool IsClicked() const;
};

// Panel component with scroll support
class Panel {
public:
    Rectangle bounds;
    std::string title;
    bool hasBorder = true;
    
    Panel(const Rectangle& rect, const std::string& ttl);
    void Draw(const Theme& theme);
    void BeginScissor();
    void EndScissor();
};

// Progress bar
class ProgressBar {
public:
    Rectangle bounds;
    float progress = 0.0f; // 0.0 to 1.0
    std::string label;
    
    ProgressBar(const Rectangle& rect, const std::string& lbl);
    void Draw(const Theme& theme);
};

// Step indicator for multi-step processes
class StepIndicator {
public:
    std::vector<std::string> steps;
    int currentStep = 0;
    int x, y, width;
    
    StepIndicator(int xPos, int yPos, int w);
    void SetSteps(const std::vector<std::string>& stepNames);
    void Draw(const Theme& theme);
};

// Command palette (modern command input)
class CommandPalette {
public:
    Rectangle bounds;
    std::string input;
    std::vector<std::string> suggestions;
    bool isActive = false;
    
    CommandPalette(const Rectangle& rect);
    void Update();
    void Draw(const Theme& theme);
    std::string GetCommand();
    void Clear();
    
private:
    bool submitted = false;
};

// Info card for displaying git objects
class InfoCard {
public:
    Rectangle bounds;
    std::string title;
    std::vector<std::pair<std::string, std::string>> fields;
    
    InfoCard(const Rectangle& rect, const std::string& ttl);
    void AddField(const std::string& label, const std::string& value);
    void Draw(const Theme& theme);
    void Clear();
};

// Main GUI Manager
class GUIManager {
public:
    GUIManager();
    ~GUIManager();
    
    void Initialize();
    void Update(float deltaTime);
    void Draw();
    
    // Layout helpers
    void BeginLeftPanel();
    void EndLeftPanel();
    void BeginRightPanel();
    void EndRightPanel();
    void BeginBottomBar();
    void EndBottomBar();
    
    // Widget creation helpers
    bool Button(const std::string& text, int x, int y, int width, int height);
    bool IconButton(const std::string& icon, const std::string& tooltip, int x, int y);
    void Label(const std::string& text, int x, int y, int fontSize, Color color);
    void Title(const std::string& text, int x, int y);
    void Subtitle(const std::string& text, int x, int y);
    
    // Git-specific widgets
    void GitCommandButton(const std::string& cmd, const std::string& desc, int x, int y, bool enabled);
    void StatusBadge(const std::string& text, int x, int y, Color bgColor);
    void FileItem(const std::string& filename, const std::string& status, int x, int y, bool selected);
    
    // Dialogue overlay
    void ShowDialogue(const std::string& speaker, const std::string& text);
    void HideDialogue();
    bool IsDialogueVisible() const;
    
    // Accessors
    CommandPalette& GetCommandPalette() { return commandPalette; }
    const Theme& GetTheme() const { return theme; }
    
    // Chinese font support
    void SetChineseFont(Font* font) { chineseFont = font; hasChineseFont = (font != nullptr); }
    
private:
    Theme theme;
    CommandPalette commandPalette;
    
    // Dialogue state
    bool dialogueVisible = false;
    std::string dialogueSpeaker;
    std::string dialogueText;
    float dialogueTimer = 0;
    
    // Panel bounds
    Rectangle leftPanelBounds;
    Rectangle rightPanelBounds;
    Rectangle bottomBarBounds;
    
    // Font
    Font* chineseFont = nullptr;
    bool hasChineseFont = false;
    
    void DrawDialogue();
    void DrawChineseText(const char* text, int x, int y, int fontSize, Color color);
};

} // namespace GitGUI
