#include "gui_manager.h"
#include <algorithm>
#include <iostream>

namespace GitGUI {

// Helper to darken/lighten colors
static Color AdjustColor(Color c, int delta) {
    return {
        (unsigned char)std::clamp(c.r + delta, 0, 255),
        (unsigned char)std::clamp(c.g + delta, 0, 255),
        (unsigned char)std::clamp(c.b + delta, 0, 255),
        c.a
    };
}

// GuiButton implementation
GuiButton::GuiButton(const Rectangle& rect, const std::string& txt) 
    : bounds(rect), text(txt) {}

void GuiButton::Draw(const Theme& theme) {
    if (!visible) return;
    
    // Calculate button colors from theme
    Color btnBg = theme.secondary;
    Color btnHover = theme.secondaryHover;
    Color btnActive = theme.primaryActive;
    Color textColor = theme.textPrimary;
    
    Color bgColor = btnBg;
    if (!enabled) {
        bgColor = {bgColor.r, bgColor.g, bgColor.b, 100};
    } else if (IsHovered()) {
        bgColor = btnHover;
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            bgColor = btnActive;
        }
    }
    
    // Draw button background with rounded corners
    DrawRectangleRounded(bounds, 0.15f, 8, bgColor);
    
    // Draw border
    if (IsHovered() && enabled) {
        DrawRectangleRoundedLines(bounds, 0.15f, 8, theme.primary);
    }
    
    // Draw text
    int fontSize = 18;
    int textWidth = MeasureText(text.c_str(), fontSize);
    int textX = bounds.x + (bounds.width - textWidth) / 2;
    int textY = bounds.y + (bounds.height - fontSize) / 2;
    DrawText(text.c_str(), textX, textY, fontSize, enabled ? textColor : theme.textSecondary);
}

bool GuiButton::IsHovered() const {
    return CheckCollisionPointRec(GetMousePosition(), bounds);
}

