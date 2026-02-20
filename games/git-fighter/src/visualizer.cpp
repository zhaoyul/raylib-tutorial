#include "visualizer.h"

GitVisualizer::GitVisualizer()
    : animationTime(0), animating(false), animationType(0) {}

GitVisualizer::~GitVisualizer() = default;

void GitVisualizer::DrawCommitGraph(int centerX, int centerY, int width, int height) {
    // Draw a simple placeholder commit graph
    DrawRectangle(centerX - width/2, centerY - height/2, width, height, (Color){45, 45, 55, 255});
    DrawRectangleLines(centerX - width/2, centerY - height/2, width, height, (Color){100, 100, 120, 255});

    DrawText("提交历史图", centerX - width/2 + 10, centerY - height/2 + 10, 20, LIGHTGRAY);

    // Draw sample nodes (placeholder)
    int startX = centerX - width/2 + 40;
    int y = centerY - height/2 + 60;

    // Sample commits
    const char* commits[] = {"Initial commit", "Add feature", "Fix bug", "Main"};
    Color colors[] = {GREEN, BLUE, ORANGE, PURPLE};

    for (int i = 0; i < 4; i++) {
        DrawCircle(startX + i * 80, y, 15, colors[i]);
        DrawText(commits[i], startX + i * 80 - 30, y + 25, 12, LIGHTGRAY);

        if (i < 3) {
            DrawLine(startX + i * 80 + 15, y, startX + (i+1) * 80 - 15, y, LIGHTGRAY);
        }
    }

    // Branch label
    DrawRectangle(startX + 240, y - 40, 80, 24, (Color){100, 100, 150, 200});
    DrawText("main", startX + 250, y - 36, 16, WHITE);
}

void GitVisualizer::DrawFileStatus(int x, int y, const std::string& filename, int status) {
    Color statusColor;
    const char* statusIcon;

    switch (status) {
        case 0: // Untracked
            statusColor = RED;
            statusIcon = "??";
            break;
        case 1: // Modified
            statusColor = ORANGE;
            statusIcon = "M";
            break;
        case 2: // Staged
            statusColor = GREEN;
            statusIcon = "A";
            break;
        case 3: // Committed
            statusColor = DARKGREEN;
            statusIcon = "✓";
            break;
        default:
            statusColor = GRAY;
            statusIcon = "?";
    }

    DrawText(statusIcon, x, y, 16, statusColor);
    DrawText(filename.c_str(), x + 25, y, 16, DARKGRAY);
}

void GitVisualizer::DrawWorkingDirectory(int x, int y, int width, int height) {
    DrawRectangle(x, y, width, height, WHITE);
    DrawRectangleLines(x, y, width, height, (Color){200, 200, 200, 255});
    DrawText("工作区 (Working Directory)", x + 10, y + 10, 18, DARKGRAY);
}

void GitVisualizer::DrawStagingArea(int x, int y, int width, int height) {
    DrawRectangle(x, y, width, height, (Color){250, 252, 240, 255});
    DrawRectangleLines(x, y, width, height, (Color){180, 200, 180, 255});
    DrawText("暂存区 (Staging Area)", x + 10, y + 10, 18, DARKGRAY);
}

void GitVisualizer::UpdateAnimation(float deltaTime) {
    if (animating) {
        animationTime += deltaTime;
        if (animationTime > 1.0f) {
            animating = false;
            animationTime = 0;
        }
    }
}

void GitVisualizer::StartCommitAnimation() {
    animating = true;
    animationType = 1;
    animationTime = 0;
}

void GitVisualizer::StartMergeAnimation() {
    animating = true;
    animationType = 2;
    animationTime = 0;
}

bool GitVisualizer::IsAnimating() const {
    return animating;
}
