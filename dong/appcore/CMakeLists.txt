cmake_minimum_required(VERSION 3.16)
project(dong_appcore LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

set(APPCORE_SOURCES
    src/app.c
)

if(DONG_BACKEND STREQUAL "gpu")
    list(APPEND APPCORE_SOURCES
        src/scene3d_gpu.c
        src/scene3d_world_gpu.cpp
        src/dock_stub_gpu.c
        src/overlay_stub_gpu.c
        ${CMAKE_SOURCE_DIR}/backends/gpu/dong_gpu_app_platform.c
    )
else()
    list(APPEND APPCORE_SOURCES
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
endif()

add_library(dong_appcore SHARED ${APPCORE_SOURCES})

# Keep dong_appcore.dll alongside the example executables (top-level build dir) instead of this
# subdirectory's own binary dir; see backends/gpu/CMakeLists.txt for why this matters on Windows.
set_target_properties(dong_appcore PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

target_include_directories(dong_appcore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_include_directories(dong_appcore PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(dong_appcore PRIVATE DONG_APPCORE_BUILDING_DLL)

if(DONG_BACKEND STREQUAL "gpu")
    target_compile_definitions(dong_appcore PRIVATE DONG_BACKEND_GPU=1)
    target_compile_definitions(dong_appcore PRIVATE
        DONG_GPU_SHADER_DIR="${CMAKE_SOURCE_DIR}/backends/gpu/shaders"
    )
    target_include_directories(dong_appcore PRIVATE
        ${CMAKE_SOURCE_DIR}/backends/gpu
        ${CMAKE_SOURCE_DIR}/third_party/gpu/src
    )
    target_link_libraries(dong_appcore PUBLIC dong dong_gpu_backend dong_render)
    if(TARGET gpu)
        target_link_libraries(dong_appcore PRIVATE gpu)
    endif()
else()
    target_include_directories(dong_appcore PRIVATE
        ${CMAKE_SOURCE_DIR}/backends/sdl
    )

    set(SDL3_PREFIX "${CMAKE_SOURCE_DIR}/zig-out/sdl3" CACHE PATH "SDL3 install prefix")
    target_include_directories(dong_appcore PRIVATE "${SDL3_PREFIX}/include")
    target_link_directories(dong_appcore PRIVATE "${SDL3_PREFIX}/lib")
    target_link_libraries(dong_appcore PUBLIC SDL3)

    target_include_directories(dong_appcore PRIVATE "${CMAKE_SOURCE_DIR}/third_party/SDL_shadercross/include")

    if(WIN32 OR APPLE)
        set(DXC_LIB_PATH "${CMAKE_SOURCE_DIR}/third_party/dxc/lib/x64" CACHE PATH "DXC library path")
        target_link_directories(dong_appcore PRIVATE ${DXC_LIB_PATH})
        target_link_libraries(dong_appcore PRIVATE dxcompiler)
    endif()

    set(VULKAN_SDK_PATH "$ENV{VULKAN_SDK}" CACHE PATH "Vulkan SDK path")
    if(VULKAN_SDK_PATH)
        if(WIN32)
            target_link_directories(dong_appcore PRIVATE "${VULKAN_SDK_PATH}/Lib")
        else()
            target_link_directories(dong_appcore PRIVATE "${VULKAN_SDK_PATH}/lib")
        endif()
        target_link_libraries(dong_appcore PRIVATE spirv-cross-c-shared)
    endif()

    target_link_libraries(dong_appcore PUBLIC dong dong_sdl_backend dong_render)
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(dong_appcore PRIVATE ubsan stdc++)
endif()

if(DONG_OPTIMIZE_SIZE)
    if(WIN32 AND NOT MINGW)
        target_link_options(dong_appcore PRIVATE /OPT:REF /OPT:ICF=3)
    else()
        target_compile_options(dong_appcore PRIVATE -ffunction-sections -fdata-sections)
        target_compile_options(dong_appcore PRIVATE -fvisibility=hidden -fvisibility-inlines-hidden)
        target_link_options(dong_appcore PRIVATE -Wl,--gc-sections -Wl,--icf=safe)
    endif()
endif()

install(TARGETS dong_appcore
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include/dong/appcore
    FILES_MATCHING PATTERN "*.h"
)
