# 🎵 Snake v3-audio - 音效与数据存储

## 概述

这是 Snake 游戏的 **v3-audio** 版本，在 v2-fx 基础上添加了：
- 🎵 音效系统（程序化生成 + 文件加载）
- 🏆 JSON 高分榜存储
- ⚙️ 设置菜单（音量、难度）
- 💾 数据持久化

## 🎮 新增特性

### 音效系统

| 事件       | 音效类型    | 说明           |
|------------|-------------|----------------|
| 吃普通食物 | EAT_NORMAL  | 440Hz "哔"声   |
| 吃金色食物 | EAT_GOLDEN  | 880Hz 高音     |
| 吃加速食物 | EAT_SPEED   | 660Hz 上升音效 |
| 吃减速食物 | EAT_SLOW    | 330Hz 低音     |
| 碰撞       | COLLISION   | 噪音           |
| 游戏结束   | GAME_OVER   | 220Hz 低音延长 |
| 获得生命   | EXTRA_LIFE  | 1760Hz 高音    |
| 菜单选择   | MENU_SELECT | 短促提示音     |

### 高分榜系统
- 本地 JSON 文件存储 (`data/highscores.json`)
- 保存前 10 名
- 记录玩家名字、分数、长度、日期
- 游戏结束时自动检查是否上榜

### 设置菜单
- **主音量**: 0-100%
- **音效音量**: 0-100%
- **音乐音量**: 0-100%
- **难度**: 简单/普通/困难
- **静音**: M 键切换

### 数据持久化
```
data/
├── highscores.json    # 高分榜
└── settings.json      # 游戏设置
```

## 📁 新增文件

```
v3-audio/
├── audio_system.h/cpp     # 音效系统（单例模式）
├── highscore.h/cpp        # 高分榜管理
├── settings.h/cpp         # 设置管理
├── game.h/cpp             # 更新后的游戏逻辑
└── README.md              # 本文件
```

## 🎓 学习要点

### 1. 单例模式
```cpp
class AudioSystem {
    static AudioSystem* instance;
    AudioSystem();  // 私有构造函数
public:
    static AudioSystem& getInstance() {
        if (instance == nullptr) {
            instance = new AudioSystem();
        }
        return *instance;
    }
};

// 使用
AudioSystem::getInstance().play(SoundType::EAT_NORMAL);
```

### 2. 程序化音效生成
```cpp
Sound createBeepSound(float frequency, float duration) {
    const int sampleRate = 44100;
    const int sampleCount = sampleRate * duration;
    short* data = new short[sampleCount];

    for (int i = 0; i < sampleCount; i++) {
        float t = static_cast<float>(i) / sampleRate;
        float envelope = 1.0f - (i / sampleCount);  // 包络
        float sample = sin(2 * PI * frequency * t) * envelope * 0.5f;
        data[i] = sample * 32767;
    }

    Wave wave = {data, sampleCount, sampleRate, 16, 1};
    return LoadSoundFromWave(wave);
}
```

### 3. 简单的 JSON 序列化
```cpp
// 不使用外部库，手动生成 JSON
std::string HighScoreEntry::toJson() const {
    return "{\"name\":\"" + name + "\","
           "\"score\":" + std::to_string(score) + "," +
           "\"date\":\"" + date + "\"}";
}
```

### 4. 文件 I/O
```cpp
// 保存
std::ofstream file("data/highscores.json");
file << "[";
for (size_t i = 0; i < entries.size(); i++) {
    if (i > 0) file << ",";
    file << entries[i].toJson();
}
file << "]";

// 加载
std::ifstream file("data/highscores.json");
std::string json((std::istreambuf_iterator<char>(file)),
                  std::istreambuf_iterator<char>());
```

## 🏗️ 构建和运行

```bash
# 在项目根目录
cmake --build build --target snake-v3-audio
./build/bin/snake-phases/snake-v3-audio
```

**首次运行**会自动创建 `data/` 目录和默认设置文件。

## 🔧 扩展挑战

1. **加载外部音频文件**：
   ```cpp
   AudioSystem::getInstance().loadSound(
       SoundType::EAT_NORMAL,
       "resources/sounds/eat.wav"
   );
   ```

2. **背景音乐循环**：
   - 使用 `LoadMusicStream()` 加载音乐
   - 在每帧调用 `UpdateMusicStream()`

3. **更多设置选项**：
   - 画面质量
   - 粒子密度
   - 按键自定义

4. **统计数据**：
   - 总游戏次数
   - 累计得分
   - 平均生存时间

## 📝 版本对比

| 特性 | v2-fx | v3-audio (当前) |
|------|-------|-----------------|
| 音效系统 | ❌ | ✅ (程序化生成) |
| 高分榜 | ❌ | ✅ (JSON 存储) |
| 设置菜单 | ❌ | ✅ |
| 数据持久化 | ❌ | ✅ |
| 音量控制 | ❌ | ✅ |

## 📚 相关资源

- [Raylib 音频文档](https://www.raylib.com/cheatsheet/cheatsheet.html)
- [单例模式](https://refactoring.guru/design-patterns/singleton/cpp)
- [音频波形基础](https://en.wikipedia.org/wiki/Sine_wave)
- [JSON 格式](https://www.json.org/)

---

**上一版本**: [v2-fx](../v2-fx/)
**下一版本**: v4-multi (双人模式与关卡编辑器) - 待实现
