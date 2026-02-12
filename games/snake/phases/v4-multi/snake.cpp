#include "snake.h"

Snake::Snake(int startX, int startY, int gridW, int gridH)
    : direction(Direction::RIGHT), nextDirection(Direction::RIGHT),
      growthPending(0), gridWidth(gridW), gridHeight(gridH) {
    // 初始长度3
    body.push_back({startX, startY});
    body.push_back({startX - 1, startY});
    body.push_back({startX - 2, startY});
}

void Snake::updateDirection() {
    // 玩家1 - WASD控制
    if (IsKeyPressed(KEY_W) && !isOpposite(direction, Direction::UP)) {
        nextDirection = Direction::UP;
    }
    if (IsKeyPressed(KEY_S) && !isOpposite(direction, Direction::DOWN)) {
        nextDirection = Direction::DOWN;
    }
    if (IsKeyPressed(KEY_A) && !isOpposite(direction, Direction::LEFT)) {
        nextDirection = Direction::LEFT;
    }
    if (IsKeyPressed(KEY_D) && !isOpposite(direction, Direction::RIGHT)) {
        nextDirection = Direction::RIGHT;
    }
}

void Snake::updateDirectionPlayer2() {
    // 玩家2 - 方向键控制
    if (IsKeyPressed(KEY_UP) && !isOpposite(direction, Direction::UP)) {
        nextDirection = Direction::UP;
    }
    if (IsKeyPressed(KEY_DOWN) && !isOpposite(direction, Direction::DOWN)) {
        nextDirection = Direction::DOWN;
    }
    if (IsKeyPressed(KEY_LEFT) && !isOpposite(direction, Direction::LEFT)) {
        nextDirection = Direction::LEFT;
    }
    if (IsKeyPressed(KEY_RIGHT) && !isOpposite(direction, Direction::RIGHT)) {
        nextDirection = Direction::RIGHT;
    }
}

void Snake::setNextDirection(Direction dir) {
    if (!isOpposite(direction, dir)) {
        nextDirection = dir;
    }
}

bool Snake::move() {
    direction = nextDirection;

    Position newHead = body.front();
    switch (direction) {
        case Direction::UP:    newHead.y--; break;
        case Direction::DOWN:  newHead.y++; break;
        case Direction::LEFT:  newHead.x--; break;
        case Direction::RIGHT: newHead.x++; break;
    }

    // 检查墙壁碰撞
    if (checkWallCollision(newHead)) {
        return false;
    }

    // 检查自身碰撞
    if (checkSelfCollision(newHead)) {
        return false;
    }

    // 移动
    body.push_front(newHead);

    // 处理生长
    if (growthPending > 0) {
        growthPending--;
    } else {
        body.pop_back();
    }

    return true;
}

void Snake::draw(int gridSize) const {
    for (size_t i = 0; i < body.size(); i++) {
        const auto& pos = body[i];
        Color color = (i == 0) ? DARKGREEN : GREEN;

        // 蛇头稍微大一点
        int padding = (i == 0) ? 1 : 2;
        DrawRectangle(pos.x * gridSize + padding, pos.y * gridSize + padding,
                      gridSize - padding * 2, gridSize - padding * 2, color);
    }
}

void Snake::grow(int amount) {
    growthPending += amount;
}

bool Snake::checkSelfCollision(const Position& pos) const {
    for (const auto& segment : body) {
        if (segment == pos) {
            return true;
        }
    }
    return false;
}

bool Snake::checkWallCollision(const Position& pos) const {
    return pos.x < 0 || pos.x >= gridWidth || pos.y < 0 || pos.y >= gridHeight;
}

void Snake::reset(int startX, int startY) {
    body.clear();
    body.push_back({startX, startY});
    body.push_back({startX - 1, startY});
    body.push_back({startX - 2, startY});
    direction = Direction::RIGHT;
    nextDirection = Direction::RIGHT;
    growthPending = 0;
}

bool Snake::isOpposite(Direction a, Direction b) const {
    return (a == Direction::UP && b == Direction::DOWN) ||
           (a == Direction::DOWN && b == Direction::UP) ||
           (a == Direction::LEFT && b == Direction::RIGHT) ||
           (a == Direction::RIGHT && b == Direction::LEFT);
}
