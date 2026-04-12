# Raylib 游戏开发教程 / Raylib Game Development Tutorial

一个围绕 C++、CMake、Raylib 和 Raygui 的教程仓库：先学基础章节，再阅读完整游戏，再按需进入 Snake 渐进版本和 Janet REPL 实验环境。

This repository teaches C++, CMake, Raylib, and Raygui through ordered tutorial chapters, complete game examples, progressive Snake phases, and an optional Janet REPL workflow.

## 仓库内容 / What Is In This Repo

### 教程章节 / Chapters

| 路径 | 主题 | 目标 |
|------|------|------|
| `chapters/01-cpp-basics` | C++ basics | 变量、函数、类、容器 |
| `chapters/02-cmake-intro` | CMake intro | 目标、依赖、构建目录 |
| `chapters/03-raylib-basics` | Raylib basics | 窗口、绘图、输入 |
| `chapters/04-game-loop` | Game loop | update/render/delta time |
| `chapters/05-collision` | Collision | AABB、圆形碰撞 |
| `chapters/06-game-states` | Game states | 菜单、暂停、结束状态 |
| `chapters/07-raygui-basics` | Raygui basics | 按钮、滑块、文本框 |
| `chapters/08-raygui-advanced` | Raygui advanced | 样式、布局、复杂界面 |

### 游戏与示例 / Games And Examples

| 路径 | 内容 | 重点 |
|------|------|------|
| `games/brick-breaker` | Brick Breaker | 碰撞响应、状态管理 |
| `games/snake` | Snake base game | 网格移动、`std::deque`、状态机 |
| `games/snake/phases` | Snake phases | 从 v0 到 v4 的渐进扩展 |
| `games/tetris` | Tetris | 二维数组、旋转、消行 |
| `games/tank-battle` | Tank Battle | 多对象管理、子弹与 AI |
| `games/tower-defense` | Tower Defense | 路径、范围检测、波次 |
| `games/fps` | FPS | 3D camera、射击与场景 |
| `games/git-fighter` | Git tutorial game | libgit2、可视化、关卡脚本 |

### 可选模块 / Optional Module

| 路径 | 内容 |
|------|------|
| `janet/` | Janet + Raylib interop module, REPL workflow, NetREPL examples |

## 学习顺序 / Suggested Learning Order

1. 从 `chapters/01` 到 `chapters/08` 依次阅读和运行。
2. 选一个完整游戏开始拆读，通常建议 `games/brick-breaker` 或 `games/snake`。
3. 想继续扩展 Snake 时，再进入 `games/snake/phases/README.md` 和 `games/snake/ROADMAP.md`。
4. 需要 REPL 原型、热替换或脚本化实验时，再启用 `janet/`。

## 依赖与前置条件 / Prerequisites

### 基础依赖 / Base Requirements

- C++17 compiler
- CMake 3.15+
- Git
- 首次配置时需要网络，用于拉取 `raylib` 和 `raygui`

### 完整构建额外依赖 / Extra Requirements For Full Builds

`games/git-fighter` 会在 `BUILD_GAMES=ON` 时一并参与配置，因此完整构建还需要：

- `pkg-config`
- `libgit2` development files

### Linux 图形库依赖 / Linux Graphics Packages

Raylib desktop builds need X11/OpenGL development packages on Linux. See [docs/BUILD.md](docs/BUILD.md) for concrete package lists.

## 快速开始 / Quick Start

### 方案 A: 先跑教程章节（推荐第一次使用） / Profile A: Chapters-Only First Run

这条路径避开了 `git-fighter` 的 `libgit2` 依赖，更适合第一次验证工具链。

This is the safest first build because it avoids the extra `git-fighter` dependency chain.

```bash
cmake -S . -B build \
  -DBUILD_GAMES=OFF \
  -DBUILD_SNAKE_PHASES=OFF \
  -DBUILD_JANET=OFF \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5

cmake --build build --target cpp-basics chapter07_raygui_basics -j

./build/bin/chapters/cpp-basics
./build/bin/chapters/chapter07_raygui_basics
```

### 方案 B: 构建整个教程仓库 / Profile B: Full Tutorial Build

先安装 `pkg-config` 和 `libgit2`，然后再配置：

