# CODEBUDDY.md

This file provides guidance to CodeBuddy Code when working with code in this repository.

## Project Overview

**Dong** is a GPU-accelerated HTML/CSS rendering engine written in C++20, designed for embedding in games and interactive applications. It provides a complete web-like UI system with DOM, CSS styling, layout (Yoga), JavaScript (QuickJS), and hardware-accelerated rendering via SDL_GPU.

**Working Directory**: All commands should be executed from the `dong/` subdirectory unless otherwise specified.

## Build System

### Primary Build Commands

The project uses **Zig** as the build orchestrator, which manages CMake builds for all dependencies and the main library:

```bash
cd dong
zig build                    # Build everything (deps + dong + examples)
zig build examples           # Build all examples and install to zig-out/bin
zig build deps               # Build only third-party dependencies
```

### Build Individual Dependencies

```bash
zig build freetype           # Build FreeType only
zig build harfbuzz           # Build HarfBuzz only
zig build msdfgen            # Build msdfgen only
zig build sdl3               # Build SDL3 only
zig build quickjs            # Build QuickJS only
```

### Running Examples

```bash
zig build run                # Run interactive demo
zig build run-simple         # Run simple demo
zig build run-complete       # Run complete demo
zig build run-feature-tests  # Run feature tests
```

### HTML Rendering Test Tool

The `html_render_test` tool renders HTML to BMP files for visual verification:

```bash
# Single frame render
zig build run-html-test -- <html_file> [output.bmp] [width] [height]

# Multi-frame render (for debugging flickering/non-determinism)
zig build run-html-test -- <html_file> [output.bmp] [width] [height] --frames N [--frame-ms MS] [--no-update]
```

**Key parameters:**
- `--frames N`: Export N frames (outputs `xxx_f0.bmp ... xxx_fN.bmp`)
- `--frame-ms MS`: Sleep MS milliseconds between frames
- `--no-update`: Skip `dong_view_update()` calls (isolates rendering-only issues)

### Batch Rendering All Test Cases

```bash
zig build render-all-tests   # Renders all HTML files in examples/data/tests/
```

## Configuration

### Build Environment Setup

Copy `build.env.example` to `build.env` and configure paths:

```bash
cd dong
cp build.env.example build.env
# Edit build.env to set VULKAN_SDK_PATH, DXC_LIB_PATH, etc.
```

Platform-specific examples available:
- `build.env.windows.example`
- `build.env.macos.example`

## Architecture Overview

### Directory Structure

```
dong/
├── src/
│   ├── animation/          # Animation controller
│   ├── core/               # Context, view, profiler, logging
│   ├── dom/                # DOM tree, CSS parser, style engine, event system
│   │   ├── css/            # CSS parsing, selector matching, computed styles
│   │   ├── dom/            # DOM nodes, observers
│   │   └── html/           # HTML parser (lexbor-based)
│   ├── input/              # Input adapters (SDL3)
│   ├── layout/             # Layout engine (Yoga-based flexbox)
│   ├── platform/           # Platform abstraction
│   ├── render/             # Rendering pipeline
│   │   ├── painter/        # Display list construction
│   │   └── sdl_render/     # SDL_GPU backend
│   └── script/             # JavaScript integration (QuickJS)
├── include/
│   ├── dong.h              # Main public C API
│   ├── dong_legacy_view.h  # Legacy view API (transitioning)
│   └── dong_plugin_api.h   # Plugin system API
├── examples/               # Example applications
│   ├── data/               # HTML test files and assets
│   │   └── tests/          # Regression test HTML files
│   └── features/           # Feature test framework
├── scripts/tools/          # Python debugging/analysis tools
├── third_party/            # Dependencies (freetype, harfbuzz, SDL3, etc.)
└── build.zig               # Zig build orchestrator
```

### Rendering Pipeline

The rendering system follows a multi-stage pipeline:

1. **HTML Parsing** (`dom/html/html_parser.cpp`): Parses HTML into DOM tree using lexbor
2. **CSS Processing** (`dom/css/`): Parses stylesheets, matches selectors, computes styles
3. **Layout** (`layout/`): Computes element positions/sizes using Yoga flexbox engine
4. **Display List Building** (`render/painter.cpp`): Traverses DOM/layout to build GPU commands
5. **GPU Rendering** (`render/sdl_render/`): Executes display list via SDL_GPU

**Key classes:**
- `dom::DOMNode`: DOM tree node (see `src/dom/dom/dom_node.hpp`)
- `layout::Engine`: Layout engine interface (see `src/layout/layout_engine.hpp`)
- `render::Painter`: Display list builder (see `src/render/painter.hpp`)
- `render::DisplayList`: GPU command buffer (see `src/render/display_list.hpp`)

