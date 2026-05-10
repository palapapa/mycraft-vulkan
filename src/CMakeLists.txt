file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.cpp)
target_sources(mycraft-vulkan PRIVATE ${SOURCES})
