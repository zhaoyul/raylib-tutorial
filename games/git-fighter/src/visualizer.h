#pragma once
#include "raylib.h"
#include <vector>
#include <string>

// Git visualization components
class GitVisualizer {
public:
    GitVisualizer();
    ~GitVisualizer();

    // Draw commit graph (DAG)
    void DrawCommitGraph(int centerX, int centerY, int width, int height);

    // Draw file status
    void DrawFileStatus(int x, int y, const std::string& filename, int status);

    // Draw working directory
    void DrawWorkingDirectory(int x, int y, int width, int height);

    // Draw staging area
    void DrawStagingArea(int x, int y, int width, int height);

    // Animation
    void UpdateAnimation(float deltaTime);
    void StartCommitAnimation();
    void StartMergeAnimation();
    bool IsAnimating() const;

private:
    float animationTime;
    bool animating;
    int animationType;  // 0=none, 1=commit, 2=merge, 3=rebase
};
