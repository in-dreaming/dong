# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**Dong** is a GPU-accelerated HTML/CSS rendering engine written in C++20, designed for embedding in games and interactive applications. It provides a complete web-like UI system with DOM, CSS styling, layout (Yoga), JavaScript (QuickJS), and hardware-accelerated rendering via SDL_GPU.

## Build System

All commands must be run from the `dong/` subdirectory. The project uses **Zig** as the build orchestrator, which manages CMake builds for all dependencies and the main library.

```bash
cd dong

# Build commands
zig build                    # Build everything (deps + dong + examples)
zig build examples           # Build all examples to zig-out/bin
zig build deps               # Build only third-party dependencies

# Run examples
zig build run                # Run interactive demo (legacy)
zig build run-simple         # Run simple demo (legacy)
zig build run-complete       # Run complete demo (legacy)
zig build run-feature-tests  # Run feature tests

# New simplified examples (AppCore-based)
# After build, run from zig-out/bin:
# ./minimal_dong_demo        # ~50 line demo
# ./3d_screens_simple        # ~100 line 3D demo

# HTML rendering test tool
zig build run-html-test -- <html_file> [output.bmp] [width] [height]
zig build run-html-test -- <html_file> [output.bmp] [width] [height] --frames N [--frame-ms MS] [--no-update]
zig build render-all-tests   # Renders all HTML files in examples/data/tests/
```

### Build Configuration

Copy `build.env.example` to `build.env` and configure paths (VULKAN_SDK_PATH, DXC_LIB_PATH, etc.). Platform-specific examples: `build.env.windows.example`, `build.env.macos.example`.

### Windows Notes
- Uses `clang-cl` + Ninja for CMake builds
- If using WSL bash, use `cmd.exe /c zig build` to avoid zig missing
- Always builds in Release mode to avoid `_ITERATOR_DEBUG_LEVEL` mismatches
- DXC libraries expected in `third_party/dxc_2025_07_14/lib/x64`

## Architecture

### Library Architecture (Refactored)

| Library | Purpose | Dependencies |
|---------|---------|--------------|
| **dong.dll** | Platform-agnostic core (DOM/CSS/Layout/Script) | QuickJS, Lexbor, Yoga, FreeType, HarfBuzz |
| **dong_sdl_backend.dll** | SDL-based Platform implementation | SDL3, dong.dll |
| **dong_appcore.dll** | High-level application framework | dong.dll, dong_sdl_backend.dll, SDL3 |
| **dong_legacy.lib** | Legacy SDL-coupled code (transition) | SDL3, FreeType, HarfBuzz |
| **dong_plugin_sdl.dll** | Platform plugin for advanced integration | SDL3, FFmpeg |

### Platform Abstraction Layer

The core `dong.dll` uses dependency injection through the Platform singleton:

```c
// Platform singleton (dong_platform.h)
DongPlatform* dong_platform_get(void);
void dong_platform_set_gpu_driver(DongPlatform*, DongGPUDriver*);
void dong_platform_set_surface_factory(DongPlatform*, DongSurfaceFactory*);

// Convenience macros (C++)
DONG_GPU()       // Get registered GPU driver
DONG_SURFACES()  // Get registered Surface factory
```

**GPUDriver Interface** (`dong_gpu_driver.h`):
```c
typedef struct DongGPUDriverVTable {
    int (*initialize)(DongGPUDriver* driver);
    void (*shutdown)(DongGPUDriver* driver);
    DongGPUTexture (*create_texture)(DongGPUDriver*, const DongGPUTextureDesc*);
    int (*execute)(DongGPUDriver*, const void* command_list);
    // ... more methods
} DongGPUDriverVTable;
```

**Surface Interface** (`dong_surface.h`):
```c
typedef struct DongSurfaceVTable {
    uint32_t (*get_width)(DongSurface*);
    uint32_t (*get_height)(DongSurface*);
    DongGPUTexture (*get_texture)(DongSurface*);
    void (*mark_dirty)(DongSurface*);
    // ... more methods
} DongSurfaceVTable;
```

### AppCore Framework

The `dong_appcore` library provides high-level APIs for rapid application development:

```c
// Create a Dong application (~50 lines total)
dong_app_config_t cfg = { .title = "Demo", .width = 800, .height = 600, .enable_dong = 1 };
dong_app_t* app = dong_app_create(&cfg);
dong_app_run(app, NULL, NULL);  // Blocking main loop
dong_app_destroy(app);

// 3D scene with HTML screens (~100 lines total)
dong_scene3d_t* scene = dong_scene3d_create(app);
dong_scene3d_add_screen_simple(scene, "ui.html", 800, 600, 0, 2, -5);
dong_scene3d_add_overlay(scene, "<div>HUD</div>", 200, 50);
```

