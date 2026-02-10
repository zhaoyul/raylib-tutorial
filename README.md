# Raylib 游戏开发教程 / Raylib Game Development Tutorial

## 📚 项目简介 / Project Introduction

这是一个从零开始学习 C++、CMake 和 Raylib 的完整教程项目。通过构建经典游戏来掌握游戏开发的核心概念。

This is a comprehensive tutorial project for learning C++, CMake, and Raylib from scratch. Master game development core concepts by building classic games.

## 🎮 游戏项目 / Game Projects

本教程包含以下游戏项目：

1. **打砖块 (Brick Breaker)** - 学习基础游戏循环、碰撞检测
2. **贪吃蛇 (Snake)** - 学习游戏状态管理、数据结构
3. **俄罗斯方块 (Tetris)** - 学习二维数组、旋转算法
4. **坦克大战 (Tank Battle)** - 学习精灵动画、多对象管理
5. **塔防游戏 (Tower Defense)** - 学习路径寻找、升级系统
6. **第一人称射击 (FPS)** - 学习3D图形、相机控制

## 📖 学习章节 / Learning Chapters

- **第1章: C++ 基础** - 变量、函数、类、指针
- **第2章: CMake 入门** - 构建系统、依赖管理
- **第3章: Raylib 基础** - 窗口、绘图、输入处理
- **第4章: 游戏循环** - 更新、渲染、帧率控制
- **第5章: 碰撞检测** - AABB、圆形碰撞、SAT
- **第6章: 游戏状态** - 状态机、UI系统

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
```

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
│   └── 06-game-states/
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
