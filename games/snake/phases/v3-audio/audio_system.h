#pragma once
#include "raylib.h"
#include <string>
#include <unordered_map>
#include <memory>

// ============================================================
// 音效类型枚举
// ============================================================
enum class SoundType {
    EAT_NORMAL,     // 吃普通食物
    EAT_GOLDEN,     // 吃金色食物
    EAT_SPEED,      // 吃加速食物
    EAT_SLOW,       // 吃减速食物
    COLLISION,      // 碰撞
    GAME_OVER,      // 游戏结束
    EXTRA_LIFE,     // 获得额外生命
    PAUSE,          // 暂停
    MENU_SELECT,    // 菜单选择
    BACKGROUND,     // 背景音乐
};

// ============================================================
// 音频系统 - 管理所有音效和音乐
// ============================================================
class AudioSystem {
private:
    // 音效资源
    std::unordered_map<SoundType, Sound> sounds;
    Music backgroundMusic;
    bool musicLoaded;

    // 音量设置 (0.0 - 1.0)
    float masterVolume;
    float sfxVolume;
    float musicVolume;
    bool muted;

    // 单例模式
    static AudioSystem* instance;
    AudioSystem();

public:
    ~AudioSystem();

    // 获取单例
    static AudioSystem& getInstance();

    // 初始化
    void init();
    void shutdown();

    // 加载音效
    bool loadSound(SoundType type, const std::string& filePath);
    bool loadBackgroundMusic(const std::string& filePath);

    // 播放
    void play(SoundType type);
    void playBackgroundMusic();
    void stopBackgroundMusic();
    void pauseBackgroundMusic();
    void resumeBackgroundMusic();

    // 音量控制
    void setMasterVolume(float volume);
    void setSfxVolume(float volume);
    void setMusicVolume(float volume);
    void mute(bool mute);
    void toggleMute();

    float getMasterVolume() const { return masterVolume; }
    float getSfxVolume() const { return sfxVolume; }
    float getMusicVolume() const { return musicVolume; }
    bool isMuted() const { return muted; }

    // 更新（需要在每帧调用以更新音乐流）
    void update();

    // 检查是否正在播放背景音乐
    bool isMusicPlaying() const;

    // 生成默认音效（如果没有音频文件）
    void generateDefaultSounds();

private:
    // 创建程序化音效
    Sound createBeepSound(float frequency, float duration);
    Sound createNoiseSound(float duration);
};
