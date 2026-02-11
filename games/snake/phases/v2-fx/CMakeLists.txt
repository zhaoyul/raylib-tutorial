cmake_minimum_required(VERSION 3.15)

# v2-fx - 粒子特效版本
project(snake-v2-fx VERSION 1.2.0 LANGUAGES CXX)

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
    particle.cpp
    particle.h
    screenshake.cpp
    screenshake.h
)

# 创建可执行文件
add_executable(snake-v2-fx ${SOURCES})

# 链接 Raylib
target_link_libraries(snake-v2-fx raylib)

# 设置输出目录
set_target_properties(snake-v2-fx PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/snake-phases"
)

# 编译选项
target_compile_features(snake-v2-fx PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(snake-v2-fx PRIVATE /W4)
else()
    target_compile_options(snake-v2-fx PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Windows 特定设置
if(WIN32)
    target_link_libraries(snake-v2-fx winmm)
endif()
