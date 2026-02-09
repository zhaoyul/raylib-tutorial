cmake_minimum_required(VERSION 3.15)
project(RaylibTutorial VERSION 1.0.0 LANGUAGES CXX)

# C++ 标准 / C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 导出编译命令 / Export compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 添加 CMake 模块路径 / Add CMake module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# 编译选项 / Compile options
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 查找或下载 Raylib / Find or download Raylib
find_package(raylib 5.0 QUIET)
if (NOT raylib_FOUND)
    message(STATUS "Raylib not found, fetching from GitHub...")
    include(FetchContent)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.0
        GIT_SHALLOW 1
    )
    FetchContent_MakeAvailable(raylib)
endif()

# 选项：构建教程章节 / Option: Build tutorial chapters
option(BUILD_CHAPTERS "Build tutorial chapters" ON)

# 选项：构建游戏 / Option: Build games
option(BUILD_GAMES "Build game projects" ON)

# 添加章节 / Add chapters
if(BUILD_CHAPTERS)
    add_subdirectory(chapters/01-cpp-basics)
    add_subdirectory(chapters/02-cmake-intro)
    add_subdirectory(chapters/03-raylib-basics)
    add_subdirectory(chapters/04-game-loop)
    add_subdirectory(chapters/05-collision)
    add_subdirectory(chapters/06-game-states)
endif()

# 添加游戏 / Add games
if(BUILD_GAMES)
    add_subdirectory(games/brick-breaker)
    add_subdirectory(games/snake)
    add_subdirectory(games/tetris)
    add_subdirectory(games/tank-battle)
    add_subdirectory(games/tower-defense)
    add_subdirectory(games/fps)
endif()
