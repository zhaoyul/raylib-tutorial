# Raylib 游戏开发教程 / Raylib Game Development Tutorial

## 📚 项目简介 / Project Introduction

这是一个从零开始学习 C++、CMake 和 Raylib 的完整教程项目。通过构建经典游戏来掌握游戏开发的核心概念。

This is a comprehensive tutorial project for learning C++, CMake, and Raylib from scratch. Master game development core concepts by building classic games.

## 🎮 游戏项目 / Game Projects

本教程包含以下游戏项目：

1. **打砖块 (Brick Breaker)** - 学习基础游戏循环、碰撞检测
2. **贪吃蛇 (Snake)** - 学习游戏状态管理、数据结构、粒子系统、存档系统
3. **俄罗斯方块 (Tetris)** - 学习二维数组、旋转算法
4. **坦克大战 (Tank Battle)** - 学习精灵动画、多对象管理
5. **塔防游戏 (Tower Defense)** - 学习路径寻找、升级系统
6. **第一人称射击 (FPS)** - 学习3D图形、相机控制

### 🐍 Snake 游戏扩展路线图

贪吃蛇游戏设计为**渐进式学习项目**，通过多个阶段逐步引入新的编程概念。每个阶段都是独立的代码版本，可在 `games/snake/phases/` 目录找到。

| 阶段         | 功能                  | 教学内容                       | 难度     |
|--------------|-----------------------|--------------------------------|----------|
| **v0-base**  | 经典贪吃蛇            | `std::deque`, 游戏循环, 状态机 | ⭐       |
| **v1-items** | 多种食物 + 障碍物     | 枚举类, 继承, 多态碰撞         | ⭐⭐     |
| **v2-fx**    | 粒子系统 + 动画       | 对象池, 时间管理, 缓动函数     | ⭐⭐     |
| **v3-audio** | 音效 + 高分榜         | 资源管理, JSON, 文件I/O        | ⭐⭐⭐   |
| **v4-multi** | 双人模式 + 关卡编辑器 | 设计模式, 序列化, 架构         | ⭐⭐⭐⭐ |

<details>
<summary>📋 各阶段详细说明 (点击展开)</summary>

#### 🎯 v0-base - 经典贪吃蛇
**核心功能：**
- 蛇的移动和成长机制
- 食物随机生成
- 墙壁和自身碰撞检测
- 分数系统和速度递增

**学习目标：**
- `std::deque` 双端队列管理蛇身
- 基于时间的移动控制 (`GetFrameTime()`)
- 简单状态机 (MENU/PLAYING/GAME_OVER)
- 基础 Raylib 绘图 API

---

#### 🍎 v1-items - 道具与障碍系统
**新增功能：**
- **多种食物类型：**
  - 普通食物 (+10分)
  - 金色食物 (+50分，限时出现)
  - 速度食物 (临时加速/减速)
- **障碍物系统：**
  - 随机生成墙壁
  - 随关卡增加的障碍密度
- **生命系统：** 3条命，撞墙不立即结束

**学习目标：**
- `enum class` 强类型枚举
- 继承和多态：`Item` 基类 + 各种道具派生类
- 虚函数实现多态行为
- 更复杂的碰撞检测逻辑

**代码结构变化：**
```cpp
class Item {
public:
    virtual void onEat(Snake& snake, Game& game) = 0;
    virtual Color getColor() const = 0;
    virtual ~Item() = default;
};
```

---

#### ✨ v2-fx - 视觉特效
**新增功能：**
- **粒子系统：**
  - 吃食物时的爆炸效果
  - 蛇移动时的轨迹拖尾
  - 游戏结束时的消散动画
- **平滑动画：**
  - 蛇头朝向旋转
  - 身体间的平滑连接
  - 食物浮动动画
- **屏幕震动：** 碰撞时的反馈

**学习目标：**
- 对象池模式 (Object Pool) 管理粒子
- 缓动函数 (Easing Functions)
- 向量数学基础 (`Vector2` 运算)
- 透明度混合和颜色插值

**关键算法：**
```cpp
// 线性插值 (Lerp)
Vector2 Lerp(Vector2 a, Vector2 b, float t) {
    return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t};
}
```

---

#### 🎵 v3-audio - 音效与数据持久化
**新增功能：**
- **音效系统：**
  - 吃食物音效 (不同食物不同音调)
  - 碰撞音效
  - 背景音乐 (循环播放)
