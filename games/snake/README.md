# 贪吃蛇 / Snake

## 游戏简介 / Game Description

经典贪吃蛇游戏。控制蛇吃食物，蛇会变长。

Classic snake game. Control the snake to eat food and grow longer.

## 游戏特性 / Features

- 蛇的移动和成长
- 食物生成系统
- 碰撞检测（墙壁和自身）
- 分数系统
- 难度递增

## 操作说明 / Controls

- **方向键 / WASD**: 控制方向
- **空格键**: 开始游戏
- **ESC**: 返回菜单

## 学习要点 / Learning Points

- 网格系统
- 链表/数组数据结构 (`std::deque`)
- 游戏速度控制
- 自碰撞检测
- 简单状态机

## 🚀 扩展路线图 / Roadmap

这是 **v0-base** 基础版本。完整的渐进式学习路线请看：

📄 **[ROADMAP.md](./ROADMAP.md)** - 详细的多阶段开发计划

### 各版本概览

| 版本               | 功能              | 学习重点                |
|--------------------|-------------------|-------------------------|
| **v0-base** (当前) | 经典贪吃蛇        | `std::deque`, 状态机    |
| **v1-items**       | 多种食物 + 障碍物 | 继承、多态、枚举类      |
| **v2-fx**          | 粒子系统 + 动画   | 对象池、缓动函数        |
| **v3-audio**       | 音效 + 高分榜     | JSON、文件I/O、资源管理 |
| **v4-multi**       | 双人模式 + 编辑器 | ECS、BFS寻路、设计模式  |

### 快速开始扩展

```bash
# 查看 v0 代码
cat games/snake/main.cpp

# 复制一份开始扩展
cp -r games/snake games/snake-myversion

# 参考 ROADMAP.md 的 Phase 1 开始实现
```

## 📝 代码亮点 / Code Highlights

### 蛇身数据结构
```cpp
std::deque<Position> snake;
// push_front() 添加新头部 - O(1)
// pop_back() 移除尾部 - O(1)
```

### 帧率无关的移动
```cpp
float moveTimer = 0;
float moveInterval = 0.15f;  // 每0.15秒移动一次

// 在每一帧：
moveTimer += GetFrameTime();
if (moveTimer >= moveInterval) {
    moveTimer = 0;
    // 移动蛇...
}
```

### 简单状态机
```cpp
enum GameState { MENU, PLAYING, GAME_OVER };
GameState state = MENU;

switch (state) {
    case MENU: /* 处理菜单 */ break;
    case PLAYING: /* 处理游戏 */ break;
    case GAME_OVER: /* 处理结束 */ break;
}
```

## 🔧 扩展挑战 / Extension Challenges

1. **简单：** 添加暂停功能 (P 键)
2. **中等：** 实现可调节的游戏速度
3. **困难：** 添加一个自动寻路的 AI 蛇

## 📚 相关章节

- [Chapter 3: Raylib 基础](../../chapters/03-raylib-basics/)
- [Chapter 4: 游戏循环](../../chapters/04-game-loop/)
- [Chapter 5: 碰撞检测](../../chapters/05-collision/)
- [Chapter 6: 游戏状态](../../chapters/06-game-states/)
