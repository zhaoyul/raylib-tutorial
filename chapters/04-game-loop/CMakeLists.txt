cmake_minimum_required(VERSION 3.15)

add_executable(game-loop main.cpp)
target_link_libraries(game-loop raylib)

set_target_properties(game-loop PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/chapters"
)

if(WIN32)
    target_link_libraries(game-loop winmm)
endif()
