#include "settings.h"
#include "audio_system.h"
#include <fstream>
#include <sstream>
#include <cstdio>
#include <sys/stat.h>

// ============================================================
// Settings 实现
// ============================================================
std::string Settings::toJson() const {
    std::stringstream ss;
    ss << "{"
       << "\"masterVolume\":" << masterVolume << ","
       << "\"sfxVolume\":" << sfxVolume << ","
       << "\"musicVolume\":" << musicVolume << ","
       << "\"muted\":" << (muted ? "true" : "false") << ","
       << "\"difficulty\":" << static_cast<int>(difficulty) << ","
       << "\"showFPS\":" << (showFPS ? "true" : "false") << ","
       << "\"fullscreen\":" << (fullscreen ? "true" : "false")
       << "}";
    return ss.str();
}

Settings Settings::fromJson(const std::string& json) {
    Settings s;

    // 简单的键值解析
    auto parseFloat = [&json](const std::string& key) -> float {
        size_t pos = json.find("\"" + key + "\":");
        if (pos != std::string::npos) {
            pos += key.length() + 3;
            return std::stof(json.substr(pos));
        }
        return 0.0f;
    };

    auto parseBool = [&json](const std::string& key) -> bool {
        size_t pos = json.find("\"" + key + "\":");
        if (pos != std::string::npos) {
            pos += key.length() + 3;
            return json.substr(pos, 4) == "true";
        }
        return false;
    };

    auto parseInt = [&json](const std::string& key) -> int {
        size_t pos = json.find("\"" + key + "\":");
        if (pos != std::string::npos) {
            pos += key.length() + 3;
            return std::stoi(json.substr(pos));
        }
        return 0;
    };

    s.masterVolume = parseFloat("masterVolume");
    s.sfxVolume = parseFloat("sfxVolume");
    s.musicVolume = parseFloat("musicVolume");
    s.muted = parseBool("muted");
    s.difficulty = static_cast<Settings::Difficulty>(parseInt("difficulty"));
    s.showFPS = parseBool("showFPS");
    s.fullscreen = parseBool("fullscreen");

    return s;
}

// ============================================================
// SettingsManager 实现
// ============================================================
SettingsManager::SettingsManager(const std::string& fname)
    : filename(fname) {
    load();
}

bool SettingsManager::load() {
    std::string fullPath = getFullPath();
    std::ifstream file(fullPath);

    if (!file.is_open()) {
        // 使用默认设置
        current = Settings();
        return true;
    }

    std::string json((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
    current = Settings::fromJson(json);

    return true;
}

bool SettingsManager::save() const {
    ensureDirectory();
    std::string fullPath = getFullPath();
    std::ofstream file(fullPath);

    if (!file.is_open()) {
        return false;
    }

    file << current.toJson();
    return true;
}

void SettingsManager::applyToAudio() {
    AudioSystem& audio = AudioSystem::getInstance();
    audio.setMasterVolume(current.masterVolume);
    audio.setSfxVolume(current.sfxVolume);
    audio.setMusicVolume(current.musicVolume);
    audio.mute(current.muted);
}

std::string SettingsManager::getFullPath() const {
    return "data/" + filename;
}

void SettingsManager::ensureDirectory() const {
    #ifdef _WIN32
    _mkdir("data");
    #else
    mkdir("data", 0755);
    #endif
}
