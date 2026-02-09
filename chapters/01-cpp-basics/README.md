# 第1章: C++ 基础 / Chapter 1: C++ Basics

## 学习目标 / Learning Objectives

- 理解 C++ 基本语法
- 掌握变量和数据类型
- 学习函数定义和调用
- 了解类和对象

## 内容概览 / Content Overview

### 1.1 Hello World

最简单的 C++ 程序：

```cpp
#include <iostream>

int main() {
    std::cout << "Hello, Raylib Tutorial!" << std::endl;
    return 0;
}
```

### 1.2 变量和数据类型 / Variables and Data Types

```cpp
// 基本数据类型
int score = 0;          // 整数
float speed = 5.0f;     // 浮点数
double position = 10.5; // 双精度浮点数
bool isAlive = true;    // 布尔值
char grade = 'A';       // 字符
```

### 1.3 函数 / Functions

```cpp
// 函数定义
int add(int a, int b) {
    return a + b;
}

// 函数调用
int result = add(3, 4); // result = 7
```

### 1.4 类和对象 / Classes and Objects

```cpp
class Player {
public:
    int health;
    float x, y;
    
    void move(float dx, float dy) {
        x += dx;
        y += dy;
    }
};

// 创建对象
Player player;
player.health = 100;
player.move(5.0f, 0.0f);
```

## 实践示例 / Practice Examples

本章的示例代码展示了：
1. 基本输出程序
2. 变量操作示例
3. 简单的计算器程序
4. 基础类的使用

## 下一步 / Next Steps

完成本章后，继续学习第2章：CMake 入门
