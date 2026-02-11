#pragma once
#include "raylib.h"
#include <memory>
#include <string>

// 前向声明
class Snake;
class Game;

// 食物类型枚举
enum class ItemType {
    NORMAL,     // 普通食物
    GOLDEN,     // 金色食物（高分，限时）
    SPEED_UP,   // 加速
    SLOW_DOWN   // 减速
};

// ============================================================
// 道具基类 - 使用多态实现不同效果
// ============================================================
class Item {
protected:
    int x, y;           // 位置
    float lifetime;     // 剩余生命周期（秒）
    bool expired;       // 是否已过期

public:
    Item(int x, int y, float lifetime = -1.0f);
    virtual ~Item() = default;

    // 纯虚函数 - 子类必须实现
    virtual void onEat(Snake& snake, Game& game) = 0;
    virtual Color getColor() const = 0;
    virtual int getScore() const = 0;
    virtual ItemType getType() const = 0;
    virtual std::string getName() const = 0;
    virtual float getEffectDuration() const { return 0.0f; }

    // 通用方法
    void update(float deltaTime);
    bool isExpired() const { return expired; }
    float getRemainingLife() const { return lifetime; }

    int getX() const { return x; }
    int getY() const { return y; }

    // 绘制食物（可被子类重写）
    virtual void draw(int gridSize) const;
};

// ============================================================
// 具体食物类型
// ============================================================

// 普通食物
class NormalFood : public Item {
public:
    NormalFood(int x, int y);

    void onEat(Snake& snake, Game& game) override;
    Color getColor() const override { return RED; }
    int getScore() const override { return 10; }
    ItemType getType() const override { return ItemType::NORMAL; }
    std::string getName() const override { return "普通食物"; }
};

// 金色食物（限时，高分）
class GoldenFood : public Item {
public:
    GoldenFood(int x, int y);

    void onEat(Snake& snake, Game& game) override;
    Color getColor() const override { return GOLD; }
    int getScore() const override { return 50; }
    ItemType getType() const override { return ItemType::GOLDEN; }
    std::string getName() const override { return "金色食物"; }

    // 金色食物有闪烁效果
    void draw(int gridSize) const override;
};

// 加速食物
class SpeedUpFood : public Item {
public:
    SpeedUpFood(int x, int y);

    void onEat(Snake& snake, Game& game) override;
    Color getColor() const override { return SKYBLUE; }
    int getScore() const override { return 15; }
    ItemType getType() const override { return ItemType::SPEED_UP; }
    std::string getName() const override { return "加速食物"; }
    float getEffectDuration() const override { return 5.0f; } // 5秒效果
};

// 减速食物
class SlowDownFood : public Item {
public:
    SlowDownFood(int x, int y);

    void onEat(Snake& snake, Game& game) override;
    Color getColor() const override { return PURPLE; }
    int getScore() const override { return 20; }
    ItemType getType() const override { return ItemType::SLOW_DOWN; }
    std::string getName() const override { return "减速食物"; }
    float getEffectDuration() const override { return 5.0f; }
};

// ============================================================
// 食物工厂 - 使用工厂模式创建不同类型的食物
// ============================================================
class ItemFactory {
public:
    // 随机创建一种食物
    static std::unique_ptr<Item> createRandomItem(int x, int y);

    // 按概率创建食物
    // 普通: 70%, 金色: 10%, 加速: 12%, 减速: 8%
    static std::unique_ptr<Item> createWeightedItem(int x, int y);
};
