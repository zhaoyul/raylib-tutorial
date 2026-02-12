#pragma once
#include <string>

// ============================================================
// 设置结构
// ============================================================
struct Settings {
    // 音量 (0.0 - 1.0)
    float masterVolume = 1.0f;
    float sfxVolume = 0.8f;
    float musicVolume = 0.5f;
    bool muted = false;

    // 难度
    enum class Difficulty { EASY, NORMAL, HARD };
    Difficulty difficulty = Difficulty::NORMAL;

    // 显示
    bool showFPS = false;
    bool fullscreen = false;

    // 键位（预留）
    // int keyUp = KEY_UP;
    // int keyDown = KEY_DOWN;

    // 序列化
    std::string toJson() const;
    static Settings fromJson(const std::string& json);
};

// ============================================================
// 设置管理器
// ============================================================
class SettingsManager {
private:
    Settings current;
    std::string filename;

public:
    SettingsManager(const std::string& filename = "settings.json");

    // 加载和保存
    bool load();
    bool save() const;

    // 获取和设置
    Settings& get() { return current; }
    const Settings& get() const { return current; }

    void set(const Settings& s) { current = s; }

    // 应用设置到音频系统
    void applyToAudio();

private:
    std::string getFullPath() const;
    void ensureDirectory() const;
};
