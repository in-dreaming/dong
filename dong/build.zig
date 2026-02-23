const std = @import("std");
const builtin = @import("builtin");

// =============================================================================
// DXC Configuration
// =============================================================================
const DXC_VERSION = "v1.8.2407";
const DXC_DATE = "2024_07_31"; // Release date for download URL
const DXC_DIR = "third_party/dxc";

// =============================================================================
// Build Configuration (loaded from build.env)
// =============================================================================
const BuildConfig = struct {
    vulkan_sdk_path: ?[]const u8 = null,
    dxc_lib_path: ?[]const u8 = null,
    dxc_include_path: []const u8 = DXC_DIR ++ "/inc",
    android_ndk_path: ?[]const u8 = null,
    ios_sdk_path: ?[]const u8 = null,
    enable_ffmpeg: bool = true, // auto-detected if not set in build.env
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
        const trimmed_line = if (line.len > 0 and line[line.len - 1] == '\r')
            line[0 .. line.len - 1]
        else
            line;

        const stripped = std.mem.trim(u8, trimmed_line, " \t");
        if (stripped.len == 0 or stripped[0] == '#') continue;

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
            } else if (std.mem.eql(u8, key, "ANDROID_NDK_PATH")) {
                config.android_ndk_path = allocator.dupe(u8, value) catch null;
            } else if (std.mem.eql(u8, key, "IOS_SDK_PATH")) {
                config.ios_sdk_path = allocator.dupe(u8, value) catch null;
            } else if (std.mem.eql(u8, key, "ENABLE_FFMPEG")) {
                config.enable_ffmpeg = std.mem.eql(u8, value, "1") or
                    std.ascii.eqlIgnoreCase(value, "ON") or
                    std.ascii.eqlIgnoreCase(value, "TRUE") or
                    std.ascii.eqlIgnoreCase(value, "YES");
            }
        }
    }

    // Auto-detect: disable FFmpeg if the source tree is absent
    if (config.enable_ffmpeg) {
        std.fs.cwd().access("third_party/ffmpeg/configure", .{}) catch {
            config.enable_ffmpeg = false;
        };
    }

    return config;
}

// =============================================================================
// Platform Detection Helpers
// =============================================================================
const PlatformInfo = struct {
    is_windows: bool,
    is_macos: bool,
    is_linux: bool,
    is_ios: bool,
    is_android: bool,
    is_ohos: bool, // HarmonyOS
    is_mobile: bool,
    is_desktop: bool,
    is_native: bool, // Building for host platform
    lib_prefix: []const u8,
    static_lib_ext: []const u8,
    shared_lib_ext: []const u8,
    target_triple: []const u8,
};

fn getPlatformInfo(b: *std.Build, target: std.Build.ResolvedTarget) PlatformInfo {
    const os_tag = target.result.os.tag;
    const abi = target.result.abi;
    const cpu_arch = target.result.cpu.arch;

    const is_windows = os_tag == .windows;
    const is_macos = os_tag == .macos;
    const is_ios = os_tag == .ios;
    const is_linux = os_tag == .linux;
    const is_android = is_linux and (abi == .android or abi == .androideabi);
    const is_ohos = is_linux and (abi == .ohos or abi == .ohoseabi);

    // Check if building for native platform
    const host_os = builtin.os.tag;
    const host_cpu = builtin.cpu.arch;
    const is_native = (os_tag == host_os) and (cpu_arch == host_cpu);

    // Build target triple string
    const arch_str = @tagName(cpu_arch);
    const os_str = @tagName(os_tag);
    const abi_str = @tagName(abi);
    const target_triple = b.fmt("{s}-{s}-{s}", .{ arch_str, os_str, abi_str });

    return .{
        .is_windows = is_windows,
        .is_macos = is_macos,
        .is_linux = is_linux and !is_android and !is_ohos,
        .is_ios = is_ios,
        .is_android = is_android,
        .is_ohos = is_ohos,
        .is_mobile = is_ios or is_android or is_ohos,
        .is_desktop = is_windows or is_macos or (is_linux and !is_android and !is_ohos),
        .is_native = is_native,
        .lib_prefix = if (is_windows) "" else "lib",
        .static_lib_ext = if (is_windows) ".lib" else ".a",
        .shared_lib_ext = if (is_windows) ".dll" else if (is_macos or is_ios) ".dylib" else ".so",
        .target_triple = target_triple,
    };
}

// =============================================================================
// Build Options
// =============================================================================
const BuildOptions = struct {
    enable_ffmpeg: bool,
    enable_sdl: bool,
    android_api_level: u32,
    ios_deployment_target: []const u8,
    libs_only: bool, // Only build static libraries (for mobile)
};

fn getBuildOptions(b: *std.Build, platform: PlatformInfo) BuildOptions {
    const enable_ffmpeg = b.option(bool, "ffmpeg", "Enable FFmpeg support (default: true on desktop)") orelse platform.is_desktop;
    const enable_sdl = b.option(bool, "sdl", "Enable SDL3 backend (default: true on desktop)") orelse platform.is_desktop;
    const android_api_level = b.option(u32, "android-api", "Android API level (default: 21)") orelse 21;
    const ios_target = b.option([]const u8, "ios-target", "iOS deployment target (default: 12.0)") orelse "12.0";
    const libs_only = b.option(bool, "libs-only", "Only build static libraries") orelse platform.is_mobile;

    return .{
        .enable_ffmpeg = enable_ffmpeg,
        .enable_sdl = enable_sdl,
        .android_api_level = android_api_level,
        .ios_deployment_target = ios_target,
        .libs_only = libs_only,
    };
}

