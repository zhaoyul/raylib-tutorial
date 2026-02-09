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

### 构建步骤 / Build Instructions

```bash
# 克隆仓库 / Clone repository
git clone https://github.com/zhaoyul/raylib-tutorial.git
cd raylib-tutorial

# 创建构建目录 / Create build directory
mkdir build && cd build

# 配置和构建 / Configure and build
cmake ..
cmake --build .

# 运行游戏 (示例) / Run game (example)
./games/brick-breaker/brick-breaker
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