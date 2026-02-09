cmake_minimum_required(VERSION 3.15)

add_executable(cpp-basics main.cpp)

# 设置输出目录
set_target_properties(cpp-basics PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/chapters"
)