// =============================================================================
// Main Build Function
// =============================================================================
pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    const platform = getPlatformInfo(b, target);
    const options = getBuildOptions(b, platform);

    // Load build configuration
    var config = loadBuildConfig(b.allocator);

    // Print build info
    std.debug.print("Building for: {s} (native: {})\n", .{ platform.target_triple, platform.is_native });
    if (platform.is_android) {
        std.debug.print("  Android API level: {}\n", .{options.android_api_level});
    }
    if (platform.is_ios) {
        std.debug.print("  iOS deployment target: {s}\n", .{options.ios_deployment_target});
    }

    // ==========================================================================
    // DXC Auto-download (Windows desktop only)
    // ==========================================================================
    const dxc_step = ensureDxc(b, platform, &config);

    // Build type for CMake (SDL3 fallback)
    const cmake_build_type = if (platform.is_windows) "Release" else switch (optimize) {
        .Debug => "Debug",
        .ReleaseSafe, .ReleaseFast, .ReleaseSmall => "Release",
    };

    // ==========================================================================
    // QuickJS (Pure Zig Build)
    // ==========================================================================
    const quickjs = buildQuickJS(b, target, optimize, platform);

    // ==========================================================================
    // Lexbor (Pure Zig Build)
    // ==========================================================================
    const lexbor = buildLexbor(b, target, optimize, platform);

    // ==========================================================================
    // Yoga (Pure Zig Build)
    // ==========================================================================
    const yoga = buildYoga(b, target, optimize);

    // ==========================================================================
    // FreeType (Pure Zig Build)
    // ==========================================================================
    const freetype = buildFreeType(b, target, optimize, platform);

    // ==========================================================================
    // HarfBuzz (Pure Zig Build)
    // ==========================================================================
    const harfbuzz = buildHarfBuzz(b, target, optimize, platform, freetype);

    // ==========================================================================
    // msdfgen (Pure Zig Build)
    // ==========================================================================
    const msdfgen = buildMsdfgen(b, target, optimize, platform, freetype);

    // ==========================================================================
    // SDL3 via CMake (shared) - too complex for initial pure Zig migration
    // ==========================================================================
    const sdl3_build_dir = "third_party/sdl/build-zig";
    const sdl3_prefix = "zig-out/sdl3";

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

    if (platform.is_windows) {
        sdl3_cmake_args.appendSlice(&.{
            "-G", "Ninja",
            "-DCMAKE_C_COMPILER=clang-cl",
            "-DCMAKE_CXX_COMPILER=clang-cl",
            "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL",
        }) catch unreachable;
    } else if (platform.is_linux) {
        sdl3_cmake_args.appendSlice(&.{
            "-G", "Ninja",
            "-DSDL_UNIX_CONSOLE_BUILD=ON",
        }) catch unreachable;
    }

    const sdl3_cmake_config = b.addSystemCommand(sdl3_cmake_args.items);
    const sdl3_cmake_build = b.addSystemCommand(&.{ "cmake", "--build", sdl3_build_dir, "--config", cmake_build_type });
    sdl3_cmake_build.step.dependOn(&sdl3_cmake_config.step);
    const sdl3_cmake_install = b.addSystemCommand(&.{ "cmake", "--install", sdl3_build_dir, "--prefix", sdl3_prefix });
    sdl3_cmake_install.step.dependOn(&sdl3_cmake_build.step);

    // ==========================================================================
    // Mobile/Cross-compile: libs-only mode
    // For mobile platforms, we only build the static libraries
    // ==========================================================================
    if (options.libs_only) {
        // Build Dong Core for mobile
        const dong_core = buildDongCore(b, target, optimize, platform, quickjs, lexbor, yoga, freetype, harfbuzz, msdfgen);

        // For mobile/cross-compile, just install the static libraries
        const libs_only_step = b.step("libs", "Build only static libraries (for mobile integration)");
        libs_only_step.dependOn(&quickjs.step);
        libs_only_step.dependOn(&lexbor.step);
        libs_only_step.dependOn(&yoga.step);
        libs_only_step.dependOn(&freetype.step);
        libs_only_step.dependOn(&harfbuzz.step);
        libs_only_step.dependOn(&msdfgen.step);
        libs_only_step.dependOn(&dong_core.step);
        b.getInstallStep().dependOn(libs_only_step);

        // Still provide individual build steps for libs-only mode
        const quickjs_step = b.step("quickjs", "Build QuickJS only");
        quickjs_step.dependOn(&quickjs.step);

        const lexbor_step = b.step("lexbor", "Build Lexbor only");
        lexbor_step.dependOn(&lexbor.step);

        const yoga_step = b.step("yoga", "Build Yoga only");
        yoga_step.dependOn(&yoga.step);

        const freetype_step = b.step("freetype", "Build FreeType only");
        freetype_step.dependOn(&freetype.step);

        const harfbuzz_step = b.step("harfbuzz", "Build HarfBuzz only");
        harfbuzz_step.dependOn(&harfbuzz.step);

        const msdfgen_step = b.step("msdfgen", "Build msdfgen only");
        msdfgen_step.dependOn(&msdfgen.step);

        const dong_core_step = b.step("dong-core", "Build Dong Core only");
        dong_core_step.dependOn(&dong_core.step);

        const deps_step = b.step("deps", "Build all third-party dependencies");
        deps_step.dependOn(libs_only_step);

        // Print info about cross-compilation
        std.debug.print("\n=== Cross-compilation mode ===\n", .{});
        std.debug.print("Target: {s}\n", .{platform.target_triple});
        std.debug.print("Building static libraries only.\n", .{});
        std.debug.print("Output: zig-out/lib/\n\n", .{});
        return; // Skip CMake build for mobile targets
    }

    // ==========================================================================
    // Dong core + examples via CMake (for now - will migrate later)
    // ==========================================================================
    const cmake_build_dir = "build-cmake";

    const vulkan_sdk_cmake_arg = if (config.vulkan_sdk_path) |vk_path|
        b.fmt("-DVULKAN_SDK_PATH={s}", .{vk_path})
    else
        "-DVULKAN_SDK_PATH=";

    const dxc_lib_cmake_arg = if (config.dxc_lib_path) |dxc_path|
        b.fmt("-DDXC_LIB_PATH={s}", .{dxc_path})
    else if (platform.is_windows)
        b.fmt("-DDXC_LIB_PATH={s}/lib/x64", .{DXC_DIR})
    else if (platform.is_macos)
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

    if (platform.is_windows) {
        dong_cmake_args.appendSlice(&.{
            "-G", "Ninja",
            "-DCMAKE_C_COMPILER=clang-cl",
            "-DCMAKE_CXX_COMPILER=clang-cl",
            "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL",
        }) catch unreachable;
    } else if (platform.is_linux) {
        dong_cmake_args.appendSlice(&.{
            "-G", "Ninja",
        }) catch unreachable;
    }

    // Always pass FFmpeg flag explicitly to override any stale CMake cache value
    dong_cmake_args.append(
        if (config.enable_ffmpeg) "-DDONG_PLUGIN_SDL_ENABLE_FFMPEG=ON" else "-DDONG_PLUGIN_SDL_ENABLE_FFMPEG=OFF",
    ) catch unreachable;

    const cmake_config = b.addSystemCommand(dong_cmake_args.items);
    // Depend on Zig-built libraries being installed first
    cmake_config.step.dependOn(&quickjs.step);
    cmake_config.step.dependOn(&lexbor.step);
    cmake_config.step.dependOn(&yoga.step);
    cmake_config.step.dependOn(&freetype.step);
    cmake_config.step.dependOn(&harfbuzz.step);
    cmake_config.step.dependOn(&msdfgen.step);
    cmake_config.step.dependOn(&sdl3_cmake_install.step);
    // Depend on DXC being downloaded
    if (dxc_step) |step| {
        cmake_config.step.dependOn(step);
    }

    const cmake_build = b.addSystemCommand(&.{ "cmake", "--build", cmake_build_dir, "--config", cmake_build_type });
    cmake_build.step.dependOn(&cmake_config.step);

    // Install step
    const cmake_install = if (platform.is_windows)
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

    if (platform.is_windows) {
        const copy_runtime = b.addSystemCommand(&.{
            "powershell",
            "-NoProfile",
            "-Command",
            b.fmt(
                "& {{ $build = '{s}'; $bin = 'zig-out/bin'; $ff = Join-Path $build 'plugins/sdl/ffmpeg_install/bin'; if (Test-Path $ff) {{ cmake -E copy_directory $ff $bin | Out-Null }}; $plugin = Join-Path $build 'plugins/sdl/dong_plugin_sdl.dll'; if (Test-Path $plugin) {{ cmake -E copy_if_different $plugin $bin | Out-Null }} }}",
                .{cmake_build_dir},
            ),
        });
        copy_runtime.step.dependOn(&cmake_build.step);
        b.getInstallStep().dependOn(&copy_runtime.step);
    }

    // ==========================================================================
    // Build Steps
    // ==========================================================================
    const examples_step = b.step("examples", "Build all examples and install to zig-out/bin");
    examples_step.dependOn(&cmake_install.step);

    // Individual dependency build steps
    const deps_step = b.step("deps", "Build all third-party dependencies only");
    deps_step.dependOn(&quickjs.step);
    deps_step.dependOn(&lexbor.step);
    deps_step.dependOn(&yoga.step);
    deps_step.dependOn(&freetype.step);
    deps_step.dependOn(&harfbuzz.step);
    deps_step.dependOn(&msdfgen.step);
    deps_step.dependOn(&sdl3_cmake_install.step);

    const quickjs_step = b.step("quickjs", "Build QuickJS only");
    quickjs_step.dependOn(&quickjs.step);

    const lexbor_step = b.step("lexbor", "Build Lexbor only");
    lexbor_step.dependOn(&lexbor.step);

    const yoga_step = b.step("yoga", "Build Yoga only");
    yoga_step.dependOn(&yoga.step);

    const freetype_step = b.step("freetype", "Build FreeType only");
    freetype_step.dependOn(&freetype.step);

    const harfbuzz_step = b.step("harfbuzz", "Build HarfBuzz only");
    harfbuzz_step.dependOn(&harfbuzz.step);

    const msdfgen_step = b.step("msdfgen", "Build msdfgen only");
    msdfgen_step.dependOn(&msdfgen.step);

    const sdl3_step = b.step("sdl3", "Build SDL3 only");
    sdl3_step.dependOn(&sdl3_cmake_install.step);

    // ==========================================================================
    // Dong Core (Pure Zig Build) - Desktop mode only for now
    // ==========================================================================
    const dong_core = buildDongCore(b, target, optimize, platform, quickjs, lexbor, yoga, freetype, harfbuzz, msdfgen);
    const dong_core_step = b.step("dong-core", "Build Dong Core (pure Zig)");
    dong_core_step.dependOn(&dong_core.step);

    // ==========================================================================
    // SDL Backend (Pure Zig Build) - Desktop mode only
    // ==========================================================================
    const sdl_backend = buildSDLBackend(b, target, optimize, platform, config, dong_core);
    const sdl_backend_step = b.step("sdl-backend", "Build SDL Backend (pure Zig)");
    sdl_backend_step.dependOn(&sdl_backend.step);

    // ==========================================================================
    // Run shortcuts
    // ==========================================================================
    const run_demo = b.addSystemCommand(&.{
        if (platform.is_windows) "zig-out\\bin\\interactive_demo_new.exe" else "zig-out/bin/interactive_demo_new",
    });
    run_demo.step.dependOn(&cmake_install.step);
    const run_step = b.step("run", "Run interactive demo");
    run_step.dependOn(&run_demo.step);

    const run_simple = b.addSystemCommand(&.{
        if (platform.is_windows) "zig-out\\bin\\simple_demo.exe" else "zig-out/bin/simple_demo",
    });
    run_simple.step.dependOn(&cmake_install.step);
    const run_simple_step = b.step("run-simple", "Run simple demo");
    run_simple_step.dependOn(&run_simple.step);

    const run_complete = b.addSystemCommand(&.{
        if (platform.is_windows) "zig-out\\bin\\complete_demo.exe" else "zig-out/bin/complete_demo",
    });
    run_complete.step.dependOn(&cmake_install.step);
    const run_complete_step = b.step("run-complete", "Run complete demo");
    run_complete_step.dependOn(&run_complete.step);

    // Feature tests
    const run_feature_tests = b.addSystemCommand(&.{
        if (platform.is_windows) "zig-out\\bin\\run_feature_tests.exe" else "zig-out/bin/run_feature_tests",
    });
    run_feature_tests.step.dependOn(&cmake_install.step);
    const run_feature_tests_step = b.step("run-feature-tests", "Run feature tests");
    run_feature_tests_step.dependOn(&run_feature_tests.step);

    // HTML render test
    const run_html_test = b.addSystemCommand(&.{
        if (platform.is_windows) "zig-out\\bin\\html_render_test.exe" else "zig-out/bin/html_render_test",
    });
    run_html_test.step.dependOn(&cmake_install.step);
    if (b.args) |args| {
        run_html_test.addArgs(args);
    }
    const run_html_test_step = b.step("run-html-test", "Run HTML render test (pass args with --)");
    run_html_test_step.dependOn(&run_html_test.step);

    // Batch render all test HTML files
    const render_all_tests = b.addSystemCommand(if (platform.is_windows) &.{
        "cmd", "/c", "for %f in (zig-out\\bin\\data\\tests\\*.html) do zig-out\\bin\\html_render_test.exe %f zig-out\\tmp\\tests\\%~nf.bmp 800 600",
    } else &.{
        "sh", "-c", "for f in zig-out/bin/data/tests/*.html; do ./zig-out/bin/html_render_test \"$f\" \"zig-out/tmp/tests/$(basename \"$f\" .html).bmp\" 800 600; done",
    });
    render_all_tests.step.dependOn(&cmake_install.step);
    const render_all_step = b.step("render-all-tests", "Render all test HTML files to BMP");
    render_all_step.dependOn(&render_all_tests.step);

    // 3D screens simple demo
    const run_3d_simple = b.addSystemCommand(&.{
        if (platform.is_windows) "zig-out\\bin\\3d_screens_simple.exe" else "zig-out/bin/3d_screens_simple",
    });
    run_3d_simple.step.dependOn(&cmake_install.step);
    const run_3d_simple_step = b.step("run-3d-simple", "Run 3D screens simple demo");
    run_3d_simple_step.dependOn(&run_3d_simple.step);
}