- **高分榜系统：**
  - 本地存储前10名
  - 玩家名字输入
  - 按分数排序
- **设置菜单：**
  - 音量调节
  - 难度选择 (影响速度和障碍数)
  - 按键自定义

**学习目标：**
- Raylib 音频 API (`LoadSound`, `PlaySound`)
- JSON 解析 (使用 [nlohmann/json](https://github.com/nlohmann/json))
- 文件 I/O 和错误处理
- 资源管理器模式 (RAII)

**数据结构：**
```cpp
struct HighScore {
    std::string name;
    int score;
    int length;
    std::string date;
};
```

---

#### 👥 v4-multi - 多人模式与关卡编辑器
**新增功能：**
- **双人模式：**
  - 本地双人同屏对战
  - 竞争吃食物，可互相阻挡
  - 先到指定分数者获胜
- **关卡编辑器：**
  - 可视化编辑障碍位置
  - 保存/加载关卡文件
  - 预设关卡 + 自定义关卡
- **AI 对手：**
  - 简单寻路算法 (BFS)
  - 可调整难度

**学习目标：**
- 组件-实体系统 (ECS) 基础
- BFS 路径寻找算法
- 工厂模式创建游戏对象
- 序列化/反序列化
- 现代 C++ 特性 (lambda, smart pointers)

**设计模式应用：**
- **单例模式：** 资源管理器、配置管理器
- **观察者模式：** 事件系统 (吃食物、碰撞事件)
- **状态模式：** 更复杂的状态机
- **策略模式：** AI 行为切换

</details>

## 📖 学习章节 / Learning Chapters

- **第1章: C++ 基础** - 变量、函数、类、指针
- **第2章: CMake 入门** - 构建系统、依赖管理
- **第3章: Raylib 基础** - 窗口、绘图、输入处理
- **第4章: 游戏循环** - 更新、渲染、帧率控制
- **第5章: 碰撞检测** - AABB、圆形碰撞、SAT
- **第6章: 游戏状态** - 状态机、基础UI绘制
- **第7章: Raygui 基础** - 按钮、滑块、文本框等控件使用
- **第8章: Raygui 高级应用** - 样式定制、布局管理、复杂界面

## 🚀 快速开始 / Quick Start

### 环境要求 / Requirements

- C++ 编译器 (GCC 7+, Clang 6+, MSVC 2017+)
- CMake 3.15+
- Git
- 图形库依赖（Linux需要 X11 开发库）

> **注意：** 在无图形界面的服务器环境中，需要安装图形库依赖才能编译。详见 [构建指南](docs/BUILD.md)。

### 构建步骤 / Build Instructions

```bash
# 克隆仓库 / Clone repository
git clone https://github.com/zhaoyul/raylib-tutorial.git
cd raylib-tutorial

# Linux: 安装图形库依赖
sudo apt install libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev

# 创建构建目录 / Create build directory
mkdir build && cd build

# 配置和构建 / Configure and build
cmake ..
cmake --build .

# 运行游戏 (示例) / Run game (example)
./bin/games/brick-breaker
./bin/games/snake          # 🐍 试试贪吃蛇！
```

### 🐍 快速体验 Snake 游戏

```bash
# 构建并运行 Snake
cmake --build . --target snake
./bin/games/snake

# 在 Emacs 中开发 (使用 .dir-locals.el 配置)
emacs games/snake/main.cpp
# 然后按 M-x compile 即可编译运行
```

**想扩展 Snake？** 查看 [games/snake/ROADMAP.md](games/snake/ROADMAP.md) 了解完整的渐进式开发计划！

> 详细构建说明请参考 [构建指南 / Build Guide](docs/BUILD.md)

## 🧪 Janet REPL 实验环境 / Janet REPL Playground

新增可选的 Janet + Raylib 互操作模块，支持在 REPL 中驱动窗口、绘制与状态更新，适合做实时原型和热重载实验。

> 详细流程与示例请参考 [Janet 文档 / Janet Guide](docs/JANET.md)

### Janet 编译 / 开发 / 调试 / 测试 / NetREPL

下面的命令都假设你在仓库根目录 `raylib-tutorial/`。

#### 1) 编译模块 (CMake)

启用 Janet 子工程并只编译 `raylib_janet` 目标：

```bash
cmake -S . -B build -DBUILD_JANET=ON -DBUILD_GAMES=OFF -DBUILD_CHAPTERS=OFF
cmake --build build --target raylib_janet -j
```

产物路径：`build/janet/raylib.so`（macOS/Linux）或 `build/janet/raylib.dll`（Windows）。

#### 2) 开发 (推荐的环境变量)

将“本仓库里的模块”和“本地构建的 native module”都加入 `JANET_PATH`：

```bash
export JANET_PATH="$(pwd)/build/janet:$(pwd)/janet"
```

如果你要用 NetREPL（下面第 5 节），还需要把 `spork` 也加入路径（见第 5 节）。

#### 3) 调试 (Debug)

最小化复现（只验证能 import）：

```bash
JANET_PATH="$(pwd)/build/janet" janet -e '(import raylib) (print "raylib loaded")'
```

使用 lldb 调试 Janet 进程（方便定位 `raylib_janet.cpp` 崩溃/断点）：

```bash
lldb -- janet -e '(import raylib) (print "loaded")'
```

改了 `janet/raylib_janet.cpp` 后的循环一般是：

```bash
cmake --build build --target raylib_janet -j
JANET_PATH="$(pwd)/build/janet:$(pwd)/janet" janet janet/examples/smoke.janet
```

#### 4) 测试 (Smoke Tests)

仓库内置了几个“不会卡死”的冒烟脚本：

```bash
# 只测 native module + raylib 能开窗/绘制/退出
JANET_PATH="$(pwd)/build/janet" janet janet/examples/smoke.janet

# 测 workflow 主循环 (不依赖 NetREPL)
JANET_PATH="$(pwd)/build/janet:$(pwd)/janet" janet janet/examples/workflow-smoke.janet
```

建议在 CI 或脚本里配合 `timeout`（macOS Homebrew 自带）避免窗口挂住：

```bash
timeout 12s env JANET_PATH="$(pwd)/build/janet" janet janet/examples/smoke.janet
```

#### 5) NetREPL (网络 REPL, 用于远程/热更新控制)

本项目使用的是 **spork/netrepl**（不是 Clojure 的 nREPL）。

先把 `spork` 安装到本仓库的本地模块树（不污染全局，生成 `jpm_tree/`）：

```bash
jpm -l install spork
```

然后把 `spork` 加到 `JANET_PATH`：

```bash
export JANET_PATH="$(pwd)/build/janet:$(pwd)/janet:$(pwd)/jpm_tree/lib"
```

启动 host（会开 Raylib 窗口并监听 9365 端口）：

```bash
janet janet/examples/netrepl-host.janet
```

另开终端启动 client（交互式）：

```bash
janet janet/examples/netrepl-client.janet
```

连接后你可以在 client 输入 Janet 表达式，实时修改 host 进程中的状态，例如：

```clojure
(put demo-state :x 500)
(put demo-state :y 100)
```

如果只想做自动化验证，也可以运行非交互 smoke client：

```bash
janet janet/examples/netrepl-smoke-client.janet
```

## 📁 项目结构 / Project Structure

```
raylib-tutorial/
├── chapters/           # 教程章节代码
│   ├── 01-cpp-basics/
│   ├── 02-cmake-intro/
│   ├── 03-raylib-basics/
│   ├── 04-game-loop/
│   ├── 05-collision/
│   ├── 06-game-states/
│   ├── 07-raygui-basics/
│   └── 08-raygui-advanced/
├── games/             # 完整游戏项目
│   ├── brick-breaker/
│   ├── snake/
│   ├── tetris/
│   ├── tank-battle/
│   ├── tower-defense/
│   └── fps/
├── janet/             # Janet 互操作模块与 REPL 工作流
├── docs/              # 详细文档
└── cmake/             # CMake 工具脚本
```

## 📝 学习路径 / Learning Path

1. 从 `chapters/` 目录按顺序学习基础知识
2. 每章包含示例代码和详细说明文档
3. 完成章节学习后，开始构建 `games/` 中的项目
4. 每个游戏都有完整的源码和构建说明

## 🤝 贡献 / Contributing

欢迎提交问题和改进建议！

## 📄 许可证 / License

MIT License

## 🔗 相关资源 / Resources

- [Raylib Official Website](https://www.raylib.com/)
- [CMake Documentation](https://cmake.org/documentation/)
- [C++ Reference](https://en.cppreference.com/)
