cmake_minimum_required(VERSION 3.15)
project(RaylibTutorial VERSION 1.0.0 LANGUAGES CXX)

# C++ 标准 / C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Web 构建：输出可直接在浏览器打开的 html 启动文件
if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

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

# 下载 Raygui（单头文件 GUI 库）/ Download Raygui
include(FetchContent)
FetchContent_Declare(
    raygui
    GIT_REPOSITORY https://github.com/raysan5/raygui.git
    GIT_TAG 4.0
    GIT_SHALLOW 1
)
FetchContent_MakeAvailable(raygui)
# 设置 raygui 头文件路径变量
set(RAYGUI_INCLUDE_DIR ${raygui_SOURCE_DIR}/src)

# 选项：构建教程章节 / Option: Build tutorial chapters
option(BUILD_CHAPTERS "Build tutorial chapters" ON)

# 选项：构建游戏 / Option: Build games
option(BUILD_GAMES "Build game projects" ON)

# 选项：构建 Snake 扩展版本 / Option: Build Snake phases
option(BUILD_SNAKE_PHASES "Build Snake game phases (v1-items, etc.)" ON)

# 选项：构建 Janet 互操作模块 / Option: Build Janet interop module
option(BUILD_JANET "Build Janet raylib module" OFF)

# 添加章节 / Add chapters
if(BUILD_CHAPTERS)
    add_subdirectory(chapters/01-cpp-basics)
    add_subdirectory(chapters/02-cmake-intro)
    add_subdirectory(chapters/03-raylib-basics)
    add_subdirectory(chapters/04-game-loop)
    add_subdirectory(chapters/05-collision)
    add_subdirectory(chapters/06-game-states)
    add_subdirectory(chapters/07-raygui-basics)
    add_subdirectory(chapters/08-raygui-advanced)
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

# 添加 Snake 扩展版本 / Add Snake phases
if(BUILD_SNAKE_PHASES)
    add_subdirectory(games/snake/phases/v1-items)
    add_subdirectory(games/snake/phases/v2-fx)
    add_subdirectory(games/snake/phases/v3-audio)
    add_subdirectory(games/snake/phases/v4-multi)
endif()

# 添加 Janet 模块 / Add Janet module
if(BUILD_JANET)
    add_subdirectory(janet)
endif()