// =============================================================================
// QuickJS Build
// =============================================================================
fn buildQuickJS(
    b: *std.Build,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    platform: PlatformInfo,
) *std.Build.Step.InstallArtifact {
    const quickjs = b.addStaticLibrary(.{
        .name = "quickjs",
        .target = target,
        .optimize = optimize,
    });

    const quickjs_src = "third_party/quickjs";
    const quickjs_compat = "third_party/quickjs_make/compat";

    quickjs.addCSourceFiles(.{
        .files = &.{
            quickjs_src ++ "/quickjs.c",
            quickjs_src ++ "/libregexp.c",
            quickjs_src ++ "/libunicode.c",
            quickjs_src ++ "/cutils.c",
            quickjs_src ++ "/dtoa.c",
        },
        .flags = &.{
            "-std=gnu11", // Use GNU C extensions for inline assembly support
            "-D_GNU_SOURCE",
            "-DCONFIG_VERSION=\"2024-02-14\"",
        },
    });

    quickjs.addIncludePath(b.path(quickjs_src));
    quickjs.linkLibC();

    if (platform.is_windows) {
        // Windows-specific settings
        quickjs.addIncludePath(b.path(quickjs_compat));
        quickjs.root_module.addCMacro("_CRT_SECURE_NO_WARNINGS", "");
        quickjs.root_module.addCMacro("_CRT_NONSTDC_NO_DEPRECATE", "");
        // Use EMSCRIPTEN=1 to disable CONFIG_ATOMICS (avoids pthread_cond dependency)
        quickjs.root_module.addCMacro("EMSCRIPTEN", "1");
        quickjs.root_module.addCMacro("CONFIG_STACK_CHECK", "");
    }

    quickjs.installHeader(b.path(quickjs_src ++ "/quickjs.h"), "quickjs.h");
    quickjs.installHeader(b.path(quickjs_src ++ "/quickjs-libc.h"), "quickjs-libc.h");

    return b.addInstallArtifact(quickjs, .{});
}

