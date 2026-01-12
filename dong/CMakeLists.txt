cmake_minimum_required(VERSION 3.16)
project(dong LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Windows MSVC runtime
if(WIN32 AND NOT MINGW)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    add_compile_options(/MD /permissive-)
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
# QuickJS library (static for all platforms - ensures ABI compatibility)
# =============================================================================
add_library(quickjs STATIC IMPORTED GLOBAL)
if(WIN32)
    set_target_properties(quickjs PROPERTIES
        IMPORTED_LOCATION "${QUICKJS_PREFIX}/lib/quickjs.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${QUICKJS_PREFIX}/include"
    )
else()
    set_target_properties(quickjs PROPERTIES
        IMPORTED_LOCATION "${QUICKJS_PREFIX}/lib/libquickjs.a"
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
# LEXBOR_STATIC must be PUBLIC so that consumers of lexbor also see it
# Otherwise LXB_API expands to __declspec(dllimport) causing link warnings and crashes
target_compile_definitions(lexbor PUBLIC LEXBOR_STATIC)

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
# Dong libraries
# =============================================================================
# dong: platform-agnostic core (will become dong.dll)
add_library(dong SHARED
    src/api_bindings.cpp
    src/core/context.cpp
    # DOM module (modular structure)
    src/dom/dom_manager.cpp
    src/dom/event_system.cpp
    src/dom/focus_manager.cpp
    src/dom/input_element.cpp
    src/dom/css/css_value.cpp
    src/dom/css/css_parser.cpp
    src/dom/css/selector_matcher.cpp
    src/dom/css/style_engine.cpp
    src/dom/dom/dom_node.cpp
    src/dom/dom/observer.cpp
    src/dom/html/html_parser.cpp
    # Animation
    src/animation/animation_controller.cpp
    # Layout
    src/layout/layout_engine.cpp
    # Script
    src/script/script_engine.cpp
    src/script/js_bindings.cpp
    # Render
    src/render/painter.cpp
    src/render/render_surface.cpp
    src/render/resource_manager.cpp
    src/render/font_resolver.cpp
    src/render/font_finder.cpp
    src/render/text_shaper.cpp
    src/render/font_metrics.cpp
)

target_include_directories(dong PUBLIC
    include
    src
    ${QUICKJS_PREFIX}/include
    third_party/lexbor/source
    third_party/yoga
    ${FREETYPE_PREFIX}/include/freetype2
    ${HARFBUZZ_PREFIX}/include/harfbuzz
    ${MSDFGEN_PREFIX}/include
)

# Define DONG_BUILDING_DLL for export macros
target_compile_definitions(dong PRIVATE DONG_BUILDING_DLL)

target_link_libraries(dong PUBLIC quickjs lexbor yoga)

# Link FreeType, HarfBuzz, msdfgen
if(WIN32)
    target_link_libraries(dong PUBLIC
        "${FREETYPE_PREFIX}/lib/freetype.lib"
        "${HARFBUZZ_PREFIX}/lib/harfbuzz.lib"
        "${MSDFGEN_PREFIX}/lib/msdfgen-core.lib"
        "${MSDFGEN_PREFIX}/lib/msdfgen-ext.lib"
    )
    # Link clang compiler-rt builtins for __udivti3 etc (used by QuickJS BigInt)
    # QuickJS is built with Zig/Clang, so we always need builtins regardless of host compiler
    # Try LLVM installation first (has __udivti3), then fallback to VS bundled version
    if(NOT DEFINED CLANG_RT_BUILTIN)
        file(GLOB CLANG_RT_BUILTINS_LLVM
            "C:/env/llvm/*/lib/clang/*/lib/windows/clang_rt.builtins-x86_64.lib"
        )
        file(GLOB CLANG_RT_BUILTINS_VS
            "C:/Program Files/Microsoft Visual Studio/2022/*/VC/Tools/MSVC/*/lib/x64/clang_rt.builtins-x86_64.lib"
        )
        if(CLANG_RT_BUILTINS_LLVM)
            list(GET CLANG_RT_BUILTINS_LLVM 0 CLANG_RT_BUILTIN)
            set(CLANG_RT_BUILTIN "${CLANG_RT_BUILTIN}" CACHE FILEPATH "Clang RT builtins library" FORCE)
        elseif(CLANG_RT_BUILTINS_VS)
            list(GET CLANG_RT_BUILTINS_VS 0 CLANG_RT_BUILTIN)
            set(CLANG_RT_BUILTIN "${CLANG_RT_BUILTIN}" CACHE FILEPATH "Clang RT builtins library" FORCE)
        endif()
    endif()
    if(CLANG_RT_BUILTIN)
        target_link_libraries(dong PUBLIC "${CLANG_RT_BUILTIN}")
    endif()
else()
    target_link_libraries(dong PUBLIC
        "${FREETYPE_PREFIX}/lib/libfreetype.a"
        "${HARFBUZZ_PREFIX}/lib/libharfbuzz.a"
        "${MSDFGEN_PREFIX}/lib/libmsdfgen-core.a"
        "${MSDFGEN_PREFIX}/lib/libmsdfgen-ext.a"
    )
endif()

# dong_legacy: temporary compatibility library (still SDL-coupled)
add_library(dong_legacy STATIC
    src/api_bindings_legacy.cpp
    src/core/context.cpp
    src/core/view.cpp
    # DOM module (modular structure)
    src/dom/dom_manager.cpp
    src/dom/event_system.cpp
    src/dom/focus_manager.cpp
    src/dom/input_element.cpp
    src/dom/css/css_value.cpp
    src/dom/css/css_parser.cpp
    src/dom/css/selector_matcher.cpp
    src/dom/css/style_engine.cpp
    src/dom/dom/dom_node.cpp
    src/dom/dom/observer.cpp
    src/dom/html/html_parser.cpp
    # Animation
    src/animation/animation_controller.cpp
    # Layout
    src/layout/layout_engine.cpp
    # Script
    src/script/script_engine.cpp
    src/script/js_bindings.cpp
    # Render
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
    src/render/sdl_render/gpu_driver_sdl_init.cpp
    src/render/sdl_render/gpu_driver_sdl_resources.cpp
    src/render/sdl_render/gpu_driver_sdl_fonts.cpp
    src/render/sdl_render/gpu_driver_factory.cpp
    src/render/shader_manager.cpp
    src/render/glyph_atlas.cpp
    # Platform
    src/platform/sdl3_window.cpp
    src/input/input_adapter_sdl3.cpp
    third_party/SDL_shadercross/src/SDL_shadercross.c
)

target_include_directories(dong_legacy PUBLIC
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
        target_include_directories(dong_legacy PUBLIC "${VULKAN_SDK_PATH}/Include/spirv_cross")
    else()
        target_include_directories(dong_legacy PUBLIC "${VULKAN_SDK_PATH}/include/spirv_cross")
    endif()
endif()

target_compile_definitions(dong_legacy PRIVATE SDL_SHADERCROSS_DXC DONG_USE_SDL_IMAGE)

target_link_libraries(dong_legacy PUBLIC quickjs lexbor yoga)

if(WIN32)
    target_link_libraries(dong_legacy PUBLIC
        "${FREETYPE_PREFIX}/lib/freetype.lib"
        "${HARFBUZZ_PREFIX}/lib/harfbuzz.lib"
        "${MSDFGEN_PREFIX}/lib/msdfgen-core.lib"
        "${MSDFGEN_PREFIX}/lib/msdfgen-ext.lib"
    )
    # Link clang compiler-rt builtins for __udivti3 etc (used by QuickJS BigInt)
    if(CLANG_RT_BUILTIN)
        target_link_libraries(dong_legacy PUBLIC "${CLANG_RT_BUILTIN}")
    endif()
else()
    target_link_libraries(dong_legacy PUBLIC
        "${FREETYPE_PREFIX}/lib/libfreetype.a"
        "${HARFBUZZ_PREFIX}/lib/libharfbuzz.a"
        "${MSDFGEN_PREFIX}/lib/libmsdfgen-core.a"
        "${MSDFGEN_PREFIX}/lib/libmsdfgen-ext.a"
    )
endif()

# =============================================================================
# Plugins
# =============================================================================
option(BUILD_PLUGINS "Build dong plugins" ON)
if(BUILD_PLUGINS)
    add_subdirectory(plugins/sdl)
endif()

# =============================================================================
# Apps
# =============================================================================
option(BUILD_APPS "Build dong apps" ON)
if(BUILD_APPS)
    add_subdirectory(apps/dong_app)
endif()

# =============================================================================
# Tools
# =============================================================================
option(BUILD_TOOLS "Build dong tools" ON)
if(BUILD_TOOLS)
    add_subdirectory(tools/runner)
endif()

# =============================================================================
# Examples
# =============================================================================
option(BUILD_EXAMPLES "Build examples" ON)

if(BUILD_EXAMPLES)
    # Common function to configure examples
    function(add_dong_example name source)
        add_executable(${name} ${source})
        target_link_libraries(${name} PRIVATE dong_legacy)
        
        # SDL3
        target_include_directories(${name} PRIVATE "${SDL3_PREFIX}/include")
        if(WIN32)
            target_link_directories(${name} PRIVATE "${SDL3_PREFIX}/lib")
            target_link_libraries(${name} PRIVATE SDL3)
        elseif(APPLE)
            target_link_directories(${name} PRIVATE "${SDL3_PREFIX}/lib")
            target_link_libraries(${name} PRIVATE SDL3)
        else()
            target_link_directories(${name} PRIVATE "${SDL3_PREFIX}/lib")
            target_link_libraries(${name} PRIVATE SDL3)
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
        # Minimal set + must-keep demos
        integration_demo
        sdl_gpu_demo

        gpu_screenshot_demo
        gpu_screenshot_demo_fontonly
        gpu_screenshot_demo_basic_layout
        gpu_screenshot_demo_glyph_stress
        gpu_screenshot_analysis

        interactive_demo

        3d_cube
        3d_screen
        3d_screen_html
        3d_screen_script

        # HTML render test tool
        html_render_test
    )

    foreach(example ${CPP_EXAMPLES})
        if(EXISTS "${CMAKE_SOURCE_DIR}/examples/${example}.cpp")
            add_dong_example(${example} "examples/${example}.cpp")
        endif()
    endforeach()

    # Feature test runner
    if(EXISTS "${CMAKE_SOURCE_DIR}/examples/features/run_feature_tests.cpp")
        add_executable(run_feature_tests
            examples/features/run_feature_tests.cpp
            examples/features/feature_test_framework.cpp
        )
        target_link_libraries(run_feature_tests PRIVATE dong_legacy)
        target_include_directories(run_feature_tests PRIVATE "${SDL3_PREFIX}/include")
        if(WIN32)
            target_link_directories(run_feature_tests PRIVATE "${SDL3_PREFIX}/lib")
            target_link_libraries(run_feature_tests PRIVATE SDL3)
        else()
            target_link_directories(run_feature_tests PRIVATE "${SDL3_PREFIX}/lib")
            target_link_libraries(run_feature_tests PRIVATE SDL3)
        endif()
        if(VULKAN_SDK_PATH)
            if(WIN32)
                target_link_directories(run_feature_tests PRIVATE "${VULKAN_SDK_PATH}/Lib")
            else()
                target_link_directories(run_feature_tests PRIVATE "${VULKAN_SDK_PATH}/lib")
            endif()
        endif()
        target_link_libraries(run_feature_tests PRIVATE spirv-cross-c-shared)
        target_link_directories(run_feature_tests PRIVATE ${DXC_LIB_PATH})
        target_link_libraries(run_feature_tests PRIVATE dxcompiler)
        if(WIN32)
            target_link_libraries(run_feature_tests PRIVATE user32 gdi32 ole32 shell32 advapi32)
        elseif(APPLE)
            target_link_libraries(run_feature_tests PRIVATE
                "-framework CoreFoundation"
                "-framework CoreGraphics"
                "-framework CoreText"
            )
        endif()
        
        # Install run_feature_tests to bin directory
        install(TARGETS run_feature_tests RUNTIME DESTINATION bin)
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

# Install examples
if(BUILD_EXAMPLES)
    # Get all example targets
    set(EXAMPLE_TARGETS simple_demo basic_demo)
    foreach(example ${CPP_EXAMPLES})
        if(TARGET ${example})
            list(APPEND EXAMPLE_TARGETS ${example})
        endif()
    endforeach()
    if(TARGET msdf_cpu_test)
        list(APPEND EXAMPLE_TARGETS msdf_cpu_test)
    endif()
    
    install(TARGETS ${EXAMPLE_TARGETS}
        RUNTIME DESTINATION bin
    )
    
    # Install data directory for examples
    if(EXISTS "${CMAKE_SOURCE_DIR}/examples/data")
        install(DIRECTORY "${CMAKE_SOURCE_DIR}/examples/data"
            DESTINATION bin
            FILES_MATCHING
                PATTERN "*.html"
                PATTERN "*.js"
                PATTERN "*.css"
                PATTERN "*.png"
        )
    endif()
endif()

# Copy SDL3 DLL/dylib to bin
if(WIN32)
    install(FILES "${SDL3_PREFIX}/bin/SDL3.dll" DESTINATION bin)
elseif(APPLE)
    file(GLOB SDL3_DYLIBS "${SDL3_PREFIX}/lib/libSDL3*.dylib")
    install(FILES ${SDL3_DYLIBS} DESTINATION lib)
else()
    file(GLOB SDL3_SOS "${SDL3_PREFIX}/lib/libSDL3*.so*")
    install(FILES ${SDL3_SOS} DESTINATION lib)
endif()
