cmake_minimum_required(VERSION 3.16)
project(dong LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# =============================================================================
# macOS: Ensure installed executables can locate dylibs in ../lib
#
# We install binaries to:  <prefix>/bin
# and shared libraries to: <prefix>/lib
# Some targets link against dylibs with install name "@rpath/...".
# Without an LC_RPATH in the executable, dyld cannot resolve them.
# =============================================================================
if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
    set(CMAKE_INSTALL_RPATH "@executable_path/../lib")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
endif()

# Windows MSVC runtime
if(WIN32 AND NOT MINGW)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    add_compile_options(/MD /permissive-)
endif()

# =============================================================================
# Binary size optimization (controlled by -DDONG_OPTIMIZE_SIZE=ON)
# =============================================================================
option(DONG_OPTIMIZE_SIZE "Optimize for binary size (LTO, /O1, no RTTI, etc.)" OFF)

if(DONG_OPTIMIZE_SIZE)
    # (a) Use size-optimized codegen instead of speed-optimized
    if(WIN32 AND NOT MINGW)
        # clang-cl: /O1 favors size over speed
        string(REPLACE "/O2" "/O1" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
        string(REPLACE "/O2" "/O1" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
    else()
        string(REPLACE "-O2" "-Os" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
        string(REPLACE "-O2" "-Os" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
    endif()

    # (b) Enable Link-Time Optimization for cross-unit dead code elimination
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
    if(ipo_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    else()
        message(WARNING "LTO not supported: ${ipo_error}")
    endif()

    message(STATUS "DONG_OPTIMIZE_SIZE: ON (LTO=${CMAKE_INTERPROCEDURAL_OPTIMIZATION}, size-opt codegen)")
endif()

# =============================================================================
# Configuration
# =============================================================================
set(VULKAN_SDK_PATH "$ENV{VULKAN_SDK}" CACHE PATH "Vulkan SDK path")
# DXC path: Windows uses lib/x64, Linux uses lib
if(WIN32)
    set(DXC_LIB_PATH "third_party/dxc/lib/x64" CACHE PATH "DXC library path")
else()
    set(DXC_LIB_PATH "third_party/dxc/lib" CACHE PATH "DXC library path")
endif()
set(DXC_INCLUDE_PATH "third_party/dxc/include" CACHE PATH "DXC include path")

# =============================================================================
# Third-party dependencies (built by zig build or manually)
# =============================================================================
set(FREETYPE_PREFIX "${CMAKE_SOURCE_DIR}/zig-out/freetype")
set(HARFBUZZ_PREFIX "${CMAKE_SOURCE_DIR}/zig-out/harfbuzz")
set(MSDFGEN_PREFIX "${CMAKE_SOURCE_DIR}/zig-out/msdfgen")
set(SDL3_PREFIX "${CMAKE_SOURCE_DIR}/zig-out/sdl3")
set(ZIG_OUT_PREFIX "${CMAKE_SOURCE_DIR}/zig-out")

# =============================================================================
# QuickJS library (static for all platforms - ensures ABI compatibility)
# =============================================================================
add_library(quickjs STATIC IMPORTED GLOBAL)
if(WIN32)
    set_target_properties(quickjs PROPERTIES
        IMPORTED_LOCATION "${ZIG_OUT_PREFIX}/lib/quickjs.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${ZIG_OUT_PREFIX}/include"
    )
else()
    set_target_properties(quickjs PROPERTIES
        IMPORTED_LOCATION "${ZIG_OUT_PREFIX}/lib/libquickjs.a"
        INTERFACE_INCLUDE_DIRECTORIES "${ZIG_OUT_PREFIX}/include"
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
set_target_properties(lexbor PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(lexbor PUBLIC third_party/lexbor/source)
# LEXBOR_STATIC must be PUBLIC so that consumers of lexbor also see it
# Otherwise LXB_API expands to __declspec(dllimport) causing link warnings and crashes
target_compile_definitions(lexbor PUBLIC LEXBOR_STATIC)

if(DONG_OPTIMIZE_SIZE AND NOT WIN32)
    target_compile_options(lexbor PRIVATE -ffunction-sections -fdata-sections)
endif()

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
set_target_properties(yoga PROPERTIES POSITION_INDEPENDENT_CODE ON)
# Yoga requires C++20
set_target_properties(yoga PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
if(NOT MSVC)
    target_compile_options(yoga PRIVATE -fno-exceptions -fno-rtti)
else()
    target_compile_options(yoga PRIVATE /EHs-c- /GR-)
endif()

if(DONG_OPTIMIZE_SIZE AND NOT WIN32)
    target_compile_options(yoga PRIVATE -ffunction-sections -fdata-sections)
endif()

# =============================================================================
# Dong libraries
# =============================================================================
# dong: platform-agnostic core (will become dong.dll)
add_library(dong SHARED
    src/api_bindings.cpp
    src/render/drawlist_emitter.cpp
    src/devtools/devtools_overlay.cpp
    src/world/world_text.cpp
    src/world/world_text_api.cpp
    src/world/decal.cpp
    src/world/decal_api.cpp
    src/core/context.cpp
    src/core/engine_view.cpp
    src/core/platform.c
    src/core/profiler.cpp
    # Input
    src/input/spatial_nav.cpp
    # DOM module (modular structure)
    src/dom/dom_manager.cpp
    src/dom/event_system.cpp
    src/dom/focus_manager.cpp
    src/dom/drag_manager.cpp
    src/dom/input_element.cpp
    src/dom/select_element.cpp
    src/dom/details_element.cpp
    src/dom/dialog_element.cpp
    src/dom/range.cpp
    src/dom/selection.cpp
    src/dom/text_hit_testing.cpp
    src/dom/contenteditable.cpp
    src/dom/editing_commands.cpp
    src/dom/css/css_value.cpp
    src/dom/css/css_parser.cpp
    src/dom/css/css_property_handlers.cpp
    src/dom/css/css_value_parser.cpp
    src/dom/css/css_shorthand_parser.cpp
    src/dom/css/css_function_parser.cpp
    src/dom/css/css_at_rule_parser.cpp
    src/dom/css/computed_style.cpp
    src/dom/css/selector_matcher.cpp
    src/dom/css/style_engine.cpp
    src/dom/css/style_engine_apply.cpp
    src/dom/css/style_engine_cascade.cpp
    src/dom/css/style_engine_inheritance.cpp
    src/dom/css/style_engine_pseudo.cpp
    src/dom/css/css_environment.cpp
    src/dom/dom/dom_node.cpp
    src/dom/dom/observer.cpp
    src/dom/html/html_parser.cpp
    # Animation
    src/animation/animation_controller.cpp
    # Layout
    src/layout/layout_engine.cpp
    src/layout/aspect_ratio_resolver.cpp
    src/layout/sticky_positioning.cpp
    src/layout/display_contents.cpp
    src/layout/table_layout.cpp
    src/layout/grid_layout.cpp
    # Script
    src/script/module_loader.cpp
    src/script/script_engine.cpp
    src/script/js_bindings.cpp
    src/script/js_node_bindings.cpp
    src/script/js_observer_bindings.cpp
    src/script/js_clipboard_bindings.cpp
    src/script/js_selection_bindings.cpp
    src/script/js_fetch_bindings.cpp
    src/script/js_text_layout_bindings.cpp
    src/script/js_scene_bindings.cpp
    src/render/scene_graph.cpp
    src/dom/scene_compiler.cpp
    # Core utilities
    src/core/resource_loader.cpp
    # Render
    src/render/painter.cpp
    src/render/painter_select_overlays.cpp
    src/render/painter/painter_media.cpp
    src/render/painter/painter_text.cpp
    src/render/painter/painter_children.cpp
    src/render/painter/painter_backdrop.cpp
    src/render/painter/painter_caret.cpp
    src/render/render_surface.cpp
    src/render/resource_manager.cpp
    src/render/font_resolver.cpp

    src/render/font_finder.cpp
    src/render/text_shaper.cpp
    src/render/font_metrics.cpp
    src/render/list_marker.cpp
    src/render/text_layout_core.cpp
    src/render/text_renderer_selector.cpp
    src/render/slug/slug_outline_loader.cpp
    src/render/slug/slug_curve_encoder.cpp
    src/render/slug/slug_band_builder.cpp
    src/render/slug/slug_font_cache.cpp
)

target_include_directories(dong PUBLIC
    include
    src
    ${ZIG_OUT_PREFIX}/include
    third_party/lexbor/source
    third_party/yoga

    # Third-party headers are installed by zig into zig-out/<dep>/include.
    ${FREETYPE_PREFIX}/include/freetype2
    ${HARFBUZZ_PREFIX}/include/harfbuzz
    ${MSDFGEN_PREFIX}/include

    third_party
)


# Define DONG_BUILDING_DLL for export macros
target_compile_definitions(dong PRIVATE DONG_BUILDING_DLL)

# Define MSDFGEN_PUBLIC to disable msdfgen-config.h requirement
target_compile_definitions(dong PUBLIC MSDFGEN_PUBLIC= MSDFGEN_EXT_PUBLIC=)

target_link_libraries(dong PUBLIC quickjs lexbor yoga)
if(WIN32)
    target_link_libraries(dong PUBLIC winhttp)
endif()

# Link FreeType, HarfBuzz, msdfgen
if(WIN32)
    target_link_libraries(dong PUBLIC
        "${ZIG_OUT_PREFIX}/lib/freetype.lib"
        "${ZIG_OUT_PREFIX}/lib/harfbuzz.lib"
        "${ZIG_OUT_PREFIX}/lib/msdfgen-core.lib"
        "${ZIG_OUT_PREFIX}/lib/msdfgen-ext.lib"
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
        "${ZIG_OUT_PREFIX}/lib/libfreetype.a"
        "${ZIG_OUT_PREFIX}/lib/libharfbuzz.a"
        "${ZIG_OUT_PREFIX}/lib/libmsdfgen.a"
    )
    # Link CoreFoundation and CoreText frameworks on macOS
    if(APPLE)
        target_link_libraries(dong PUBLIC
            "-framework CoreFoundation"
            "-framework CoreText"
        )
    endif()
endif()

# Size optimization for dong target
if(DONG_OPTIMIZE_SIZE)
    # Disable RTTI (safe: no dynamic_cast/typeid in src/)
    if(WIN32 AND NOT MINGW)
        target_compile_options(dong PRIVATE /GR-)
    else()
        target_compile_options(dong PRIVATE -fno-rtti)
    endif()

    # Linker: eliminate unreferenced functions and fold identical COMDATs
    if(WIN32)
        target_link_options(dong PRIVATE /OPT:REF /OPT:ICF=3)
    else()
        target_compile_options(dong PRIVATE -ffunction-sections -fdata-sections)
        target_link_options(dong PRIVATE -Wl,--gc-sections -Wl,--icf=safe)
        target_compile_options(dong PRIVATE -fvisibility=hidden -fvisibility-inlines-hidden)
    endif()
endif()

# dong_render: Core rendering library with SDL GPU backend
# Contains: DOM/CSS/Layout engine + SDL_GPU rendering pipeline
add_library(dong_render STATIC
    src/core/context.cpp
    src/core/profiler.cpp
    src/core/engine_view.cpp
    src/core/global_shared.cpp
    src/core/global_shared_c_api.cpp
    src/core/glyph_atlas_c_api.cpp
    src/render/glyph_atlas.cpp

    # Input
    src/input/spatial_nav.cpp

    # DOM module (modular structure)
    src/dom/dom_manager.cpp
    src/dom/event_system.cpp
    src/dom/focus_manager.cpp
    src/dom/drag_manager.cpp
    src/dom/input_element.cpp
    src/dom/select_element.cpp
    src/dom/details_element.cpp
    src/dom/dialog_element.cpp
    src/dom/range.cpp
    src/dom/selection.cpp
    src/dom/text_hit_testing.cpp
    src/dom/contenteditable.cpp
    src/dom/editing_commands.cpp
    src/dom/css/css_value.cpp
    src/dom/css/css_parser.cpp
    src/dom/css/css_property_handlers.cpp
    src/dom/css/css_value_parser.cpp
    src/dom/css/css_shorthand_parser.cpp
    src/dom/css/css_function_parser.cpp
    src/dom/css/css_at_rule_parser.cpp
    src/dom/css/computed_style.cpp
    src/dom/css/selector_matcher.cpp
    src/dom/css/style_engine.cpp
    src/dom/css/style_engine_apply.cpp
    src/dom/css/style_engine_cascade.cpp
    src/dom/css/style_engine_inheritance.cpp
    src/dom/css/style_engine_pseudo.cpp
    src/dom/css/css_environment.cpp
    src/dom/dom/dom_node.cpp
    src/dom/dom/observer.cpp
    src/dom/html/html_parser.cpp
    # Animation
    src/animation/animation_controller.cpp
    # Layout
    src/layout/layout_engine.cpp
    src/layout/aspect_ratio_resolver.cpp
    src/layout/sticky_positioning.cpp
    src/layout/display_contents.cpp
    src/layout/table_layout.cpp
    src/layout/grid_layout.cpp
    # Script
    src/script/module_loader.cpp
    src/script/script_engine.cpp
    src/script/js_bindings.cpp
    src/script/js_node_bindings.cpp
    src/script/js_observer_bindings.cpp
    src/script/js_clipboard_bindings.cpp
    src/script/js_selection_bindings.cpp
    src/script/js_fetch_bindings.cpp
    src/script/js_text_layout_bindings.cpp
    src/script/js_scene_bindings.cpp
    src/render/scene_graph.cpp
    src/dom/scene_compiler.cpp
    # Core utilities
    src/core/resource_loader.cpp
    # Render
    src/render/painter.cpp
    src/render/painter_select_overlays.cpp
    src/render/painter/painter_media.cpp
    src/render/painter/painter_text.cpp
    src/render/painter/painter_children.cpp
    src/render/painter/painter_backdrop.cpp
    src/render/painter/painter_caret.cpp
    src/render/render_surface.cpp
    src/render/resource_manager.cpp
    # NOTE: SDL-specific render files have been migrated to backends/sdl/

    # Migration complete - sdl_gpu_driver.cpp now handles all GPU rendering

    # Core render files (platform-agnostic)
    src/render/font_resolver.cpp
    src/render/font_finder.cpp
    src/render/text_shaper.cpp
    src/render/font_metrics.cpp
    src/render/list_marker.cpp
    src/render/text_layout_core.cpp
    src/render/text_renderer_selector.cpp
    src/render/slug/slug_outline_loader.cpp
    src/render/slug/slug_curve_encoder.cpp
    src/render/slug/slug_band_builder.cpp
    src/render/slug/slug_font_cache.cpp

    # GlyphAtlas (shared by all views via GlobalShared)
    src/render/glyph_atlas.cpp
    # SDL_shadercross for shader compilation
    third_party/SDL_shadercross/src/SDL_shadercross.c
)

set_target_properties(dong_render PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(dong_render PUBLIC
    include
    src
    ${ZIG_OUT_PREFIX}/include
    third_party/lexbor/source
    third_party/yoga
    third_party/sdl/include
    third_party/SDL_shadercross/include

    # Third-party headers are installed by zig into zig-out/<dep>/include.
    ${FREETYPE_PREFIX}/include/freetype2
    ${HARFBUZZ_PREFIX}/include/harfbuzz
    ${MSDFGEN_PREFIX}/include

    third_party
    ${DXC_INCLUDE_PATH}
)


if(VULKAN_SDK_PATH)
    if(WIN32)
        target_include_directories(dong_render PUBLIC "${VULKAN_SDK_PATH}/Include/spirv_cross")
    else()
        target_include_directories(dong_render PUBLIC "${VULKAN_SDK_PATH}/include/spirv_cross")
    endif()
endif()

target_compile_definitions(dong_render PRIVATE DONG_USE_SDL_IMAGE DONG_SDL_SHADER_DIR=\"${CMAKE_SOURCE_DIR}/src/render/sdl_render/shaders\" MSDFGEN_PUBLIC= MSDFGEN_EXT_PUBLIC=)

# DXC only on Windows/macOS
if(WIN32 OR APPLE)
    target_compile_definitions(dong_render PRIVATE SDL_SHADERCROSS_DXC)
endif()

target_link_libraries(dong_render PUBLIC dong quickjs lexbor yoga)


if(WIN32)
    target_link_libraries(dong_render PUBLIC
        "${ZIG_OUT_PREFIX}/lib/freetype.lib"
        "${ZIG_OUT_PREFIX}/lib/harfbuzz.lib"
        "${ZIG_OUT_PREFIX}/lib/msdfgen-core.lib"
        "${ZIG_OUT_PREFIX}/lib/msdfgen-ext.lib"
    )
    # Link clang compiler-rt builtins for __udivti3 etc (used by QuickJS BigInt)
    if(CLANG_RT_BUILTIN)
        target_link_libraries(dong_render PUBLIC "${CLANG_RT_BUILTIN}")
    endif()
else()
    target_link_libraries(dong_render PUBLIC
        "${ZIG_OUT_PREFIX}/lib/libfreetype.a"
        "${ZIG_OUT_PREFIX}/lib/libharfbuzz.a"
        "${ZIG_OUT_PREFIX}/lib/libmsdfgen.a"
    )
endif()

# Size optimization for dong_render target
if(DONG_OPTIMIZE_SIZE)
    if(WIN32 AND NOT MINGW)
        target_compile_options(dong_render PRIVATE /GR-)
    else()
        target_compile_options(dong_render PRIVATE -fno-rtti -ffunction-sections -fdata-sections)
        target_compile_options(dong_render PRIVATE -fvisibility=hidden -fvisibility-inlines-hidden)
    endif()
endif()

# =============================================================================
# Backends
# =============================================================================
option(BUILD_BACKENDS "Build dong backends" ON)
if(BUILD_BACKENDS)
    add_subdirectory(backends/sdl)
endif()

# =============================================================================
# AppCore
# =============================================================================
option(BUILD_APPCORE "Build dong appcore library" ON)
if(BUILD_APPCORE)
    add_subdirectory(appcore)
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
        target_link_libraries(${name} PRIVATE dong dong_sdl_backend)
        
        # SDL3
        target_include_directories(${name} PRIVATE "${SDL3_PREFIX}/include")
        target_include_directories(${name} PRIVATE "${CMAKE_SOURCE_DIR}/backends/sdl")

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
        
        # DXC (Windows/macOS only)
        if(WIN32 OR APPLE)
            target_link_directories(${name} PRIVATE ${DXC_LIB_PATH})
            target_link_libraries(${name} PRIVATE dxcompiler)
        endif()
        
        # Platform libraries
        if(WIN32)
            target_link_libraries(${name} PRIVATE user32 gdi32 ole32 shell32 advapi32)
        elseif(APPLE)
            target_link_libraries(${name} PRIVATE
                "-framework CoreFoundation"
                "-framework CoreGraphics"
                "-framework CoreText"
            )
        else()
            # Linux: link ubsan and stdc++ for Zig Debug builds
            target_link_libraries(${name} PRIVATE ubsan stdc++)
        endif()
    endfunction()

    # C++ examples (legacy - using dong_render API)
    # NOTE: These are kept for reference but replaced by AppCore-based equivalents:
    #   - interactive_demo -> interactive_demo_new (C, ~200 lines vs 450)
    #   - 3d_screen_script -> 3d_screens_simple (C, ~130 lines vs 1500+)
    set(CPP_EXAMPLES
        # Legacy demos - kept for reference, uncomment to build
        # interactive_demo
        # 3d_screen_script

        # HTML render test tool (still useful for development)
        html_render_test
    )

    foreach(example ${CPP_EXAMPLES})
        if(EXISTS "${CMAKE_SOURCE_DIR}/examples/${example}.cpp")
            add_dong_example(${example} "examples/${example}.cpp")
        endif()
    endforeach()

    # ==========================================================================
    # New AppCore-based examples (simplified API)
    # ==========================================================================
    if(BUILD_APPCORE)
        # Helper function for AppCore examples
        function(add_appcore_example name source)
            add_executable(${name} ${source})

            # SDL3 link directory (AppCore depends on SDL3)
            target_link_directories(${name} PRIVATE "${SDL3_PREFIX}/lib")

            target_link_libraries(${name} PRIVATE dong_appcore)

            # Vulkan SDK for SPIRV cross (needed by dong_render via SDL_shadercross)
            if(VULKAN_SDK_PATH)
                if(WIN32)
                    target_link_directories(${name} PRIVATE "${VULKAN_SDK_PATH}/Lib")
                else()
                    target_link_directories(${name} PRIVATE "${VULKAN_SDK_PATH}/lib")
                endif()
                target_link_libraries(${name} PRIVATE spirv-cross-c-shared)
            endif()

            # DXC library (needed by dong_render via SDL_shadercross on Windows/macOS)
            if(WIN32 OR APPLE)
                target_link_directories(${name} PRIVATE ${DXC_LIB_PATH})
                target_link_libraries(${name} PRIVATE dxcompiler)
            endif()

            # Platform libraries
            if(WIN32)
                target_link_libraries(${name} PRIVATE user32 gdi32 ole32 shell32 advapi32)
            elseif(APPLE)
                target_link_libraries(${name} PRIVATE
                    "-framework CoreFoundation"
                    "-framework CoreGraphics"
                    "-framework CoreText"
                )
            else()
                # Linux: link ubsan and stdc++ for Zig Debug builds
                target_link_libraries(${name} PRIVATE ubsan stdc++)
            endif()
        endfunction()

        # Minimal demo (~50 lines)
        if(EXISTS "${CMAKE_SOURCE_DIR}/examples/minimal/main.c")
            add_appcore_example(minimal_dong_demo "examples/minimal/main.c")
        endif()

        # Interactive demo (simplified AppCore version)
        if(EXISTS "${CMAKE_SOURCE_DIR}/examples/interactive_demo_new.c")
            add_appcore_example(interactive_demo_new "examples/interactive_demo_new.c")
        endif()

        # HDR demo
        if(EXISTS "${CMAKE_SOURCE_DIR}/examples/hdr_demo.c")
            add_appcore_example(hdr_demo "examples/hdr_demo.c")
        endif()

        # 3D screens simple demo (~100 lines)
        if(EXISTS "${CMAKE_SOURCE_DIR}/examples/3d_screens_simple/main.c")
            add_appcore_example(3d_screens_simple "examples/3d_screens_simple/main.c")
        endif()

        # Multi-view shared JS demo (needs direct SDL + backend access for offscreen compositing)
        if(EXISTS "${CMAKE_SOURCE_DIR}/examples/multiview_1js.cpp")
            add_appcore_example(multiview_1js "examples/multiview_1js.cpp")
            target_include_directories(multiview_1js PRIVATE
                "${CMAKE_SOURCE_DIR}/backends/sdl"
                "${SDL3_PREFIX}/include"
            )
        endif()

        # Dock demo (multi-window docking system)
        if(EXISTS "${CMAKE_SOURCE_DIR}/examples/dock_demo.c")
            add_appcore_example(dock_demo "examples/dock_demo.c")
            target_include_directories(dock_demo PRIVATE "${SDL3_PREFIX}/include")
        endif()
    endif()

    # Feature test runner
    if(EXISTS "${CMAKE_SOURCE_DIR}/examples/features/run_feature_tests.cpp")
        add_executable(run_feature_tests
            examples/features/run_feature_tests.cpp
            examples/features/feature_test_framework.cpp
        )
        target_link_libraries(run_feature_tests PRIVATE dong dong_sdl_backend)
        target_include_directories(run_feature_tests PRIVATE "${SDL3_PREFIX}/include")
        target_include_directories(run_feature_tests PRIVATE "${CMAKE_SOURCE_DIR}/backends/sdl")

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
        
        # DXC only on Windows/macOS
        if(WIN32 OR APPLE)
            target_link_directories(run_feature_tests PRIVATE ${DXC_LIB_PATH})
            target_link_libraries(run_feature_tests PRIVATE dxcompiler)
        endif()
        
        if(WIN32)
            target_link_libraries(run_feature_tests PRIVATE user32 gdi32 ole32 shell32 advapi32)
        elseif(APPLE)
            target_link_libraries(run_feature_tests PRIVATE
                "-framework CoreFoundation"
                "-framework CoreGraphics"
                "-framework CoreText"
            )
        else()
            # Linux: link ubsan and stdc++ for Zig Debug builds
            target_link_libraries(run_feature_tests PRIVATE ubsan stdc++)
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
    RUNTIME DESTINATION bin
)


install(DIRECTORY include/ DESTINATION include)

# Install examples
if(BUILD_EXAMPLES)
    # Get all example targets
    set(EXAMPLE_TARGETS)
    foreach(example ${CPP_EXAMPLES})
        if(TARGET ${example})
            list(APPEND EXAMPLE_TARGETS ${example})
        endif()
    endforeach()
    if(TARGET msdf_cpu_test)
        list(APPEND EXAMPLE_TARGETS msdf_cpu_test)
    endif()

    # Add AppCore examples
    if(BUILD_APPCORE)
        if(TARGET minimal_dong_demo)
            list(APPEND EXAMPLE_TARGETS minimal_dong_demo)
        endif()
        if(TARGET interactive_demo_new)
            list(APPEND EXAMPLE_TARGETS interactive_demo_new)
        endif()
        if(TARGET hdr_demo)
            list(APPEND EXAMPLE_TARGETS hdr_demo)
        endif()
        if(TARGET 3d_screens_simple)
            list(APPEND EXAMPLE_TARGETS 3d_screens_simple)
        endif()
        if(TARGET multiview_1js)
            list(APPEND EXAMPLE_TARGETS multiview_1js)
        endif()
        if(TARGET dock_demo)
            list(APPEND EXAMPLE_TARGETS dock_demo)
        endif()
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
                PATTERN "*.json"
                PATTERN "*.txt"
                PATTERN "*.png"
                PATTERN "*.mp4"
        )
    endif()
endif()

# Copy SDL3 DLL/dylib to bin
if(WIN32)
    install(FILES "${SDL3_PREFIX}/bin/SDL3.dll" DESTINATION bin)
elseif(APPLE)
    file(GLOB SDL3_DYLIBS "${SDL3_PREFIX}/lib/libSDL3*.dylib")
    # dylib needs executable bit on macOS, so treat as PROGRAMS to preserve runnable perms
    install(PROGRAMS ${SDL3_DYLIBS} DESTINATION lib)

    # VulkanSDK runtime dylibs used by SDL_shadercross (and therefore by dong_sdl_backend)
    # The executable records these as @rpath/..., so we must ship them alongside our own dylibs.
    # NOTE: On macOS these are often symlinks (e.g. libspirv-cross-c-shared.0.dylib -> *.0.67.0.dylib),
    # so we install the whole set to keep symlinks valid.
    set(VULKAN_RUNTIME_LIB_DIR "")
    if(DXC_LIB_PATH AND EXISTS "${DXC_LIB_PATH}/libdxcompiler.dylib")
        set(VULKAN_RUNTIME_LIB_DIR "${DXC_LIB_PATH}")
    elseif(VULKAN_SDK_PATH AND EXISTS "${VULKAN_SDK_PATH}/lib/libdxcompiler.dylib")
        set(VULKAN_RUNTIME_LIB_DIR "${VULKAN_SDK_PATH}/lib")
    elseif(EXISTS "/usr/local/lib/libdxcompiler.dylib")
        set(VULKAN_RUNTIME_LIB_DIR "/usr/local/lib")
    endif()

    if(VULKAN_RUNTIME_LIB_DIR)
        file(GLOB VULKAN_RUNTIME_DYLIBS
            "${VULKAN_RUNTIME_LIB_DIR}/libdxcompiler.dylib"
            "${VULKAN_RUNTIME_LIB_DIR}/libspirv-cross-c-shared*.dylib"
        )
        install(PROGRAMS ${VULKAN_RUNTIME_DYLIBS} DESTINATION lib)
    endif()
else()
    file(GLOB SDL3_SOS "${SDL3_PREFIX}/lib/libSDL3*.so*")
    install(FILES ${SDL3_SOS} DESTINATION lib)
endif()