### Rendering Pipeline

1. **HTML Parsing** (`src/dom/html/html_parser.cpp`): Parses HTML into DOM tree using lexbor
2. **CSS Processing** (`src/dom/css/`): Parses stylesheets, matches selectors, computes styles
3. **Layout** (`src/layout/layout_engine.cpp`): Computes element positions/sizes using Yoga flexbox engine
4. **Display List Building** (`src/render/painter.cpp`): Traverses DOM/layout to build GPU commands
5. **GPU Rendering** (`src/render/sdl_render/`): Executes display list via SDL_GPU with HLSL shaders

### Key Classes

- `dom::DOMNode` (`src/dom/dom/dom_node.hpp`): DOM tree node
- `layout::Engine` (`src/layout/layout_engine.hpp`): Layout engine interface
- `render::Painter` (`src/render/painter.hpp`): Display list builder
- `render::DisplayList` (`src/render/display_list.hpp`): GPU command buffer

### Public C API (`include/dong.h`)

```c
// Engine lifecycle (API version: 2)
dong_engine_create(const dong_engine_desc_t* desc, dong_engine_t** out_engine);
dong_engine_destroy(dong_engine_t* engine);
dong_engine_tick(dong_engine_t* engine);

// Content loading
dong_engine_load_html(dong_engine_t* engine, const char* html);
dong_engine_resize(dong_engine_t* engine, uint32_t width, uint32_t height);
dong_engine_set_gpu(dong_engine_t* engine, void* device, void* window);

// Input events
dong_engine_send_mouse_move(dong_engine_t* engine, int32_t x, int32_t y);
dong_engine_send_mouse_button(dong_engine_t* engine, int32_t button, int pressed);
dong_engine_send_mouse_wheel(dong_engine_t* engine, float delta_x, float delta_y);
dong_engine_send_key(dong_engine_t* engine, uint32_t key_code, int pressed);
dong_engine_send_text(dong_engine_t* engine, const char* text);

// JavaScript
dong_engine_eval_script(dong_engine_t* engine, const char* code);

// GPU command list access
const void* dong_engine_get_command_list(dong_engine_t* engine);
void dong_engine_invalidate_commands(dong_engine_t* engine);
```

## Key Conventions

### Input Event Convention
**Scroll wheel semantics**: `delta_y` positive = scroll down (content moves up). Callers must translate platform values to this semantic before passing to API.

### Logging
- Environment variable `DONG_LOG_LEVEL` to set level (default: INFO)
- Environment variable `DONG_LOG_TO_STDOUT=1` to output to stdout instead of stderr

### Output Paths
Use absolute paths for output directories to avoid nested `zig-out/zig-out/` paths.

## Third-Party Dependencies

All built via CMake, orchestrated by Zig:
- **FreeType**: Font rasterization
- **HarfBuzz**: Text shaping (depends on FreeType)
- **msdfgen**: Multi-channel signed distance field glyph generation
- **SDL3**: Window/input/GPU abstraction
- **QuickJS**: JavaScript engine
- **lexbor**: Fast HTML parser
- **Yoga**: Flexbox layout engine
- **FFmpeg**: Video decoding (in SDL plugin)
- **DXC**: DirectX shader compiler (for HLSL→SPIV on Vulkan)

## Debugging Tools

Located in `scripts/tools/`:
- `html_baseline_render.py`: Playwright-based browser rendering baseline
- `vl_tool_multi.py`: Multi-frame diff analysis with optional LLM
- `run_multiframe_regress.py`: Automated non-determinism detection
- `run_baseline_compare.py`: Automated browser comparison
- `compare_screenshots.py`: Pixel-level screenshot comparison

## Development Workflow

This project uses Document-Driven Development (DDD). For multi-file changes, use the `/ddd:*` slash commands:

```bash
/ddd:1-plan <feature>   # Phase 1: Planning & Design
/ddd:2-docs             # Phase 2: Update documentation
/ddd:3-code-plan        # Phase 3: Plan code changes
/ddd:4-code             # Phase 4: Implement & verify
/ddd:5-finish           # Phase 5: Cleanup & finalize
/ddd:status             # Check current progress
```

## Philosophy

- **Ruthless simplicity**: Minimize abstractions, start minimal, avoid future-proofing
- **Bricks and studs**: Each module is a self-contained "brick" with defined connectors (interfaces)
- **Regenerate over edit**: Prefer regenerating modules within bounded context over line-by-line edits
- **Fix root causes**: Do not work around issues by changing rules—fix the underlying problem
