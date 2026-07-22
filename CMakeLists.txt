cmake_minimum_required(VERSION 4.2)
project(mycraft-vulkan
    VERSION 0.0.1
    DESCRIPTION "palapapa's 3D renderer in Vulkan"
    HOMEPAGE_URL https://github.com/palapapa/mycraft-vulkan
    LANGUAGES CXX)
include(CheckIPOSupported)
option(ENABLE_CLANG_TIDY
    "Whether to run clang-tidy when building."
    OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(PROJECT_IS_TOP_LEVEL)
    set(COMPILE_COMMANDS_LINK_TARGET
        "${CMAKE_BINARY_DIR}/compile_commands.json")
    cmake_path(RELATIVE_PATH COMPILE_COMMANDS_LINK_TARGET
        BASE_DIRECTORY "${CMAKE_SOURCE_DIR}")
    file(CREATE_LINK
        "${COMPILE_COMMANDS_LINK_TARGET}"
        "${CMAKE_SOURCE_DIR}/compile_commands.json"
        SYMBOLIC)
endif()

add_executable(mycraft-vulkan)
target_compile_features(mycraft-vulkan PRIVATE cxx_std_26)
target_compile_definitions(mycraft-vulkan PRIVATE
    VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
    VULKAN_HPP_USE_STD_EXPECTED
    VULKAN_HPP_NO_EXCEPTIONS
    VK_NO_PROTOTYPES=1 # When VK_NO_PROTOTYPES=1, VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1 automatically.
    QUILL_DISABLE_NON_PREFIXED_MACROS
    QUILL_DETAILED_FUNCTION_NAME
    GLFW_INCLUDE_VULKAN
    GLFW_INCLUDE_NONE)
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
    -Wno-unused-macros
    -Wno-shadow-field-in-constructor
    -ferror-limit=0)
target_compile_options(mycraft-vulkan PRIVATE
    "$<$<CXX_COMPILER_ID:GNU>:${GCC_WARNINGS}>"
    "$<$<CXX_COMPILER_ID:Clang>:${CLANG_WARNINGS}>")
check_ipo_supported(RESULT IS_IPO_SUPPORTED)

if(IS_IPO_SUPPORTED)
    set_target_properties(mycraft-vulkan PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION TRUE
        INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
endif()

if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY NAMES clang-tidy)

    if(CLANG_TIDY)
        set_target_properties(mycraft-vulkan PROPERTIES
            CXX_CLANG_TIDY "${CLANG_TIDY};-p;${CMAKE_BINARY_DIR}")
    else()
        message(WARNING "ENABLE_CLANG_TIDY is enabled but clang-tidy is not found. Skipping.")
    endif()
endif()

find_package(quill CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(VulkanHeaders CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Ktx CONFIG REQUIRED)
find_package(fastgltf CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS leaf)
find_package(vk-bootstrap CONFIG REQUIRED)
target_link_libraries(mycraft-vulkan
    PRIVATE quill::quill
    PRIVATE EnTT::EnTT
    PRIVATE Vulkan::Headers
    PRIVATE GPUOpen::VulkanMemoryAllocator
    PRIVATE glfw
    PRIVATE glm::glm
    PRIVATE KTX::ktx
    PRIVATE fastgltf::fastgltf
    PRIVATE Boost::leaf
    PRIVATE vk-bootstrap::vk-bootstrap)
add_subdirectory(src)
add_subdirectory(include/mycraft-vulkan)
