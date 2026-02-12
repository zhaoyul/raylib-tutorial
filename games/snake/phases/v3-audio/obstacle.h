#pragma once
#include "raylib.h"
#include <vector>

// 前向声明
class Snake;

// ============================================================
// 障碍物类 - 表示墙壁或障碍物
// ============================================================
class Obstacle {
private:
    int x, y;

public:
    Obstacle(int x, int y, bool /* destructible */ = false);

    void draw(int gridSize) const;
    bool checkCollision(int px, int py) const;

    int getX() const { return x; }
    int getY() const { return y; }
};

// ============================================================
// 障碍物管理器 - 管理所有障碍物
// ============================================================
class ObstacleManager {
private:
    std::vector<Obstacle> obstacles;
    int gridWidth, gridHeight;

public:
    ObstacleManager(int gridW, int gridH);

    // 生成障碍物
    void generate(int count, const Snake& snake);
    void clear();

    // 检查碰撞
    bool checkCollision(int x, int y) const;

    // 绘制
    void draw(int gridSize) const;

    // 获取数量
    int getCount() const { return static_cast<int>(obstacles.size()); }

    // 获取所有障碍物（用于生成物品时避开）
    const std::vector<Obstacle>& getObstacles() const { return obstacles; }

private:
    bool isValidPosition(int x, int y, const Snake& snake) const;
};
