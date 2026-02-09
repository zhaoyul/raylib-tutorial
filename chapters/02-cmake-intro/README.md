# 第2章: CMake 入门 / Chapter 2: CMake Introduction

## 学习目标 / Learning Objectives

- 理解 CMake 的作用
- 学习编写基本的 CMakeLists.txt
- 掌握项目构建流程
- 了解依赖管理

## 内容概览 / Content Overview

### 2.1 什么是 CMake？

CMake 是一个跨平台的构建系统生成器，它可以：
- 生成适合不同平台的构建文件（Makefile, Visual Studio 项目等）
- 管理项目依赖
- 简化复杂项目的构建过程

### 2.2 基本的 CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.15)
project(MyGame VERSION 1.0.0)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)

# 添加可执行文件
add_executable(my-game main.cpp)
```

### 2.3 构建流程

```bash
# 创建构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 构建项目
cmake --build .

# 运行程序
./my-game
```

### 2.4 添加库依赖

```cmake
# 查找外部库
find_package(raylib REQUIRED)

# 链接库到目标
target_link_libraries(my-game raylib)
```

### 2.5 多文件项目

```cmake
# 添加多个源文件
add_executable(my-game 
    main.cpp
    player.cpp
    enemy.cpp
)
```

## 实践示例 / Practice Examples

本章包含：
1. 单文件项目示例
2. 多文件项目示例
3. 使用外部库的示例

## 下一步 / Next Steps

完成本章后，继续学习第3章：Raylib 基础
