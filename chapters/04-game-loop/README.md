# 第4章: 游戏循环 / Chapter 4: Game Loop

## 学习目标 / Learning Objectives

- 理解游戏循环的结构
- 学习更新和渲染分离
- 掌握帧率控制
- 了解游戏状态管理

## 内容概览 / Content Overview

### 4.1 游戏循环结构

```cpp
while (!WindowShouldClose()) {
    // 1. 处理输入
    HandleInput();

    // 2. 更新游戏状态
    Update(deltaTime);

    // 3. 渲染画面
    Render();
}
```

### 4.2 Delta Time

使用 delta time 确保游戏在不同帧率下表现一致：

```cpp
float deltaTime = GetFrameTime();
position += velocity * deltaTime;
```

## 实践示例 / Practice Examples

本章包含完整的游戏循环示例，展示了如何组织游戏代码。
