#include "git_console.h"
#include "level_manager.h"
#include <algorithm>
#include <cstring>

GitConsole::GitConsole()
    : bounds{0, 0, 800, 300}
    , visible(false)
    , cursorBlinkTimer(0)
    , cursorVisible(true)
    , scrollOffset(0)
    , maxHistoryLines(100)
    , historyIndex(-1)
{
}

GitConsole::~GitConsole() = default;

void GitConsole::Initialize(int x, int y, int width, int height) {
    bounds = {(float)x, (float)y, (float)width, (float)height};
    visible = false;
    inputBuffer.clear();
    outputHistory.clear();
    scrollOffset = 0;
    
    // Add welcome message
    AddOutput("Git Console - 输入 git 命令直接操作", {100, 200, 255, 255});
    AddOutput("例如: git init, git add ., git commit -m \"message\"", {150, 150, 150, 255});
    AddOutput("按 TAB 或点击按钮打开/关闭控制台", {150, 150, 150, 255});
    AddOutput("---", GRAY);
}

void GitConsole::SetCommandCallback(std::function<void(const std::string&)> callback) {
    commandCallback = callback;
}

void GitConsole::SetFont(const LevelFont* font) {
    chineseFont = font;
}

void GitConsole::Show() {
    visible = true;
}

void GitConsole::Hide() {
    visible = false;
}

void GitConsole::Toggle() {
    visible = !visible;
}

void GitConsole::AddOutput(const std::string& message, Color color) {
    outputHistory.push_back({message, color});
    
    // Limit history size
    if (outputHistory.size() > maxHistoryLines) {
        outputHistory.erase(outputHistory.begin());
    }
    
    // Auto-scroll to bottom
    int visibleLines = (int)(bounds.height - 60) / 20;
    if (outputHistory.size() > visibleLines) {
        scrollOffset = outputHistory.size() - visibleLines;
    }
}

void GitConsole::Clear() {
    outputHistory.clear();
    scrollOffset = 0;
}

void GitConsole::Update(float deltaTime) {
    if (!visible) {
        // Toggle with TAB key even when hidden
        if (IsKeyPressed(KEY_TAB)) {
            Toggle();
        }
        return;
    }
    
    // Update cursor blink
    cursorBlinkTimer += deltaTime;
    if (cursorBlinkTimer > 0.5f) {
        cursorBlinkTimer = 0;
        cursorVisible = !cursorVisible;
    }
    
    // Handle toggle
    if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_ESCAPE)) {
        Hide();
        return;
    }
    
    // Handle keyboard input
    int key = GetCharPressed();
    while (key > 0) {
        if (key >= 32 && key <= 126) {
            inputBuffer += (char)key;
        }
        key = GetCharPressed();
    }
    
    // Backspace
    if (IsKeyPressed(KEY_BACKSPACE) && !inputBuffer.empty()) {
        inputBuffer.pop_back();
    }
    
    // Enter - execute command
    if (IsKeyPressed(KEY_ENTER)) {
        ExecuteCommand(inputBuffer);
        inputBuffer.clear();
    }
    
    // History navigation
    if (IsKeyPressed(KEY_UP)) {
        if (historyIndex < (int)commandHistory.size() - 1) {
            historyIndex++;
            inputBuffer = commandHistory[commandHistory.size() - 1 - historyIndex];
        }
    }
    if (IsKeyPressed(KEY_DOWN)) {
        if (historyIndex > 0) {
            historyIndex--;
            inputBuffer = commandHistory[commandHistory.size() - 1 - historyIndex];
        } else if (historyIndex == 0) {
            historyIndex = -1;
            inputBuffer.clear();
        }
    }
    
    // Scroll with mouse wheel
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        scrollOffset -= (int)wheel;
        if (scrollOffset < 0) scrollOffset = 0;
        int maxScroll = std::max(0, (int)outputHistory.size() - 10);
        if (scrollOffset > maxScroll) scrollOffset = maxScroll;
    }
}

