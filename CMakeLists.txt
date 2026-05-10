cmake_minimum_required(VERSION 4.2)
project(mycraft-vulkan
    VERSION 0.0.1
    DESCRIPTION "palapapa's 3D renderer in Vulkan"
    HOMEPAGE_URL https://github.com/palapapa/mycraft-vulkan
    LANGUAGES CXX)
include(CheckIPOSupported)
option(ENABLE_CLANG_TIDY
    "Whether to run clang-tidy when building."
    ${PROJECT_IS_TOP_LEVEL})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_executable(mycraft-vulkan)
target_compile_features(mycraft-vulkan PRIVATE cxx_std_26)
set(GCC_WARNINGS
    -Wall
    -Wextra
    -Wpedantic)
set(CLANG_WARNINGS
    -Weverything
    -Wno-c++98-compat
    -Wno-c++98-compat-pedantic
    -Wno-pre-c23-compat
    -Wno-reserved-macro-identifier
    -Wno-padded
    -Wno-unsafe-buffer-usage
    -Wno-empty-translation-unit
    -Wno-switch-default
    -Wno-weak-vtables
    -Wno-exit-time-destructors
    -Wno-global-constructors
    -ferror-limit=0)
target_compile_options(mycraft-vulkan PRIVATE
    "$<$<CXX_COMPILER_ID:GNU>:${GCC_WARNINGS}>"
    "$<$<CXX_COMPILER_ID:Clang>:${CLANG_WARNINGS}>")
check_ipo_supported(RESULT IS_IPO_SUPPORTED)
if (IS_IPO_SUPPORTED)
    set_target_properties(mycraft-vulkan PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION TRUE
        INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
endif ()
if (ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY NAMES clang-tidy)
    if (CLANG_TIDY)
        set_target_properties(mycraft-vulkan PROPERTIES
            CXX_CLANG_TIDY "${CLANG_TIDY};-p;${CMAKE_BINARY_DIR}")
    else ()
        message(WARNING "ENABLE_CLANG_TIDY is enabled but clang-tidy is not found. Skipping.")
    endif ()
endif ()
add_subdirectory(src)
add_subdirectory(include/mycraft-vulkan)
