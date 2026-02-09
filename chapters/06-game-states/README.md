# 第6章: 游戏状态 / Chapter 6: Game States

## 学习目标 / Learning Objectives

- 理解游戏状态机
- 学习状态切换
- 实现菜单系统
- 掌握UI绘制

## 游戏状态

常见的游戏状态：
- MENU - 主菜单
- PLAYING - 游戏进行中
- PAUSED - 暂停
- GAME_OVER - 游戏结束

```cpp
enum GameState {
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER
};
```
