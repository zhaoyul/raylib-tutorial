# 第5章: 碰撞检测 / Chapter 5: Collision Detection

## 学习目标 / Learning Objectives

- 掌握基本碰撞检测算法
- 学习 AABB 碰撞
- 了解圆形碰撞检测
- 实现碰撞响应

## 碰撞检测类型

### 矩形碰撞 (AABB)
```cpp
bool CheckCollisionRecs(Rectangle rec1, Rectangle rec2);
```

### 圆形碰撞
```cpp
bool CheckCollisionCircles(Vector2 center1, float radius1,
                          Vector2 center2, float radius2);
```

### 点和矩形碰撞
```cpp
bool CheckCollisionPointRec(Vector2 point, Rectangle rec);
```
