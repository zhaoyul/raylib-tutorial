cmake_minimum_required(VERSION 3.15)

# 第7章: Raygui 基础
project(chapter07_raygui_basics)

# 创建可执行文件 - 使用明确名称而不是变量，便于 .dir-locals.el 解析
add_executable(chapter07_raygui_basics main.cpp)

# 链接 Raylib
target_link_libraries(chapter07_raygui_basics raylib)

# 包含目录（raylib 和 raygui）
target_include_directories(chapter07_raygui_basics PRIVATE 
    ${RAYGUI_INCLUDE_DIR}
)

# 设置输出目录
set_target_properties(chapter07_raygui_basics PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/chapters
)
