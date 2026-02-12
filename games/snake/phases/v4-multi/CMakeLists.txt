cmake_minimum_required(VERSION 3.15)

# v4-multi - 双人模式与关卡编辑器
project(snake-v4-multi VERSION 1.4.0 LANGUAGES CXX)

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
    audio_system.cpp
    audio_system.h
    highscore.cpp
    highscore.h
    settings.cpp
    settings.h
    level.cpp
    level.h
)

# 创建可执行文件
add_executable(snake-v4-multi ${SOURCES})

# 链接 Raylib
target_link_libraries(snake-v4-multi raylib)

# 设置输出目录
set_target_properties(snake-v4-multi PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/snake-phases"
)

# 编译选项
target_compile_features(snake-v4-multi PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(snake-v4-multi PRIVATE /W4)
else()
    target_compile_options(snake-v4-multi PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Windows 特定设置
if(WIN32)
    target_link_libraries(snake-v4-multi winmm)
endif()
