cmake_minimum_required(VERSION 3.15)

# v3-audio - 音效与数据存储版本
project(snake-v3-audio VERSION 1.3.0 LANGUAGES CXX)

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
)

# 创建可执行文件
add_executable(snake-v3-audio ${SOURCES})

# 链接 Raylib
target_link_libraries(snake-v3-audio raylib)

# 设置输出目录
set_target_properties(snake-v3-audio PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/snake-phases"
)

# 编译选项
target_compile_features(snake-v3-audio PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(snake-v3-audio PRIVATE /W4)
else()
    target_compile_options(snake-v3-audio PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Windows 特定设置
if(WIN32)
    target_link_libraries(snake-v3-audio winmm)
endif()