Install `pkg-config` and `libgit2` first, then configure the whole repo:

```bash
cmake -S . -B build -DCMAKE_POLICY_VERSION_MINIMUM=3.5
cmake --build build -j

./build/bin/games/snake
./build/bin/games/brick-breaker
./build/bin/git-fighter
./build/bin/snake-phases/snake-v1-items
```

### 方案 C: 只玩 Snake 扩展阶段 / Profile C: Snake Phases Only

```bash
cmake -S . -B build \
  -DBUILD_CHAPTERS=OFF \
  -DBUILD_GAMES=OFF \
  -DBUILD_SNAKE_PHASES=ON \
  -DBUILD_JANET=OFF \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5

cmake --build build --target snake-v1-items snake-v2-fx -j

./build/bin/snake-phases/snake-v1-items
./build/bin/snake-phases/snake-v2-fx
```

### 方案 D: 只启用 Janet / Profile D: Janet Only

```bash
cmake -S . -B build \
  -DBUILD_CHAPTERS=OFF \
  -DBUILD_GAMES=OFF \
  -DBUILD_SNAKE_PHASES=OFF \
  -DBUILD_JANET=ON \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5

cmake --build build --target raylib_janet -j
```

更多 Janet 工作流见 [docs/JANET.md](docs/JANET.md)。

## 运行产物位置 / Output Layout

| 构建内容 | 输出目录 | 示例 |
|----------|----------|------|
| Chapters | `build/bin/chapters/` | `cpp-basics`, `chapter07_raygui_basics` |
| Main games | `build/bin/games/` | `snake`, `tetris`, `fps` |
| Snake phases | `build/bin/snake-phases/` | `snake-v1-items`, `snake-v4-multi` |
| Git Fighter + demos | `build/bin/` | `git-fighter`, `demo-graph`, `demo-split-view` |
| Janet module | `build/janet/` | `raylib.so` / `raylib.dll` |

## 文档导航 / Documentation Map

- [docs/BUILD.md](docs/BUILD.md): platform prerequisites, build profiles, troubleshooting
- [docs/GUIDE.md](docs/GUIDE.md): broader learning guide
- [docs/JANET.md](docs/JANET.md): Janet REPL and NetREPL workflow
- [docs/SUMMARY.md](docs/SUMMARY.md): repo-wide structure summary
- [games/snake/ROADMAP.md](games/snake/ROADMAP.md): Snake progression plan
- [games/git-fighter/README.md](games/git-fighter/README.md): Git Fighter build and run notes
- [data/fonts/README.md](data/fonts/README.md): Chinese font setup for the Raygui chapters

## 常见问题 / Troubleshooting Highlights

### `Compatibility with CMake < 3.5 has been removed`

This happens with CMake 4 when configuring the fetched Raylib 5.0 source. Add this flag:

```bash
-DCMAKE_POLICY_VERSION_MINIMUM=3.5
```

Keeping that flag in your configure command is safe.

### `Package 'libgit2' not found`

`BUILD_GAMES=ON` includes `games/git-fighter`, which requires `pkg-config` and `libgit2`.

You have two options:

1. Install `pkg-config` and `libgit2` development files, then rerun CMake.
2. For a simpler first run, configure with `-DBUILD_GAMES=OFF`.

### Raygui 章节只显示英文 / Raygui Chapters Fall Back To English

`chapters/07-raygui-basics` and `chapters/08-raygui-advanced` look for fonts under `data/fonts/`. Download one of the supported fonts described in [data/fonts/README.md](data/fonts/README.md).

### 无图形环境 / Headless Or CI Environment

Desktop builds need graphics libraries and a usable display environment. On servers or CI runners, install the OS graphics packages first and use a display server or virtual display when running GUI programs.

## 仓库结构 / Repository Layout

```text
raylib-tutorial/
├── chapters/               # Ordered tutorial chapters
├── games/                  # Full game projects and Git Fighter
├── janet/                  # Janet interop module and examples
├── docs/                   # Build, guide, summary, Janet docs
├── data/fonts/             # Optional Chinese fonts for Raygui chapters
├── CMakeLists.txt          # Top-level build entry
└── .dir-locals.el          # Editor helpers
```

## 许可证 / License

MIT License.
