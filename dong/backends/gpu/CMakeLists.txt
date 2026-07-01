cmake_minimum_required(VERSION 3.24)
project(dong_gpu_backend LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

option(DONG_GPU_EMBED_IN_DREAMING "Link in-dreaming/gpu into dong_gpu_backend" ON)

set(DONG_GPU_BACKEND_SOURCES
    dong_gpu_platform.c
    dong_gpu_image_decoder.c
    gpu_gpu_driver.cpp
    gpu_gpu_driver_vtable.cpp
    gpu_gpu_driver_init.cpp
    gpu_gpu_driver_execute.cpp
    gpu_gpu_driver_execute_dispatch.cpp
    gpu_gpu_driver_slug.cpp
    gpu_gpu_driver_ui_graph.cpp
    gpu_pipeline_registry.cpp
    gpu_resource_manager.cpp
    gpu_upload_queue.cpp
    gpu_image_atlas.cpp
    gpu_gtc_service.cpp
    scene3d_world_draw.cpp
)

add_library(dong_gpu_backend SHARED ${DONG_GPU_BACKEND_SOURCES})

# Keep dong_gpu_backend.dll alongside the example executables (top-level build dir) instead of
# this subdirectory's own binary dir. Windows' default DLL search order only checks the loading
# process's own directory (not arbitrary subfolders), so a nested output dir here would make
# every GPU example fail to launch with STATUS_DLL_NOT_FOUND outside of `cmake --install`.
set_target_properties(dong_gpu_backend PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

target_include_directories(dong_gpu_backend PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src

    # stb_image for image decoding (dong_gpu_image_decoder.c)
    ${CMAKE_SOURCE_DIR}/third_party/gpu_texture_compress/external/stb
)

target_compile_definitions(dong_gpu_backend PRIVATE DONG_GPU_BUILDING_DLL)
target_compile_definitions(dong_gpu_backend PRIVATE
    DONG_GPU_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders"
)

if(DONG_GPU_EMBED_IN_DREAMING)
    set(GPU_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GPU_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GPU_INSTALL OFF CACHE BOOL "" FORCE)
    set(GPU_ENABLE_METAL OFF CACHE BOOL "" FORCE)

    add_subdirectory(
        ${CMAKE_SOURCE_DIR}/third_party/gpu
        ${CMAKE_BINARY_DIR}/dong_in_dreaming_gpu
        EXCLUDE_FROM_ALL
    )

    # Dong uses clang-cl without /permissive-; relax slang-rhi Werror when embedded.
    if(TARGET slang-rhi-warnings-as-errors)
        set_property(TARGET slang-rhi-warnings-as-errors PROPERTY INTERFACE_COMPILE_OPTIONS "")
    endif()

    if(TARGET gpu)
        if(MSVC)
            target_compile_options(gpu PRIVATE /W0 /wd4996)
        else()
            target_compile_options(gpu PRIVATE -w)
        endif()
        target_compile_definitions(dong_gpu_backend PRIVATE DONG_HAS_IN_DREAMING_GPU=1)
        target_include_directories(dong_gpu_backend PRIVATE
            ${CMAKE_SOURCE_DIR}/third_party/gpu/src
        )
        target_link_libraries(dong_gpu_backend PRIVATE gpu)
    endif()
    if(TARGET slang-rhi)
        if(MSVC)
            target_compile_options(slang-rhi PRIVATE /W0 /wd4996 /std:c++20)
        else()
            target_compile_options(slang-rhi PRIVATE -w -Wno-error)
        endif()
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            target_compile_options(slang-rhi PRIVATE -Wno-c++98-compat -Wno-error)
        endif()
    endif()
    if(TARGET SDL3-static)
        if(MSVC)
            target_compile_options(SDL3-static PRIVATE /W0)
        endif()
    endif()
    if(NOT TARGET gpu)
        message(WARNING "in-dreaming/gpu target not found; building gpu backend stub")
    endif()
endif()

target_link_libraries(dong_gpu_backend PUBLIC dong dong_render)

if(EXISTS "${CMAKE_SOURCE_DIR}/zig-out/sdl3/lib")
    set(SDL3_PREFIX "${CMAKE_SOURCE_DIR}/zig-out/sdl3")
    target_include_directories(dong_gpu_backend PRIVATE "${SDL3_PREFIX}/include")
    target_link_directories(dong_gpu_backend PRIVATE "${SDL3_PREFIX}/lib")
    target_link_libraries(dong_gpu_backend PRIVATE SDL3)
endif()

if(DONG_OPTIMIZE_SIZE)
    if(WIN32 AND NOT MINGW)
        target_link_options(dong_gpu_backend PRIVATE /OPT:REF /OPT:ICF=3)
    else()
        target_compile_options(dong_gpu_backend PRIVATE -ffunction-sections -fdata-sections)
        target_compile_options(dong_gpu_backend PRIVATE -fvisibility=hidden -fvisibility-inlines-hidden)
        target_link_options(dong_gpu_backend PRIVATE -Wl,--gc-sections -Wl,--icf=safe)
    endif()
endif()

install(TARGETS dong_gpu_backend
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES
    dong_gpu_platform.h
    DESTINATION include/dong/backends/gpu
)
