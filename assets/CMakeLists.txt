find_program(SLANGC
    NAMES slangc
    HINTS "$ENV{VULKAN_SDK}/bin"
    DOC "Path to the slangc Slang compiler."
    REQUIRED
)
file(GLOB_RECURSE ASSETS CONFIGURE_DEPENDS *)

# We don't want to recopy assets if only this file changed.
list(REMOVE_ITEM ASSETS "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt")
set(ASSETS_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}")
set(ASSETS_DESTINATION "${PROJECT_BINARY_DIR}/assets")

# To allow assets to be copied, that they are copied whenever an asset is
# modified, added, or deleted, and that if no asset changes, don't recopy them,
# we utilize two files, the manifest and the stamp.
#
# The manifest is a list of path of all assets. It gets written at configure
# time. Its purpose is to let CMake know if any asset has been added or deleted.
# It can do this because file(GENERATE) doesn't update the output file when the
# content is the same, so its modification time doesn't get updated if the
# project is reconfigured and no asset changed.
#
# The stamp file is an empty file that is used only for its modification time.
# Whenever the assets directory has been copied, its modification time is
# updated.
#
# With these two files, we can achieve everything we want:
#
# 1. When an asset is modified, since mycraft-vulkan depends on
# mycraft-vulkan-assets, which depends on the stamp file, which depends on
# the list of assets, which just became newer than the stamp file, the stamp
# file has to be "rebuilt", which runs the custom command that recopies the
# assets directory and updates the stamp file's modification time so that the
# stamp file is newer than the list of assets again.
#
# 2. When an asset is added, because of CONFIGURE_DEPENDS, the project is
# reconfigured, which regenerates the manifest file, and since the list of
# assets changed, the manifest file's modification time becomes newer than
# the stamp file. Because the stamp file depends on the manifest file, the
# stamp file has to be "rebuilt", and then the process goes like 1. which
# recopies the assets directory.
#
# 3. When an asset is deleted, the process goes like 2. because of
# CONFIGURE_DEPENDS.
#
# Note that the manifest file can't be an empty file that's only used for its
# modification time, which you would touch at configure time. Otherwise, the
# assets get recopied whenever the project is reconfigured, even if no asset
# changed.
set(ASSET_MANIFEST "${CMAKE_CURRENT_BINARY_DIR}/asset-manifest.txt")
set(ASSET_COPIED_STAMP "${CMAKE_CURRENT_BINARY_DIR}/asset-copied.stamp")
string(JOIN "\n" ASSET_MANIFEST_CONTENT ${ASSETS})
file(GENERATE
    OUTPUT "${ASSET_MANIFEST}"
    CONTENT "${ASSET_MANIFEST_CONTENT}\n")
add_custom_command(OUTPUT "${ASSET_COPIED_STAMP}"
    COMMAND "${CMAKE_COMMAND}" -E rm -rf "${ASSETS_DESTINATION}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${ASSETS_DESTINATION}"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${ASSETS_SOURCE}" "${ASSETS_DESTINATION}"

    # Even though ASSETS don't contain this file, ASSETS_SOURCE is copied as a
    # whole, so it would still include this file.
    COMMAND "${CMAKE_COMMAND}" -E rm "${ASSETS_DESTINATION}/CMakeLists.txt"
    COMMAND "${CMAKE_COMMAND}" -E touch "${ASSET_COPIED_STAMP}"
    COMMENT "Copying assets to build directory"
    DEPENDS ${ASSETS} "${ASSET_MANIFEST}"
    VERBATIM)
file(GLOB_RECURSE SHADERS CONFIGURE_DEPENDS shaders/*.slang)

# Exclude modules. They are only imported so they don't need to be compiled.
foreach(SHADER IN LISTS SHADERS)
    set(SHADER_MODULES "${ASSETS_SOURCE}/shaders/imports")
    cmake_path(IS_PREFIX SHADER_MODULES "${SHADER}" NORMALIZE IS_MODULE)
    if(IS_MODULE)
        list(REMOVE_ITEM SHADERS "${SHADER}")
    endif()
endforeach()

foreach(SHADER IN LISTS SHADERS)
    file(RELATIVE_PATH SHADER_RELATIVE_PATH
        "${ASSETS_SOURCE}/shaders"
        "${SHADER}")
    cmake_path(REPLACE_EXTENSION SHADER_RELATIVE_PATH
        LAST_ONLY .spv
        OUTPUT_VARIABLE SHADER_OUTPUT_RELATIVE_PATH)
    set(SHADER_OUTPUT
        "${ASSETS_DESTINATION}/shaders/${SHADER_OUTPUT_RELATIVE_PATH}")
    set(SHADER_DEPFILE
        "${CMAKE_CURRENT_BINARY_DIR}/shader-depfiles/${SHADER_RELATIVE_PATH}.d")
    cmake_path(REPLACE_EXTENSION SHADER_RELATIVE_PATH
        LAST_ONLY .json
        OUTPUT_VARIABLE SHADER_REFLECTION_OUTPUT_RELATIVE_PATH)
    set(SHADER_REFLECTION_OUTPUT
        "${ASSETS_DESTINATION}/shaders/${SHADER_REFLECTION_OUTPUT_RELATIVE_PATH}")
    cmake_path(GET SHADER_OUTPUT PARENT_PATH SHADER_OUTPUT_DIRECTORY)
    cmake_path(GET SHADER_DEPFILE PARENT_PATH SHADER_DEPFILE_DIRECTORY)
    add_custom_command(OUTPUT "${SHADER_OUTPUT}" "${SHADER_REFLECTION_OUTPUT}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${SHADER_OUTPUT_DIRECTORY}" "${SHADER_DEPFILE_DIRECTORY}"
        COMMAND "${SLANGC}"
        "${SHADER}"
        -std 2026
        -target spirv
        -profile spirv_1_6
        -capability spvDescriptorHeapEXT
        -fvk-use-entrypoint-name
        -fvk-use-scalar-layout
        "$<$<CONFIG:Debug,RelWithDebInfo>:-g3>"
        "$<$<CONFIG:Release,RelWithDebInfo,MinSizeRel>:-O3>"
        -depfile "${SHADER_DEPFILE}"
        -I "${ASSETS_SOURCE}/shaders/imports"
        -Wall
        -Wpedantic
        -o "${SHADER_OUTPUT}"
        -reflection-json "${SHADER_REFLECTION_OUTPUT}"

        # Make sure we compile shaders after assets have been copied to avoid
        # the compiled shaders getting deleted.
        DEPENDS "${SHADER_SOURCE}" "${ASSET_COPIED_STAMP}"
        DEPFILE "${SHADER_DEPFILE}"
        COMMENT "Compiling Slang shader ${SHADER_RELATIVE_PATH}"
        COMMAND_EXPAND_LISTS
        VERBATIM)
    list(APPEND SHADER_OUTPUTS "${SHADER_OUTPUT}")
endforeach()

add_custom_target(mycraft-vulkan-assets
    DEPENDS "${ASSET_COPIED_STAMP}" ${SHADER_OUTPUTS})
add_dependencies(mycraft-vulkan mycraft-vulkan-assets)
