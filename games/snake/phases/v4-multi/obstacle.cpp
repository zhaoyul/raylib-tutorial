#include "obstacle.h"
#include "snake.h"

// ============================================================
// Obstacle 实现
// ============================================================
Obstacle::Obstacle(int x, int y, bool /* destructible */)
    : x(x), y(y) {
}

void Obstacle::draw(int gridSize) const {
    int px = x * gridSize;
    int py = y * gridSize;

    // 绘制墙壁 - 使用灰色和深灰色营造立体感
    DrawRectangle(px + 1, py + 1, gridSize - 2, gridSize - 2, GRAY);

    // 高光
    DrawLine(px + 2, py + 2, px + gridSize - 3, py + 2, LIGHTGRAY);
    DrawLine(px + 2, py + 2, px + 2, py + gridSize - 3, LIGHTGRAY);

    // 阴影
    DrawLine(px + gridSize - 3, py + 3, px + gridSize - 3, py + gridSize - 3, DARKGRAY);
    DrawLine(px + 3, py + gridSize - 3, px + gridSize - 3, py + gridSize - 3, DARKGRAY);
}

bool Obstacle::checkCollision(int px, int py) const {
    return x == px && y == py;
}

// ============================================================
// ObstacleManager 实现
// ============================================================
ObstacleManager::ObstacleManager(int gridW, int gridH)
    : gridWidth(gridW), gridHeight(gridH) {
}

void ObstacleManager::generate(int count, const Snake& snake) {
    obstacles.clear();

    int attempts = 0;
    int maxAttempts = count * 100;  // 防止无限循环

    while (static_cast<int>(obstacles.size()) < count && attempts < maxAttempts) {
        attempts++;

        int x = GetRandomValue(0, gridWidth - 1);
        int y = GetRandomValue(0, gridHeight - 1);

        // 检查是否与蛇或已有障碍物重叠
        if (isValidPosition(x, y, snake)) {
            obstacles.emplace_back(x, y, false);
        }
    }
}

void ObstacleManager::addObstacle(int x, int y) {
    if (x < 0 || x >= gridWidth || y < 0 || y >= gridHeight) {
        return;
    }
    if (checkCollision(x, y)) {
        return;
    }
    obstacles.emplace_back(x, y, false);
}

void ObstacleManager::clear() {
    obstacles.clear();
}

bool ObstacleManager::checkCollision(int x, int y) const {
    for (const auto& obs : obstacles) {
        if (obs.checkCollision(x, y)) {
            return true;
        }
    }
    return false;
}

void ObstacleManager::draw(int gridSize) const {
    for (const auto& obs : obstacles) {
        obs.draw(gridSize);
    }
}

bool ObstacleManager::isValidPosition(int x, int y, const Snake& snake) const {
    // 检查是否在蛇身上
    for (const auto& segment : snake.getBody()) {
        if (segment.x == x && segment.y == y) {
            return false;
        }
    }

    // 检查是否与已有障碍物重叠
    for (const auto& obs : obstacles) {
        if (obs.getX() == x && obs.getY() == y) {
            return false;
        }
    }

    // 留出中间区域给蛇出生
    int midX = gridWidth / 2;
    int midY = gridHeight / 2;
    if (abs(x - midX) < 3 && abs(y - midY) < 3) {
        return false;
    }

    return true;
}
