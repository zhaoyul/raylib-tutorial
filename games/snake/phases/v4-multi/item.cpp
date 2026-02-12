#include "item.h"
#include "snake.h"
#include "game.h"
#include <cmath>

// ============================================================
// Item 基类实现
// ============================================================
Item::Item(int x, int y, float lifetime)
    : x(x), y(y), lifetime(lifetime), expired(false) {
}

void Item::update(float deltaTime) {
    if (lifetime > 0) {
        lifetime -= deltaTime;
        if (lifetime <= 0) {
            expired = true;
        }
    }
}

void Item::draw(int gridSize) const {
    Color c = getColor();
    DrawRectangle(x * gridSize + 2, y * gridSize + 2,
                  gridSize - 4, gridSize - 4, c);
}

// ============================================================
// NormalFood 实现
// ============================================================
NormalFood::NormalFood(int x, int y) : Item(x, y, -1.0f) {
}

void NormalFood::onEat(Snake& snake, Game& game) {
    snake.grow(1);
    game.addScore(getScore());
}

// ============================================================
// GoldenFood 实现
// ============================================================
GoldenFood::GoldenFood(int x, int y) : Item(x, y, 5.0f) { // 5秒后消失
}

void GoldenFood::onEat(Snake& snake, Game& game) {
    snake.grow(3);
    game.addScore(getScore());
}

void GoldenFood::draw(int gridSize) const {
    // 金色食物闪烁效果
    float flash = (sin(GetTime() * 10) + 1.0f) * 0.5f; // 0~1 闪烁
    Color c = ColorAlpha(GOLD, 0.7f + flash * 0.3f);

    // 外圈
    DrawRectangle(x * gridSize + 1, y * gridSize + 1,
                  gridSize - 2, gridSize - 2, c);
    // 内圈
    DrawRectangle(x * gridSize + 4, y * gridSize + 4,
                  gridSize - 8, gridSize - 8, WHITE);
}

// ============================================================
// SpeedUpFood 实现
// ============================================================
SpeedUpFood::SpeedUpFood(int x, int y) : Item(x, y, -1.0f) {
}

void SpeedUpFood::onEat(Snake& snake, Game& game) {
    snake.grow(1);
    game.addScore(getScore());
    game.applySpeedEffect(0.5f, 5.0f); // 速度减半（更快）
}

// ============================================================
// SlowDownFood 实现
// ============================================================
SlowDownFood::SlowDownFood(int x, int y) : Item(x, y, -1.0f) {
}

void SlowDownFood::onEat(Snake& snake, Game& game) {
    snake.grow(1);
    game.addScore(getScore());
    game.applySpeedEffect(2.0f, 5.0f); // 速度加倍（更慢）
}

// ============================================================
// ItemFactory 实现
// ============================================================
std::unique_ptr<Item> ItemFactory::createRandomItem(int x, int y) {
    int type = GetRandomValue(0, 3);
    switch (type) {
        case 0: return std::make_unique<NormalFood>(x, y);
        case 1: return std::make_unique<GoldenFood>(x, y);
        case 2: return std::make_unique<SpeedUpFood>(x, y);
        case 3: return std::make_unique<SlowDownFood>(x, y);
        default: return std::make_unique<NormalFood>(x, y);
    }
}

std::unique_ptr<Item> ItemFactory::createWeightedItem(int x, int y) {
    int roll = GetRandomValue(1, 100);

    if (roll <= 70) {
        return std::make_unique<NormalFood>(x, y);
    } else if (roll <= 80) {
        return std::make_unique<GoldenFood>(x, y);
    } else if (roll <= 92) {
        return std::make_unique<SpeedUpFood>(x, y);
    } else {
        return std::make_unique<SlowDownFood>(x, y);
    }
}
