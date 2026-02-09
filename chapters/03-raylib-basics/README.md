# 第3章: Raylib 基础 / Chapter 3: Raylib Basics

## 学习目标 / Learning Objectives

- 了解 Raylib 库的功能
- 创建第一个 Raylib 窗口
- 学习基本图形绘制
- 掌握键盘和鼠标输入

## 内容概览 / Content Overview

### 3.1 什么是 Raylib？

Raylib 是一个简单易用的游戏开发库，提供：
- 窗口管理
- 2D/3D 图形绘制
- 输入处理（键盘、鼠标、游戏手柄）
- 音频播放
- 碰撞检测

### 3.2 创建窗口

```cpp
#include "raylib.h"

int main() {
    // 初始化窗口
    InitWindow(800, 600, "My First Raylib Window");
    SetTargetFPS(60);
    
    // 主循环
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Hello Raylib!", 300, 250, 20, BLACK);
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
```

### 3.3 绘制图形

```cpp
// 绘制圆形
DrawCircle(400, 300, 50, RED);

// 绘制矩形
DrawRectangle(100, 100, 200, 100, BLUE);

// 绘制线条
DrawLine(0, 0, 800, 600, GREEN);
```

### 3.4 处理输入

```cpp
// 键盘输入
if (IsKeyDown(KEY_RIGHT)) {
    x += speed;
}

// 鼠标输入
if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    Vector2 mousePos = GetMousePosition();
}
```

## 实践示例 / Practice Examples

本章包含：
1. 基本窗口程序
2. 图形绘制示例
3. 交互式移动方块
4. 简单的绘图应用

## 下一步 / Next Steps

完成本章后，继续学习第4章：游戏循环
