cmake_minimum_required(VERSION 3.16)
project(dong_sdl_backend LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

# =============================================================================
# SDL Backend Library
# =============================================================================
# This library provides SDL-based implementations of the Platform abstraction.
# It can be built as a shared or static library.

add_library(dong_sdl_backend SHARED
    dong_sdl_platform.c
    dong_sdl_gpu_bridge.cpp
    # dong_sdl_execute.cpp  # Removed - using bridge instead
    dong_sdl_image_decoder.c
    dong_sdl_image_atlas.c
    dong_astc_encoder.c
    dong_bc_encoder.c

    # SDL Plugin (Phase 1: Plugin injection)
    dong_sdl_plugin.cpp

    # SDL Window (migrated from src/platform/)
    sdl_window.cpp

    # SDL Input Adapter (migrated from src/input/)
    sdl_input_adapter.cpp

    # SDL execution chain (migrated from src/render/)
    ${CMAKE_SOURCE_DIR}/src/render/render_surface.cpp

    ${CMAKE_SOURCE_DIR}/src/render/resource_manager.cpp

    ${CMAKE_SOURCE_DIR}/src/render/font_resolver.cpp
    ${CMAKE_SOURCE_DIR}/src/render/font_finder.cpp
    ${CMAKE_SOURCE_DIR}/src/render/text_shaper.cpp
    ${CMAKE_SOURCE_DIR}/src/render/font_metrics.cpp

    # Phase 2: SDL GPU Driver (migrated from src/render/)
    sdl_gpu_driver.cpp
    sdl_gpu_driver_init.cpp
    sdl_gpu_driver_execute.cpp
    sdl_gpu_driver_resources.cpp
    sdl_gpu_driver_fonts.cpp

    # Phase 5: SDL-specific rendering components (migrated from src/render/)
    sdl_gpu_device.cpp
    sdl_shader_manager.cpp
    # sdl_glyph_atlas.cpp  # Removed - using src/render/glyph_atlas.cpp instead
    sdl_gpu_surface.cpp
    sdl_gpu_painter.cpp

    # TODO(Phase 5 complete): Remove these after full migration
    # ${CMAKE_SOURCE_DIR}/src/render/gpu_device.cpp
    # ${CMAKE_SOURCE_DIR}/src/render/gpu_surface.cpp
    # ${CMAKE_SOURCE_DIR}/src/render/gpu_painter.cpp
    # ${CMAKE_SOURCE_DIR}/src/render/shader_manager.cpp
    # ${CMAKE_SOURCE_DIR}/src/render/glyph_atlas.cpp

    # Shader cross-compile support (required by ShaderManager)
    ${CMAKE_SOURCE_DIR}/third_party/SDL_shadercross/src/SDL_shadercross.c
)


target_include_directories(dong_sdl_backend PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include

    # Internal headers used by the SDL executor (not part of public API)
    ${CMAKE_SOURCE_DIR}/src

    # stb_image for image decoding
    ${CMAKE_SOURCE_DIR}/third_party/sdl/src/video

    ${CMAKE_SOURCE_DIR}/third_party/SDL_shadercross/include
    ${DXC_INCLUDE_PATH}

    ${FREETYPE_PREFIX}/include/freetype2
    ${HARFBUZZ_PREFIX}/include/harfbuzz
    ${MSDFGEN_PREFIX}/include
)

# spirv-cross headers (used by SDL_shadercross)
if(VULKAN_SDK_PATH)
    if(WIN32)
        target_include_directories(dong_sdl_backend PRIVATE "${VULKAN_SDK_PATH}/Include/spirv_cross")
    else()
        target_include_directories(dong_sdl_backend PRIVATE "${VULKAN_SDK_PATH}/include/spirv_cross")
    endif()
endif()



# Define build macro for DLL exports
target_compile_definitions(dong_sdl_backend PRIVATE
    DONG_SDL_BUILDING_DLL
    SDL_SHADERCROSS_DXC
    DONG_SDL_SHADER_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/shaders\"
)


# SDL3 dependency
set(SDL3_PREFIX "${CMAKE_SOURCE_DIR}/zig-out/sdl3" CACHE PATH "SDL3 install prefix")
target_include_directories(dong_sdl_backend PRIVATE "${SDL3_PREFIX}/include")
target_link_directories(dong_sdl_backend PRIVATE "${SDL3_PREFIX}/lib")
target_link_libraries(dong_sdl_backend PRIVATE SDL3)

# Shader compilation deps (DXC + spirv-cross)
if(WIN32)
    target_link_directories(dong_sdl_backend PRIVATE ${DXC_LIB_PATH})
    target_link_libraries(dong_sdl_backend PRIVATE dxcompiler)
endif()

if(VULKAN_SDK_PATH)
    if(WIN32)
        target_link_directories(dong_sdl_backend PRIVATE "${VULKAN_SDK_PATH}/Lib")
    else()
        target_link_directories(dong_sdl_backend PRIVATE "${VULKAN_SDK_PATH}/lib")
    endif()
    target_link_libraries(dong_sdl_backend PRIVATE spirv-cross-c-shared)
endif()

# Link to core dong library for platform singleton + shared subsystems
target_link_libraries(dong_sdl_backend PRIVATE dong dong_render)

# Text shaping/rasterization deps (required by glyph atlas)
if(WIN32)
    target_link_libraries(dong_sdl_backend PRIVATE
        "${FREETYPE_PREFIX}/lib/freetype.lib"
        "${HARFBUZZ_PREFIX}/lib/harfbuzz.lib"
        "${MSDFGEN_PREFIX}/lib/msdfgen-core.lib"
        "${MSDFGEN_PREFIX}/lib/msdfgen-ext.lib"
    )
else()
    target_link_libraries(dong_sdl_backend PRIVATE
        "${FREETYPE_PREFIX}/lib/libfreetype.a"
        "${HARFBUZZ_PREFIX}/lib/libharfbuzz.a"
        "${MSDFGEN_PREFIX}/lib/libmsdfgen-core.a"
        "${MSDFGEN_PREFIX}/lib/libmsdfgen-ext.a"
    )
endif()

# Platform libraries
if(WIN32)
    target_link_libraries(dong_sdl_backend PRIVATE user32 gdi32 ole32 shell32 advapi32)
elseif(APPLE)
    target_link_libraries(dong_sdl_backend PRIVATE
        "-framework CoreFoundation"
        "-framework CoreGraphics"
        "-framework CoreText"
    )
endif()


# Install
install(TARGETS dong_sdl_backend
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES
    dong_sdl_platform.h
    dong_sdl_image_decoder.h
    dong_sdl_plugin.h
    DESTINATION include/dong/backends/sdl
)
