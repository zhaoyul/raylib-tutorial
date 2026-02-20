#pragma once
#include "raylib.h"
#include <string>
#include <vector>

// Command input system
class CommandInput {
public:
    CommandInput();

    void Update();
    void Draw(int x, int y, int width, int height);

    std::string GetCommand() const { return currentCommand; }
    void Clear();
    bool IsSubmitted() const { return submitted; }
    void ResetSubmission() { submitted = false; }

private:
    std::string currentCommand;
    bool submitted;
    float cursorTimer;
    bool showCursor;
};

// UI Manager
class UIManager {
public:
    UIManager();
    ~UIManager();

    void Initialize();
    void Update(float deltaTime);
    void Draw();

    // Dialogue system
    void ShowDialogue(const std::string& speaker, const std::string& text);
    void HideDialogue();
    bool IsDialogueVisible() const;

    // Command input
    CommandInput& GetCommandInput() { return commandInput; }

private:
    CommandInput commandInput;
    bool dialogueVisible;
    std::string dialogueSpeaker;
    std::string dialogueText;
};
