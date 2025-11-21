# Build System Architecture

## Design Principles

### 1. Function Isolation
Each build concern is isolated into its own function with clear input/output:

```zig
fn buildXxxDemo(
    b: *std.Build,                    // Build context
    lib: *std.Build.Step.Compile,     // Core library dependency
    target: std.Build.ResolvedTarget, // Target platform
    optimize: std.builtin.OptimizeMode// Optimization level
) void
```

**Benefits:**
- Functions are independently testable
- Easy to add/remove/modify demos
- Clear responsibilities
- Reduced coupling

### 2. Hierarchical Organization

```
build()
  ├── buildDongLibrary()          [Returns library for linking]
  │   ├── buildQuickJS()
  │   ├── buildLexbor()
  │   ├── buildYoga()
  │   ├── buildSkia()
  │   ├── buildDongCore()
  │   └── linkPlatformLibs()
  │
  └── buildDemos()                [Orchestrates all demos]
      ├── buildBasicDemo()
      ├── buildDomTest()
      ├── buildStyleCascadeTest()
      ├── buildJsApiTest()
      ├── buildRenderAndScriptDemo()
      ├── buildCompleteDemo()
      └── buildIntegrationDemo()
```

Each level has single responsibility:
- `build()`: High-level orchestration
- `buildDongLibrary()`: Aggregate all dependencies
- `buildDemos()`: Dispatcher to individual demo builders
- `buildXxxDemo()`: Individual demo setup

### 3. Shared Infrastructure

#### Platform Library Linking
```zig
fn linkPlatformLibs(lib: *std.Build.Step.Compile, target: std.Build.ResolvedTarget) void
```
- Windows: kernel32, user32, gdi32
- Linux: c, m
- macOS: CoreFoundation, CoreGraphics

Each demo uses the same function - DRY principle.

#### Include Paths
- Public API: `include/`
- Internal: `src/`
- All demos get both paths automatically

### 4. Executable Builder Pattern

```zig
fn buildXxxDemo(...) void {
    // 1. Create
    const exe = b.addExecutable(...)
    
    // 2. Link library
    exe.linkLibrary(lib)
    exe.linkLibCpp()
    
    // 3. Paths
    exe.addIncludePath(...)
    
    // 4. Source
    exe.addCSourceFile(.{ .file = ..., .flags = ... })
    
    // 5. Platform libs
    linkPlatformLibs(exe, target)
    
    // 6. Install
    b.installBinFile(exe.getEmittedBin(), "xxx_demo")
    
    // 7. Run step
    const run_step = b.step("run-xxx", "Run xxx demo")
    const run = b.addRunArtifact(exe)
    run_step.dependOn(&run.step)
}
```

This standardized pattern makes demos easy to:
- Add
- Modify
- Debug
- Extend

### 5. Dependency Graph

```
libdong.a (static library)
  ├── QuickJS (C)
  ├── Lexbor (C)
  ├── Yoga (C++)
  ├── Skia (precompiled)
  └── DongCore (C++)
      └── All subsystems

Each demo executable
  ├── Links: libdong.a
  ├── Includes: include/dong.h + src/
  ├── Language: C or C++
  └── Platform: Determined at build time
```

## Maintenance Guide

### Adding a New Demo

1. Create source file: `examples/my_demo.cpp`
2. Add builder function in `build.zig`:

```zig
fn buildMyDemo(b: *std.Build, lib: *std.Build.Step.Compile, target: std.Build.ResolvedTarget, optimize: std.builtin.OptimizeMode) void {
    const exe = b.addExecutable(.{
        .name = "my_demo",
        .target = target,
        .optimize = optimize,
        .root_source_file = null,
    });
    
    exe.linkLibrary(lib);
    exe.linkLibCpp();
    exe.addIncludePath(b.path("include"));
    exe.addIncludePath(b.path("src"));
    
    exe.addCSourceFile(.{
        .file = b.path("examples/my_demo.cpp"),
        .flags = &.{"-std=c++17"},
    });
    
    linkPlatformLibs(exe, target);
    b.installBinFile(exe.getEmittedBin(), "my_demo");
    
    const run_step = b.step("run-my", "Run my demo");
    const run = b.addRunArtifact(exe);
    run_step.dependOn(&run.step);
}
```

3. Call in `buildDemos()`:
```zig
buildMyDemo(b, lib, target, optimize);
```

### Modifying Library Dependencies

Each dependency has isolated builder:
- `buildQuickJS()` - QuickJS configuration
- `buildLexbor()` - Lexbor sources
- `buildYoga()` - Yoga configuration
- `buildSkia()` - Skia precompiled libs
- `buildDongCore()` - Core sources

Modify only the relevant function to avoid unintended effects.

### Cross-Platform Considerations

Platform-specific logic concentrated in:
- `linkPlatformLibs()` - System library dependencies
- `linkSkiaSystemLibs()` - Graphics framework selection
- Individual `buildXxx()` functions conditionally check for directories

Example:
```zig
if (!dirExists(b.build_root.handle, "third_party/quickjs")) return;
```

## Performance Notes

### Build Time
- Library builds once, cached
- Demos build independently but link to cached library
- Incremental builds only recompile changed files

### File Organization
```
dong/
├── include/               [Public API - 1 file]
├── src/                   [Implementation - 14 files]
├── examples/              [7 demo sources]
├── third_party/
│   ├── quickjs/           [Cached library]
│   ├── lexbor/            [Cached library]
│   ├── yoga/              [Cached library]
│   └── skia/              [Precompiled]
└── build.zig              [Build orchestration]
```

## Verification Checklist

When adding/modifying demos:
- [ ] Function signature matches pattern
- [ ] All include paths added
- [ ] Source language standard specified (`-std=c++17`)
- [ ] Platform libs linked via `linkPlatformLibs()`
- [ ] Binary installed to `zig-out/bin/`
- [ ] Run step created with unique name
- [ ] Called from `buildDemos()` dispatcher

