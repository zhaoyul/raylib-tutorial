#include "raylib.h"
#include "git_visualization.h"
#include <iostream>

using namespace GitVis;

int main() {
    InitWindow(1280, 800, "Git Visualization - Split View Demo");
    SetTargetFPS(60);

    // Create split view
    SplitGitView splitView;
    splitView.Initialize(0, 0, 1280, 800);

    // Setup commit panel with sample data
    auto* commitPanel = splitView.GetCommitPanel();

    // Add commits
    CommitNode c1;
    c1.hash = "a1b2c3d";
    c1.shortHash = "a1b2c3d";
    c1.message = "Initial commit";
    c1.author = "小王";
    c1.timestamp = 1700000000;
    c1.radius = 20;
    c1.position = {200, 100};
    c1.targetPos = {200, 100};
    c1.alpha = 1;
    c1.scale = 1;

    CommitNode c2;
    c2.hash = "e4f5g6h";
    c2.shortHash = "e4f5g6h";
    c2.message = "Add main.cpp";
    c2.author = "小王";
    c2.timestamp = 1699996400;
    c2.parents.push_back("a1b2c3d");
    c2.radius = 20;
    c2.position = {200, 200};
    c2.targetPos = {200, 200};
    c2.alpha = 1;
    c2.scale = 1;

    CommitNode c3;
    c3.hash = "i7j8k9l";
    c3.shortHash = "i7j8k9l";
    c3.message = "Add config";
    c3.author = "小王";
    c3.timestamp = 1699992800;
    c3.parents.push_back("e4f5g6h");
    c3.radius = 20;
    c3.position = {200, 300};
    c3.targetPos = {200, 300};
    c3.alpha = 1;
    c3.scale = 1;
    c3.branches.push_back("main");

    // Feature branch
    CommitNode c4;
    c4.hash = "m0n1o2p";
    c4.shortHash = "m0n1o2p";
    c4.message = "Feature: login";
    c4.author = "小李";
    c4.timestamp = 1699992800;
    c4.parents.push_back("e4f5g6h");
    c4.radius = 20;
    c4.position = {350, 300};
    c4.targetPos = {350, 300};
    c4.alpha = 1;
    c4.scale = 1;
    c4.branches.push_back("feature/login");

    // Merge commit
    CommitNode c5;
    c5.hash = "q3r4s5t";
    c5.shortHash = "q3r4s5t";
    c5.message = "Merge branch 'feature/login'";
    c5.author = "小王";
    c5.timestamp = 1699989200;
    c5.parents.push_back("i7j8k9l");
    c5.parents.push_back("m0n1o2p");
    c5.radius = 20;
    c5.position = {200, 400};
    c5.targetPos = {200, 400};
    c5.alpha = 1;
    c5.scale = 1;
    c5.branches.push_back("main");

    commitPanel->AddCommit(c1);
    commitPanel->AddCommit(c2);
    commitPanel->AddCommit(c3);
    commitPanel->AddCommit(c4);
    commitPanel->AddCommit(c5);

    commitPanel->AddBranch("main", "q3r4s5t", {100, 200, 255, 255});
    commitPanel->AddBranch("feature/login", "m0n1o2p", {255, 150, 100, 255});
    commitPanel->SetHEAD("q3r4s5t");

    commitPanel->RecalculateLayout();

    // Setup callback to sync with structure panel
    commitPanel->onNodeSelected = [&splitView](const CommitNode& node) {
        splitView.OnCommitSelected(node.hash);
    };

    // Initial structure view
    splitView.OnCommitSelected("q3r4s5t");

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        splitView.Update(deltaTime);

        // Keyboard shortcuts
        if (IsKeyPressed(KEY_R)) {
            commitPanel->RecalculateLayout();
        }

        BeginDrawing();
        ClearBackground((Color){25, 25, 35, 255});

        splitView.Draw();

        // Instructions
        DrawRectangle(10, 10, 400, 80, {40, 44, 52, 200});
        DrawText("Split View Demo", 20, 15, 20, WHITE);
        DrawText("Upper: Commit graph (drag to pan, scroll to zoom)", 20, 40, 14, LIGHTGRAY);
        DrawText("Lower: Internal structure (click tree nodes to expand)", 20, 60, 14, LIGHTGRAY);
        DrawText("Drag middle bar to resize panels | [R] reset layout", 20, 75, 12, GRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
