# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

> **Documentation**: User-facing docs live in [`docs/`](../docs/). Maintainer docs in [`docs/developer/`](../docs/developer/). Human contributors see [`CONTRIBUTING.md`](../CONTRIBUTING.md).

## Project Overview

**Dong** is a GPU-accelerated HTML/CSS rendering engine written in C++20, designed for embedding in games and interactive applications. It provides a complete web-like UI system with DOM, CSS styling, layout (Yoga), JavaScript (Porffor AOT compiler), and hardware-accelerated rendering via SDL_GPU.

## Build System

All commands must be run from the `dong/` subdirectory. The project uses **Zig** as the primary build system with **pure Zig builds** for most dependencies and CMake fallback for complex libraries (SDL3, Dong core).

```bash
cd dong

# Build commands
zig build                    # Build everything (deps + dong + examples)
zig build examples           # Build all examples to zig-out/bin
zig build deps               # Build only third-party dependencies

# Individual dependency builds (pure Zig)
zig build lexbor             # Build Lexbor only
zig build yoga               # Build Yoga only
zig build freetype           # Build FreeType only
zig build harfbuzz           # Build HarfBuzz only
zig build msdfgen            # Build msdfgen only
zig build sdl3               # Build SDL3 only (CMake)

# React/Preact examples (pre-built bundles copied automatically by zig build)
zig build react              # Build React bundles from source (requires Node.js)
zig build preact             # Build Preact bundles from source (requires Node.js)

# Run examples (AppCore-based, recommended)
# After build, run from zig-out/bin:
./minimal_dong_demo          # Minimal demo (~50 lines)
./interactive_demo_new       # Interactive demo (~200 lines)
./3d_screens_simple          # 3D HTML screens demo (~130 lines)

# Run React/Preact examples (set higher script timeout for bundle eval)
set DONG_SCRIPT_TIMEOUT_MS=10000   # Windows
export DONG_SCRIPT_TIMEOUT_MS=10000 # Linux/macOS
./dong_app --html data/react-counter/index.html
./dong_app --html data/react-game-ui/index.html
./dong_app --html data/react-todo-classic/index.html
./dong_app --html data/preact-counter/index.html
./dong_app --html data/preact-game-ui/index.html
./dong_app --html data/preact-todo-classic/index.html

# Development tools
zig build run-html-test -- <html_file> [output.bmp] [width] [height]
zig build run-feature-tests  # Run feature tests
zig build render-all-tests   # Renders all HTML files in examples/data/tests/
```

### Cross-Compilation

The build system supports cross-compilation to multiple platforms from Windows:

```bash
# Linux x64 (glibc)
zig build -Dtarget=x86_64-linux-gnu -Dlibs-only=true -p zig-out-linux-x64 deps

# Linux ARM64 (glibc)
zig build -Dtarget=aarch64-linux-gnu -Dlibs-only=true -p zig-out-linux-arm64 deps

# Linux x64 (musl, fully static)
zig build -Dtarget=x86_64-linux-musl -Dlibs-only=true -p zig-out-linux-musl-x64 deps

# Linux ARM64 (musl, fully static)
zig build -Dtarget=aarch64-linux-musl -Dlibs-only=true -p zig-out-linux-musl-arm64 deps
```

Cross-compilation helper script:
```bash
python scripts/cross_compile.py all         # Build all Linux targets
python scripts/cross_compile.py linux-arm64 # Build specific target
python scripts/cross_compile.py list        # List available targets
```

**Note**: macOS/iOS cross-compilation requires macOS SDK (only available on macOS host).

Mobile targets automatically enter "libs-only" mode, building static libraries for integration into native apps.

### Build Configuration

Copy `build.env.example` to `build.env` and configure paths (VULKAN_SDK_PATH, DXC_LIB_PATH, etc.). Platform-specific examples:
- `build.env.windows.example` - Windows configuration
- `build.env.macos.example` - macOS configuration
- `build.env.android.example` - Android cross-compilation
- `build.env.ios.example` - iOS cross-compilation
- `build.env.ohos.example` - HarmonyOS cross-compilation

### Windows Notes
- Uses `clang-cl` + Ninja for CMake builds
- If using WSL bash, use `cmd.exe /c zig build` to avoid zig missing
- Always builds in Release mode to avoid `_ITERATOR_DEBUG_LEVEL` mismatches
- DXC libraries expected in `third_party/dxc_2025_07_14/lib/x64`

### Dependency Build System

