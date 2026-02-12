#include "particle.h"
#include <cmath>

// ============================================================
// Particle 实现
// ============================================================
void Particle::update(float deltaTime) {
    if (!active) return;

    // 应用加速度
    velocity.x += acceleration.x * deltaTime;
    velocity.y += acceleration.y * deltaTime;

    // 应用速度
    position.x += velocity.x * deltaTime;
    position.y += velocity.y * deltaTime;

    // 旋转
    rotation += rotationSpeed * deltaTime;

    // 减少生命
    life -= deltaTime;
    if (life <= 0) {
        active = false;
    }
}

void Particle::draw() const {
    if (!active) return;

    float alpha = getAlpha();
    Color c = color;
    c.a = static_cast<unsigned char>(c.a * alpha);

    // 绘制带旋转的矩形
    Rectangle rect = {position.x, position.y, size, size};
    Vector2 origin = {size / 2, size / 2};
    DrawRectanglePro(rect, origin, rotation, c);
}

// ============================================================
// EmitterConfig 实现
// ============================================================
EmitterConfig EmitterConfig::explosion(Vector2 pos, Color color) {
    EmitterConfig config;
    config.position = pos;
    config.count = 20;
    config.minSpeed = 50.0f;
    config.maxSpeed = 200.0f;
    config.minSize = 3.0f;
    config.maxSize = 8.0f;
    config.life = 0.8f;
    config.startColor = color;
    config.endColor = Fade(color, 0.0f);
    config.spread = 360.0f;  // 全方向
    config.gravity = 100.0f;
    config.fadeSize = true;
    config.fadeAlpha = true;
    return config;
}

EmitterConfig EmitterConfig::trail(Vector2 pos, Color color) {
    EmitterConfig config;
    config.position = pos;
    config.count = 1;
    config.minSpeed = 0.0f;
    config.maxSpeed = 10.0f;
    config.minSize = 2.0f;
    config.maxSize = 4.0f;
    config.life = 0.5f;
    config.startColor = Fade(color, 0.6f);
    config.endColor = Fade(color, 0.0f);
    config.spread = 360.0f;
    config.gravity = 0.0f;
    config.fadeSize = true;
    config.fadeAlpha = true;
    return config;
}

EmitterConfig EmitterConfig::sparkle(Vector2 pos) {
    EmitterConfig config;
    config.position = pos;
    config.count = 5;
    config.minSpeed = 20.0f;
    config.maxSpeed = 80.0f;
    config.minSize = 1.0f;
    config.maxSize = 3.0f;
    config.life = 0.6f;
    config.startColor = GOLD;
    config.endColor = Fade(ORANGE, 0.0f);
    config.spread = 360.0f;
    config.gravity = 50.0f;
    config.fadeSize = true;
    config.fadeAlpha = true;
    return config;
}

// ============================================================
// ParticleSystem 实现
// ============================================================
ParticleSystem::ParticleSystem() : nextIndex(0) {
    particles.resize(MAX_PARTICLES);
    for (auto& p : particles) {
        p.active = false;
    }
}

void ParticleSystem::emit(const EmitterConfig& config) {
    for (int i = 0; i < config.count; i++) {
        Particle* p = getNextParticle();
        if (!p) break;

        // 随机角度
        float angle = GetRandomValue(0, static_cast<int>(config.spread)) * DEG2RAD;
        if (config.spread < 360.0f) {
            // 如果扩散角度小于360度，需要调整基准角度
            angle -= config.spread * DEG2RAD / 2;
        }

        // 随机速度
        float speed = GetRandomValue(
            static_cast<int>(config.minSpeed),
            static_cast<int>(config.maxSpeed)
        );

        p->position = config.position;
        p->velocity = {cosf(angle) * speed, sinf(angle) * speed};
        p->acceleration = {0.0f, config.gravity};
        p->color = config.startColor;
        p->size = GetRandomValue(static_cast<int>(config.minSize), static_cast<int>(config.maxSize));
        p->life = config.life;
        p->maxLife = config.life;
        p->rotation = GetRandomValue(0, 360);
        p->rotationSpeed = GetRandomValue(-180, 180);
        p->active = true;
    }
}

void ParticleSystem::emitExplosion(Vector2 pos, Color color, int count) {
    EmitterConfig config = EmitterConfig::explosion(pos, color);
    config.count = count;
    emit(config);
}

