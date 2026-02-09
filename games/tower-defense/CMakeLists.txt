cmake_minimum_required(VERSION 3.15)

add_executable(tower-defense main.cpp)
target_link_libraries(tower-defense raylib)

set_target_properties(tower-defense PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/games"
)

if(WIN32)
    target_link_libraries(tower-defense winmm)
endif()