// =============================================================================
// Lexbor Build
// =============================================================================
fn buildLexbor(
    b: *std.Build,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    platform: PlatformInfo,
) *std.Build.Step.InstallArtifact {
    const lexbor = b.addStaticLibrary(.{
        .name = "lexbor_static",
        .target = target,
        .optimize = optimize,
    });

    const lexbor_src = "third_party/lexbor/source";

    // Collect all lexbor source files
    var sources = std.ArrayList([]const u8).init(b.allocator);

    // Core modules to include
    const modules = [_][]const u8{
        "lexbor/core",
        "lexbor/css",
        "lexbor/css/at_rule",
        "lexbor/css/property",
        "lexbor/css/selectors",
        "lexbor/css/syntax",
        "lexbor/css/syntax/tokenizer",
        "lexbor/dom",
        "lexbor/dom/interfaces",
        "lexbor/encoding",
        "lexbor/engine",
        "lexbor/html",
        "lexbor/html/interfaces",
        "lexbor/ns",
        "lexbor/punycode",
        "lexbor/selectors",
        "lexbor/tag",
        "lexbor/unicode",
        "lexbor/url",
        "lexbor/utils",
    };

    for (modules) |mod| {
        const mod_path = b.fmt("{s}/{s}", .{ lexbor_src, mod });
        var dir = std.fs.cwd().openDir(mod_path, .{ .iterate = true }) catch continue;
        defer dir.close();

        var iter = dir.iterate();
        while (iter.next() catch null) |entry| {
            if (entry.kind == .file and std.mem.endsWith(u8, entry.name, ".c")) {
                const full_path = b.fmt("{s}/{s}", .{ mod_path, entry.name });
                sources.append(b.allocator.dupe(u8, full_path) catch unreachable) catch unreachable;
            }
        }
    }

    // Add platform-specific port files
    if (platform.is_windows) {
        const win_port = b.fmt("{s}/lexbor/ports/windows_nt/lexbor/core", .{lexbor_src});
        var dir = std.fs.cwd().openDir(win_port, .{ .iterate = true }) catch |err| blk: {
            std.debug.print("Warning: Could not open Windows port dir: {}\n", .{err});
            break :blk null;
        };
        if (dir) |*d| {
            defer d.close();
            var iter = d.iterate();
            while (iter.next() catch null) |entry| {
                if (entry.kind == .file and std.mem.endsWith(u8, entry.name, ".c")) {
                    const full_path = b.fmt("{s}/{s}", .{ win_port, entry.name });
                    sources.append(b.allocator.dupe(u8, full_path) catch unreachable) catch unreachable;
                }
            }
        }
    } else {
        const posix_port = b.fmt("{s}/lexbor/ports/posix/lexbor/core", .{lexbor_src});
        var dir = std.fs.cwd().openDir(posix_port, .{ .iterate = true }) catch |err| blk: {
            std.debug.print("Warning: Could not open POSIX port dir: {}\n", .{err});
            break :blk null;
        };
        if (dir) |*d| {
            defer d.close();
            var iter = d.iterate();
            while (iter.next() catch null) |entry| {
                if (entry.kind == .file and std.mem.endsWith(u8, entry.name, ".c")) {
                    const full_path = b.fmt("{s}/{s}", .{ posix_port, entry.name });
                    sources.append(b.allocator.dupe(u8, full_path) catch unreachable) catch unreachable;
                }
            }
        }
    }

    // Convert to slice for addCSourceFiles
    const source_slice = sources.toOwnedSlice() catch unreachable;
    lexbor.addCSourceFiles(.{
        .files = source_slice,
        .flags = &.{"-std=c11"},
    });

    lexbor.addIncludePath(b.path(lexbor_src));
    lexbor.root_module.addCMacro("LEXBOR_STATIC", "");
    lexbor.linkLibC();

    return b.addInstallArtifact(lexbor, .{});
}

