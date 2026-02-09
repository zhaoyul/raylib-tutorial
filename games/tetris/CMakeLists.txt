cmake_minimum_required(VERSION 3.15)

add_executable(tetris main.cpp)
target_link_libraries(tetris raylib)

set_target_properties(tetris PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/games"
)

if(WIN32)
    target_link_libraries(tetris winmm)
endif()
