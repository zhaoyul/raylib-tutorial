cmake_minimum_required(VERSION 3.15)

add_executable(tank-battle main.cpp)
target_link_libraries(tank-battle raylib)

set_target_properties(tank-battle PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/games"
)

if(WIN32)
    target_link_libraries(tank-battle winmm)
endif()