// =============================================================================
// Yoga Build
// =============================================================================
fn buildYoga(
    b: *std.Build,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
) *std.Build.Step.InstallArtifact {
    const yoga = b.addStaticLibrary(.{
        .name = "yogacore",
        .target = target,
        .optimize = optimize,
    });

    const yoga_src = "third_party/yoga";

    yoga.addCSourceFiles(.{
        .files = &.{
            yoga_src ++ "/yoga/YGConfig.cpp",
            yoga_src ++ "/yoga/YGEnums.cpp",
            yoga_src ++ "/yoga/YGNode.cpp",
            yoga_src ++ "/yoga/YGNodeLayout.cpp",
            yoga_src ++ "/yoga/YGNodeStyle.cpp",
            yoga_src ++ "/yoga/YGPixelGrid.cpp",
            yoga_src ++ "/yoga/YGValue.cpp",
            yoga_src ++ "/yoga/algorithm/AbsoluteLayout.cpp",
            yoga_src ++ "/yoga/algorithm/Baseline.cpp",
            yoga_src ++ "/yoga/algorithm/Cache.cpp",
            yoga_src ++ "/yoga/algorithm/CalculateLayout.cpp",
            yoga_src ++ "/yoga/algorithm/FlexLine.cpp",
            yoga_src ++ "/yoga/algorithm/PixelGrid.cpp",
            yoga_src ++ "/yoga/config/Config.cpp",
            yoga_src ++ "/yoga/debug/AssertFatal.cpp",
            yoga_src ++ "/yoga/debug/Log.cpp",
            yoga_src ++ "/yoga/event/event.cpp",
            yoga_src ++ "/yoga/node/LayoutResults.cpp",
            yoga_src ++ "/yoga/node/Node.cpp",
        },
        .flags = &.{
            "-std=c++20",
            "-fno-exceptions",
            "-fno-rtti",
        },
    });

    yoga.addIncludePath(b.path(yoga_src));
    yoga.linkLibCpp();

    return b.addInstallArtifact(yoga, .{});
}