### Public API

The engine exposes a C ABI for embedding (`include/dong.h`):

```c
// Engine lifecycle
dong_engine_create(const dong_engine_desc_t* desc, dong_engine_t** out_engine);
dong_engine_destroy(dong_engine_t* engine);
dong_engine_tick(dong_engine_t* engine);  // Update + render frame

// Content loading
dong_engine_load_html(dong_engine_t* engine, const char* html);
dong_engine_resize(dong_engine_t* engine, uint32_t width, uint32_t height);

// Input events
dong_engine_send_mouse_move(dong_engine_t* engine, int32_t x, int32_t y);
dong_engine_send_key(dong_engine_t* engine, uint32_t key_code, int pressed);
dong_engine_send_text(dong_engine_t* engine, const char* text);

// JavaScript
dong_engine_eval_script(dong_engine_t* engine, const char* code);
```

### Third-Party Dependencies

All built via CMake, orchestrated by Zig:
- **FreeType**: Font rasterization
- **HarfBuzz**: Text shaping (depends on FreeType)
- **msdfgen**: Multi-channel signed distance field glyph generation
- **SDL3**: Window/input/GPU abstraction
- **QuickJS**: JavaScript engine
- **lexbor**: Fast HTML parser
- **Yoga**: Flexbox layout engine

## Debugging Multi-Frame Rendering Issues

The codebase includes a comprehensive toolchain for debugging flickering/non-deterministic rendering:

### Workflow for Debugging Flickering

1. **Generate browser baseline** (reference rendering):
   ```bash
   python scripts/tools/html_baseline_render.py <test.html> --out base.png --width 800 --height 600
   ```

2. **Capture multiple frames from dong**:
   ```bash
   zig build run-html-test -- <test.html> output_dir/ 800 600 --frames 120 --frame-ms 16
   ```

3. **Analyze frame-by-frame differences**:
   ```bash
   python scripts/tools/vl_tool_multi.py <test.html> \
     --base base.png \
     --glob "output_dir/*_f*.bmp" \
     --out-image merged.png \
     --out-json report.json \
     --no-llm
   ```

   Output:
   - `merged.png`: Visual comparison grid (BASE | FRAME_i | DIFF per row)
   - `report.json`: Per-frame diff bounding boxes

4. **Optional: LLM-assisted analysis**:
   ```bash
   set OPENROUTER_API_KEY=...
   set OPENROUTER_MODEL=x-ai/grok-4.1-fast
   python scripts/tools/vl_tool_multi.py <test.html> --base base.png --glob "..." --out-image merged.png --out-json report.json
   ```

### Automated Test Suite Verification

**Multi-frame self-consistency check** (detects non-determinism):
```bash
python scripts/tools/run_multiframe_regress.py --frames 60 --frame-ms 0
```

**Browser baseline comparison** (comprehensive correctness check):
```bash
python scripts/tools/run_baseline_compare.py --frames 60 --frame-ms 16
python scripts/tools/run_baseline_compare.py --case cursor_test --frames 120 --frame-ms 16  # Single test
```

### Debugging Tools

Located in `scripts/tools/`:
- `html_baseline_render.py`: Playwright-based browser rendering baseline
- `vl_tool_multi.py`: Multi-frame diff analysis with optional LLM
- `vl_tool.py`: Single-frame verification
- `run_multiframe_regress.py`: Automated non-determinism detection
- `run_baseline_compare.py`: Automated browser comparison
- `compare_screenshots.py`: Pixel-level screenshot comparison
- `auto_profile_loop.py`: Automated profiling loop

## Platform-Specific Notes

### Windows
- Uses `clang-cl` + Ninja for CMake builds
- Always builds in Release mode to avoid `_ITERATOR_DEBUG_LEVEL` mismatches
- DXC libraries expected in `third_party/dxc_2025_07_14/lib/x64`

### macOS
- DXC libraries expected in `/usr/local/lib` or configure via `build.env`

## Code Conventions

When modifying code, adhere to these project rules:

1. **Never create documentation files proactively** - Only create/modify `.md` files when explicitly requested
2. **Use absolute paths for output directories** in tools to avoid nested `zig-out/zig-out/` paths
3. **Do not work around issues by changing rules** - Fix root causes, not symptoms
4. **Build artifacts location**: All outputs go to `zig-out/bin/` after `zig build`

## Common File References

When referencing code locations, use the format `file_path:line_number`:
- DOM nodes: `src/dom/dom/dom_node.cpp:123`
- Layout engine: `src/layout/layout_engine.hpp:45`
- Painter: `src/render/painter.cpp:678`
- API bindings: `src/api_bindings.cpp:234`
