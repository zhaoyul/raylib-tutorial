#include "screenshake.h"
#include <cmath>

ScreenShake::ScreenShake() 
    : intensity(0.0f), duration(0.0f), maxDuration(0.0f), 
      active(false), offset({0.0f, 0.0f}) {
}

void ScreenShake::start(float inten, float dur) {
    // 如果新的震动更强，覆盖旧的
    if (inten > intensity || !active) {
        intensity = inten;
        duration = dur;
        maxDuration = dur;
        active = true;
    }
}

void ScreenShake::update(float deltaTime) {
    if (!active) {
        offset = {0.0f, 0.0f};
        return;
    }
    
    duration -= deltaTime;
    if (duration <= 0.0f) {
        active = false;
        offset = {0.0f, 0.0f};
        intensity = 0.0f;
        return;
    }
    
    // 计算当前震动强度（随时间衰减）
    float ratio = duration / maxDuration;
    float currentIntensity = intensity * ratio;
    
    // 随机偏移
    offset.x = GetRandomValue(-static_cast<int>(currentIntensity), 
                               static_cast<int>(currentIntensity));
    offset.y = GetRandomValue(-static_cast<int>(currentIntensity), 
                               static_cast<int>(currentIntensity));
}

void ScreenShake::begin() {
    if (active) {
        BeginScissorMode(
            static_cast<int>(offset.x), 
            static_cast<int>(offset.y), 
            GetScreenWidth(), 
            GetScreenHeight()
        );
    }
}

void ScreenShake::end() {
    if (active) {
        EndScissorMode();
    }
}

float ScreenShake::getRemainingRatio() const {
    if (maxDuration <= 0.0f) return 0.0f;
    return duration / maxDuration;
}