// =============================================================================
// FreeType Build
// =============================================================================
fn buildFreeType(
    b: *std.Build,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    platform: PlatformInfo,
) *std.Build.Step.InstallArtifact {
    const freetype = b.addStaticLibrary(.{
        .name = "freetype",
        .target = target,
        .optimize = optimize,
    });

    const ft_src = "third_party/freetype/src";
    const ft_include = "third_party/freetype/include";

    // FreeType uses a single-file-per-module compilation approach
    var sources = std.ArrayList([]const u8).init(b.allocator);

    // Base module files
    sources.appendSlice(&.{
        ft_src ++ "/base/ftbase.c",
        ft_src ++ "/base/ftbbox.c",
        ft_src ++ "/base/ftbdf.c",
        ft_src ++ "/base/ftbitmap.c",
        ft_src ++ "/base/ftcid.c",
        ft_src ++ "/base/ftfstype.c",
        ft_src ++ "/base/ftgasp.c",
        ft_src ++ "/base/ftglyph.c",
        ft_src ++ "/base/ftgxval.c",
        ft_src ++ "/base/ftinit.c",
        ft_src ++ "/base/ftmm.c",
        ft_src ++ "/base/ftotval.c",
        ft_src ++ "/base/ftpatent.c",
        ft_src ++ "/base/ftpfr.c",
        ft_src ++ "/base/ftstroke.c",
        ft_src ++ "/base/ftsynth.c",
        ft_src ++ "/base/fttype1.c",
        ft_src ++ "/base/ftwinfnt.c",
    }) catch unreachable;

    // Platform-specific system/debug files
    if (platform.is_windows) {
        sources.appendSlice(&.{
            "third_party/freetype/builds/windows/ftsystem.c",
            "third_party/freetype/builds/windows/ftdebug.c",
        }) catch unreachable;
    } else {
        sources.appendSlice(&.{
            ft_src ++ "/base/ftsystem.c",
            ft_src ++ "/base/ftdebug.c",
        }) catch unreachable;
    }

    // Font format modules
    sources.appendSlice(&.{
        ft_src ++ "/truetype/truetype.c",
        ft_src ++ "/type1/type1.c",
        ft_src ++ "/cff/cff.c",
        ft_src ++ "/cid/type1cid.c",
        ft_src ++ "/pfr/pfr.c",
        ft_src ++ "/type42/type42.c",
        ft_src ++ "/winfonts/winfnt.c",
        ft_src ++ "/pcf/pcf.c",
        ft_src ++ "/bdf/bdf.c",
        ft_src ++ "/sfnt/sfnt.c",
    }) catch unreachable;

    // Rasterizer modules
    sources.appendSlice(&.{
        ft_src ++ "/smooth/smooth.c",
        ft_src ++ "/raster/raster.c",
        ft_src ++ "/sdf/sdf.c",
        ft_src ++ "/svg/svg.c",
    }) catch unreachable;

    // Helper modules
    sources.appendSlice(&.{
        ft_src ++ "/autofit/autofit.c",
        ft_src ++ "/pshinter/pshinter.c",
        ft_src ++ "/psaux/psaux.c",
        ft_src ++ "/psnames/psnames.c",
        ft_src ++ "/gzip/ftgzip.c",
        ft_src ++ "/lzw/ftlzw.c",
    }) catch unreachable;

    const source_slice = sources.toOwnedSlice() catch unreachable;
    freetype.addCSourceFiles(.{
        .files = source_slice,
        .flags = &.{
            "-DFT2_BUILD_LIBRARY",
        },
    });

    freetype.addIncludePath(b.path(ft_include));
    freetype.linkLibC();

    return b.addInstallArtifact(freetype, .{});
}

// =============================================================================
// HarfBuzz Build
// =============================================================================
fn buildHarfBuzz(
    b: *std.Build,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    platform: PlatformInfo,
    freetype_artifact: *std.Build.Step.InstallArtifact,
) *std.Build.Step.InstallArtifact {
    _ = platform;

    const harfbuzz = b.addStaticLibrary(.{
        .name = "harfbuzz",
        .target = target,
        .optimize = optimize,
    });

    const hb_src = "third_party/harfbuzz/src";

    // HarfBuzz provides an amalgamation file for easier building
    harfbuzz.addCSourceFiles(.{
        .files = &.{
            hb_src ++ "/harfbuzz.cc",
        },
        .flags = &.{
            "-std=c++17",
            "-DHAVE_FREETYPE",
            "-DHB_NO_MT", // Disable multi-threading for simpler build
        },
    });

    harfbuzz.addIncludePath(b.path(hb_src));
    harfbuzz.addIncludePath(b.path("third_party/freetype/include"));
    harfbuzz.linkLibCpp();

    // Depend on FreeType being built
    harfbuzz.step.dependOn(&freetype_artifact.step);

    return b.addInstallArtifact(harfbuzz, .{});
}

// =============================================================================
// msdfgen Build
// =============================================================================
fn buildMsdfgen(
    b: *std.Build,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    platform: PlatformInfo,
    freetype_artifact: *std.Build.Step.InstallArtifact,
) *std.Build.Step.InstallArtifact {
    _ = platform;

    const msdfgen = b.addStaticLibrary(.{
        .name = "msdfgen",
        .target = target,
        .optimize = optimize,
    });

    const msdf_root = "third_party/msdfgen";

    // Core sources
    msdfgen.addCSourceFiles(.{
        .files = &.{
            msdf_root ++ "/core/Contour.cpp",
            msdf_root ++ "/core/DistanceMapping.cpp",
            msdf_root ++ "/core/EdgeHolder.cpp",
            msdf_root ++ "/core/MSDFErrorCorrection.cpp",
            msdf_root ++ "/core/Projection.cpp",
            msdf_root ++ "/core/Scanline.cpp",
            msdf_root ++ "/core/Shape.cpp",
            msdf_root ++ "/core/contour-combiners.cpp",
            msdf_root ++ "/core/convergent-curve-ordering.cpp",
            msdf_root ++ "/core/edge-coloring.cpp",
            msdf_root ++ "/core/edge-segments.cpp",
            msdf_root ++ "/core/edge-selectors.cpp",
            msdf_root ++ "/core/equation-solver.cpp",
            msdf_root ++ "/core/msdf-error-correction.cpp",
            msdf_root ++ "/core/msdfgen.cpp",
            msdf_root ++ "/core/rasterization.cpp",
            msdf_root ++ "/core/render-sdf.cpp",
            msdf_root ++ "/core/sdf-error-estimation.cpp",
            msdf_root ++ "/core/shape-description.cpp",
            // Extension sources (font loading)
            msdf_root ++ "/ext/import-font.cpp",
            msdf_root ++ "/ext/resolve-shape-geometry.cpp",
        },
        .flags = &.{
            "-std=c++11",
            "-DMSDFGEN_USE_CPP11",
            "-DMSDFGEN_DISABLE_SVG",
            "-DMSDFGEN_DISABLE_PNG",
            "-DMSDFGEN_PUBLIC=",
            "-DMSDFGEN_EXT_PUBLIC=",
        },
    });

    msdfgen.addIncludePath(b.path(msdf_root));
    msdfgen.addIncludePath(b.path("third_party/freetype/include"));
    msdfgen.linkLibCpp();

    // Depend on FreeType being built
    msdfgen.step.dependOn(&freetype_artifact.step);

    return b.addInstallArtifact(msdfgen, .{});
}

