cmake_minimum_required(VERSION 3.16)
project(dong_appcore LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

# =============================================================================
# AppCore Library
# =============================================================================
# This library provides high-level application framework APIs.
# Uses dong_render for full GPU rendering support.

add_library(dong_appcore SHARED
    src/app.c
    src/scene3d.c
    src/overlay.c
    src/dock.c
    src/dock_title.c
    src/dock_events.c
    src/dock_drag.c
    src/dock_window.c
    src/dock_lifecycle.c
    src/dock_render.c
    src/dock_api_misc.c
    src/dock_layout_io.c
    src/dock_pane.c
    src/dock_pane_api.c
    src/dock_split.c
)

target_include_directories(dong_appcore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_include_directories(dong_appcore PRIVATE
    ${CMAKE_SOURCE_DIR}/backends/sdl
)


# Define build macro for DLL exports
target_compile_definitions(dong_appcore PRIVATE DONG_APPCORE_BUILDING_DLL)

# SDL3 dependency
set(SDL3_PREFIX "${CMAKE_SOURCE_DIR}/zig-out/sdl3" CACHE PATH "SDL3 install prefix")
target_include_directories(dong_appcore PRIVATE "${SDL3_PREFIX}/include")
target_link_directories(dong_appcore PRIVATE "${SDL3_PREFIX}/lib")
target_link_libraries(dong_appcore PUBLIC SDL3)

# SDL_ShaderCross for HLSL compilation
target_include_directories(dong_appcore PRIVATE "${CMAKE_SOURCE_DIR}/third_party/SDL_shadercross/include")

# DXC library path for shader compilation (Windows/macOS only)
if(WIN32 OR APPLE)
    set(DXC_LIB_PATH "${CMAKE_SOURCE_DIR}/third_party/dxc/lib/x64" CACHE PATH "DXC library path")
    target_link_directories(dong_appcore PRIVATE ${DXC_LIB_PATH})
    target_link_libraries(dong_appcore PRIVATE dxcompiler)
endif()

# Vulkan SDK for SPIRV cross
set(VULKAN_SDK_PATH "$ENV{VULKAN_SDK}" CACHE PATH "Vulkan SDK path")
if(VULKAN_SDK_PATH)
    if(WIN32)
        target_link_directories(dong_appcore PRIVATE "${VULKAN_SDK_PATH}/Lib")
    else()
        target_link_directories(dong_appcore PRIVATE "${VULKAN_SDK_PATH}/lib")
    endif()
    target_link_libraries(dong_appcore PRIVATE spirv-cross-c-shared)
endif()

# Link to core dong.dll for dong_engine_* API
# Link to SDL backend for Platform injection + GPU execution
# Keep dong_render for legacy dong_view-based Scene3D/Overlay (will be removed once fully migrated).
target_link_libraries(dong_appcore PUBLIC dong dong_sdl_backend dong_render)

# Linux: link ubsan and stdc++ for Zig Debug builds
if(UNIX AND NOT APPLE)
    target_link_libraries(dong_appcore PRIVATE ubsan stdc++)
endif()


# Install
install(TARGETS dong_appcore
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include/dong/appcore
    FILES_MATCHING PATTERN "*.h"
)

