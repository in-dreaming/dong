const std = @import("std");
const builtin = @import("builtin");

// =============================================================================
// Build Configuration (loaded from build.env)
// =============================================================================
const BuildConfig = struct {
    vulkan_sdk_path: ?[]const u8 = null,
    dxc_lib_path: ?[]const u8 = null,
    dxc_include_path: []const u8 = "third_party/DXC-1.8.2505.1/include",
};

fn loadBuildConfig(allocator: std.mem.Allocator) BuildConfig {
    var config = BuildConfig{};

    const env_file = std.fs.cwd().openFile("build.env", .{}) catch {
        std.debug.print("Note: build.env not found, using defaults. Copy build.env.example to build.env and configure paths.\n", .{});
        return config;
    };
    defer env_file.close();

    var buf_reader = std.io.bufferedReader(env_file.reader());
    var line_buf: [1024]u8 = undefined;

    while (buf_reader.reader().readUntilDelimiterOrEof(&line_buf, '\n') catch null) |line| {
        // Remove \r if present (Windows line endings)
        const trimmed_line = if (line.len > 0 and line[line.len - 1] == '\r')
            line[0 .. line.len - 1]
        else
            line;

        // Skip empty lines and comments
        const stripped = std.mem.trim(u8, trimmed_line, " \t");
        if (stripped.len == 0 or stripped[0] == '#') continue;

        // Parse KEY=VALUE
        if (std.mem.indexOf(u8, stripped, "=")) |eq_pos| {
            const key = std.mem.trim(u8, stripped[0..eq_pos], " \t");
            const value = std.mem.trim(u8, stripped[eq_pos + 1 ..], " \t");

            if (value.len == 0) continue;

            if (std.mem.eql(u8, key, "VULKAN_SDK_PATH")) {
                config.vulkan_sdk_path = allocator.dupe(u8, value) catch null;
            } else if (std.mem.eql(u8, key, "DXC_LIB_PATH")) {
                config.dxc_lib_path = allocator.dupe(u8, value) catch null;
            } else if (std.mem.eql(u8, key, "DXC_INCLUDE_PATH")) {
                config.dxc_include_path = allocator.dupe(u8, value) catch "third_party/DXC-1.8.2505.1/include";
            }
        }
    }

    return config;
}