void ParticleSystem::emitTrail(Vector2 pos, Color color) {
    // 随机决定是否发射轨迹粒子（避免太多）
    if (GetRandomValue(0, 10) < 3) {  // 30% 概率
        EmitterConfig config = EmitterConfig::trail(pos, color);
        emit(config);
    }
}

void ParticleSystem::emitSparkle(Vector2 pos, Color color) {
    EmitterConfig config = EmitterConfig::sparkle(pos);
    config.startColor = color;
    config.endColor = Fade(color, 0.0f);
    emit(config);
}

void ParticleSystem::update(float deltaTime) {
    for (auto& p : particles) {
        if (p.active) {
            p.update(deltaTime);
        }
    }
}

void ParticleSystem::draw() {
    for (const auto& p : particles) {
        if (p.active) {
            p.draw();
        }
    }
}

void ParticleSystem::clear() {
    for (auto& p : particles) {
        p.active = false;
    }
    nextIndex = 0;
}

size_t ParticleSystem::getActiveCount() const {
    size_t count = 0;
    for (const auto& p : particles) {
        if (p.active) count++;
    }
    return count;
}

Particle* ParticleSystem::getNextParticle() {
    // 从 nextIndex 开始寻找非活跃粒子
    size_t startIndex = nextIndex;
    do {
        if (!particles[nextIndex].active) {
            return &particles[nextIndex];
        }
        nextIndex = (nextIndex + 1) % MAX_PARTICLES;
    } while (nextIndex != startIndex);

    // 如果没有可用粒子，返回 nullptr
    return nullptr;
}

// ============================================================
// 缓动函数实现
// ============================================================
namespace Easing {
    float linear(float t) {
        return t;
    }

    float easeInQuad(float t) {
        return t * t;
    }

    float easeOutQuad(float t) {
        return 1.0f - (1.0f - t) * (1.0f - t);
    }

    float easeInOutQuad(float t) {
        return t < 0.5f ? 2.0f * t * t : 1.0f - powf(-2.0f * t + 2.0f, 2.0f) / 2.0f;
    }

    float easeInCubic(float t) {
        return t * t * t;
    }

    float easeOutCubic(float t) {
        return 1.0f - powf(1.0f - t, 3.0f);
    }

    float easeInOutCubic(float t) {
        return t < 0.5f ? 4.0f * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 3.0f) / 2.0f;
    }

    float easeInSine(float t) {
        return 1.0f - cosf(t * PI / 2.0f);
    }

    float easeOutSine(float t) {
        return sinf(t * PI / 2.0f);
    }

    float easeInOutSine(float t) {
        return -(cosf(PI * t) - 1.0f) / 2.0f;
    }

    float easeInExpo(float t) {
        return t == 0.0f ? 0.0f : powf(2.0f, 10.0f * (t - 1.0f));
    }

    float easeOutExpo(float t) {
        return t == 1.0f ? 1.0f : 1.0f - powf(2.0f, -10.0f * t);
    }

    float easeOutBounce(float t) {
        const float n1 = 7.5625f;
        const float d1 = 2.75f;

        if (t < 1.0f / d1) {
            return n1 * t * t;
        } else if (t < 2.0f / d1) {
            t -= 1.5f / d1;
            return n1 * t * t + 0.75f;
        } else if (t < 2.5f / d1) {
            t -= 2.25f / d1;
            return n1 * t * t + 0.9375f;
        } else {
            t -= 2.625f / d1;
            return n1 * t * t + 0.984375f;
        }
    }

    float easeOutBack(float t) {
        const float c1 = 1.70158f;
        const float c3 = c1 + 1.0f;
        return 1.0f + c3 * powf(t - 1.0f, 3.0f) + c1 * powf(t - 1.0f, 2.0f);
    }

    Color lerpColor(Color a, Color b, float t) {
        Color result;
        result.r = static_cast<unsigned char>(a.r + (b.r - a.r) * t);
        result.g = static_cast<unsigned char>(a.g + (b.g - a.g) * t);
        result.b = static_cast<unsigned char>(a.b + (b.b - a.b) * t);
        result.a = static_cast<unsigned char>(a.a + (b.a - a.a) * t);
        return result;
    }
}
