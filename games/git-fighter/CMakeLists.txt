cmake_minimum_required(VERSION 3.15)
project(GitFighter VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find or fetch raylib
find_package(raylib 5.0 QUIET)
if (NOT raylib_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.0
        GIT_SHALLOW 1
    )
    FetchContent_MakeAvailable(raylib)
endif()

# Find libgit2
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBGIT2 REQUIRED libgit2)

# Main executable
add_executable(git-fighter
    src/main.cpp
    src/game.cpp
    src/level_manager.cpp
    src/level_font.cpp
    src/git_wrapper.cpp
    src/visualizer.cpp
    src/ui_manager.cpp
    src/git_graph.cpp
    src/git_visualization.cpp
    src/git_visualization_panels.cpp
    src/gui_manager.cpp
    src/ui_theme.cpp
    src/git_console.cpp
    levels/level_01_realgit.cpp
    levels/level_02_branch.cpp
    levels/level_03_merge.cpp
    levels/level_04_remote.cpp
    levels/level_01_gui.cpp
)

# Demo executables
add_executable(demo-graph
    src/demo_graph.cpp
    src/git_graph.cpp
    src/git_visualization.cpp
    src/git_visualization_panels.cpp
    src/git_wrapper.cpp
)

add_executable(demo-split-view
    src/demo_split_view.cpp
    src/git_visualization.cpp
    src/git_visualization_panels.cpp
    src/git_wrapper.cpp
)

target_include_directories(git-fighter PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/levels
    ${LIBGIT2_INCLUDE_DIRS}
)

target_link_libraries(git-fighter 
    raylib
    ${LIBGIT2_LIBRARIES}
)

target_include_directories(demo-graph PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${LIBGIT2_INCLUDE_DIRS}
)

target_link_libraries(demo-graph raylib ${LIBGIT2_LIBRARIES})

target_compile_options(git-fighter PRIVATE ${LIBGIT2_CFLAGS_OTHER})

set_target_properties(git-fighter PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

set_target_properties(demo-graph PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

set_target_properties(demo-split-view PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_include_directories(demo-split-view PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${LIBGIT2_INCLUDE_DIRS}
)

target_link_libraries(demo-split-view raylib ${LIBGIT2_LIBRARIES})