// =============================================================================
// Dong Core Build (Pure Zig)
// =============================================================================
fn buildDongCore(
    b: *std.Build,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    platform: PlatformInfo,
    quickjs_artifact: *std.Build.Step.InstallArtifact,
    lexbor_artifact: *std.Build.Step.InstallArtifact,
    yoga_artifact: *std.Build.Step.InstallArtifact,
    freetype_artifact: *std.Build.Step.InstallArtifact,
    harfbuzz_artifact: *std.Build.Step.InstallArtifact,
    msdfgen_artifact: *std.Build.Step.InstallArtifact,
) *std.Build.Step.InstallArtifact {
    const dong_core = b.addStaticLibrary(.{
        .name = "dong_core",
        .target = target,
        .optimize = optimize,
    });

    // Core C++ sources
    dong_core.addCSourceFiles(.{
        .files = &.{
            // Core
            "src/api_bindings.cpp",
            "src/core/context.cpp",
            "src/core/engine_view.cpp",
            "src/core/profiler.cpp",
            "src/core/global_shared.cpp",
            "src/core/global_shared_c_api.cpp",
            "src/core/glyph_atlas_c_api.cpp",
            // DOM
            "src/dom/dom_manager.cpp",
            "src/dom/event_system.cpp",
            "src/dom/focus_manager.cpp",
            "src/dom/input_element.cpp",
            "src/dom/select_element.cpp",
            "src/dom/css/css_value.cpp",
            "src/dom/css/css_parser.cpp",
            "src/dom/css/computed_style.cpp",
            "src/dom/css/selector_matcher.cpp",
            "src/dom/css/style_engine.cpp",
            "src/dom/dom/dom_node.cpp",
            "src/dom/dom/observer.cpp",
            "src/dom/html/html_parser.cpp",
            // Animation
            "src/animation/animation_controller.cpp",
            // Layout
            "src/layout/layout_engine.cpp",
            "src/layout/aspect_ratio_resolver.cpp",
            "src/layout/display_contents.cpp",
            "src/layout/sticky_positioning.cpp",
            // Script
            "src/script/script_engine.cpp",
            "src/script/js_bindings.cpp",
            "src/script/js_node_bindings.cpp",
            "src/script/js_observer_bindings.cpp",
            // Render (platform-agnostic)
            "src/render/painter.cpp",
            "src/render/painter/painter_media.cpp",
            "src/render/painter/painter_text.cpp",
            "src/render/painter/painter_children.cpp",
            "src/render/render_surface.cpp",
            "src/render/resource_manager.cpp",
            "src/render/font_resolver.cpp",
            "src/render/font_finder.cpp",
            "src/render/text_shaper.cpp",
            "src/render/font_metrics.cpp",
            "src/render/glyph_atlas.cpp",
        },
        .flags = &.{
            "-std=c++20",
            "-DDONG_BUILDING_DLL",
        },
    });

    // Core C sources
    dong_core.addCSourceFiles(.{
        .files = &.{
            "src/core/platform.c",
        },
        .flags = &.{"-std=c11"},
    });

    // Include directories
    dong_core.addIncludePath(b.path("include"));
    dong_core.addIncludePath(b.path("src"));
    dong_core.addIncludePath(b.path("third_party/quickjs"));
    dong_core.addIncludePath(b.path("third_party/lexbor/source"));
    dong_core.addIncludePath(b.path("third_party/yoga"));
    dong_core.addIncludePath(b.path("third_party/freetype/include"));
    dong_core.addIncludePath(b.path("third_party/harfbuzz/src"));
    dong_core.addIncludePath(b.path("third_party")); // For msdfgen/msdfgen.h

    dong_core.linkLibCpp();
    dong_core.linkLibC();

    // Platform-specific settings
    if (platform.is_windows) {
        dong_core.root_module.addCMacro("_CRT_SECURE_NO_WARNINGS", "");
        dong_core.root_module.addCMacro("NOMINMAX", "");
        dong_core.linkSystemLibrary("user32");
        dong_core.linkSystemLibrary("gdi32");
        dong_core.linkSystemLibrary("ole32");
        dong_core.linkSystemLibrary("shell32");
        dong_core.linkSystemLibrary("advapi32");
    }

    // Dependencies
    dong_core.step.dependOn(&quickjs_artifact.step);
    dong_core.step.dependOn(&lexbor_artifact.step);
    dong_core.step.dependOn(&yoga_artifact.step);
    dong_core.step.dependOn(&freetype_artifact.step);
    dong_core.step.dependOn(&harfbuzz_artifact.step);
    dong_core.step.dependOn(&msdfgen_artifact.step);

    return b.addInstallArtifact(dong_core, .{});
}

