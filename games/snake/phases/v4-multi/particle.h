#pragma once
#include "raylib.h"
#include <vector>
#include <functional>

// ============================================================
// 粒子结构
// ============================================================
struct Particle {
    Vector2 position;      // 位置
    Vector2 velocity;      // 速度
    Vector2 acceleration;  // 加速度
    Color color;           // 颜色
    float size;            // 大小
    float life;            // 剩余生命（秒）
    float maxLife;         // 最大生命
    float rotation;        // 旋转角度
    float rotationSpeed;   // 旋转速度
    bool active;           // 是否激活

    // 更新和绘制
    void update(float deltaTime);
    void draw() const;
    float getAlpha() const { return life / maxLife; }
};

// ============================================================
// 粒子发射器配置
// ============================================================
struct EmitterConfig {
    Vector2 position;           // 发射位置
    int count;                  // 发射数量
    float minSpeed, maxSpeed;   // 速度范围
    float minSize, maxSize;     // 大小范围
    float life;                 // 生命周期
    Color startColor;           // 起始颜色
    Color endColor;             // 结束颜色（用于渐变）
    float spread;               // 扩散角度（度）
    float gravity;              // 重力
    bool fadeSize;              // 是否随时间缩小
    bool fadeAlpha;             // // 是否随时间淡出

    // 默认配置
    static EmitterConfig explosion(Vector2 pos, Color color);
    static EmitterConfig trail(Vector2 pos, Color color);
    static EmitterConfig sparkle(Vector2 pos);
};

// ============================================================
// 粒子系统 - 使用对象池模式
// ============================================================
class ParticleSystem {
private:
    static constexpr size_t MAX_PARTICLES = 1000;
    std::vector<Particle> particles;
    size_t nextIndex;  // 下一个可用的粒子索引

public:
    ParticleSystem();

    // 发射粒子
    void emit(const EmitterConfig& config);
    void emitExplosion(Vector2 pos, Color color, int count = 20);
    void emitTrail(Vector2 pos, Color color);
    void emitSparkle(Vector2 pos, Color color);

    // 更新和绘制
    void update(float deltaTime);
    void draw();

    // 清除所有粒子
    void clear();

    // 获取活跃粒子数
    size_t getActiveCount() const;

private:
    // 获取下一个可用粒子
    Particle* getNextParticle();
};

// ============================================================
// 缓动函数
// ============================================================
namespace Easing {
    // 线性
    float linear(float t);

    // 二次方
    float easeInQuad(float t);
    float easeOutQuad(float t);
    float easeInOutQuad(float t);

    // 三次方
    float easeInCubic(float t);
    float easeOutCubic(float t);
    float easeInOutCubic(float t);

    // 正弦
    float easeInSine(float t);
    float easeOutSine(float t);
    float easeInOutSine(float t);

    // 指数
    float easeInExpo(float t);
    float easeOutExpo(float t);

    // 弹性
    float easeOutBounce(float t);

    // 回弹
    float easeOutBack(float t);

    // 颜色插值
    Color lerpColor(Color a, Color b, float t);
}
