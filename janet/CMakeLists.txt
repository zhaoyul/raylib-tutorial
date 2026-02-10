find_package(PkgConfig REQUIRED)
pkg_check_modules(JANET REQUIRED janet)

add_library(raylib_janet MODULE raylib_janet.cpp)
target_include_directories(raylib_janet SYSTEM PRIVATE ${JANET_INCLUDE_DIRS})
target_compile_options(raylib_janet PRIVATE ${JANET_CFLAGS_OTHER})
target_link_libraries(raylib_janet PRIVATE raylib)

# Janet's CLI binary (e.g. Homebrew on macOS) is often statically linked, so linking
# the module against libjanet would create a second Janet runtime and crash at import time.
# Instead, leave Janet API symbols unresolved and let them resolve against the host process.
if(APPLE)
    target_link_options(raylib_janet PRIVATE "-Wl,-undefined,dynamic_lookup")
endif()

set_target_properties(raylib_janet PROPERTIES
    PREFIX ""
    OUTPUT_NAME raylib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/janet
)