// =============================================================================
// SDL Backend Build (Pure Zig)
// =============================================================================
fn buildSDLBackend(
    b: *std.Build,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    platform: PlatformInfo,
    config: BuildConfig,
    dong_core_artifact: *std.Build.Step.InstallArtifact,
) *std.Build.Step.InstallArtifact {
    const sdl_backend = b.addStaticLibrary(.{
        .name = "dong_sdl_backend",
        .target = target,
        .optimize = optimize,
    });

    // SDL backend sources
    sdl_backend.addCSourceFiles(.{
        .files = &.{
            "backends/sdl/sdl_gpu_driver.cpp",
            "backends/sdl/sdl_gpu_driver_init.cpp",
            "backends/sdl/sdl_gpu_driver_execute.cpp",
            "backends/sdl/sdl_gpu_driver_resources.cpp",
            "backends/sdl/sdl_gpu_driver_fonts.cpp",
            "backends/sdl/sdl_gpu_device.cpp",
            "backends/sdl/sdl_gpu_surface.cpp",
            "backends/sdl/sdl_gpu_painter.cpp",
            "backends/sdl/sdl_shader_manager.cpp",
            "backends/sdl/sdl_window.cpp",
            "backends/sdl/sdl_input_adapter.cpp",
            "backends/sdl/dong_sdl_plugin.cpp",
            "backends/sdl/dong_sdl_gpu_bridge.cpp",
            "backends/sdl/gpu_texture_compressor.cpp",
        },
        .flags = &.{
            "-std=c++20",
            "-DDONG_SDL_BUILDING_DLL",
        },
    });

    // SDL_shadercross
    sdl_backend.addCSourceFiles(.{
        .files = &.{
            "third_party/SDL_shadercross/src/SDL_shadercross.c",
        },
        .flags = &.{
            "-DSDL_SHADERCROSS_DXC",
            "-DSDL_SHADERCROSS_SPIRVCROSS",
        },
    });

    // Include directories
    sdl_backend.addIncludePath(b.path("include"));
    sdl_backend.addIncludePath(b.path("src"));
    sdl_backend.addIncludePath(b.path("backends/sdl"));
    sdl_backend.addIncludePath(b.path("third_party/sdl/include"));
    sdl_backend.addIncludePath(b.path("third_party/SDL_shadercross/include"));
    sdl_backend.addIncludePath(b.path("third_party/quickjs"));
    sdl_backend.addIncludePath(b.path("third_party/lexbor/source"));
    sdl_backend.addIncludePath(b.path("third_party/yoga"));
    sdl_backend.addIncludePath(b.path("third_party/freetype/include"));
    sdl_backend.addIncludePath(b.path("third_party/harfbuzz/src"));
    sdl_backend.addIncludePath(b.path("third_party")); // For msdfgen/msdfgen.h

    // DXC include path (can be absolute or relative)
    if (std.fs.path.isAbsolute(config.dxc_include_path)) {
        sdl_backend.addIncludePath(.{ .cwd_relative = config.dxc_include_path });
    } else {
        sdl_backend.addIncludePath(b.path(config.dxc_include_path));
    }

    // Vulkan SDK include for spirv_cross_c.h
    if (config.vulkan_sdk_path) |vk_path| {
        if (platform.is_windows) {
            const vk_include = b.fmt("{s}/Include/spirv_cross", .{vk_path});
            sdl_backend.addIncludePath(.{ .cwd_relative = vk_include });
        } else {
            const vk_include = b.fmt("{s}/include/spirv_cross", .{vk_path});
            sdl_backend.addIncludePath(.{ .cwd_relative = vk_include });
        }
    }

    sdl_backend.linkLibCpp();
    sdl_backend.linkLibC();

    // Platform-specific settings
    if (platform.is_windows) {
        sdl_backend.root_module.addCMacro("_CRT_SECURE_NO_WARNINGS", "");
        sdl_backend.root_module.addCMacro("NOMINMAX", "");
        sdl_backend.linkSystemLibrary("user32");
        sdl_backend.linkSystemLibrary("gdi32");
    }

    // Shader directory define
    sdl_backend.root_module.addCMacro("DONG_SDL_SHADER_DIR", "\"backends/sdl/shaders\"");

    // Dependencies
    sdl_backend.step.dependOn(&dong_core_artifact.step);

    return b.addInstallArtifact(sdl_backend, .{});
}

// =============================================================================
// DXC Auto-download
// =============================================================================
fn ensureDxc(b: *std.Build, platform: PlatformInfo, config: *BuildConfig) ?*std.Build.Step {
    // DXC is only needed for Windows desktop builds with SDL backend
    if (!platform.is_windows or !platform.is_native) {
        return null;
    }

    // Check if DXC already exists
    const dxc_lib_path = config.dxc_lib_path orelse (DXC_DIR ++ "/lib/x64");
    const dxc_dll_path = DXC_DIR ++ "/bin/x64/dxcompiler.dll";

    // Check if DXC is already downloaded
    if (std.fs.cwd().access(dxc_dll_path, .{})) |_| {
        std.debug.print("DXC found at: {s}\n", .{DXC_DIR});
        config.dxc_lib_path = dxc_lib_path;
        return null;
    } else |_| {
        std.debug.print("DXC not found, will download...\n", .{});
    }

    // Create download step using PowerShell (Windows)
    const download_step = b.addSystemCommand(&.{
        "powershell",
        "-NoProfile",
        "-ExecutionPolicy", "Bypass",
        "-Command",
        b.fmt(
            \\$ErrorActionPreference = 'Stop'
            \\$version = '{s}'
            \\$date = '{s}'
            \\$outDir = '{s}'
            \\$zipFile = "$env:TEMP\dxc.zip"
            \\$url = "https://github.com/microsoft/DirectXShaderCompiler/releases/download/$version/dxc_$date.zip"
            \\
            \\Write-Host "Downloading DXC $version..."
            \\Write-Host "URL: $url"
            \\
            \\# Download
            \\[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
            \\Invoke-WebRequest -Uri $url -OutFile $zipFile -UseBasicParsing
            \\
            \\# Extract
            \\Write-Host "Extracting to $outDir..."
            \\if (Test-Path $outDir) {{ Remove-Item -Recurse -Force $outDir }}
            \\Expand-Archive -Path $zipFile -DestinationPath $outDir -Force
            \\
            \\# Cleanup
            \\Remove-Item $zipFile -Force
            \\
            \\Write-Host "DXC $version installed to $outDir"
        , .{ DXC_VERSION, DXC_DATE, DXC_DIR }),
    });

    config.dxc_lib_path = dxc_lib_path;

    return &download_step.step;
}