| Dependency | Build Method | Notes |
|------------|--------------|-------|
| Lexbor | Pure Zig | Platform-specific ports |
| Yoga | Pure Zig | C++20, -fno-exceptions |
| FreeType | Pure Zig | Module-based compilation |
| HarfBuzz | Pure Zig | Amalgamation build |
| msdfgen | Pure Zig | Core + ext modules |
| Dong Core | Pure Zig | Main C++ library (C++20) |
| SDL Backend | Pure Zig | SDL GPU driver (C++20) |
| SDL3 | CMake | Complex platform code |
| Porffor | Node.js AOT compile | JS→Wasm→C script engine; sole script engine (see `docs/developer/porffor/`) |

### Pure Zig Build Steps

```bash
# Build individual components with pure Zig
zig build dong-core     # Build Dong Core library
zig build sdl-backend   # Build SDL backend library

# Cross-compile to Linux ARM64
zig build -Dtarget=aarch64-linux-gnu dong-core
```

### Shader Pre-compilation

For mobile platforms, pre-compile shaders to SPIRV:

```bash
# Pre-compile to SPIRV for Vulkan
python scripts/precompile_shaders.py --vulkan

# Generate C header with embedded bytecode
python scripts/precompile_shaders.py --vulkan --header

# Pre-compile to Metal (requires spirv-cross)
python scripts/precompile_shaders.py --metal
```

### DXC Download

Download DXC compiler binaries:

```bash
python scripts/download_dxc.py --version 2025_07_14
```

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
| **dong.dll** | Platform-agnostic core (DOM/CSS/Layout/Script) | Porffor (AOT script engine), Lexbor, Yoga, FreeType, HarfBuzz |
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

### GPU / frame stats（轻量）
- `DONG_GPU_STATS=1`：每帧在 `SDLGPUDriver::execute` 末尾多打一行 `[GPUStats]`（instanced uber 批次数、instance 总数、uniform uber draw 数、`uber_gpu_draws_total` 等），用于对比 draw 合批收益。
- Chrome Trace：`DONG_PROFILER=1` 启用 `src/core/profiler`；GPU 相关 scope 带 `category="gpu"`（如 `GPU::execute`、`GPU::uberInstancedDraw`、`GPU::prepareUberQuadInstances`）；帧末可调用 `dong_profiler_dump(path.json)` 导出（见 `profiler.h`）。

### GPU Compressed Textures
- Environment variable `DONG_ATLAS_FORMAT=BC7` or `DONG_ATLAS_FORMAT=ASTC` to enable compressed atlas
- GPU decompression is automatic (hardware supported)

### Output Paths
Use absolute paths for output directories to avoid nested `zig-out/zig-out/` paths.

## CSS Layout Features

Dong supports modern CSS layout features with standards compliance:

### position: sticky

- **Layout-time space reservation + render-time visual adjustment**: Sticky elements participate in normal flow during layout (siblings are aware of their space), then visually adjust position during rendering based on scroll offset
- **Sticky positioning containing block creation**: Proper containing block hierarchy for descendants
- **Clamped to containing block's padding box**: Not content box
- **Nested sticky support**: Multiple sticky elements stack correctly
- **Implementation**: `src/layout/sticky_positioning.cpp`
- **Test cases**: `test_sticky_scroll_top.html`, `test_sticky_scroll_bottom.html`, `test_sticky_parent_clamp.html`, `test_sticky_nested.html`

### aspect-ratio

- **Pre-layout dimension resolution before Yoga**: Calculates missing dimension from aspect ratio before Yoga layout
- **CSS Box Sizing Module Level 4 compliant**: Transferred size suggestions, min/max constraint interaction
- **Replaced elements support**: Uses natural width/height of images/videos
- **Flex/Grid constraint mode**: When both dimensions specified, aspect-ratio acts as constraint
- **Implementation**: `src/layout/aspect_ratio_resolver.cpp`
- **Test cases**: `test_aspect_ratio_width_auto_height.html`, `test_aspect_ratio_min_max.html`, `test_aspect_ratio_flex.html`

### display: contents

- **Yoga tree surgery (skip node creation)**: Contents elements don't create Yoga nodes
- **Children attached to grandparent**: Direct children become children of grandparent in layout
- **Pseudo-elements still render**: `::before` and `::after` render correctly
- **Event bubbling through contents elements**: Events traverse through contents parents
- **Implementation**: `src/layout/display_contents.cpp`
- **Test cases**: `test_display_contents_layout.html`, `test_display_contents_pseudo.html`, `test_display_contents_events.html`

### display: flow-root

- **BFC margin collapse prevention**: Blocks margin collapse between parent and children
- **Implementation**: `src/layout/margin_collapse.cpp`
- **Future**: Float containment (when float implemented)
- **Test cases**: `test_flow_root_margin_collapse.html`

**Architecture principle**: Layout vs rendering separation
- Layout phase: Compute positions assuming no scroll
- Render phase: Apply scroll-aware visual adjustments

**Module strategy**: Yoga协同 (Yoga cooperation)
- Pre/post-process around Yoga layout
- Don't modify Yoga library itself
- Maintain dependency purity

