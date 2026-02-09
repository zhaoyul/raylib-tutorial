cmake_minimum_required(VERSION 3.15)

# 添加多个源文件
add_executable(cmake-intro 
    main.cpp
    math_utils.cpp
)

# 设置输出目录
set_target_properties(cmake-intro PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/chapters"
)
