#include "ui_manager.h"
#include <cstring>

CommandInput::CommandInput() : submitted(false), cursorTimer(0), showCursor(true) {}

void CommandInput::Update() {
    // Handle keyboard input
    int key = GetKeyPressed();
    while (key > 0) {
        if (key == KEY_ENTER) {
            submitted = true;
        } else if (key == KEY_BACKSPACE) {
            if (!currentCommand.empty()) {
                currentCommand.pop_back();
            }
        } else if (key >= 32 && key < 127) {
            currentCommand += (char)key;
        }
        key = GetKeyPressed();
    }

    // Cursor blink
    cursorTimer += GetFrameTime();
    if (cursorTimer > 0.5f) {
        showCursor = !showCursor;
        cursorTimer = 0;
    }
}

void CommandInput::Draw(int x, int y, int width, int height) {
    // Background
    DrawRectangle(x, y, width, height, (Color){40, 44, 52, 255});
    DrawRectangleLines(x, y, width, height, (Color){100, 150, 200, 255});

    // Prompt
    DrawText("$ ", x + 10, y + 10, 24, (Color){100, 200, 100, 255});

    // Command text
    DrawText(currentCommand.c_str(), x + 40, y + 10, 24, WHITE);

    // Cursor
    if (showCursor) {
        int textWidth = MeasureText(currentCommand.c_str(), 24);
        DrawRectangle(x + 40 + textWidth, y + 10, 12, 24, WHITE);
    }

    // Hint
    DrawText("输入 git 命令 (或按快捷键)", x + 10, y + height - 20, 16, GRAY);
}

void CommandInput::Clear() {
    currentCommand.clear();
    submitted = false;
}

UIManager::UIManager() : dialogueVisible(false) {}

UIManager::~UIManager() = default;

void UIManager::Initialize() {
    // Initialize UI resources
}

void UIManager::Update(float deltaTime) {
    commandInput.Update();
}

void UIManager::Draw() {
    if (dialogueVisible) {
        // Draw dialogue box
        DrawRectangle(0, 580, 1280, 140, (Color){30, 30, 40, 240});
        DrawRectangleLines(0, 580, 1280, 140, (Color){100, 150, 200, 255});

        // Speaker name
        DrawText(dialogueSpeaker.c_str(), 80, 600, 22, (Color){100, 200, 255, 255});

        // Dialogue text
        DrawText(dialogueText.c_str(), 150, 630, 24, WHITE);

        // Continue hint
        DrawText("按 [空格] 继续...", 1100, 690, 16, LIGHTGRAY);
    }
}

void UIManager::ShowDialogue(const std::string& speaker, const std::string& text) {
    dialogueSpeaker = speaker;
    dialogueText = text;
    dialogueVisible = true;
}

void UIManager::HideDialogue() {
    dialogueVisible = false;
}

bool UIManager::IsDialogueVisible() const {
    return dialogueVisible;
}
