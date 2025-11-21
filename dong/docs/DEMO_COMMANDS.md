# Demo Build Commands

All demo commands are integrated into the Zig build system.

## Quick Reference

```bash
cd dong

# Build library + all demos
zig build

# Run specific demos
zig build run-basic       # C API demo
zig build run-dom         # DOM parsing
zig build run-style       # CSS cascade
zig build run-js          # JavaScript API
zig build run-render      # Rendering + scripting
zig build run-complete    # Full pipeline demo
zig build run-integration # Integration tests (45+ checks)
```

## Output Location

Built binaries: `dong/zig-out/bin/`
- `basic_demo`
- `dom_test`
- `style_cascade_test`
- `js_api_test`
- `render_and_script_demo`
- `complete_demo`
- `integration_demo`

## Build System Architecture

### File Structure
```
dong/build.zig
├── buildDongLibrary()     # Main library (QuickJS, Lexbor, Yoga, Skia, DongCore)
└── buildDemos()
    ├── buildBasicDemo()
    ├── buildDomTest()
    ├── buildStyleCascadeTest()
    ├── buildJsApiTest()
    ├── buildRenderAndScriptDemo()
    ├── buildCompleteDemo()
    └── buildIntegrationDemo()
```

### Function Isolation Pattern

Each demo builder follows this structure:
1. **Create executable** with target/optimization settings
2. **Link core library** (`libdong.a`)
3. **Add include paths** (public API + internal headers)
4. **Add source files** with appropriate C++ standard
5. **Link platform libraries** (OS-specific dependencies)
6. **Install binary** to standard output location
7. **Create run step** for `zig build run-xxx` command

### Code Quality

- ✅ Clear function separation - no mixed responsibilities
- ✅ Explicit dependency passing - no global state
- ✅ Platform abstraction - `linkPlatformLibs()` handles OS differences
- ✅ Consistent patterns - all demos use same builder pattern
- ✅ Self-contained - each demo builder can be understood independently

