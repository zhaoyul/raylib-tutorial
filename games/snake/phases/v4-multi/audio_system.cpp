#include "audio_system.h"
#include <cstring>
#include <cmath>

// 单例实例
AudioSystem* AudioSystem::instance = nullptr;

AudioSystem::AudioSystem()
    : musicLoaded(false),
      masterVolume(1.0f),
      sfxVolume(0.8f),
      musicVolume(0.5f),
      muted(false) {
}

AudioSystem::~AudioSystem() {
    shutdown();
}

AudioSystem& AudioSystem::getInstance() {
    if (instance == nullptr) {
        instance = new AudioSystem();
    }
    return *instance;
}

void AudioSystem::init() {
    InitAudioDevice();

    // 尝试加载音效文件，如果不存在则生成默认音效
    generateDefaultSounds();

    SetMasterVolume(masterVolume);
}

void AudioSystem::shutdown() {
    static bool isShuttingDown = false;
    if (isShuttingDown) return;
    isShuttingDown = true;

    // 卸载所有音效
    for (auto& pair : sounds) {
        if (pair.second.frameCount > 0) {
            UnloadSound(pair.second);
        }
    }
    sounds.clear();

    if (musicLoaded) {
        UnloadMusicStream(backgroundMusic);
        musicLoaded = false;
    }

    // 检查音频设备是否已初始化
    if (IsAudioDeviceReady()) {
        CloseAudioDevice();
    }

    // 不要在这里 delete instance，由调用者管理
    // delete instance;
    // instance = nullptr;
}

bool AudioSystem::loadSound(SoundType type, const std::string& filePath) {
    if (FileExists(filePath.c_str())) {
        Sound sound = LoadSound(filePath.c_str());
        if (sound.frameCount > 0) {
            // 如果已经存在，先卸载旧的
            if (sounds.find(type) != sounds.end()) {
                UnloadSound(sounds[type]);
            }
            sounds[type] = sound;
            SetSoundVolume(sound, sfxVolume * masterVolume);
            return true;
        }
    }
    return false;
}

bool AudioSystem::loadBackgroundMusic(const std::string& filePath) {
    if (musicLoaded) {
        UnloadMusicStream(backgroundMusic);
        musicLoaded = false;
    }

    if (FileExists(filePath.c_str())) {
        backgroundMusic = LoadMusicStream(filePath.c_str());
        if (backgroundMusic.frameCount > 0) {
            musicLoaded = true;
            SetMusicVolume(backgroundMusic, musicVolume * masterVolume);
            return true;
        }
    }
    return false;
}

void AudioSystem::play(SoundType type) {
    if (muted) return;

    auto it = sounds.find(type);
    if (it != sounds.end()) {
        PlaySound(it->second);
    }
}

void AudioSystem::playBackgroundMusic() {
    if (musicLoaded && !muted) {
        PlayMusicStream(backgroundMusic);
    }
}

void AudioSystem::stopBackgroundMusic() {
    if (musicLoaded) {
        StopMusicStream(backgroundMusic);
    }
}

void AudioSystem::pauseBackgroundMusic() {
    if (musicLoaded) {
        PauseMusicStream(backgroundMusic);
    }
}

void AudioSystem::resumeBackgroundMusic() {
    if (musicLoaded && !muted) {
        ResumeMusicStream(backgroundMusic);
    }
}

void AudioSystem::setMasterVolume(float volume) {
    masterVolume = volume;
    SetMasterVolume(volume);

    // 更新所有音效音量
    for (auto& pair : sounds) {
        SetSoundVolume(pair.second, sfxVolume * masterVolume);
    }

    if (musicLoaded) {
        SetMusicVolume(backgroundMusic, musicVolume * masterVolume);
    }
}

void AudioSystem::setSfxVolume(float volume) {
    sfxVolume = volume;
    for (auto& pair : sounds) {
        SetSoundVolume(pair.second, sfxVolume * masterVolume);
    }
}

