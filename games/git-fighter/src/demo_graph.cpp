#include "raylib.h"
#include "git_graph.h"
#include <iostream>

using namespace GitVis;

int main() {
    // Initialize window
    InitWindow(1280, 720, "Git Graph Visualization Demo");
    SetTargetFPS(60);

    // Create graph with simulated data
    GitGraphRenderer* graph = CommitGraphBuilder::BuildFromSimulatedData();

    // Camera control
    Vector2 dragStart = {0, 0};
    bool isDragging = false;

    // Animation for new commit demo
    float demoTimer = 0;
    bool addedNewCommit = false;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        // Update graph animations
        graph->UpdateAnimations(deltaTime);

        // Handle input
        // Mouse drag to pan
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();
            VisualNode* clickedNode = graph->GetNodeAtPosition(mousePos);

            if (clickedNode) {
                graph->SelectNode(clickedNode->hash);
                std::cout << "Selected: " << clickedNode->hash << " - " << clickedNode->message << std::endl;
            } else {
                dragStart = GetMousePosition();
                isDragging = true;
            }
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            isDragging = false;
        }

        if (isDragging) {
            Vector2 currentPos = GetMousePosition();
            Vector2 delta = {currentPos.x - dragStart.x, currentPos.y - dragStart.y};
            graph->Pan(delta);
            dragStart = currentPos;
        }

        // Zoom with mouse wheel
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            graph->Zoom(1.0f + wheel * 0.1f);
        }

        // Keyboard shortcuts
        if (IsKeyPressed(KEY_R)) {
            graph->ZoomToFit();
        }

        if (IsKeyPressed(KEY_C)) {
            graph->CenterOnCommit("y9z0a1b");  // Center on HEAD
        }

        // Demo: Add new commit after 3 seconds
        demoTimer += deltaTime;
        if (demoTimer > 3.0f && !addedNewCommit) {
            addedNewCommit = true;
            // Add a new commit for animation demo
            graph->AddCommit("c2d3e4f", "Demo: new feature added", {"y9z0a1b"}, "小王", 1699900000);
            graph->SetHEAD("c2d3e4f");
            graph->RecalculateLayout();
            graph->StartCommitAnimation("c2d3e4f");
        }

        // Drawing
        BeginDrawing();
        ClearBackground((Color){20, 20, 30, 255});

        // Draw title
        DrawText("Git Graph Visualization Demo", 20, 20, 30, WHITE);
        DrawText("Controls: [Drag] Pan  |  [Scroll] Zoom  |  [R] Reset view  |  [C] Center on HEAD  |  [Click] Select node",
                 20, 60, 16, LIGHTGRAY);

        // Draw the graph
        graph->Draw(20, 100, 1240, 560);

        // Draw instructions
        DrawRectangle(20, 680, 1240, 30, (Color){40, 44, 52, 200});
        DrawText("Visualizing: init -> add -> branch -> merge workflow", 30, 687, 18, GREEN);

        EndDrawing();
    }

    // Cleanup
    graph->Shutdown();
    delete graph;

    CloseWindow();
    return 0;
}
