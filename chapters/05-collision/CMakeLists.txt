cmake_minimum_required(VERSION 3.15)

add_executable(collision main.cpp)
target_link_libraries(collision raylib)

set_target_properties(collision PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/chapters"
)

if(WIN32)
    target_link_libraries(collision winmm)
endif()
