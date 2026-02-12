#pragma once
#include "raylib.h"
#include <deque>
#include <vector>

// 方向枚举
enum class Direction { UP, DOWN, LEFT, RIGHT };

// 位置结构
struct Position {
    int x, y;
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

// ============================================================
// Snake 类 - 管理蛇的状态和行为
// ============================================================
class Snake {
private:
    std::deque<Position> body;      // 蛇身，头部在 front
    Direction direction;            // 当前方向
    Direction nextDirection;        // 下一帧方向（防止一帧内多次转向）
    int growthPending;              // 待增长的长度

    int gridWidth, gridHeight;      // 边界

public:
    Snake(int startX, int startY, int gridW, int gridH);

    // 更新和绘制
    void updateDirection();         // 根据输入更新方向（玩家1 - WASD）
    void updateDirectionPlayer2();  // 根据输入更新方向（玩家2 - 方向键）
    void setNextDirection(Direction dir); // 直接设置方向（用于AI或其他控制）
    bool move();                    // 移动一步，返回是否存活
    void draw(int gridSize) const;

    // 生长
    void grow(int amount);

    // 碰撞检测
    bool checkSelfCollision(const Position& pos) const;
    bool checkWallCollision(const Position& pos) const;

    // 获取状态
    Position getHead() const { return body.front(); }
    const std::deque<Position>& getBody() const { return body; }
    int getLength() const { return static_cast<int>(body.size()); }
    Direction getDirection() const { return direction; }

    // 重置
    void reset(int startX, int startY);

private:
    bool isOpposite(Direction a, Direction b) const;
};
