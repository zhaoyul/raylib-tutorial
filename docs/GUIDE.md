# Raylib 游戏开发完整指南 / Complete Raylib Game Development Guide

## 目录 / Table of Contents

1. [环境搭建](#环境搭建)
2. [C++ 基础](#c-基础)
3. [CMake 构建系统](#cmake-构建系统)
4. [Raylib 入门](#raylib-入门)
5. [游戏开发基础](#游戏开发基础)
6. [游戏项目实战](#游戏项目实战)

---

## 环境搭建

### Windows

1. 安装 Visual Studio 2019+ 或 MinGW
2. 安装 CMake (3.15+)
3. 克隆本仓库

### Linux

```bash
sudo apt install build-essential cmake git
```

### macOS

```bash
brew install cmake
```

---

## C++ 基础

参见 `chapters/01-cpp-basics/`

### 核心概念

1. **变量和数据类型**
   - 基本类型：int, float, double, bool, char
   - 复合类型：数组, 结构体, 类

2. **函数**
   - 函数定义和声明
   - 参数传递（值传递、引用传递）
   - 返回值

3. **类和对象**
   - 封装、继承、多态
   - 构造函数和析构函数
   - 成员函数和成员变量

4. **STL 容器**
   - vector - 动态数组
   - deque - 双端队列
   - map - 键值对

---

## CMake 构建系统

参见 `chapters/02-cmake-intro/`

### 基本 CMakeLists.txt 结构

```cmake
cmake_minimum_required(VERSION 3.15)
project(MyGame)

set(CMAKE_CXX_STANDARD 17)

find_package(raylib REQUIRED)

add_executable(my-game main.cpp)
target_link_libraries(my-game raylib)
```

### 常用命令

```bash
# 配置
cmake -B build

# 编译
cmake --build build

# 清理
cmake --build build --target clean
```

---

## Raylib 入门

参见 `chapters/03-raylib-basics/`

### 核心功能

1. **窗口管理**
   ```cpp
   InitWindow(800, 600, "Title");
   SetTargetFPS(60);
   while (!WindowShouldClose()) { ... }
   CloseWindow();
   ```

2. **绘图函数**
   ```cpp
   DrawCircle(x, y, radius, color);
   DrawRectangle(x, y, width, height, color);
   DrawText(text, x, y, fontSize, color);
   ```

3. **输入处理**
   ```cpp
   IsKeyDown(KEY_SPACE);
   IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
   GetMousePosition();
   ```

4. **碰撞检测**
   ```cpp
   CheckCollisionRecs(rect1, rect2);
   CheckCollisionCircles(center1, r1, center2, r2);
   ```

---

## 游戏开发基础

### 游戏循环

```cpp
while (!WindowShouldClose()) {
    // 1. 输入处理
    HandleInput();
    
    // 2. 更新游戏逻辑
    Update(deltaTime);
    
    // 3. 渲染
    BeginDrawing();
    Draw();
    EndDrawing();
}
```

### 游戏状态管理

```cpp
enum GameState {
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER
};
```

### Raygui UI 系统

Raygui 是 Raylib 的官方 GUI 库：

```cpp
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

// 基础控件
if (GuiButton((Rectangle){10, 10, 120, 30}, "开始游戏")) {
    currentState = PLAYING;
}

// 滑块
float volume = GuiSlider((Rectangle){10, 50, 200, 20}, 
                         "音量", NULL, 0.5f, 0.0f, 1.0f);

// 样式定制
GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(BLUE));
```

### Delta Time

使用 delta time 确保游戏速度独立于帧率：

```cpp
float deltaTime = GetFrameTime();
position += velocity * deltaTime;
```

---

## 游戏项目实战

### 1. 打砖块 (Brick Breaker)

**学习重点：**
- 基础物理模拟
- 碰撞检测和响应
- 游戏状态管理

**代码位置：** `games/brick-breaker/`

### 2. 贪吃蛇 (Snake)

**学习重点：**
- 网格系统
- 数据结构（deque）
- 自碰撞检测

**代码位置：** `games/snake/`

### 3. 俄罗斯方块 (Tetris)

**学习重点：**
- 二维数组操作
- 旋转算法
- 行消除逻辑

**代码位置：** `games/tetris/`

### 4. 坦克大战 (Tank Battle)

**学习重点：**
- 多对象管理
- 子弹系统
- 简单AI

**代码位置：** `games/tank-battle/`

### 5. 塔防 (Tower Defense)

**学习重点：**
- 路径系统
- 范围检测
- 资源管理

**代码位置：** `games/tower-defense/`

### 6. 第一人称射击 (FPS)

**学习重点：**
- 3D 相机控制
- 射线检测
- 3D 碰撞

**代码位置：** `games/fps/`

---

## 进阶主题

### 性能优化

1. **对象池**：重用对象而不是频繁创建销毁
2. **空间分区**：使用四叉树等结构优化碰撞检测
3. **批量渲染**：减少绘制调用次数

### 设计模式

1. **状态模式**：管理游戏状态
2. **观察者模式**：事件系统
3. **工厂模式**：创建游戏对象

### 扩展功能

1. **音效和音乐**
   ```cpp
   InitAudioDevice();
   Sound sound = LoadSound("sound.wav");
   PlaySound(sound);
   ```

2. **粒子系统**：爆炸、火花等特效

3. **存档系统**：保存和加载游戏进度

---

## 学习路径建议

### 初学者（1-2周）
1. 完成 Chapter 1-3
2. 实现 Brick Breaker 和 Snake

### 中级（3-4周）
1. 完成 Chapter 4-6
2. 学习 Chapter 7-8 (Raygui UI 开发)
3. 实现 Tetris 和 Tank Battle
4. 学习基本设计模式

### 高级（5-8周）
1. 实现 Tower Defense 和 FPS
2. 添加高级功能（音效、粒子系统）
3. 优化性能
4. 尝试创建自己的游戏

---

## 常见问题

### Q: 如何调试游戏？

使用 `TraceLog()` 函数输出调试信息：
```cpp
TraceLog(LOG_INFO, "Player position: %f, %f", x, y);
```

### Q: 如何提高帧率？

1. 减少不必要的绘制调用
2. 优化碰撞检测
3. 使用对象池

### Q: 如何添加音效？

```cpp
InitAudioDevice();
Sound fxCoin = LoadSound("coin.wav");
PlaySound(fxCoin);
```

---

## 资源推荐

- [Raylib 官方文档](https://www.raylib.com/)
- [Raylib Cheatsheet](https://www.raylib.com/cheatsheet/cheatsheet.html)
- [C++ Reference](https://en.cppreference.com/)
- [LearnCpp.com](https://www.learncpp.com/)

---

## 贡献指南

欢迎提交问题报告和改进建议！

1. Fork 本仓库
2. 创建特性分支
3. 提交更改
4. 发起 Pull Request

---

## 许可证

MIT License - 详见 LICENSE 文件
