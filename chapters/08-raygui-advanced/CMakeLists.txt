cmake_minimum_required(VERSION 3.15)

# 第8章: Raygui 高级应用
project(chapter08_raygui_advanced)

# 创建可执行文件 - 使用明确名称而不是变量，便于 .dir-locals.el 解析
add_executable(chapter08_raygui_advanced main.cpp)

# 链接 Raylib
target_link_libraries(chapter08_raygui_advanced raylib)

# 包含目录（raylib 和 raygui）
target_include_directories(chapter08_raygui_advanced PRIVATE 
    ${RAYGUI_INCLUDE_DIR}
)

# 设置输出目录
set_target_properties(chapter08_raygui_advanced PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/chapters
)

# Test executable
add_executable(test_chinese test_chinese.cpp)
target_link_libraries(test_chinese raylib)
target_include_directories(test_chinese PRIVATE ${RAYGUI_INCLUDE_DIR})
set_target_properties(test_chinese PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
