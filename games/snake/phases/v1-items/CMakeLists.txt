cmake_minimum_required(VERSION 3.15)

# v1-items - 道具与障碍系统版本
project(snake-v1-items VERSION 1.1.0 LANGUAGES CXX)

# 源文件
set(SOURCES
    main.cpp
    game.cpp
    game.h
    snake.cpp
    snake.h
    item.cpp
    item.h
    obstacle.cpp
    obstacle.h
)

# 创建可执行文件
add_executable(snake-v1-items ${SOURCES})

# 链接 Raylib
target_link_libraries(snake-v1-items raylib)

# 设置输出目录
set_target_properties(snake-v1-items PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/snake-phases"
)

# 编译选项
target_compile_features(snake-v1-items PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(snake-v1-items PRIVATE /W4)
else()
    target_compile_options(snake-v1-items PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Windows 特定设置
if(WIN32)
    target_link_libraries(snake-v1-items winmm)
endif()