See `docs/reference/css-subset.md` for detailed specifications and implementation notes.

## List Styling

Dong supports CSS list styling properties and `::marker` pseudo-element for customizable list markers:

### list-style Properties

- **list-style-type**: Marker types (disc, circle, square, decimal, lower-alpha, upper-alpha, lower-roman, upper-roman, none)
- **list-style-position**: Marker positioning (outside = margin area, inside = content flow)
- **list-style**: Shorthand property (e.g., `list-style: circle inside`)
- **Default styles**: `<ul>` defaults to `disc`, `<ol>` defaults to `decimal`
- **Counter management**: Each `<ul>`/`<ol>` has independent counter scope for nested lists
- **`<ol start="N">`**: Sets initial counter value

### ::marker Pseudo-element

- **Automatic generation**: `<li>` elements automatically generate `::marker` pseudo-elements
- **Independent styling**: Style markers separately via `li::marker { color: red; font-size: 20px; }`
- **Inherits text styles**: Markers inherit font properties from parent `<li>`
- **Counter scoping**: Nested lists have independent counters

**Example**:
```html
<style>
  ul { list-style-type: disc; }
  ol { list-style-type: decimal; }
  li::marker { color: red; font-weight: bold; }
</style>
<ul>
  <li>Unordered item (red bold disc)</li>
</ul>
<ol>
  <li>Ordered item (red bold "1.")</li>
</ol>
```

**Implementation**:
- Marker generation: `src/render/list_marker.cpp` (pure function)
- CSS parsing: `src/dom/css/css_parser.cpp` (property handlers)
- Pseudo-element creation: `src/dom/css/style_engine.cpp`
- Rendering: `src/render/painter/painter_marker.cpp`

**Test cases**: `test_list_markers_basic.html`, `test_list_style_types.html`, `test_list_position.html`, `test_marker_pseudo.html`, `test_list_nested.html`, `test_ol_start.html`

See `docs/reference/css-subset.md` for complete list system documentation.

## Form Elements

Dong supports standard HTML form elements with full interactive functionality:

### Select Elements

The `<select>` element provides dropdown list functionality with complete interaction support:

**Features**:
- Click to expand/collapse dropdown
- Option selection via mouse click
- Keyboard navigation (Arrow keys, Enter, Escape)
- JavaScript property access (`selectedIndex`, `value`, `options`)
- `change` event dispatched on selection
- CSS styling (border, background, padding, focus states)

**Example**:
```html
<select id="country">
  <option value="us">United States</option>
  <option value="uk">United Kingdom</option>
  <option value="ca">Canada</option>
</select>

<script>
  const select = document.getElementById('country');
  select.addEventListener('change', (e) => {
    console.log('Selected:', e.target.value);
  });

  // JavaScript API
  console.log(select.selectedIndex);  // Current selected index
  console.log(select.value);          // Current selected value
  console.log(select.options.length); // Number of options
</script>
```

**Implementation**:
- State management: `src/dom/select_element.hpp/cpp`
- Rendering: `src/render/painter/painter_select.cpp`
- Event routing: `src/core/engine_view.cpp`

**Test cases**: `examples/data/tests/test_select_keyboard.html`, `test_select_basic.html`

### Input Elements

The `<input>` and `<textarea>` elements support text editing with full interaction:

**Features**:
- Text input with cursor positioning
- Selection (copy/paste-ready)
- Keyboard editing (Backspace, Delete, Arrow keys)
- `maxlength` validation
- `placeholder` rendering
- `readonly` and `disabled` states

**Implementation**: `src/dom/input_element.hpp/cpp`

### Other Form Elements

- `<button>` - Click events, focus management
- Checkbox/Radio - Visual state, `checked` property
- `<label>` - `for` attribute association

## Third-Party Dependencies

All built via CMake, orchestrated by Zig:
- **FreeType**: Font rasterization
- **HarfBuzz**: Text shaping (depends on FreeType)
- **msdfgen**: Multi-channel signed distance field glyph generation
- **SDL3**: Window/input/GPU abstraction
- **Porffor**: JS→Wasm→C AOT compiler; sole JavaScript engine (Node.js-based, invoked at build time)
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

See [docs/developer/README.md](../docs/developer/README.md) for maintainer docs and Phase specs.

## Philosophy

- **Ruthless simplicity**: Minimize abstractions, start minimal, avoid future-proofing
- **Bricks and studs**: Each module is a self-contained "brick" with defined connectors (interfaces)
- **Regenerate over edit**: Prefer regenerating modules within bounded context over line-by-line edits
- **Fix root causes**: Do not work around issues by changing rules—fix the underlying problem
- **Plugin injection**: If Core needs to access something in backend, make it a plugin interface
