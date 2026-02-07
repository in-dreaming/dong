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

# Run examples (AppCore-based, recommended)
# After build, run from zig-out/bin:
./minimal_dong_demo          # Minimal demo (~50 lines)
./interactive_demo_new       # Interactive demo (~200 lines)
./3d_screens_simple          # 3D HTML screens demo (~130 lines)

# Development tools
zig build run-html-test -- <html_file> [output.bmp] [width] [height]
zig build run-feature-tests  # Run feature tests
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

### Core Design Principle

**Core層无SDL依赖**: `dong.dll` (Core) 是平台无关的，所有平台相关实现通过 Plugin 注入。Backend 通过 `DongPlugin` vtable 注入 `DongGPUDriver` 等接口到 Platform 单例。

```
Applications (dong_appcore)
         │
    dong.dll (Core: DOM/CSS/Layout/Script, 平台无关)
         │ Platform Singleton (接口注入点)
         │  - DongGPUDriver*    (由 backend 注入)
         │  - DongImageDecoder* (由 backend 注入)
         │  - DongFileSystem*   (由 backend 注入)
         │
    backends/sdl/ (SDL 实现: sdl_gpu_driver, sdl_window, shaders/*.hlsl)
```

### Library Architecture

| Library | Purpose | Dependencies |
|---------|---------|--------------|
| **dong.dll** | Platform-agnostic core (DOM/CSS/Layout/Script) | QuickJS, Lexbor, Yoga, FreeType, HarfBuzz |
| **dong_sdl_backend.dll** | SDL backend (GPU渲染/窗口/输入) | SDL3, SDL_ShaderCross, DXC |
| **dong_appcore.dll** | High-level application framework | dong.dll, dong_sdl_backend.dll |
| **dong_plugin_sdl.dll** | Platform plugin for video/FFmpeg | SDL3, FFmpeg |

### Key Directories

- `src/core/` - View, Context, GlobalShared (资源共享)
- `src/dom/` - DOM tree, CSS parsing, selector matching
- `src/layout/` - Yoga flexbox layout engine wrapper
- `src/render/` - Display list, Painter (platform-agnostic command generation)
- `backends/sdl/` - SDL GPU driver, shaders, window management
- `include/` - Public C API headers

### Rendering Pipeline

1. **HTML Parsing** (`src/dom/html/html_parser.cpp`): Parses HTML into DOM tree using lexbor
2. **CSS Processing** (`src/dom/css/`): Parses stylesheets, matches selectors, computes styles
3. **Layout** (`src/layout/layout_engine.cpp`): Computes element positions/sizes using Yoga
4. **Display List Building** (`src/render/painter.cpp`): Traverses DOM/layout to build GPU commands
5. **GPU Rendering** (`backends/sdl/sdl_gpu_driver_execute.cpp`): Executes display list via SDL_GPU with HLSL shaders

### GlobalShared (Multi-View Resource Sharing)

同进程多 View 共享 GlyphAtlas，避免每个 View 创建独立的字图集。23 屏幕场景节省 ~936MB GPU 内存。

- C API: `dong_global_shared.h`
- Implementation: `src/core/global_shared.hpp/cpp`

### AppCore Framework (Recommended API)

**Simple Application** (`dong_app.h`):
```c
dong_app_config_t cfg = {0};
cfg.title = "Demo";
cfg.width = 800;
cfg.height = 600;
cfg.enable_dong = 1;

dong_app_t* app = dong_app_create(&cfg);
dong_app_load_html(app, "<html><body><h1>Hello!</h1></body></html>");
dong_app_run(app, NULL, NULL);  // Blocking main loop
dong_app_destroy(app);
```

**3D Scene with HTML Screens** (`dong_scene3d.h`):
```c
dong_scene3d_t* scene = dong_scene3d_create(app);

dong_screen3d_config_t scfg = {0};
scfg.html_content = "<html>...</html>";
scfg.width = 800; scfg.height = 600;
scfg.pos_x = 0; scfg.pos_y = 2; scfg.pos_z = -5;
dong_scene3d_add_screen(scene, &scfg);

while (dong_app_is_running(app)) {
    dong_app_poll_events(app);
    dong_scene3d_update(scene, dong_app_get_delta_time(app));
    dong_scene3d_render(scene);
    dong_app_present(app);
}
```

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
```

## Key Conventions

### Input Event Convention
**Scroll wheel semantics**: `delta_y` positive = scroll down (content moves up). Callers must translate platform values to this semantic before passing to API.

### Logging
- Use `DONG_LOG_ERROR/WARN/INFO/DEBUG` macros (defined in `src/core/log.h`)
- Environment variable `DONG_LOG_LEVEL` to set level (default: INFO)
- Environment variable `DONG_LOG_TO_STDOUT=1` to output to stdout instead of stderr

### GPU Compressed Textures
- Environment variable `DONG_ATLAS_FORMAT=BC7` or `DONG_ATLAS_FORMAT=ASTC` to enable compressed atlas
- GPU decompression is automatic (hardware supported)

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
- **DXC**: DirectX shader compiler (for HLSL→SPIRV on Vulkan)

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

## Refactoring Progress

See `doc/重构遗留.md` for current refactoring status and TODO items.

## Philosophy

- **Ruthless simplicity**: Minimize abstractions, start minimal, avoid future-proofing
- **Bricks and studs**: Each module is a self-contained "brick" with defined connectors (interfaces)
- **Regenerate over edit**: Prefer regenerating modules within bounded context over line-by-line edits
- **Fix root causes**: Do not work around issues by changing rules—fix the underlying problem
- **Plugin injection**: If Core needs to access something in backend, make it a plugin interface
