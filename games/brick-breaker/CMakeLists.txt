cmake_minimum_required(VERSION 3.15)

add_executable(brick-breaker main.cpp)
target_link_libraries(brick-breaker raylib)

set_target_properties(brick-breaker PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/games"
)

if(WIN32)
    target_link_libraries(brick-breaker winmm)
endif()