bool GuiButton::IsClicked() const {
    return IsHovered() && enabled && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

// Panel implementation
Panel::Panel(const Rectangle& rect, const std::string& ttl) 
    : bounds(rect), title(ttl) {}

void Panel::Draw(const Theme& theme) {
    // Panel background
    DrawRectangleRounded(bounds, 0.05f, 8, theme.panelBg);
    
    // Border
    if (hasBorder) {
        DrawRectangleRoundedLines(bounds, 0.05f, 8, theme.panelBorder);
    }
    
    // Title bar
    Rectangle titleBar = {bounds.x, bounds.y, bounds.width, 35};
    Color titleBg = AdjustColor(theme.panelBg, 10);
    DrawRectangleRounded(titleBar, 0.05f, 8, titleBg);
    
    // Title text
    DrawText(title.c_str(), bounds.x + 15, bounds.y + 8, 20, theme.primary);
}

void Panel::BeginScissor() {
    BeginScissorMode((int)bounds.x, (int)bounds.y + 35, (int)bounds.width, (int)bounds.height - 35);
}

void Panel::EndScissor() {
    EndScissorMode();
}

// ProgressBar implementation
ProgressBar::ProgressBar(const Rectangle& rect, const std::string& lbl) 
    : bounds(rect), label(lbl), progress(0.0f) {}

void ProgressBar::Draw(const Theme& theme) {
    // Background
    Color inputBg = AdjustColor(theme.panelBg, -10);
    DrawRectangleRounded(bounds, 0.2f, 8, inputBg);
    
    // Progress fill
    if (progress > 0) {
        float fillWidth = bounds.width * progress;
        Rectangle fillRect = {bounds.x, bounds.y, fillWidth, bounds.height};
        DrawRectangleRounded(fillRect, 0.2f, 8, theme.success);
    }
    
    // Border
    DrawRectangleRoundedLines(bounds, 0.2f, 8, theme.panelBorder);
    
    // Label
    int fontSize = 14;
    int textWidth = MeasureText(label.c_str(), fontSize);
    int textX = bounds.x + (bounds.width - textWidth) / 2;
    int textY = bounds.y + (bounds.height - fontSize) / 2;
    DrawText(label.c_str(), textX, textY, fontSize, theme.textPrimary);
}

// StepIndicator implementation
StepIndicator::StepIndicator(int xPos, int yPos, int w) 
    : x(xPos), y(yPos), width(w), currentStep(0) {}

void StepIndicator::SetSteps(const std::vector<std::string>& stepNames) {
    steps = stepNames;
}

void StepIndicator::Draw(const Theme& theme) {
    if (steps.empty()) return;
    
    int stepCount = steps.size();
    int stepWidth = width / stepCount;
    int circleRadius = 12;
    
    for (size_t i = 0; i < steps.size(); i++) {
        int stepX = x + i * stepWidth + stepWidth / 2;
        int stepY = y;
        
        // Draw connecting line
        if (i < steps.size() - 1) {
            int lineStartX = stepX + circleRadius;
            int lineEndX = x + (i + 1) * stepWidth + stepWidth / 2 - circleRadius;
            Color lineColor = (i < currentStep) ? theme.success : theme.panelBorder;
            DrawLine(lineStartX, stepY, lineEndX, stepY, lineColor);
        }
        
        // Draw circle
        Color circleColor;
        if (i < currentStep) {
            circleColor = theme.success;  // Completed
        } else if (i == currentStep) {
            circleColor = theme.primary;  // Current
        } else {
            circleColor = theme.secondary;  // Future
        }
        DrawCircle(stepX, stepY, circleRadius, circleColor);
        
        // Draw step number or checkmark
        if (i < currentStep) {
            DrawText("v", stepX - 4, stepY - 8, 16, theme.textPrimary);
        } else {
            DrawText(TextFormat("%d", (int)i + 1), stepX - 4, stepY - 8, 14, theme.textPrimary);
        }
        
        // Draw label
        int labelWidth = MeasureText(steps[i].c_str(), 12);
        int labelX = stepX - labelWidth / 2;
        Color labelColor = (i <= currentStep) ? theme.textPrimary : theme.textSecondary;
        DrawText(steps[i].c_str(), labelX, stepY + 20, 12, labelColor);
    }
}

// CommandPalette implementation
CommandPalette::CommandPalette(const Rectangle& rect) 
    : bounds(rect) {}

void CommandPalette::Update() {
    if (!isActive) return;
    
    // Handle keyboard input
    int key = GetCharPressed();
    while (key > 0) {
        if (key >= 32 && key <= 125) {
            input += (char)key;
        }
        key = GetCharPressed();
    }
    
    if (IsKeyPressed(KEY_BACKSPACE) && !input.empty()) {
        input.pop_back();
    }
    
    if (IsKeyPressed(KEY_ENTER)) {
        submitted = true;
    }
}

void CommandPalette::Draw(const Theme& theme) {
    // Background
    Color inputBg = AdjustColor(theme.panelBg, -10);
    DrawRectangleRounded(bounds, 0.1f, 8, inputBg);
    
    // Border (glow when active)
    Color borderColor = isActive ? theme.primary : theme.panelBorder;
    DrawRectangleRoundedLines(bounds, 0.1f, 8, borderColor);
    
    // Prompt icon
    DrawText(">", bounds.x + 15, bounds.y + (bounds.height - 24) / 2, 24, theme.primary);
    
    // Input text
    int textX = bounds.x + 40;
    int textY = bounds.y + (bounds.height - 24) / 2;
    DrawText(input.c_str(), textX, textY, 20, theme.textPrimary);
    
    // Cursor
    if (isActive && ((int)(GetTime() * 2) % 2 == 0)) {
        int cursorX = textX + MeasureText(input.c_str(), 20);
        DrawLine(cursorX, textY, cursorX, textY + 20, theme.primary);
    }
}

std::string CommandPalette::GetCommand() {
    if (submitted) {
        submitted = false;
        std::string cmd = input;
        return cmd;
    }
    return "";
}

void CommandPalette::Clear() {
    input.clear();
}

// InfoCard implementation
InfoCard::InfoCard(const Rectangle& rect, const std::string& ttl) 
    : bounds(rect), title(ttl) {}

void InfoCard::AddField(const std::string& label, const std::string& value) {
    fields.push_back({label, value});
}

void InfoCard::Draw(const Theme& theme) {
    // Background
    Color cardBg = AdjustColor(theme.panelBg, 5);
    cardBg.a = 240;
    DrawRectangleRounded(bounds, 0.08f, 8, cardBg);
    DrawRectangleRoundedLines(bounds, 0.08f, 8, theme.panelBorder);
    
    // Title
    DrawText(title.c_str(), bounds.x + 15, bounds.y + 12, 18, theme.primary);
    
    // Fields
    int y = bounds.y + 45;
    for (const auto& [label, value] : fields) {
        // Label
        DrawText(label.c_str(), bounds.x + 15, y, 14, theme.textSecondary);
        // Value
        int labelWidth = MeasureText(label.c_str(), 14);
        DrawText(value.c_str(), bounds.x + 15 + labelWidth + 10, y, 14, theme.textPrimary);
        y += 25;
    }
}

void InfoCard::Clear() {
    fields.clear();
}

// GUIManager implementation
GUIManager::GUIManager() 
    : commandPalette({0, 0, 400, 50}) {
    // Initialize panel bounds (will be recalculated based on screen size)
    leftPanelBounds = {0, 0, 300, 720};
    rightPanelBounds = {300, 0, 980, 720};
    bottomBarBounds = {0, 650, 1280, 70};
}

GUIManager::~GUIManager() = default;

void GUIManager::Initialize() {
    // Try to load Chinese font
    // This would be set from the game level
}

void GUIManager::Update(float deltaTime) {
    commandPalette.Update();
    
    // Update dialogue timer
    if (dialogueVisible) {
        dialogueTimer += deltaTime;
    }
}

void GUIManager::Draw() {
    // Draw panels (if needed as background)
    
    // Draw dialogue if visible
    if (dialogueVisible) {
        DrawDialogue();
    }
}

void GUIManager::DrawDialogue() {
    // Semi-transparent overlay
    DrawRectangle(0, 500, 1280, 220, {0, 0, 0, 180});
    
    // Dialogue box
    Rectangle dialogueBox = {100, 520, 1080, 180};
    DrawRectangleRounded(dialogueBox, 0.05f, 8, theme.panelBg);
    DrawRectangleRoundedLines(dialogueBox, 0.05f, 8, theme.panelBorder);
    
    // Speaker avatar (left side)
    DrawCircle(160, 585, 40, theme.primary);
    DrawText(dialogueSpeaker.c_str(), 140, 578, 16, theme.textPrimary);
    
    // Text content
    DrawChineseText(dialogueText.c_str(), 220, 540, 22, theme.textPrimary);
    
    // Continue indicator
    if ((int)(dialogueTimer * 2) % 2 == 0) {
        DrawText("v", 1200, 680, 20, theme.primary);
    }
}

void GUIManager::DrawChineseText(const char* text, int x, int y, int fontSize, Color color) {
    if (hasChineseFont && chineseFont != nullptr && chineseFont->texture.id != 0) {
        // Calculate spacing based on font size
        float spacing = (float)fontSize * 0.05f;
        if (spacing < 1.0f) spacing = 1.0f;
        
        Vector2 pos = {(float)x, (float)y};
        DrawTextEx(*chineseFont, text, pos, (float)fontSize, spacing, color);
    } else {
        // Fallback to default font
        DrawText(text, x, y, fontSize, color);
    }
}

// Layout helpers
void GUIManager::BeginLeftPanel() {
    // Could set scissor mode here
}

void GUIManager::EndLeftPanel() {
    EndScissorMode();
}

void GUIManager::BeginRightPanel() {
    // Could set scissor mode here
}

void GUIManager::EndRightPanel() {
    EndScissorMode();
}

void GUIManager::BeginBottomBar() {
    // Draw bottom bar background
    DrawRectangleRounded(bottomBarBounds, 0.05f, 8, theme.panelBg);
    DrawRectangleRoundedLines(bottomBarBounds, 0.05f, 8, theme.panelBorder);
}

void GUIManager::EndBottomBar() {}

// Widget helpers - rename to avoid conflict
bool GUIManager::Button(const std::string& text, int x, int y, int width, int height) {
    GuiButton btn({(float)x, (float)y, (float)width, (float)height}, text);
    btn.Draw(theme);
    return btn.IsClicked();
}

bool GUIManager::IconButton(const std::string& icon, const std::string& tooltip, int x, int y) {
    Rectangle bounds = {(float)x, (float)y, 40, 40};
    
    // Draw button
    Color bg = theme.secondary;
    if (CheckCollisionPointRec(GetMousePosition(), bounds)) {
        bg = theme.secondaryHover;
        // Show tooltip
        DrawRectangle(x, y - 30, 100, 25, theme.panelBg);
        DrawText(tooltip.c_str(), x + 5, y - 25, 12, theme.textSecondary);
    }
    
    DrawRectangleRounded(bounds, 0.2f, 8, bg);
    DrawText(icon.c_str(), x + 12, y + 10, 20, theme.textPrimary);
    
    return CheckCollisionPointRec(GetMousePosition(), bounds) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void GUIManager::Label(const std::string& text, int x, int y, int fontSize, Color color) {
    DrawChineseText(text.c_str(), x, y, fontSize, color);
}

void GUIManager::Title(const std::string& text, int x, int y) {
    DrawChineseText(text.c_str(), x, y, 28, theme.primary);
}

void GUIManager::Subtitle(const std::string& text, int x, int y) {
    DrawChineseText(text.c_str(), x, y, 18, theme.textSecondary);
}

// Git-specific widgets
void GUIManager::GitCommandButton(const std::string& cmd, const std::string& desc, int x, int y, bool enabled) {
    Rectangle bounds = {(float)x, (float)y, 200, 50};
    
    Color btnBg = theme.secondary;
    Color bg = enabled ? btnBg : Color{btnBg.r, btnBg.g, btnBg.b, 100};
    if (enabled && CheckCollisionPointRec(GetMousePosition(), bounds)) {
        bg = theme.secondaryHover;
    }
    
    DrawRectangleRounded(bounds, 0.15f, 8, bg);
    DrawRectangleRoundedLines(bounds, 0.15f, 8, enabled ? theme.primary : theme.panelBorder);
    
    // Command
    DrawText(cmd.c_str(), x + 15, y + 8, 20, enabled ? theme.primary : theme.textSecondary);
    // Description
    DrawText(desc.c_str(), x + 15, y + 28, 12, theme.textSecondary);
}

void GUIManager::StatusBadge(const std::string& text, int x, int y, Color bgColor) {
    int padding = 8;
    int fontSize = 12;
    int textWidth = MeasureText(text.c_str(), fontSize);
    int width = textWidth + padding * 2;
    int height = fontSize + padding;
    
    DrawRectangleRounded({(float)x, (float)y, (float)width, (float)height}, 0.3f, 8, bgColor);
    DrawText(text.c_str(), x + padding, y + padding / 2, fontSize, theme.textPrimary);
}

void GUIManager::FileItem(const std::string& filename, const std::string& status, int x, int y, bool selected) {
    int height = 30;
    Rectangle bounds = {(float)x, (float)y, 280, (float)height};
    
    // Background
    if (selected) {
        DrawRectangleRounded(bounds, 0.1f, 8, theme.secondary);
    }
    
    // Filename
    DrawText(filename.c_str(), x + 10, y + 8, 14, theme.textPrimary);
    
    // Status badge (right aligned)
    Color statusColor;
    if (status == "STAGED" || status == "COMMITTED") statusColor = theme.success;
    else if (status == "MODIFIED") statusColor = theme.warning;
    else statusColor = theme.error;
    
    int statusWidth = MeasureText(status.c_str(), 12);
    DrawText(status.c_str(), x + 270 - statusWidth, y + 9, 12, statusColor);
    
    // Separator line
    Color sepColor = theme.panelBorder;
    sepColor.a = 50;
    DrawLine(x + 10, y + height - 1, x + 270, y + height - 1, sepColor);
}

// Dialogue
void GUIManager::ShowDialogue(const std::string& speaker, const std::string& text) {
    dialogueSpeaker = speaker;
    dialogueText = text;
    dialogueVisible = true;
    dialogueTimer = 0;
}

void GUIManager::HideDialogue() {
    dialogueVisible = false;
}

bool GUIManager::IsDialogueVisible() const {
    return dialogueVisible;
}

} // namespace GitGUI
