#pragma once
#include "raylib.h"

// ============================================================
// 屏幕震动效果
// ============================================================
class ScreenShake {
private:
    float intensity;      // 震动强度（像素）
    float duration;       // 剩余持续时间
    float maxDuration;    // 最大持续时间
    bool active;

    Vector2 offset;       // 当前偏移

public:
    ScreenShake();

    // 开始震动
    void start(float intensity, float duration);

    // 更新
    void update(float deltaTime);

    // 应用震动（修改当前的 Camera2D 或渲染偏移）
    void begin();
    void end();

    // 获取当前偏移
    Vector2 getOffset() const { return offset; }

    // 是否活跃
    bool isActive() const { return active; }

    // 获取剩余时间比例（0-1）
    float getRemainingRatio() const;
};
