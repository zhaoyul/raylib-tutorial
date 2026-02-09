cmake_minimum_required(VERSION 3.15)

add_executable(fps main.cpp)
target_link_libraries(fps raylib)

set_target_properties(fps PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/games"
)

if(WIN32)
    target_link_libraries(fps winmm)
endif()
