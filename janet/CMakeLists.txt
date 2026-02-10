find_package(PkgConfig REQUIRED)
pkg_check_modules(JANET REQUIRED janet)

add_library(raylib_janet MODULE raylib_janet.cpp)
target_include_directories(raylib_janet PRIVATE ${JANET_INCLUDE_DIRS})
target_link_directories(raylib_janet PRIVATE ${JANET_LIBRARY_DIRS})
target_compile_options(raylib_janet PRIVATE ${JANET_CFLAGS_OTHER})
target_link_libraries(raylib_janet PRIVATE raylib ${JANET_LIBRARIES})

set_target_properties(raylib_janet PROPERTIES
    PREFIX ""
    OUTPUT_NAME raylib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/janet
)