void AudioSystem::setMusicVolume(float volume) {
    musicVolume = volume;
    if (musicLoaded) {
        SetMusicVolume(backgroundMusic, musicVolume * masterVolume);
    }
}

void AudioSystem::mute(bool m) {
    muted = m;
    if (muted) {
        SetMasterVolume(0.0f);
    } else {
        SetMasterVolume(masterVolume);
    }
}

void AudioSystem::toggleMute() {
    mute(!muted);
}

void AudioSystem::update() {
    if (musicLoaded && IsMusicStreamPlaying(backgroundMusic)) {
        UpdateMusicStream(backgroundMusic);
    }
}

bool AudioSystem::isMusicPlaying() const {
    return musicLoaded && IsMusicStreamPlaying(backgroundMusic);
}

// ============================================================
// 生成默认音效（程序化音效）
// ============================================================
void AudioSystem::generateDefaultSounds() {
    // 吃普通食物 - 短促的"哔"声
    sounds[SoundType::EAT_NORMAL] = createBeepSound(440.0f, 0.1f);  // A4

    // 吃金色食物 - 高音
    sounds[SoundType::EAT_GOLDEN] = createBeepSound(880.0f, 0.2f);  // A5

    // 吃加速食物 - 上升的音效
    sounds[SoundType::EAT_SPEED] = createBeepSound(660.0f, 0.15f);  // E5

    // 吃减速食物 - 低音
    sounds[SoundType::EAT_SLOW] = createBeepSound(330.0f, 0.15f);   // E4

    // 碰撞 - 噪音
    sounds[SoundType::COLLISION] = createNoiseSound(0.2f);

    // 游戏结束 - 低音
    sounds[SoundType::GAME_OVER] = createBeepSound(220.0f, 0.5f);   // A3

    // 额外生命 - 高音序列
    sounds[SoundType::EXTRA_LIFE] = createBeepSound(1760.0f, 0.3f); // A6

    // 暂停 - 短音
    sounds[SoundType::PAUSE] = createBeepSound(523.0f, 0.05f);      // C5

    // 菜单选择 - 很短的音
    sounds[SoundType::MENU_SELECT] = createBeepSound(659.0f, 0.03f); // E5
}

Sound AudioSystem::createBeepSound(float frequency, float duration) {
    // 简单的正弦波音效生成
    const int sampleRate = 44100;
    const int sampleCount = static_cast<int>(sampleRate * duration);

    // 创建音频数据（16位有符号整数）
    short* data = new short[sampleCount];

    for (int i = 0; i < sampleCount; i++) {
        float t = static_cast<float>(i) / sampleRate;
        // 正弦波 + 简单的包络
        float envelope = 1.0f - (static_cast<float>(i) / sampleCount);
        float sample = sinf(2.0f * PI * frequency * t) * envelope * 0.5f;
        data[i] = static_cast<short>(sample * 32767);
    }

    // 创建 Wave
    Wave wave;
    wave.frameCount = sampleCount;
    wave.sampleRate = sampleRate;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = data;

    Sound sound = LoadSoundFromWave(wave);

    // 清理（LoadSoundFromWave 会复制数据）
    delete[] data;

    return sound;
}

Sound AudioSystem::createNoiseSound(float duration) {
    const int sampleRate = 44100;
    const int sampleCount = static_cast<int>(sampleRate * duration);

    short* data = new short[sampleCount];

    for (int i = 0; i < sampleCount; i++) {
        float envelope = 1.0f - (static_cast<float>(i) / sampleCount);
        float sample = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * envelope * 0.5f;
        data[i] = static_cast<short>(sample * 32767);
    }

    Wave wave;
    wave.frameCount = sampleCount;
    wave.sampleRate = sampleRate;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = data;

    Sound sound = LoadSoundFromWave(wave);
    delete[] data;

    return sound;
}
