cmake_minimum_required(VERSION 3.16)
project(dong LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Windows MSVC runtime
if(WIN32 AND NOT MINGW)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    add_compile_options(/MD)
endif()

# =============================================================================
# Configuration
# =============================================================================
set(VULKAN_SDK_PATH "$ENV{VULKAN_SDK}" CACHE PATH "Vulkan SDK path")
set(DXC_LIB_PATH "third_party/dxc_2025_07_14/lib/x64" CACHE PATH "DXC library path")
set(DXC_INCLUDE_PATH "third_party/dxc_2025_07_14/include" CACHE PATH "DXC include path")

# =============================================================================
# Third-party dependencies (built by zig build or manually)
# =============================================================================
set(FREETYPE_PREFIX "${CMAKE_SOURCE_DIR}/zig-out/freetype")
set(HARFBUZZ_PREFIX "${CMAKE_SOURCE_DIR}/zig-out/harfbuzz")
set(MSDFGEN_PREFIX "${CMAKE_SOURCE_DIR}/zig-out/msdfgen")
set(SDL3_PREFIX "${CMAKE_SOURCE_DIR}/zig-out/sdl3")
set(QUICKJS_PREFIX "${CMAKE_SOURCE_DIR}/zig-out/quickjs")

# =============================================================================
# QuickJS library (pre-built DLL from MinGW)
# =============================================================================
add_library(quickjs SHARED IMPORTED GLOBAL)
if(WIN32)
    set_target_properties(quickjs PROPERTIES
        IMPORTED_LOCATION "${QUICKJS_PREFIX}/bin/quickjs.dll"
        IMPORTED_IMPLIB "${QUICKJS_PREFIX}/lib/quickjs.dll.a"
        INTERFACE_INCLUDE_DIRECTORIES "${QUICKJS_PREFIX}/include"
    )
    # For MinGW-built DLL, we need to copy runtime dependencies
    # The main project links against the import library
else()
    set_target_properties(quickjs PROPERTIES
        IMPORTED_LOCATION "${QUICKJS_PREFIX}/lib/libquickjs.so"
        INTERFACE_INCLUDE_DIRECTORIES "${QUICKJS_PREFIX}/include"
    )
endif()

# =============================================================================
# Lexbor library
# =============================================================================
file(GLOB_RECURSE LEXBOR_SOURCES "third_party/lexbor/source/*.c")
# Filter out platform-specific files
if(WIN32)
    list(FILTER LEXBOR_SOURCES EXCLUDE REGEX ".*/ports/posix/.*")
else()
    list(FILTER LEXBOR_SOURCES EXCLUDE REGEX ".*/ports/windows.*")
endif()

add_library(lexbor STATIC ${LEXBOR_SOURCES})
target_include_directories(lexbor PUBLIC third_party/lexbor/source)
target_compile_definitions(lexbor PRIVATE LEXBOR_STATIC)

# =============================================================================
# Yoga library
# =============================================================================
add_library(yoga STATIC
    third_party/yoga/yoga/YGConfig.cpp
    third_party/yoga/yoga/YGEnums.cpp
    third_party/yoga/yoga/YGNode.cpp
    third_party/yoga/yoga/YGNodeLayout.cpp
    third_party/yoga/yoga/YGNodeStyle.cpp
    third_party/yoga/yoga/YGValue.cpp
    third_party/yoga/yoga/algorithm/AbsoluteLayout.cpp
    third_party/yoga/yoga/algorithm/Baseline.cpp
    third_party/yoga/yoga/algorithm/Cache.cpp
    third_party/yoga/yoga/algorithm/CalculateLayout.cpp
    third_party/yoga/yoga/algorithm/FlexLine.cpp
    third_party/yoga/yoga/algorithm/PixelGrid.cpp
    third_party/yoga/yoga/config/Config.cpp
    third_party/yoga/yoga/debug/AssertFatal.cpp
    third_party/yoga/yoga/debug/Log.cpp
    third_party/yoga/yoga/event/event.cpp
    third_party/yoga/yoga/node/LayoutResults.cpp
    third_party/yoga/yoga/node/Node.cpp
)
target_include_directories(yoga PUBLIC third_party/yoga)
# Yoga requires C++20
set_target_properties(yoga PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
if(NOT MSVC)
    target_compile_options(yoga PRIVATE -fno-exceptions -fno-rtti)
else()
    target_compile_options(yoga PRIVATE /EHs-c- /GR-)
endif()

# =============================================================================
# Dong core library
# =============================================================================
add_library(dong STATIC
    src/api_bindings.cpp
    src/core/context.cpp
    src/core/view.cpp
    src/dom/dom_manager.cpp
    src/dom/dom_node.cpp
    src/dom/event_system.cpp
    src/dom/parser.cpp
    src/dom/style_engine.cpp
    src/layout/layout_engine.cpp
    src/script/script_engine.cpp
    src/script/js_bindings.cpp
    src/render/painter.cpp
    src/render/render_surface.cpp
    src/render/resource_manager.cpp
    src/render/gpu_device.cpp
    src/render/gpu_surface.cpp
    src/render/font_resolver.cpp
    src/render/font_finder.cpp
    src/render/text_shaper.cpp
    src/render/font_metrics.cpp
    src/render/gpu_painter.cpp
    src/render/gpu_driver_sdl.cpp
    src/render/shader_manager.cpp
    src/render/glyph_atlas.cpp
    src/platform/sdl3_window.cpp
    src/input/input_adapter_sdl3.cpp
    src/dom/focus_manager.cpp
    src/dom/input_element.cpp
    third_party/SDL_shadercross/src/SDL_shadercross.c
)

target_include_directories(dong PUBLIC
    include
    src
    ${QUICKJS_PREFIX}/include
    third_party/lexbor/source
    third_party/yoga
    third_party/sdl/include
    third_party/SDL_shadercross/include
    ${FREETYPE_PREFIX}/include/freetype2
    ${HARFBUZZ_PREFIX}/include/harfbuzz
    ${MSDFGEN_PREFIX}/include
    ${DXC_INCLUDE_PATH}
)

if(VULKAN_SDK_PATH)
    if(WIN32)
        target_include_directories(dong PUBLIC "${VULKAN_SDK_PATH}/Include/spirv_cross")
    else()
        target_include_directories(dong PUBLIC "${VULKAN_SDK_PATH}/include/spirv_cross")
    endif()
endif()

target_compile_definitions(dong PRIVATE SDL_SHADERCROSS_DXC)

target_link_libraries(dong PUBLIC quickjs lexbor yoga)

# Link FreeType, HarfBuzz, msdfgen
if(WIN32)
    target_link_libraries(dong PUBLIC
        "${FREETYPE_PREFIX}/lib/freetype.lib"
        "${HARFBUZZ_PREFIX}/lib/harfbuzz.lib"
        "${MSDFGEN_PREFIX}/lib/msdfgen-core.lib"
        "${MSDFGEN_PREFIX}/lib/msdfgen-ext.lib"
    )
else()
    target_link_libraries(dong PUBLIC
        "${FREETYPE_PREFIX}/lib/libfreetype.a"
        "${HARFBUZZ_PREFIX}/lib/libharfbuzz.a"
        "${MSDFGEN_PREFIX}/lib/libmsdfgen-core.a"
        "${MSDFGEN_PREFIX}/lib/libmsdfgen-ext.a"
    )
endif()

# =============================================================================
# Examples
# =============================================================================
option(BUILD_EXAMPLES "Build examples" ON)

if(BUILD_EXAMPLES)
    # Common function to configure examples
    function(add_dong_example name source)
        add_executable(${name} ${source})
        target_link_libraries(${name} PRIVATE dong)
        
        # SDL3
        target_include_directories(${name} PRIVATE "${SDL3_PREFIX}/include")
        if(WIN32)
            target_link_directories(${name} PRIVATE "${SDL3_PREFIX}/lib")
            target_link_libraries(${name} PRIVATE SDL3)
        else()
            target_link_libraries(${name} PRIVATE "${SDL3_PREFIX}/lib/libSDL3.so")
        endif()
        
        # Vulkan SDK
        if(VULKAN_SDK_PATH)
            if(WIN32)
                target_link_directories(${name} PRIVATE "${VULKAN_SDK_PATH}/Lib")
            else()
                target_link_directories(${name} PRIVATE "${VULKAN_SDK_PATH}/lib")
            endif()
        endif()
        target_link_libraries(${name} PRIVATE spirv-cross-c-shared)
        
        # DXC
        target_link_directories(${name} PRIVATE ${DXC_LIB_PATH})
        target_link_libraries(${name} PRIVATE dxcompiler)
        
        # Platform libraries
        if(WIN32)
            target_link_libraries(${name} PRIVATE user32 gdi32 ole32 shell32 advapi32)
        elseif(APPLE)
            target_link_libraries(${name} PRIVATE
                "-framework CoreFoundation"
                "-framework CoreGraphics"
                "-framework CoreText"
            )
        endif()
    endfunction()

    # C examples
    add_dong_example(simple_demo examples/simple_demo.c)
    add_dong_example(basic_demo examples/basic_demo.c)

    # C++ examples
    set(CPP_EXAMPLES
        complete_demo
        dom_test
        integration_demo
        js_api_test
        render_and_script_demo
        style_cascade_test
        dom_api_demo
        style_modification_demo
        js_return_value_demo
        performance_demo
        dirty_rect_optimization_demo
        js_event_demo
        comprehensive_features_demo
        image_rendering_demo
        gpu_view_demo
        gpu_screenshot_demo
        gpu_screenshot_demo_basic_layout
        gpu_screenshot_demo_glyph_stress
        gpu_screenshot_analysis
        gpu_texture_demo
        interactive_demo
        offscreen_test
        simple_offscreen
        msdf_soft_render
        isolation_demo
        sdl_gpu_demo
    )

    foreach(example ${CPP_EXAMPLES})
        if(EXISTS "${CMAKE_SOURCE_DIR}/examples/${example}.cpp")
            add_dong_example(${example} "examples/${example}.cpp")
        endif()
    endforeach()

    # Special case for msdf_cpu_test
    if(EXISTS "${CMAKE_SOURCE_DIR}/examples/msdf_cpu_render_test.cpp")
        add_dong_example(msdf_cpu_test "examples/msdf_cpu_render_test.cpp")
    endif()
endif()

# =============================================================================
# Install
# =============================================================================
install(TARGETS dong lexbor yoga
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(DIRECTORY include/ DESTINATION include)

# Copy QuickJS DLL to bin
if(WIN32)
    install(FILES "${QUICKJS_PREFIX}/bin/quickjs.dll" DESTINATION bin)
endif()
