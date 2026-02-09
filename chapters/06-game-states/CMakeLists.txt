cmake_minimum_required(VERSION 3.15)

add_executable(game-states main.cpp)
target_link_libraries(game-states raylib)

set_target_properties(game-states PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/chapters"
)

if(WIN32)
    target_link_libraries(game-states winmm)
endif()