void GitConsole::ExecuteCommand(const std::string& cmd) {
    if (cmd.empty()) return;
    
    // Add to history
    AddOutput("> " + cmd, {100, 200, 255, 255});
    commandHistory.push_back(cmd);
    historyIndex = -1;
    
    // Execute callback
    if (commandCallback) {
        commandCallback(cmd);
    }
}

void GitConsole::DrawTextChinese(const char* text, int x, int y, int fontSize, Color color) {
    if (chineseFont && chineseFont->hasChineseFont) {
        chineseFont->DrawChinese(text, x, y, fontSize, color);
    } else {
        DrawText(text, x, y, fontSize, color);
    }
}

void GitConsole::Draw() {
    if (!visible) {
        // Draw small toggle button when hidden
        int screenWidth = GetScreenWidth();
        Rectangle toggleBtn = {(float)(screenWidth - 120), 60, 110, 30};
        DrawRectangleRec(toggleBtn, {40, 44, 52, 200});
        DrawRectangleLines((int)toggleBtn.x, (int)toggleBtn.y, (int)toggleBtn.width, (int)toggleBtn.height, {100, 150, 200, 255});
        DrawText("[TAB] Console", (int)toggleBtn.x + 5, (int)toggleBtn.y + 8, 14, WHITE);
        
        // Check click on button
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), toggleBtn)) {
            Show();
        }
        return;
    }
    
    // Background
    DrawRectangleRec(bounds, {20, 20, 30, 240});
    DrawRectangleLines((int)bounds.x, (int)bounds.y, (int)bounds.width, (int)bounds.height, {100, 150, 200, 255});
    
    // Title bar
    DrawRectangle((int)bounds.x, (int)bounds.y, (int)bounds.width, 30, {40, 44, 52, 255});
    DrawTextChinese("Git Console", (int)bounds.x + 10, (int)bounds.y + 7, 18, WHITE);
    DrawTextChinese("[TAB] 关闭", (int)(bounds.x + bounds.width - 100), (int)bounds.y + 7, 14, GRAY);
    
    // Output area
    DrawOutput();
    
    // Input line
    DrawInputLine();
}

void GitConsole::DrawOutput() {
    int lineHeight = 18;
    int startY = (int)bounds.y + 35;
    int maxLines = (int)(bounds.height - 65) / lineHeight;
    
    // Draw output lines
    for (int i = 0; i < maxLines && (scrollOffset + i) < (int)outputHistory.size(); i++) {
        int idx = scrollOffset + i;
        const auto& msg = outputHistory[idx];
        int y = startY + i * lineHeight;
        DrawTextChinese(msg.text.c_str(), (int)bounds.x + 10, y, 14, msg.color);
    }
    
    // Scrollbar indicator
    if (outputHistory.size() > maxLines) {
        float scrollBarHeight = bounds.height - 65;
        float thumbHeight = scrollBarHeight * maxLines / outputHistory.size();
        float thumbY = bounds.y + 35 + (scrollOffset * scrollBarHeight / outputHistory.size());
        DrawRectangle((int)(bounds.x + bounds.width - 8), (int)thumbY, 4, (int)thumbHeight, {100, 100, 120, 200});
    }
}

void GitConsole::DrawInputLine() {
    int inputY = (int)(bounds.y + bounds.height - 30);
    
    // Input background
    DrawRectangle((int)bounds.x, inputY, (int)bounds.width, 30, {30, 30, 40, 255});
    DrawLine((int)bounds.x, inputY, (int)(bounds.x + bounds.width), inputY, {100, 150, 200, 255});
    
    // Prompt
    DrawTextChinese("> ", (int)bounds.x + 10, inputY + 6, 16, {100, 200, 100, 255});
    
    // Input text
    DrawTextChinese(inputBuffer.c_str(), (int)bounds.x + 30, inputY + 6, 16, WHITE);
    
    // Cursor
    if (cursorVisible) {
        int textWidth = MeasureText(inputBuffer.c_str(), 16);
        DrawLine((int)bounds.x + 30 + textWidth, inputY + 4, (int)bounds.x + 30 + textWidth, inputY + 22, WHITE);
    }
}