// =============================================================================
// Unified Build System
// All platforms use CMake for C++ code, zig orchestrates the build
// =============================================================================
pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // Load build configuration
    const config = loadBuildConfig(b.allocator);

    const is_windows = target.result.os.tag == .windows;
    const is_macos = target.result.os.tag == .macos;
    const is_linux = target.result.os.tag == .linux;

    // Determine build type string for CMake
    // On Windows, always use Release to avoid _ITERATOR_DEBUG_LEVEL mismatch
    const cmake_build_type = if (is_windows) "Release" else switch (optimize) {
        .Debug => "Debug",
        .ReleaseSafe, .ReleaseFast, .ReleaseSmall => "Release",
    };

    // ==========================================================================
    // Third-party dependencies paths
    // ==========================================================================
    const freetype_build_dir = "third_party/freetype/build-zig";
    const freetype_prefix = "zig-out/freetype";
    const harfbuzz_build_dir = "third_party/harfbuzz/build-zig";
    const harfbuzz_prefix = "zig-out/harfbuzz";
    const msdfgen_build_dir = "third_party/msdfgen/build-zig";
    const msdfgen_prefix = "zig-out/msdfgen";
    const sdl3_build_dir = "third_party/sdl/build-zig";
    const sdl3_prefix = "zig-out/sdl3";
    const quickjs_build_dir = "third_party/quickjs_make/build-zig";
    const quickjs_prefix = "zig-out/quickjs";

    const freetype_include_abs = b.fmt("{s}/{s}/include/freetype2", .{ b.build_root.path.?, freetype_prefix });
    const freetype_lib_abs = if (is_windows)
        b.fmt("{s}/{s}/lib/freetype.lib", .{ b.build_root.path.?, freetype_prefix })
    else
        b.fmt("{s}/{s}/lib/libfreetype.a", .{ b.build_root.path.?, freetype_prefix });

    // ==========================================================================
    // QuickJS via CMake (static library for all platforms)
    // Uses compat layer on Windows for POSIX features (sys/time.h)
    // ==========================================================================
    var quickjs_cmake_args = std.ArrayList([]const u8).init(b.allocator);
    quickjs_cmake_args.appendSlice(&.{
        "cmake", "-S", "third_party/quickjs_make", "-B", quickjs_build_dir,
        b.fmt("-DCMAKE_BUILD_TYPE={s}", .{cmake_build_type}),
        "-DBUILD_SHARED_LIBS=OFF",
    }) catch unreachable;

    if (is_windows) {
        quickjs_cmake_args.appendSlice(&.{
            "-G", "Ninja",
            "-DCMAKE_C_COMPILER=clang-cl",
            "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL",
        }) catch unreachable;
    } else if (is_linux) {
        quickjs_cmake_args.appendSlice(&.{
            "-G", "Ninja",
            "-DCMAKE_C_COMPILER=zig",
            "-DCMAKE_C_COMPILER_ARG1=cc",
        }) catch unreachable;
    }

    const quickjs_cmake_config = b.addSystemCommand(quickjs_cmake_args.items);
    const quickjs_cmake_build = b.addSystemCommand(&.{ "cmake", "--build", quickjs_build_dir, "--config", "Release" });
    quickjs_cmake_build.step.dependOn(&quickjs_cmake_config.step);
    const quickjs_cmake_install = b.addSystemCommand(&.{ "cmake", "--install", quickjs_build_dir, "--prefix", quickjs_prefix });
    quickjs_cmake_install.step.dependOn(&quickjs_cmake_build.step);

    // ==========================================================================
    // FreeType via CMake (static)
    // ==========================================================================
    var freetype_cmake_args = std.ArrayList([]const u8).init(b.allocator);
    freetype_cmake_args.appendSlice(&.{
        "cmake", "-S", "third_party/freetype", "-B", freetype_build_dir,
        b.fmt("-DCMAKE_BUILD_TYPE={s}", .{cmake_build_type}),
        "-DBUILD_SHARED_LIBS=OFF",
        "-DFT_DISABLE_HARFBUZZ=ON",
        "-DFT_DISABLE_PNG=ON",
        "-DFT_DISABLE_ZLIB=ON",
        "-DFT_DISABLE_BZIP2=ON",
        "-DFT_DISABLE_BROTLI=ON",
    }) catch unreachable;

    if (is_windows) {
        freetype_cmake_args.appendSlice(&.{
            "-G", "Ninja",
            "-DCMAKE_C_COMPILER=clang-cl",
            "-DCMAKE_CXX_COMPILER=clang-cl",
            "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL",
        }) catch unreachable;
    } else if (is_linux) {
        freetype_cmake_args.appendSlice(&.{
            "-G", "Ninja",
            "-DCMAKE_C_COMPILER=zig",
            "-DCMAKE_C_COMPILER_ARG1=cc",
            "-DCMAKE_CXX_COMPILER=zig",
            "-DCMAKE_CXX_COMPILER_ARG1=c++",
        }) catch unreachable;
    }

    const freetype_cmake_config = b.addSystemCommand(freetype_cmake_args.items);
    const freetype_cmake_build = b.addSystemCommand(&.{ "cmake", "--build", freetype_build_dir, "--config", cmake_build_type });
    freetype_cmake_build.step.dependOn(&freetype_cmake_config.step);
    const freetype_cmake_install = b.addSystemCommand(&.{ "cmake", "--install", freetype_build_dir, "--prefix", freetype_prefix });
    freetype_cmake_install.step.dependOn(&freetype_cmake_build.step);

    // ==========================================================================
    // HarfBuzz via CMake (static)
    // ==========================================================================
    var harfbuzz_cmake_args = std.ArrayList([]const u8).init(b.allocator);
    harfbuzz_cmake_args.appendSlice(&.{
        "cmake", "-S", "third_party/harfbuzz", "-B", harfbuzz_build_dir,
        b.fmt("-DCMAKE_BUILD_TYPE={s}", .{cmake_build_type}),
        "-DBUILD_SHARED_LIBS=OFF",
        "-DHB_HAVE_FREETYPE=ON",
        b.fmt("-DFREETYPE_INCLUDE_DIRS={s}", .{freetype_include_abs}),
        b.fmt("-DFREETYPE_LIBRARY={s}", .{freetype_lib_abs}),
        "-DHB_HAVE_GLIB=OFF",
        "-DHB_HAVE_ICU=OFF",
        "-DHB_BUILD_UTILS=OFF",
        "-DHB_BUILD_SUBSET=OFF",
    }) catch unreachable;

    if (is_windows) {
        harfbuzz_cmake_args.appendSlice(&.{
            "-G", "Ninja",
            "-DCMAKE_C_COMPILER=clang-cl",
            "-DCMAKE_CXX_COMPILER=clang-cl",
            "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL",
        }) catch unreachable;
    } else if (is_linux) {
        harfbuzz_cmake_args.appendSlice(&.{
            "-G", "Ninja",
            "-DCMAKE_C_COMPILER=zig",
            "-DCMAKE_C_COMPILER_ARG1=cc",
            "-DCMAKE_CXX_COMPILER=zig",
            "-DCMAKE_CXX_COMPILER_ARG1=c++",
        }) catch unreachable;
    }

    const harfbuzz_cmake_config = b.addSystemCommand(harfbuzz_cmake_args.items);
    harfbuzz_cmake_config.step.dependOn(&freetype_cmake_install.step);
    const harfbuzz_cmake_build = b.addSystemCommand(&.{ "cmake", "--build", harfbuzz_build_dir, "--config", cmake_build_type });
    harfbuzz_cmake_build.step.dependOn(&harfbuzz_cmake_config.step);
    const harfbuzz_cmake_install = b.addSystemCommand(&.{ "cmake", "--install", harfbuzz_build_dir, "--prefix", harfbuzz_prefix });
    harfbuzz_cmake_install.step.dependOn(&harfbuzz_cmake_build.step);

    // ==========================================================================
    // msdfgen via CMake (static)
    // ==========================================================================
    var msdfgen_cmake_args = std.ArrayList([]const u8).init(b.allocator);
    msdfgen_cmake_args.appendSlice(&.{
        "cmake", "-S", "third_party/msdfgen", "-B", msdfgen_build_dir,
        b.fmt("-DCMAKE_BUILD_TYPE={s}", .{cmake_build_type}),
        "-DMSDFGEN_CORE_ONLY=OFF",
        "-DMSDFGEN_BUILD_STANDALONE=OFF",
        "-DMSDFGEN_USE_VCPKG=OFF",
        "-DMSDFGEN_INSTALL=ON",
        "-DBUILD_SHARED_LIBS=OFF",
        "-DMSDFGEN_USE_SKIA=OFF",
        "-DMSDFGEN_DISABLE_SVG=ON",
        "-DMSDFGEN_DISABLE_PNG=ON",
        "-DMSDFGEN_DYNAMIC_RUNTIME=ON",
        b.fmt("-DFREETYPE_INCLUDE_DIRS={s}", .{freetype_include_abs}),
        b.fmt("-DFREETYPE_LIBRARY={s}", .{freetype_lib_abs}),
    }) catch unreachable;

    if (is_windows) {
        msdfgen_cmake_args.appendSlice(&.{
            "-G", "Ninja",
            "-DCMAKE_C_COMPILER=clang-cl",
            "-DCMAKE_CXX_COMPILER=clang-cl",
            "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL",
        }) catch unreachable;
    } else if (is_linux) {
        msdfgen_cmake_args.appendSlice(&.{
            "-G", "Ninja",
            "-DCMAKE_C_COMPILER=zig",
            "-DCMAKE_C_COMPILER_ARG1=cc",
            "-DCMAKE_CXX_COMPILER=zig",
            "-DCMAKE_CXX_COMPILER_ARG1=c++",
        }) catch unreachable;
    }

    const msdfgen_cmake_config = b.addSystemCommand(msdfgen_cmake_args.items);
    msdfgen_cmake_config.step.dependOn(&freetype_cmake_install.step);
    const msdfgen_cmake_build = b.addSystemCommand(&.{ "cmake", "--build", msdfgen_build_dir, "--config", cmake_build_type });
    msdfgen_cmake_build.step.dependOn(&msdfgen_cmake_config.step);
    const msdfgen_cmake_install = b.addSystemCommand(&.{ "cmake", "--install", msdfgen_build_dir, "--prefix", msdfgen_prefix });
    msdfgen_cmake_install.step.dependOn(&msdfgen_cmake_build.step);

    // ==========================================================================
    // SDL3 via CMake (shared)
    // ==========================================================================
    var sdl3_cmake_args = std.ArrayList([]const u8).init(b.allocator);
    sdl3_cmake_args.appendSlice(&.{
        "cmake", "-S", "third_party/sdl", "-B", sdl3_build_dir,
        b.fmt("-DCMAKE_BUILD_TYPE={s}", .{cmake_build_type}),
        "-DSDL_TEST=OFF",
        "-DSDL_STATIC=OFF",
        "-DSDL_SHARED=ON",
        "-DSDL_TESTS=OFF",
        "-DSDL_EXAMPLES=OFF",
    }) catch unreachable;

    if (is_windows) {
        sdl3_cmake_args.appendSlice(&.{
            "-G", "Ninja",
            "-DCMAKE_C_COMPILER=clang-cl",
            "-DCMAKE_CXX_COMPILER=clang-cl",
            "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL",
        }) catch unreachable;
    } else if (is_linux) {
        sdl3_cmake_args.appendSlice(&.{
            "-G", "Ninja",
            "-DCMAKE_C_COMPILER=zig",
            "-DCMAKE_C_COMPILER_ARG1=cc",
            "-DCMAKE_CXX_COMPILER=zig",
            "-DCMAKE_CXX_COMPILER_ARG1=c++",
            "-DSDL_UNIX_CONSOLE_BUILD=ON",
        }) catch unreachable;
    }

    const sdl3_cmake_config = b.addSystemCommand(sdl3_cmake_args.items);
    const sdl3_cmake_build = b.addSystemCommand(&.{ "cmake", "--build", sdl3_build_dir, "--config", cmake_build_type });
    sdl3_cmake_build.step.dependOn(&sdl3_cmake_config.step);
    const sdl3_cmake_install = b.addSystemCommand(&.{ "cmake", "--install", sdl3_build_dir, "--prefix", sdl3_prefix });
    sdl3_cmake_install.step.dependOn(&sdl3_cmake_build.step);

    // ==========================================================================
    // Dong core + examples via CMake (unified for all platforms)
    // ==========================================================================
    const cmake_build_dir = "build-cmake";

    const vulkan_sdk_cmake_arg = if (config.vulkan_sdk_path) |vk_path|
        b.fmt("-DVULKAN_SDK_PATH={s}", .{vk_path})
    else
        "-DVULKAN_SDK_PATH=";

    const dxc_lib_cmake_arg = if (config.dxc_lib_path) |dxc_path|
        b.fmt("-DDXC_LIB_PATH={s}", .{dxc_path})
    else if (is_windows)
        "-DDXC_LIB_PATH=third_party/dxc_2025_07_14/lib/x64"
    else if (is_macos)
        "-DDXC_LIB_PATH=/usr/local/lib"
    else
        "-DDXC_LIB_PATH=";

    var dong_cmake_args = std.ArrayList([]const u8).init(b.allocator);
    dong_cmake_args.appendSlice(&.{
        "cmake", "-S", ".", "-B", cmake_build_dir,
        b.fmt("-DCMAKE_BUILD_TYPE={s}", .{cmake_build_type}),
        vulkan_sdk_cmake_arg,
        dxc_lib_cmake_arg,
    }) catch unreachable;

    if (is_windows) {
        dong_cmake_args.appendSlice(&.{
            "-G", "Ninja",
            "-DCMAKE_C_COMPILER=clang-cl",
            "-DCMAKE_CXX_COMPILER=clang-cl",
            "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL",
        }) catch unreachable;
    } else if (is_linux) {
        dong_cmake_args.appendSlice(&.{
            "-G", "Ninja",
            "-DCMAKE_C_COMPILER=zig",
            "-DCMAKE_C_COMPILER_ARG1=cc",
            "-DCMAKE_CXX_COMPILER=zig",
            "-DCMAKE_CXX_COMPILER_ARG1=c++",
            // WSL/devcontainers commonly lack system 'make'. Avoid building FFmpeg-from-source.
            // The plugin can still load system FFmpeg libs at runtime if present.
            "-DDONG_PLUGIN_SDL_ENABLE_FFMPEG=OFF",
        }) catch unreachable;
    }

    const cmake_config = b.addSystemCommand(dong_cmake_args.items);
    cmake_config.step.dependOn(&freetype_cmake_install.step);
    cmake_config.step.dependOn(&harfbuzz_cmake_install.step);
    cmake_config.step.dependOn(&msdfgen_cmake_install.step);
    cmake_config.step.dependOn(&sdl3_cmake_install.step);
    cmake_config.step.dependOn(&quickjs_cmake_install.step);

    const cmake_build = b.addSystemCommand(&.{ "cmake", "--build", cmake_build_dir, "--config", cmake_build_type });
    cmake_build.step.dependOn(&cmake_config.step);

    // ==========================================================================
    // Build steps
    // ==========================================================================
    
    // Install step (cmake --install)
    // On Windows, the destination exe can be locked if it's currently running.
    // Treat that specific case as a non-fatal install skip (build artifacts still exist in build-cmake).
    const cmake_install = if (is_windows)
        b.addSystemCommand(&.{
            "powershell",
            "-NoProfile",
            "-Command",
            b.fmt(
                "& {{ $out = & cmake --install {s} --prefix zig-out 2>&1; $ec = $LASTEXITCODE; if ($ec -eq 0) {{ $out }} else {{ if ($out -match 'file INSTALL cannot copy file' -and ($out -match 'Permission' -and $out -match 'denied')) {{ Write-Host 'NOTE: cmake install skipped because a target file is locked (close running exe and re-run to refresh zig-out/bin).'; exit 0 }} else {{ $out; exit $ec }} }} }}",

                .{cmake_build_dir},
            ),
        })
    else
        b.addSystemCommand(&.{ "cmake", "--install", cmake_build_dir, "--prefix", "zig-out" });
    cmake_install.step.dependOn(&cmake_build.step);
    b.getInstallStep().dependOn(&cmake_install.step);

    
    // examples step now also installs to zig-out/bin
    const examples_step = b.step("examples", "Build all examples and install to zig-out/bin");
    examples_step.dependOn(&cmake_install.step);

    // ==========================================================================
    // Individual dependency build steps (for debugging/manual builds)
    // ==========================================================================
    const deps_step = b.step("deps", "Build all third-party dependencies only");
    deps_step.dependOn(&freetype_cmake_install.step);
    deps_step.dependOn(&harfbuzz_cmake_install.step);
    deps_step.dependOn(&msdfgen_cmake_install.step);
    deps_step.dependOn(&sdl3_cmake_install.step);
    deps_step.dependOn(&quickjs_cmake_install.step);

    const freetype_step = b.step("freetype", "Build FreeType only");
    freetype_step.dependOn(&freetype_cmake_install.step);

    const harfbuzz_step = b.step("harfbuzz", "Build HarfBuzz only");
    harfbuzz_step.dependOn(&harfbuzz_cmake_install.step);

    const msdfgen_step = b.step("msdfgen", "Build msdfgen only");
    msdfgen_step.dependOn(&msdfgen_cmake_install.step);

    const sdl3_step = b.step("sdl3", "Build SDL3 only");
    sdl3_step.dependOn(&sdl3_cmake_install.step);

    const quickjs_step = b.step("quickjs", "Build QuickJS only");
    quickjs_step.dependOn(&quickjs_cmake_install.step);

    // ==========================================================================
    // Run shortcuts
    // ==========================================================================
    const run_demo = b.addSystemCommand(&.{
        if (is_windows) "zig-out\\bin\\interactive_demo.exe" else "zig-out/bin/interactive_demo",
    });
    run_demo.step.dependOn(&cmake_install.step);
    const run_step = b.step("run", "Run interactive demo");
    run_step.dependOn(&run_demo.step);

    const run_simple = b.addSystemCommand(&.{
        if (is_windows) "zig-out\\bin\\simple_demo.exe" else "zig-out/bin/simple_demo",
    });
    run_simple.step.dependOn(&cmake_install.step);
    const run_simple_step = b.step("run-simple", "Run simple demo");
    run_simple_step.dependOn(&run_simple.step);

    const run_complete = b.addSystemCommand(&.{
        if (is_windows) "zig-out\\bin\\complete_demo.exe" else "zig-out/bin/complete_demo",
    });
    run_complete.step.dependOn(&cmake_install.step);
    const run_complete_step = b.step("run-complete", "Run complete demo");
    run_complete_step.dependOn(&run_complete.step);

    // Feature tests
    const run_feature_tests = b.addSystemCommand(&.{
        if (is_windows) "zig-out\\bin\\run_feature_tests.exe" else "zig-out/bin/run_feature_tests",
    });
    run_feature_tests.step.dependOn(&cmake_install.step);
    const run_feature_tests_step = b.step("run-feature-tests", "Run feature tests");
    run_feature_tests_step.dependOn(&run_feature_tests.step);

    // HTML render test - renders HTML to BMP for visual verification
    // Usage: zig build run-html-test -- <html_file> [output.bmp] [width] [height] [frames]
    //        zig build run-html-test -- <html_file> [output.bmp] [width] [height] --frames N [--frame-ms MS] [--no-update]
    const run_html_test = b.addSystemCommand(&.{
        if (is_windows) "zig-out\\bin\\html_render_test.exe" else "zig-out/bin/html_render_test",
    });
    run_html_test.step.dependOn(&cmake_install.step);
    if (b.args) |args| {
        run_html_test.addArgs(args);
    }
    const run_html_test_step = b.step("run-html-test", "Run HTML render test (pass args with --)");
    run_html_test_step.dependOn(&run_html_test.step);

    // Batch render all test HTML files
    const render_all_tests = b.addSystemCommand(if (is_windows) &.{
        "cmd", "/c", "for %f in (zig-out\\bin\\data\\tests\\*.html) do zig-out\\bin\\html_render_test.exe %f zig-out\\tmp\\tests\\%~nf.bmp 800 600",
    } else &.{
        "sh", "-c", "for f in zig-out/bin/data/tests/*.html; do ./zig-out/bin/html_render_test \"$f\" \"zig-out/tmp/tests/$(basename \"$f\" .html).bmp\" 800 600; done",
    });
    render_all_tests.step.dependOn(&cmake_install.step);
    const render_all_step = b.step("render-all-tests", "Render all test HTML files to BMP");
    render_all_step.dependOn(&render_all_tests.step);

    // 3D screens simple demo
    const run_3d_simple = b.addSystemCommand(&.{
        if (is_windows) "zig-out\\bin\\3d_screens_simple.exe" else "zig-out/bin/3d_screens_simple",
    });
    run_3d_simple.step.dependOn(&cmake_install.step);
    const run_3d_simple_step = b.step("run-3d-simple", "Run 3D screens simple demo");
    run_3d_simple_step.dependOn(&run_3d_simple.step);
}
