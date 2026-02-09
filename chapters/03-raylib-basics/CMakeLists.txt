cmake_minimum_required(VERSION 3.15)

add_executable(raylib-basics main.cpp)

# 链接 Raylib
target_link_libraries(raylib-basics raylib)

# 设置输出目录
set_target_properties(raylib-basics PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/chapters"
)

# Windows 特定设置
if(WIN32)
    target_link_libraries(raylib-basics winmm)
endif()
