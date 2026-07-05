const std = @import("std");
const builtin = @import("builtin");

// Zig 0.16 migrated File System APIs to std.Io.Dir; alias for brevity.
const Io = std.Io;
const IoDir = std.Io.Dir;

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

fn loadBuildConfig(allocator: std.mem.Allocator, io: Io) BuildConfig {
    var config = BuildConfig{};

    const contents = IoDir.cwd().readFileAlloc(io, "build.env", allocator, .unlimited) catch {
        std.debug.print("Note: build.env not found, using defaults. Copy build.env.example to build.env and configure paths.\n", .{});
        return config;
    };
    defer allocator.free(contents);

    var line_iter = std.mem.splitScalar(u8, contents, '\n');
    while (line_iter.next()) |raw_line| {
        const trimmed_line = if (raw_line.len > 0 and raw_line[raw_line.len - 1] == '\r')
            raw_line[0 .. raw_line.len - 1]
        else
            raw_line;

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
        IoDir.cwd().access(io, "third_party/ffmpeg/configure", .{}) catch {
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

fn createRootModule(
    b: *std.Build,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
) *std.Build.Module {
    return b.createModule(.{
        .target = target,
        .optimize = optimize,
    });
}

// Zig 0.16 moved most attribute setters from Step.Compile to Build.Module.
// These tiny helpers preserve the previous call sites with minimal noise.
fn linkLibC(c: *std.Build.Step.Compile) void {
    c.root_module.link_libc = true;
}

fn linkLibCpp(c: *std.Build.Step.Compile) void {
    c.root_module.link_libcpp = true;
}

fn linkSystemLib(c: *std.Build.Step.Compile, name: []const u8) void {
    c.root_module.linkSystemLibrary(name, .{});
}

const Backend = enum { sdl, gpu, none };

fn backendToCMake(backend: Backend) []const u8 {
    return switch (backend) {
        .sdl => "sdl",
        .gpu => "gpu",
        .none => "none",
    };
}

const BuildOptions = struct {
    enable_ffmpeg: bool,
    backend: Backend,
    android_api_level: u32,
    ios_deployment_target: []const u8,
    libs_only: bool, // Only build static libraries (for mobile)
    optimize_size: bool, // Optimize for binary size (LTO, /O1, no RTTI, etc.)
    package_only: bool, // Skip examples/apps (Zig package dependency mode)
    script_engine: []const u8, // porffor (default) or quickjs
};

fn resolveBackend(b: *std.Build, platform: PlatformInfo) Backend {
    if (b.option(Backend, "backend", "Graphics backend: sdl (default on desktop), gpu, or none")) |backend| {
        return backend;
    }
    if (b.option(bool, "sdl", "Deprecated: use -Dbackend=none instead of -Dsdl=false")) |enable_sdl| {
        if (!enable_sdl) {
            std.debug.print("Note: -Dsdl=false is deprecated; use -Dbackend=none\n", .{});
            return .none;
        }
        std.debug.print("Note: -Dsdl=true is deprecated; use -Dbackend=sdl\n", .{});
        return .sdl;
    }
    return if (platform.is_desktop) .sdl else .none;
}

fn getBuildOptions(b: *std.Build, platform: PlatformInfo) BuildOptions {
    const enable_ffmpeg = b.option(bool, "ffmpeg", "Enable FFmpeg support (default: true on desktop)") orelse platform.is_desktop;
    const backend = resolveBackend(b, platform);
    const android_api_level = b.option(u32, "android-api", "Android API level (default: 21)") orelse 21;
    const ios_target = b.option([]const u8, "ios-target", "iOS deployment target (default: 12.0)") orelse "12.0";
    const libs_only = b.option(bool, "libs-only", "Only build static libraries") orelse platform.is_mobile;
    const optimize_size = b.option(bool, "optimize-size", "Optimize for binary size (LTO, /O1, no RTTI, etc.)") orelse false;
    const package_only = b.option(bool, "package-only", "Skip examples/apps (for Zig package consumers)") orelse false;
    const script_engine = b.option([]const u8, "script-engine", "Script engine: porffor (default) or quickjs") orelse "porffor";

    return .{
        .enable_ffmpeg = enable_ffmpeg,
        .backend = backend,
        .android_api_level = android_api_level,
        .ios_deployment_target = ios_target,
        .libs_only = libs_only,
        .optimize_size = optimize_size,
        .package_only = package_only,
        .script_engine = script_engine,
    };
}

// =============================================================================
// Main Build Function
// =============================================================================
pub fn build(b: *std.Build) void {
    var target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    const install_prefix = b.install_prefix;
    const host_is_windows = builtin.os.tag == .windows;

    // On Windows we build the CMake parts with clang-cl (MSVC ABI). Ensure Zig-built
    // static libs (QuickJS/FreeType/etc.) use the same ABI to avoid link/runtime mismatches.
    if (target.result.os.tag == .windows and target.result.abi == .gnu) {
        target = b.resolveTargetQuery(.{
            .cpu_arch = target.result.cpu.arch,
            .os_tag = .windows,
            .abi = .msvc,
        });
    }

    const platform = getPlatformInfo(b, target);
    const options = getBuildOptions(b, platform);

    // Load build configuration
    const io = b.graph.io;
    var config = loadBuildConfig(b.allocator, io);

    // Print build info
    std.debug.print("Building for: {s} (native: {})\n", .{ platform.target_triple, platform.is_native });
    std.debug.print("  Backend: {s}\n", .{backendToCMake(options.backend)});
    std.debug.print("  Script engine: {s}\n", .{options.script_engine});
    if (platform.is_android) {
        std.debug.print("  Android API level: {}\n", .{options.android_api_level});
    }
    if (platform.is_ios) {
        std.debug.print("  iOS deployment target: {s}\n", .{options.ios_deployment_target});
    }

    // ==========================================================================
    // DXC Auto-download (Windows/Linux desktop)
    // ==========================================================================
    const dxc_step = if (options.backend == .sdl) ensureDxc(b, platform, &config) else null;

    // Build type for CMake (SDL3 fallback)
    const cmake_build_type = if (platform.is_windows) "Release" else switch (optimize) {
        .Debug => "Debug",
        .ReleaseSafe, .ReleaseFast, .ReleaseSmall => "Release",
    };

    // Zig-built third-party static libraries must match the ABI used by the CMake build.
    // Also, on Windows, default Debug builds enable UBSan in C compilation, which then
    // requires extra runtime linkage. We build these deps in ReleaseFast to keep linking
    // simple and consistent with the CMake Release build.
    const deps_optimize: std.builtin.OptimizeMode = if (options.optimize_size)
        .ReleaseSmall
    else if (platform.is_windows)
        .ReleaseFast
    else
        optimize;

    // ==========================================================================
    // QuickJS (Pure Zig Build) — optional when -Dscript-engine=quickjs
    // ==========================================================================
    const quickjs = if (std.mem.eql(u8, options.script_engine, "quickjs"))
        buildQuickJS(b, target, deps_optimize, platform)
    else
        null;

    // ==========================================================================
    // Lexbor (Pure Zig Build)
    // ==========================================================================
    const lexbor = buildLexbor(b, target, deps_optimize, platform);

    // ==========================================================================
    // Yoga (Pure Zig Build)
    // ==========================================================================
    const yoga = buildYoga(b, target, deps_optimize);

    // ==========================================================================
    // FreeType (Pure Zig Build)
    // ==========================================================================
    const freetype = buildFreeType(b, target, deps_optimize, platform);

    // ==========================================================================
    // HarfBuzz (Pure Zig Build)
    // ==========================================================================
    const harfbuzz = buildHarfBuzz(b, target, deps_optimize, platform, freetype);

    // ==========================================================================
    // msdfgen (Pure Zig Build)
    // ==========================================================================
    const msdfgen = buildMsdfgen(b, target, deps_optimize, platform, freetype);

    // ==========================================================================
    // SDL3 via CMake (shared) - only when backend=sdl
    // ==========================================================================
    const sdl3_build_dir = "third_party/sdl/build-zig";
    const sdl3_prefix = "zig-out/sdl3";

    const sdl3_cmake_install = if (options.backend == .sdl) blk: {
        var sdl3_cmake_args = std.array_list.Managed([]const u8).init(b.allocator);
        sdl3_cmake_args.appendSlice(&.{
            "cmake",                                              "-S",             "third_party/sdl",  "-B",              sdl3_build_dir,
            b.fmt("-DCMAKE_BUILD_TYPE={s}", .{cmake_build_type}), "-DSDL_TEST=OFF", "-DSDL_STATIC=OFF", "-DSDL_SHARED=ON", "-DSDL_TESTS=OFF",
            "-DSDL_EXAMPLES=OFF",
        }) catch unreachable;

        if (platform.is_windows) {
            sdl3_cmake_args.appendSlice(&.{
                "-G",                                            "Ninja",
                "-DCMAKE_C_COMPILER=clang-cl",                   "-DCMAKE_CXX_COMPILER=clang-cl",
                "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL",
            }) catch unreachable;
        } else if (platform.is_linux) {
            sdl3_cmake_args.appendSlice(&.{
                "-G",                          "Ninja",
                "-DSDL_UNIX_CONSOLE_BUILD=ON",
                "-DCMAKE_BUILD_WITH_INSTALL_RPATH=ON",
            }) catch unreachable;
        }

        const sdl3_cmake_config = b.addSystemCommand(sdl3_cmake_args.items);
        const sdl3_cmake_build = b.addSystemCommand(&.{ "cmake", "--build", sdl3_build_dir, "--config", cmake_build_type });
        sdl3_cmake_build.step.dependOn(&sdl3_cmake_config.step);
        const install = b.addSystemCommand(&.{ "cmake", "--install", sdl3_build_dir, "--prefix", sdl3_prefix });
        install.step.dependOn(&sdl3_cmake_build.step);
        break :blk install;
    } else null;

    const core_only = options.libs_only or options.backend == .none or options.package_only;

    // ==========================================================================
    // Mobile/Cross-compile / core-only: libs-only mode
    // ==========================================================================
    if (core_only) {
        // Build Dong Core for mobile
        const dong_core = buildDongCore(b, target, optimize, platform, quickjs, lexbor, yoga, freetype, harfbuzz, msdfgen);

        // For mobile/cross-compile, just install the static libraries
        const libs_only_step = b.step("libs", "Build only static libraries (for mobile integration)");
        if (quickjs) |qj| libs_only_step.dependOn(&qj.step);
        libs_only_step.dependOn(&lexbor.step);
        libs_only_step.dependOn(&yoga.step);
        libs_only_step.dependOn(&freetype.step);
        libs_only_step.dependOn(harfbuzz);
        libs_only_step.dependOn(msdfgen);

        libs_only_step.dependOn(&dong_core.step);
        b.getInstallStep().dependOn(libs_only_step);

        // Still provide individual build steps for libs-only mode
        if (quickjs) |qj| {
            const quickjs_step = b.step("quickjs", "Build QuickJS only");
            quickjs_step.dependOn(&qj.step);
        }

        const lexbor_step = b.step("lexbor", "Build Lexbor only");
        lexbor_step.dependOn(&lexbor.step);

        const yoga_step = b.step("yoga", "Build Yoga only");
        yoga_step.dependOn(&yoga.step);

        const freetype_step = b.step("freetype", "Build FreeType only");
        freetype_step.dependOn(&freetype.step);

        const harfbuzz_step = b.step("harfbuzz", "Build HarfBuzz only");
        harfbuzz_step.dependOn(harfbuzz);

        const msdfgen_step = b.step("msdfgen", "Build msdfgen only");
        msdfgen_step.dependOn(msdfgen);

        const dong_core_step = b.step("dong-core", "Build Dong Core only");
        dong_core_step.dependOn(&dong_core.step);

        const deps_step = b.step("deps", "Build all third-party dependencies");
        deps_step.dependOn(libs_only_step);

        // Print info about cross-compilation
        std.debug.print("\n=== Cross-compilation mode ===\n", .{});
        std.debug.print("Target: {s}\n", .{platform.target_triple});
        std.debug.print("Building static libraries only.\n", .{});
        std.debug.print("Output: zig-out/lib/\n\n", .{});

        // ==========================================================================
        // libs-only Packaging (Cross-compile / Mobile integration)
        // ==========================================================================
        // In libs-only mode we package only:
        // - include/ (headers)
        // - lib/     (static/import libs)
        //
        // Note: this respects -p/--prefix because we copy from b.install_prefix.
        const libs_dist_dir = b.fmt("{s}/dist", .{install_prefix});
        const libs_package_name = b.fmt("dong_sdk_{s}", .{platform.target_triple});
        const libs_package_dir = b.fmt("{s}/{s}", .{ libs_dist_dir, libs_package_name });

        const package_libs = b.addSystemCommand(if (host_is_windows) &.{
            "powershell",
            "-NoProfile",
            "-ExecutionPolicy",
            "Bypass",
            "-Command",
            b.fmt(
                "& {{ " ++
                    "$ErrorActionPreference = 'Stop'; " ++
                    "$pkg = '{s}'; " ++
                    "if (Test-Path $pkg) {{ Remove-Item -Recurse -Force $pkg }}; " ++
                    "New-Item -ItemType Directory -Force -Path $pkg | Out-Null; " ++
                    "Copy-Item -Recurse -Force '{s}\\include' (Join-Path $pkg 'include'); " ++
                    "Copy-Item -Recurse -Force '{s}\\lib' (Join-Path $pkg 'lib'); " ++
                    "Write-Host ('Packaged to: ' + $pkg); " ++
                "}}",
                .{ libs_package_dir, install_prefix, install_prefix },
            ),
        } else &.{
            "sh",
            "-c",
            b.fmt(
                "set -euo pipefail; " ++
                    "pkg='{s}'; " ++
                    "rm -rf \"$pkg\"; mkdir -p \"$pkg\"; " ++
                    "cp -R '{s}/include' \"$pkg/include\"; " ++
                    "cp -R '{s}/lib' \"$pkg/lib\"; " ++
                    "echo \"Packaged to: $pkg\"",
                .{ libs_package_dir, install_prefix, install_prefix },
            ),
        });
        package_libs.step.dependOn(libs_only_step);

        const package_step = b.step("package", "Package SDK (libs-only) to <prefix>/dist/");
        package_step.dependOn(&package_libs.step);

        const package_zip_cmd = b.addSystemCommand(if (host_is_windows) &.{
            "powershell",
            "-NoProfile",
            "-ExecutionPolicy",
            "Bypass",
            "-Command",
            b.fmt(
                "& {{ " ++
                    "$ErrorActionPreference = 'Stop'; " ++
                    "$pkg = '{s}'; " ++
                    "$zip = '{s}/{s}.zip'; " ++
                    "if (Test-Path $zip) {{ Remove-Item -Force $zip }}; " ++
                    "Compress-Archive -Path $pkg -DestinationPath $zip -Force; " ++
                    "Write-Host ('Zip created: ' + $zip); " ++
                "}}",
                .{ libs_package_dir, libs_dist_dir, libs_package_name },
            ),
        } else &.{
            "sh",
            "-c",
            b.fmt(
                "set -euo pipefail; " ++
                    "pkg='{s}'; dist='{s}'; name='{s}'; " ++
                    "mkdir -p \"$dist\"; " ++
                    "rm -f \"$dist/$name.zip\"; " ++
                    "cd \"$dist\"; zip -r \"$name.zip\" \"$name\"",
                .{ libs_package_dir, libs_dist_dir, libs_package_name },
            ),
        });
        package_zip_cmd.step.dependOn(&package_libs.step);

        const package_zip_step = b.step("package-zip", "Package SDK (libs-only) to zip under <prefix>/dist/");
        package_zip_step.dependOn(&package_zip_cmd.step);

        exportDongPackage(b, target, optimize, options.backend, dong_core);

        return; // Skip CMake build for core-only targets
    }

    // ==========================================================================
    // Dong core + examples via CMake (for now - will migrate later)
    // ==========================================================================
    const cmake_build_dir = b.fmt("build-cmake-{s}", .{backendToCMake(options.backend)});

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

    var dong_cmake_args = std.array_list.Managed([]const u8).init(b.allocator);
    dong_cmake_args.appendSlice(&.{
        "cmake",                                              "-S",                 ".",               "-B", cmake_build_dir,
        b.fmt("-DCMAKE_BUILD_TYPE={s}", .{cmake_build_type}), vulkan_sdk_cmake_arg, dxc_lib_cmake_arg,
    }) catch unreachable;

    if (platform.is_windows) {
        dong_cmake_args.appendSlice(&.{
            "-G",                                            "Ninja",
            "-DCMAKE_C_COMPILER=clang-cl",                   "-DCMAKE_CXX_COMPILER=clang-cl",
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

    // Pass binary size optimization flag
    dong_cmake_args.append(
        if (options.optimize_size) "-DDONG_OPTIMIZE_SIZE=ON" else "-DDONG_OPTIMIZE_SIZE=OFF",
    ) catch unreachable;

    dong_cmake_args.append(
        b.fmt("-DDONG_BACKEND={s}", .{backendToCMake(options.backend)}),
    ) catch unreachable;

    dong_cmake_args.append(
        b.fmt("-DDONG_SCRIPT_ENGINE={s}", .{options.script_engine}),
    ) catch unreachable;

    const porffor_deps = b.addSystemCommand(if (host_is_windows) &.{
        "powershell",
        "-NoProfile",
        "-ExecutionPolicy",
        "Bypass",
        "-Command",
        "if (-not (Test-Path third_party/porffor/node_modules/acorn)) { Push-Location third_party/porffor; npm install; Pop-Location }",
    } else &.{
        "sh",
        "-c",
        "[ -d third_party/porffor/node_modules/acorn ] || (cd third_party/porffor && npm install)",
    });
    porffor_deps.setCwd(b.path("."));

    const porffor_compile = b.addSystemCommand(&.{ "node", "scripts/porffor_compile.mjs" });
    porffor_compile.setCwd(b.path("."));
    porffor_compile.step.dependOn(&porffor_deps.step);

    const cmake_config = b.addSystemCommand(dong_cmake_args.items);
    if (quickjs) |qj| {
        cmake_config.step.dependOn(&qj.step);
    }
    if (std.mem.eql(u8, options.script_engine, "porffor")) {
        cmake_config.step.dependOn(&porffor_compile.step);
    }
    cmake_config.step.dependOn(&lexbor.step);
    cmake_config.step.dependOn(&yoga.step);
    cmake_config.step.dependOn(&freetype.step);
    cmake_config.step.dependOn(harfbuzz);
    cmake_config.step.dependOn(msdfgen);

    if (sdl3_cmake_install) |sdl_install| {
        cmake_config.step.dependOn(&sdl_install.step);
    }
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

    if (platform.is_windows and options.backend == .sdl) {
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
    // React Examples: copy pre-built bundles to zig-out/bin/data/
    // ==========================================================================
    const react_copy = b.addSystemCommand(if (host_is_windows) &.{
        "powershell",
        "-NoProfile",
        "-ExecutionPolicy",
        "Bypass",
        "-Command",
        b.fmt(
            "& {{ $ErrorActionPreference = 'Stop'; " ++
                "$examples = @('counter', 'game-ui', 'todo-classic'); " ++
                "foreach ($ex in $examples) {{ " ++
                "  $src_b = 'react/dist/' + $ex + '/bundle.js'; " ++
                "  $src_h = 'react/examples/' + $ex + '/index.html'; " ++
                "  if ((Test-Path $src_b) -and (Test-Path $src_h)) {{ " ++
                "    $dst = '{s}/bin/data/react-' + $ex; " ++
                "    New-Item -ItemType Directory -Force -Path $dst | Out-Null; " ++
                "    Copy-Item -Force $src_b (Join-Path $dst 'bundle.js'); " ++
                "    Copy-Item -Force $src_h (Join-Path $dst 'index.html'); " ++
                "    Write-Host ('  Installed react-' + $ex); " ++
                "  }} " ++
                "}} }}",
            .{install_prefix},
        ),
    } else &.{
        "sh",
        "-c",
        b.fmt(
            "for ex in counter game-ui todo-classic; do " ++
                "  src_b=\"react/dist/$ex/bundle.js\"; " ++
                "  src_h=\"react/examples/$ex/index.html\"; " ++
                "  if [ -f \"$src_b\" ] && [ -f \"$src_h\" ]; then " ++
                "    dst='{s}/bin/data/react-$ex'; " ++
                "    mkdir -p \"$dst\"; " ++
                "    cp \"$src_b\" \"$dst/bundle.js\"; " ++
                "    cp \"$src_h\" \"$dst/index.html\"; " ++
                "    echo \"  Installed react-$ex\"; " ++
                "  fi; " ++
                "done",
            .{install_prefix},
        ),
    });
    react_copy.step.dependOn(&cmake_install.step);
    b.getInstallStep().dependOn(&react_copy.step);

    // "zig build react" — build React bundles from source, then copy to output
    const react_build_and_install = b.addSystemCommand(if (host_is_windows) &.{
        "powershell",
        "-NoProfile",
        "-ExecutionPolicy",
        "Bypass",
        "-Command",
        b.fmt(
            "& {{ $ErrorActionPreference = 'Stop'; " ++
                "Push-Location react; " ++
                "if (-not (Test-Path node_modules)) {{ npm install }}; " ++
                "node build.mjs counter; " ++
                "node build.mjs game-ui; " ++
                "node build.mjs todo-classic; " ++
                "Pop-Location; " ++
                "$examples = @('counter', 'game-ui', 'todo-classic'); " ++
                "foreach ($ex in $examples) {{ " ++
                "  $dst = '{s}/bin/data/react-' + $ex; " ++
                "  New-Item -ItemType Directory -Force -Path $dst | Out-Null; " ++
                "  Copy-Item -Force ('react/dist/' + $ex + '/bundle.js') (Join-Path $dst 'bundle.js'); " ++
                "  Copy-Item -Force ('react/examples/' + $ex + '/index.html') (Join-Path $dst 'index.html'); " ++
                "  Write-Host ('  Built & installed react-' + $ex); " ++
                "}} }}",
            .{install_prefix},
        ),
    } else &.{
        "sh",
        "-c",
        b.fmt(
            "cd react && " ++
                "[ -d node_modules ] || npm install && " ++
                "node build.mjs counter && " ++
                "node build.mjs game-ui && " ++
                "node build.mjs todo-classic && " ++
                "cd .. && " ++
                "for ex in counter game-ui todo-classic; do " ++
                "  dst='{s}/bin/data/react-$ex'; " ++
                "  mkdir -p \"$dst\"; " ++
                "  cp \"react/dist/$ex/bundle.js\" \"$dst/bundle.js\"; " ++
                "  cp \"react/examples/$ex/index.html\" \"$dst/index.html\"; " ++
                "  echo \"  Built & installed react-$ex\"; " ++
                "done",
            .{install_prefix},
        ),
    });
    const react_step = b.step("react", "Build & install React examples (requires Node.js)");
    react_step.dependOn(&react_build_and_install.step);

    // ==========================================================================
    // Preact Examples: copy pre-built bundles to zig-out/bin/data/
    // ==========================================================================
    const preact_copy = b.addSystemCommand(if (host_is_windows) &.{
        "powershell",
        "-NoProfile",
        "-ExecutionPolicy",
        "Bypass",
        "-Command",
        b.fmt(
            "& {{ $ErrorActionPreference = 'Stop'; " ++
                "$examples = @('counter', 'game-ui', 'todo-classic', 'ui-components', 'discovery-buttons', 'discovery-cards', 'discovery-inbox', 'diagnostic-inline-js', 'diagnostic-minimal-module'); " ++
                "foreach ($ex in $examples) {{ " ++
                "  $src_b = 'preact/dist/' + $ex + '/bundle.js'; " ++
                "  $src_h = 'preact/examples/' + $ex + '/index.html'; " ++
                "  if ((Test-Path $src_b) -and (Test-Path $src_h)) {{ " ++
                "    $dst = '{s}/bin/data/preact-' + $ex; " ++
                "    New-Item -ItemType Directory -Force -Path $dst | Out-Null; " ++
                "    Copy-Item -Force $src_b (Join-Path $dst 'bundle.js'); " ++
                "    Copy-Item -Force $src_h (Join-Path $dst 'index.html'); " ++
                "    Write-Host ('  Installed preact-' + $ex); " ++
                "  }} " ++
                "}} }}",
            .{install_prefix},
        ),
    } else &.{
        "sh",
        "-c",
        b.fmt(
            "for ex in counter game-ui todo-classic ui-components discovery-buttons discovery-cards discovery-inbox diagnostic-inline-js diagnostic-minimal-module; do " ++
                "  src_b=\"preact/dist/$ex/bundle.js\"; " ++
                "  src_h=\"preact/examples/$ex/index.html\"; " ++
                "  if [ -f \"$src_b\" ] && [ -f \"$src_h\" ]; then " ++
                "    dst='{s}/bin/data/preact-$ex'; " ++
                "    mkdir -p \"$dst\"; " ++
                "    cp \"$src_b\" \"$dst/bundle.js\"; " ++
                "    cp \"$src_h\" \"$dst/index.html\"; " ++
                "    echo \"  Installed preact-$ex\"; " ++
                "  fi; " ++
                "done",
            .{install_prefix},
        ),
    });
    preact_copy.step.dependOn(&cmake_install.step);
    b.getInstallStep().dependOn(&preact_copy.step);

    // "zig build preact" — build Preact bundles from source, then copy to output
    const preact_build_and_install = b.addSystemCommand(if (host_is_windows) &.{
        "powershell",
        "-NoProfile",
        "-ExecutionPolicy",
        "Bypass",
        "-Command",
        b.fmt(
            "& {{ $ErrorActionPreference = 'Stop'; " ++
                "Push-Location preact; " ++
                "if (-not (Test-Path node_modules)) {{ npm install }}; " ++
                "node build.mjs counter; " ++
                "node build.mjs game-ui; " ++
                "node build.mjs todo-classic; " ++
                "node build.mjs ui-components; " ++
                "node build.mjs discovery-buttons; " ++
                "node build.mjs discovery-cards; " ++
                "node build.mjs discovery-inbox; " ++
                "node build.mjs diagnostic-inline-js; " ++
                "node build.mjs diagnostic-minimal-module; " ++
                "Pop-Location; " ++
                "$examples = @('counter', 'game-ui', 'todo-classic', 'ui-components', 'discovery-buttons', 'discovery-cards', 'discovery-inbox', 'diagnostic-inline-js', 'diagnostic-minimal-module'); " ++
                "foreach ($ex in $examples) {{ " ++
                "  $dst = '{s}/bin/data/preact-' + $ex; " ++
                "  New-Item -ItemType Directory -Force -Path $dst | Out-Null; " ++
                "  Copy-Item -Force ('preact/dist/' + $ex + '/bundle.js') (Join-Path $dst 'bundle.js'); " ++
                "  Copy-Item -Force ('preact/examples/' + $ex + '/index.html') (Join-Path $dst 'index.html'); " ++
                "  Write-Host ('  Built & installed preact-' + $ex); " ++
                "}} }}",
            .{install_prefix},
        ),
    } else &.{
        "sh",
        "-c",
        b.fmt(
            "cd preact && " ++
                "[ -d node_modules ] || npm install && " ++
                "node build.mjs counter && " ++
                "node build.mjs game-ui && " ++
                "node build.mjs todo-classic && " ++
                "node build.mjs ui-components && " ++
                "node build.mjs discovery-buttons && " ++
                "node build.mjs discovery-cards && " ++
                "node build.mjs discovery-inbox && " ++
                "cd .. && " ++
                "for ex in counter game-ui todo-classic ui-components discovery-buttons discovery-cards discovery-inbox diagnostic-inline-js diagnostic-minimal-module; do " ++
                "  dst='{s}/bin/data/preact-$ex'; " ++
                "  mkdir -p \"$dst\"; " ++
                "  cp \"preact/dist/$ex/bundle.js\" \"$dst/bundle.js\"; " ++
                "  cp \"preact/examples/$ex/index.html\" \"$dst/index.html\"; " ++
                "  echo \"  Built & installed preact-$ex\"; " ++
                "done",
            .{install_prefix},
        ),
    });
    const preact_step = b.step("preact", "Build & install Preact examples (requires Node.js)");
    preact_step.dependOn(&preact_build_and_install.step);

    // ==========================================================================
    // Build Steps
    // ==========================================================================
    const examples_step = b.step("examples", "Build all examples and install to zig-out/bin");
    examples_step.dependOn(&cmake_install.step);

    // ==========================================================================
    // SDK + Examples Packaging
    // ==========================================================================
    // Produces a redistributable folder under zig-out/dist/ that contains:
    // - include/ (C headers)
    // - lib/     (import/static libs)
    // - bin/     (examples + runtime DLLs + data/)
    //
    // Note: keep this intentionally simple and PowerShell-based on Windows.
    const dist_dir = b.fmt("{s}/dist", .{install_prefix});
    const package_name = b.fmt("dong_sdk_examples_{s}", .{platform.target_triple});
    const package_dir = b.fmt("{s}/{s}", .{ dist_dir, package_name });

    const package_sdk = b.addSystemCommand(if (host_is_windows) &.{
        "powershell",
        "-NoProfile",
        "-ExecutionPolicy",
        "Bypass",
        "-Command",
        b.fmt(
            "& {{ " ++
                "$ErrorActionPreference = 'Stop'; " ++
                "$dist = '{s}'; " ++
                "$pkg = '{s}'; " ++
                "if (Test-Path $pkg) {{ Remove-Item -Recurse -Force $pkg }}; " ++
                "New-Item -ItemType Directory -Force -Path $pkg | Out-Null; " ++
                "Copy-Item -Recurse -Force '{s}\\include' (Join-Path $pkg 'include'); " ++
                "Copy-Item -Recurse -Force '{s}\\lib' (Join-Path $pkg 'lib'); " ++
                "$bin = '{s}\\bin'; if (Test-Path $bin) {{ Copy-Item -Recurse -Force $bin (Join-Path $pkg 'bin') }}; " ++
                "Write-Host ('Packaged to: ' + $pkg); " ++
            "}}",
            .{ dist_dir, package_dir, install_prefix, install_prefix, install_prefix },
        ),
    } else &.{
        "sh",
        "-c",
        b.fmt(
            "set -euo pipefail; " ++
                "dist='{s}'; pkg='{s}'; " ++
                "rm -rf \"$pkg\"; mkdir -p \"$pkg\"; " ++
                "cp -R '{s}/include' \"$pkg/include\"; " ++
                "cp -R '{s}/lib' \"$pkg/lib\"; " ++
                "if [ -d '{s}/bin' ]; then cp -R '{s}/bin' \"$pkg/bin\"; fi; " ++
                "echo \"Packaged to: $pkg\"",
            .{ dist_dir, package_dir, install_prefix, install_prefix, install_prefix, install_prefix, install_prefix },
        ),
    });
    package_sdk.step.dependOn(examples_step);

    const package_step = b.step("package", "Package SDK+examples to zig-out/dist/");
    package_step.dependOn(&package_sdk.step);

    const package_zip_cmd = b.addSystemCommand(if (host_is_windows) &.{
        "powershell",
        "-NoProfile",
        "-ExecutionPolicy",
        "Bypass",
        "-Command",
        b.fmt(
            "& {{ " ++
                "$ErrorActionPreference = 'Stop'; " ++
                "$pkg = '{s}'; " ++
                "$zip = '{s}/{s}.zip'; " ++
                "if (Test-Path $zip) {{ Remove-Item -Force $zip }}; " ++
                "Compress-Archive -Path $pkg -DestinationPath $zip -Force; " ++
                "Write-Host ('Zip created: ' + $zip); " ++
            "}}",
            .{ package_dir, dist_dir, package_name },
        ),
    } else &.{
        "sh",
        "-c",
        b.fmt(
            "set -euo pipefail; " ++
                "pkg='{s}'; dist='{s}'; name='{s}'; " ++
                "mkdir -p \"$dist\"; " ++
                "rm -f \"$dist/$name.zip\"; " ++
                "cd \"$dist\"; " ++
                "zip -r \"$name.zip\" \"$name\"",
            .{ package_dir, dist_dir, package_name },
        ),
    });
    package_zip_cmd.step.dependOn(&package_sdk.step);

    const package_zip_step = b.step("package-zip", "Package SDK+examples to zip under zig-out/dist/");
    package_zip_step.dependOn(&package_zip_cmd.step);

    // Individual dependency build steps
    const deps_step = b.step("deps", "Build all third-party dependencies only");
    if (quickjs) |qj| deps_step.dependOn(&qj.step);
    deps_step.dependOn(&lexbor.step);
    deps_step.dependOn(&yoga.step);
    deps_step.dependOn(&freetype.step);
    deps_step.dependOn(harfbuzz);
    deps_step.dependOn(msdfgen);

    if (sdl3_cmake_install) |sdl_install| {
        deps_step.dependOn(&sdl_install.step);
    }

    if (quickjs) |qj| {
        const quickjs_step = b.step("quickjs", "Build QuickJS only");
        quickjs_step.dependOn(&qj.step);
    }

    const lexbor_step = b.step("lexbor", "Build Lexbor only");
    lexbor_step.dependOn(&lexbor.step);

    const yoga_step = b.step("yoga", "Build Yoga only");
    yoga_step.dependOn(&yoga.step);

    const freetype_step = b.step("freetype", "Build FreeType only");
    freetype_step.dependOn(&freetype.step);

    const harfbuzz_step = b.step("harfbuzz", "Build HarfBuzz only");
    harfbuzz_step.dependOn(harfbuzz);

    const msdfgen_step = b.step("msdfgen", "Build msdfgen only");
    msdfgen_step.dependOn(msdfgen);

    const sdl3_step = b.step("sdl3", "Build SDL3 only");
    if (sdl3_cmake_install) |sdl_install| {
        sdl3_step.dependOn(&sdl_install.step);
    }

    // ==========================================================================
    // Dong Core (Pure Zig Build)
    // ==========================================================================
    const dong_core = buildDongCore(b, target, optimize, platform, quickjs, lexbor, yoga, freetype, harfbuzz, msdfgen);
    const dong_core_step = b.step("dong-core", "Build Dong Core (pure Zig)");
    dong_core_step.dependOn(&dong_core.step);

    exportDongPackage(b, target, optimize, options.backend, dong_core);

    // ==========================================================================
    // SDL Backend (Pure Zig Build)
    // ==========================================================================
    if (options.backend == .sdl) {
        const sdl_backend = buildSDLBackend(b, target, optimize, platform, config, dong_core);
        const sdl_backend_step = b.step("sdl-backend", "Build SDL Backend (pure Zig)");
        sdl_backend_step.dependOn(&sdl_backend.step);
    }

    // ==========================================================================
    // GPU Backend (CMake builds third_party/gpu + dong_gpu_backend)
    // ==========================================================================
    if (options.backend == .gpu) {
        const gpu_backend_step = b.step("gpu-backend", "Build GPU backend (via CMake)");
        gpu_backend_step.dependOn(&cmake_build.step);
    }

    // ==========================================================================
    // Run shortcuts (SDL backend + examples only)
    // ==========================================================================
    if (options.backend == .sdl) {
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

    const run_porffor_tests = b.addSystemCommand(&.{ "node", "scripts/run-porffor-tests.mjs" });
    run_porffor_tests.setCwd(b.path("."));
    run_porffor_tests.step.dependOn(&cmake_install.step);
    const run_porffor_tests_step = b.step("run-porffor-tests", "Run Porffor-ready HTML tests (CI gate)");
    run_porffor_tests_step.dependOn(&run_porffor_tests.step);

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
}

// =============================================================================
// Zig Package exports (build.zig.zon consumers)
// =============================================================================
fn exportDongPackage(
    b: *std.Build,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    backend: Backend,
    dong_core: *std.Build.Step.InstallArtifact,
) void {
    _ = backend;
    _ = dong_core;

    const dong_mod = b.addModule("dong", .{
        .root_source_file = b.path("zig/dong.zig"),
        .target = target,
        .optimize = optimize,
    });
    dong_mod.addIncludePath(b.path("include"));
    dong_mod.link_libc = true;
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
    const quickjs = b.addLibrary(.{
        .name = "quickjs",
        .linkage = .static,
        .root_module = createRootModule(b, target, optimize),
    });

    const quickjs_src = "third_party/quickjs";
    const quickjs_compat = "third_party/quickjs_make/compat";

    var c_flags = std.array_list.Managed([]const u8).init(b.allocator);
    c_flags.appendSlice(&.{
        "-std=gnu11", // Use GNU C extensions for inline assembly support
        "-D_GNU_SOURCE",
    }) catch unreachable;

    if (platform.is_windows) {
        // Zig's MSVC-targeted clang invocation may pass flags that clang considers
        // unused during compilation; don't fail the build because of that.
        c_flags.appendSlice(&.{
            "-Wno-unused-command-line-argument",
            "-fms-extensions",
            // Ensure alloca() is available on Windows.
            "-include",
            quickjs_compat ++ "/alloca.h",
        }) catch unreachable;
    }

    const c_flags_slice = c_flags.toOwnedSlice() catch unreachable;

    quickjs.root_module.addCSourceFiles(.{
        .files = &.{
            quickjs_src ++ "/quickjs.c",
            quickjs_src ++ "/libregexp.c",
            quickjs_src ++ "/libunicode.c",
            quickjs_src ++ "/cutils.c",
            quickjs_src ++ "/dtoa.c",
        },
        .flags = c_flags_slice,
    });

    quickjs.root_module.addIncludePath(b.path(quickjs_src));
    linkLibC(quickjs);

    if (platform.is_windows) {
        // Windows-specific settings
        quickjs.root_module.addIncludePath(b.path(quickjs_compat));
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
    const lexbor = b.addLibrary(.{
        .name = "lexbor_static",
        .linkage = .static,
        .root_module = createRootModule(b, target, optimize),
    });

    const lexbor_src = "third_party/lexbor/source";

    // Collect all lexbor source files
    var sources = std.array_list.Managed([]const u8).init(b.allocator);

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

    const io = b.graph.io;

    for (modules) |mod| {
        const mod_path = b.fmt("{s}/{s}", .{ lexbor_src, mod });
        var dir = IoDir.cwd().openDir(io, mod_path, .{ .iterate = true }) catch continue;
        defer dir.close(io);

        var iter = dir.iterate();
        while (iter.next(io) catch null) |entry| {
            if (entry.kind == .file and std.mem.endsWith(u8, entry.name, ".c")) {
                const full_path = b.fmt("{s}/{s}", .{ mod_path, entry.name });
                sources.append(b.allocator.dupe(u8, full_path) catch unreachable) catch unreachable;
            }
        }
    }

    // Add platform-specific port files
    if (platform.is_windows) {
        const win_port = b.fmt("{s}/lexbor/ports/windows_nt/lexbor/core", .{lexbor_src});
        var dir = IoDir.cwd().openDir(io, win_port, .{ .iterate = true }) catch |err| blk: {
            std.debug.print("Warning: Could not open Windows port dir: {}\n", .{err});
            break :blk null;
        };
        if (dir) |*d| {
            defer d.close(io);
            var iter = d.iterate();
            while (iter.next(io) catch null) |entry| {
                if (entry.kind == .file and std.mem.endsWith(u8, entry.name, ".c")) {
                    const full_path = b.fmt("{s}/{s}", .{ win_port, entry.name });
                    sources.append(b.allocator.dupe(u8, full_path) catch unreachable) catch unreachable;
                }
            }
        }
    } else {
        const posix_port = b.fmt("{s}/lexbor/ports/posix/lexbor/core", .{lexbor_src});
        var dir = IoDir.cwd().openDir(io, posix_port, .{ .iterate = true }) catch |err| blk: {
            std.debug.print("Warning: Could not open POSIX port dir: {}\n", .{err});
            break :blk null;
        };
        if (dir) |*d| {
            defer d.close(io);
            var iter = d.iterate();
            while (iter.next(io) catch null) |entry| {
                if (entry.kind == .file and std.mem.endsWith(u8, entry.name, ".c")) {
                    const full_path = b.fmt("{s}/{s}", .{ posix_port, entry.name });
                    sources.append(b.allocator.dupe(u8, full_path) catch unreachable) catch unreachable;
                }
            }
        }
    }

    // Convert to slice for addCSourceFiles
    const source_slice = sources.toOwnedSlice() catch unreachable;
    lexbor.root_module.addCSourceFiles(.{
        .files = source_slice,
        .flags = &.{"-std=c11"},
    });

    lexbor.root_module.addIncludePath(b.path(lexbor_src));
    lexbor.root_module.addCMacro("LEXBOR_STATIC", "");
    linkLibC(lexbor);

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
    const yoga = b.addLibrary(.{
        .name = "yogacore",
        .linkage = .static,
        .root_module = createRootModule(b, target, optimize),
    });

    const yoga_src = "third_party/yoga";

    yoga.root_module.addCSourceFiles(.{
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

    yoga.root_module.addIncludePath(b.path(yoga_src));
    linkLibCpp(yoga);

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
    const freetype = b.addLibrary(.{
        .name = "freetype",
        .linkage = .static,
        .root_module = createRootModule(b, target, optimize),
    });

    const ft_src = "third_party/freetype/src";
    const ft_include = "third_party/freetype/include";

    // FreeType uses a single-file-per-module compilation approach
    var sources = std.array_list.Managed([]const u8).init(b.allocator);

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
    freetype.root_module.addCSourceFiles(.{
        .files = source_slice,
        .flags = &.{
            "-DFT2_BUILD_LIBRARY",
        },
    });

    freetype.root_module.addIncludePath(b.path(ft_include));
    linkLibC(freetype);

    // Install FreeType headers for CMake to find.
    // CMake expects: <prefix>/freetype/include/freetype2/ft2build.h
    // We copy them as part of the dependency step (not only "zig build install"),
    // because the CMake configure/build steps depend on these headers.
    const ft_dst = b.fmt("{s}/freetype/include/freetype2", .{b.install_prefix});
    const copy_headers = b.addSystemCommand(if (builtin.os.tag == .windows) &.{
        "powershell",
        "-NoProfile",
        "-ExecutionPolicy",
        "Bypass",
        "-Command",
        b.fmt(
            "& {{ " ++
                "$ErrorActionPreference = 'Stop'; " ++
                "$src = '{s}'; " ++
                "$dst = '{s}'; " ++
                "New-Item -ItemType Directory -Force -Path $dst | Out-Null; " ++
                "Copy-Item -Recurse -Force (Join-Path $src '*') $dst; " ++
            "}}",
            .{ ft_include, ft_dst },
        ),
    } else &.{
        "sh",
        "-c",
        b.fmt(
            "set -euo pipefail; src='{s}'; dst='{s}'; mkdir -p \"$dst\"; cp -R \"$src\"/* \"$dst\"",
            .{ ft_include, ft_dst },
        ),
    });

    const install_artifact = b.addInstallArtifact(freetype, .{});
    install_artifact.step.dependOn(&copy_headers.step);
    return install_artifact;
}

// =============================================================================
// HarfBuzz Build
// =============================================================================

fn fileExists(io: Io, rel_path: []const u8) bool {
    IoDir.cwd().access(io, rel_path, .{}) catch return false;
    return true;
}

fn buildHarfBuzz(
    b: *std.Build,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    platform: PlatformInfo,
    freetype_artifact: *std.Build.Step.InstallArtifact,
) *std.Build.Step {
    const hb_src = "third_party/harfbuzz/src";
    const hb_amalgamation = hb_src ++ "/harfbuzz.cc";

    // Some workspaces ship only prebuilt libs under zig-out/.
    // If sources are missing, fall back to the prebuilt archive and ensure it is
    // installed to zig-out/lib (CMake links from there).
    const io = b.graph.io;
    std.debug.print("Checking file: {s}\n", .{hb_amalgamation});
    const file_exists = fileExists(io, hb_amalgamation);
    std.debug.print("File exists: {}\n", .{file_exists});
    if (!file_exists) {
        const prebuilt = b.step("harfbuzz_prebuilt", "Use prebuilt harfbuzz from zig-out/");
        prebuilt.dependOn(&freetype_artifact.step);

        if (platform.is_windows) {
            const src_lib = if (fileExists(io, "zig-out/harfbuzz/lib/harfbuzz.lib"))
                "zig-out/harfbuzz/lib/harfbuzz.lib"
            else
                "zig-out/lib/harfbuzz.lib";

            if (!fileExists(io, src_lib)) {
                @panic("harfbuzz sources missing (third_party/harfbuzz) and no prebuilt harfbuzz.lib found under zig-out/harfbuzz/lib or zig-out/lib");
            }

            const install_hb = b.addInstallFileWithDir(b.path(src_lib), .lib, "harfbuzz.lib");
            prebuilt.dependOn(&install_hb.step);
            return prebuilt;
        }

        if (fileExists(io, "zig-out/harfbuzz/lib/libharfbuzz.a")) {
            const install_hb = b.addInstallFileWithDir(b.path("zig-out/harfbuzz/lib/libharfbuzz.a"), .lib, "libharfbuzz.a");
            prebuilt.dependOn(&install_hb.step);
            return prebuilt;
        }

        if (fileExists(io, "zig-out/lib/libharfbuzz.a")) {
            return prebuilt;
        }

        @panic("harfbuzz sources missing (third_party/harfbuzz) and no prebuilt libharfbuzz.a found under zig-out");
    }

    // On Windows native builds, build HarfBuzz via its CMake to avoid libc++ linkage surprises.
    if (platform.is_windows and platform.is_native and fileExists(io, "third_party/harfbuzz/CMakeLists.txt")) {
        const build_dir = "third_party/harfbuzz/build-zig";
        // Must be absolute for CMake to resolve from its own build dir
        const freetype_inc = b.fmt("{s}/zig-out/freetype/include/freetype2", .{b.build_root.path orelse "."});
        const freetype_lib = b.fmt("{s}/zig-out/lib/freetype.lib", .{b.build_root.path orelse "."});

        var cfg_args = std.array_list.Managed([]const u8).init(b.allocator);
        cfg_args.appendSlice(&.{
            "cmake",
            "-S",
            "third_party/harfbuzz",
            "-B",
            build_dir,
            "-G",
            "Ninja",
            "-DCMAKE_BUILD_TYPE=Release",
            "-DCMAKE_C_COMPILER=clang-cl",
            "-DCMAKE_CXX_COMPILER=clang-cl",
            "-DHB_HAVE_FREETYPE=ON",
            "-DHB_BUILD_UTILS=OFF",
            "-DHB_BUILD_SUBSET=OFF",
            "-DHB_HAVE_GLIB=OFF",
            "-DHB_HAVE_ICU=OFF",
            "-DHB_HAVE_GRAPHITE2=OFF",
            "-DHB_HAVE_CAIRO=OFF",
            b.fmt("-DFREETYPE_INCLUDE_DIRS={s}", .{freetype_inc}),
            b.fmt("-DFREETYPE_LIBRARY={s}", .{freetype_lib}),
        }) catch unreachable;

        const cmake_cfg = b.addSystemCommand(cfg_args.items);
        cmake_cfg.step.dependOn(&freetype_artifact.step);

        const cmake_build = b.addSystemCommand(&.{ "cmake", "--build", build_dir, "--config", "Release", "--target", "harfbuzz" });
        cmake_build.step.dependOn(&cmake_cfg.step);

        const copy_lib = b.addSystemCommand(&.{
            "powershell",
            "-NoProfile",
            "-ExecutionPolicy",
            "Bypass",
            "-Command",
            "& { $ErrorActionPreference = 'Stop'; " ++
                "cmake -E copy_if_different 'third_party/harfbuzz/build-zig/harfbuzz.lib' 'zig-out/lib/harfbuzz.lib' | Out-Null }",
        });
        copy_lib.step.dependOn(&cmake_build.step);

        const hb_step = b.step("harfbuzz_build", "Build HarfBuzz (CMake, internal)");
        hb_step.dependOn(&copy_lib.step);
        hb_step.dependOn(&freetype_artifact.step);
        return hb_step;
    }

    const harfbuzz = b.addLibrary(.{
        .name = "harfbuzz",
        .linkage = .static,
        .root_module = createRootModule(b, target, optimize),
    });

    // HarfBuzz provides an amalgamation file for easier building
    harfbuzz.root_module.addCSourceFiles(.{
        .files = &.{
            hb_amalgamation,
        },
        .flags = &.{
            "-std=c++17",
            "-DHAVE_FREETYPE",
            "-DHB_NO_MT", // Disable multi-threading for simpler build
        },
    });

    harfbuzz.root_module.addIncludePath(b.path(hb_src));
    harfbuzz.root_module.addIncludePath(b.path("third_party/freetype/include"));
    harfbuzz.root_module.addSystemIncludePath(b.path("third_party/freetype/include"));
    linkLibCpp(harfbuzz);

    // Depend on FreeType being built
    harfbuzz.step.dependOn(&freetype_artifact.step);

    // Install HarfBuzz headers for CMake to find
    // CMake expects: <prefix>/harfbuzz/include/harfbuzz/hb-ft.h etc.
    const hb_dst = b.fmt("{s}/harfbuzz/include/harfbuzz", .{b.install_prefix});
    const copy_headers = b.addSystemCommand(if (builtin.os.tag == .windows) &.{
        "powershell",
        "-NoProfile",
        "-ExecutionPolicy",
        "Bypass",
        "-Command",
        b.fmt(
            "& {{ " ++
                "$ErrorActionPreference = 'Stop'; " ++
                "$src = '{s}'; " ++
                "$dst = '{s}'; " ++
                "New-Item -ItemType Directory -Force -Path $dst | Out-Null; " ++
                "Copy-Item -Recurse -Force (Join-Path $src '*') $dst; " ++
            "}}",
            .{ hb_src, hb_dst },
        ),
    } else &.{
        "sh",
        "-c",
        b.fmt(
            "set -euo pipefail; src='{s}'; dst='{s}'; mkdir -p \"$dst\"; cp -R \"$src\"/* \"$dst\"",
            .{ hb_src, hb_dst },
        ),
    });

    const install_artifact = b.addInstallArtifact(harfbuzz, .{});
    install_artifact.step.dependOn(&copy_headers.step);
    return &install_artifact.step;
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
) *std.Build.Step {
    const msdf_root = "third_party/msdfgen";

    const msdf_probe = msdf_root ++ "/core/Contour.cpp";
    const io = b.graph.io;

    // Some workspaces ship only prebuilt libs under zig-out/.
    if (!fileExists(io, msdf_probe)) {
        const prebuilt = b.step("msdfgen_prebuilt", "Use prebuilt msdfgen from zig-out/");
        prebuilt.dependOn(&freetype_artifact.step);

        if (platform.is_windows) {
            const core_src = "zig-out/msdfgen/lib/msdfgen-core.lib";
            const ext_src = "zig-out/msdfgen/lib/msdfgen-ext.lib";
            if (!fileExists(io, core_src) or !fileExists(io, ext_src)) {
                @panic("msdfgen sources missing (third_party/msdfgen) and prebuilt msdfgen-core/msdfgen-ext not found under zig-out/msdfgen/lib");
            }

            const install_core = b.addInstallFileWithDir(b.path(core_src), .lib, "msdfgen-core.lib");
            const install_ext = b.addInstallFileWithDir(b.path(ext_src), .lib, "msdfgen-ext.lib");
            prebuilt.dependOn(&install_core.step);
            prebuilt.dependOn(&install_ext.step);
            return prebuilt;
        }

        if (fileExists(io, "zig-out/lib/libmsdfgen.a")) {
            // Non-windows toolchains typically link libmsdfgen.a.
            return prebuilt;
        }

        @panic("msdfgen sources missing (third_party/msdfgen) and no usable prebuilt archive found under zig-out");
    }

    const common_flags = &.{
        "-std=c++11",
        "-DMSDFGEN_USE_CPP11",
        "-DMSDFGEN_DISABLE_SVG",
        "-DMSDFGEN_DISABLE_PNG",
        "-DMSDFGEN_PUBLIC=",
        "-DMSDFGEN_EXT_PUBLIC=",
    };

    // CMake expects split libs on Windows: msdfgen-core.lib + msdfgen-ext.lib.
    // Build with msdfgen's CMake on Windows to avoid libc++ linkage surprises.
    if (platform.is_windows and platform.is_native) {
        const build_dir = "third_party/msdfgen/build-zig";
        const prefix = "zig-out/msdfgen";
        // Must be absolute for CMake to resolve from its own build dir
        const freetype_inc = b.fmt("{s}/zig-out/freetype/include/freetype2", .{b.build_root.path orelse "."});
        const freetype_lib = b.fmt("{s}/zig-out/lib/freetype.lib", .{b.build_root.path orelse "."});

        var cfg_args = std.array_list.Managed([]const u8).init(b.allocator);
        cfg_args.appendSlice(&.{
            "cmake",
            "-S",
            "third_party/msdfgen",
            "-B",
            build_dir,
            "-G",
            "Ninja",
            "-DCMAKE_BUILD_TYPE=Release",
            "-DCMAKE_CXX_COMPILER=clang-cl",
            "-DMSDFGEN_INSTALL=ON",
            "-DMSDFGEN_USE_VCPKG=OFF",
            "-DMSDFGEN_BUILD_STANDALONE=OFF",
            "-DMSDFGEN_USE_SKIA=OFF",
            "-DMSDFGEN_USE_OPENMP=OFF",
            "-DMSDFGEN_DISABLE_SVG=ON",
            "-DMSDFGEN_DISABLE_PNG=ON",
            "-DMSDFGEN_DYNAMIC_RUNTIME=ON",
            b.fmt("-DFREETYPE_INCLUDE_DIRS={s}", .{freetype_inc}),
            b.fmt("-DFREETYPE_LIBRARY={s}", .{freetype_lib}),
        }) catch unreachable;

        const cmake_cfg = b.addSystemCommand(cfg_args.items);
        cmake_cfg.step.dependOn(&freetype_artifact.step);

        const cmake_build = b.addSystemCommand(&.{ "cmake", "--build", build_dir, "--config", "Release" });
        cmake_build.step.dependOn(&cmake_cfg.step);

        const cmake_install = b.addSystemCommand(&.{ "cmake", "--install", build_dir, "--prefix", prefix });
        cmake_install.step.dependOn(&cmake_build.step);

        const copy_libs = b.addSystemCommand(&.{
            "powershell",
            "-NoProfile",
            "-ExecutionPolicy",
            "Bypass",
            "-Command",
            "& { $ErrorActionPreference = 'Stop'; " ++
                "cmake -E copy_if_different 'zig-out/msdfgen/lib/msdfgen-core.lib' 'zig-out/lib/msdfgen-core.lib' | Out-Null; " ++
                "cmake -E copy_if_different 'zig-out/msdfgen/lib/msdfgen-ext.lib'  'zig-out/lib/msdfgen-ext.lib'  | Out-Null }",
        });
        copy_libs.step.dependOn(&cmake_install.step);

        const step = b.step("msdfgen_build", "Build msdfgen (CMake, internal)");
        step.dependOn(&copy_libs.step);
        return step;
    }

    // Non-windows toolchains typically link a single archive libmsdfgen.a.
    const msdfgen = b.addLibrary(.{
        .name = "msdfgen",
        .linkage = .static,
        .root_module = createRootModule(b, target, optimize),
    });
    msdfgen.root_module.addCSourceFiles(.{
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
            msdf_root ++ "/ext/import-font.cpp",
            msdf_root ++ "/ext/resolve-shape-geometry.cpp",
        },
        .flags = common_flags,
    });
    msdfgen.root_module.addIncludePath(b.path(msdf_root));
    msdfgen.root_module.addIncludePath(b.path("third_party/freetype/include"));
    linkLibCpp(msdfgen);
    msdfgen.step.dependOn(&freetype_artifact.step);

    return &b.addInstallArtifact(msdfgen, .{}).step;
}

// =============================================================================
// Dong Core Build (Pure Zig)
// =============================================================================
fn buildDongCore(
    b: *std.Build,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    platform: PlatformInfo,
    quickjs_artifact: ?*std.Build.Step.InstallArtifact,
    lexbor_artifact: *std.Build.Step.InstallArtifact,
    yoga_artifact: *std.Build.Step.InstallArtifact,
    freetype_artifact: *std.Build.Step.InstallArtifact,
    harfbuzz_step: *std.Build.Step,
    msdfgen_step: *std.Build.Step,
) *std.Build.Step.InstallArtifact {
    const dong_core = b.addLibrary(.{
        .name = "dong_core",
        .linkage = .static,
        .root_module = createRootModule(b, target, optimize),
    });

    // Core C++ sources
    dong_core.root_module.addCSourceFiles(.{
        .files = &.{
            // Core
            "src/api_bindings.cpp",
            "src/core/context.cpp",
            "src/core/engine_view.cpp",
            "src/core/profiler.cpp",
            "src/core/global_shared.cpp",
            "src/core/global_shared_c_api.cpp",
            "src/core/glyph_atlas_c_api.cpp",
            // Input
            "src/input/spatial_nav.cpp",
            // DOM
            "src/dom/dom_manager.cpp",
            "src/dom/event_system.cpp",
            "src/dom/focus_manager.cpp",
            "src/dom/drag_manager.cpp",
            "src/dom/input_element.cpp",
            "src/dom/select_element.cpp",
            "src/dom/details_element.cpp",
            "src/dom/dialog_element.cpp",
            "src/dom/range.cpp",
            "src/dom/selection.cpp",
            "src/dom/text_hit_testing.cpp",
            "src/dom/contenteditable.cpp",
            "src/dom/editing_commands.cpp",
            "src/dom/css/css_value.cpp",
            "src/dom/css/css_parser.cpp",
            "src/dom/css/css_property_handlers.cpp",
            "src/dom/css/css_value_parser.cpp",
            "src/dom/css/css_shorthand_parser.cpp",
            "src/dom/css/css_function_parser.cpp",
            "src/dom/css/css_at_rule_parser.cpp",
            "src/dom/css/computed_style.cpp",
            "src/dom/css/selector_matcher.cpp",
            "src/dom/css/style_engine.cpp",
            "src/dom/css/style_engine_apply.cpp",
            "src/dom/css/style_engine_cascade.cpp",
            "src/dom/css/style_engine_inheritance.cpp",
            "src/dom/css/style_engine_pseudo.cpp",
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
            "src/layout/table_layout.cpp",
            "src/layout/grid_layout.cpp",
            // Script
            "src/script/script_engine.cpp",
            "src/script/js_bindings.cpp",
            "src/script/js_node_bindings.cpp",
            "src/script/js_observer_bindings.cpp",
            "src/script/js_clipboard_bindings.cpp",
            "src/script/js_selection_bindings.cpp",
            "src/script/js_fetch_bindings.cpp",
            "src/script/js_text_layout_bindings.cpp",
            "src/script/js_scene_bindings.cpp",
            "src/dom/scene_compiler.cpp",
            // Render (platform-agnostic)
            "src/render/scene_graph.cpp",
            "src/render/painter.cpp",
            "src/render/painter_select_overlays.cpp",
            "src/render/painter/painter_media.cpp",
            "src/render/painter/painter_text.cpp",
            "src/render/painter/painter_children.cpp",
            "src/render/painter/painter_backdrop.cpp",
            "src/render/painter/painter_caret.cpp",

            "src/core/resource_loader.cpp",
            "src/render/render_surface.cpp",
            "src/render/resource_manager.cpp",
            "src/render/font_resolver.cpp",
            "src/render/font_finder.cpp",
            "src/render/text_shaper.cpp",
            "src/render/font_metrics.cpp",
            "src/render/glyph_atlas.cpp",
            "src/render/list_marker.cpp",
            // Text renderer selection
            "src/render/text_layout_core.cpp",
            "src/render/text_renderer_selector.cpp",
            // Slug runtime
            "src/render/slug/slug_outline_loader.cpp",
            "src/render/slug/slug_curve_encoder.cpp",
            "src/render/slug/slug_band_builder.cpp",
            "src/render/slug/slug_font_cache.cpp",
            "src/render/drawlist_emitter.cpp",
        },
        .flags = &.{
            "-std=c++20",
            "-DDONG_BUILDING_DLL",
        },
    });

    // Core C sources
    dong_core.root_module.addCSourceFiles(.{
        .files = &.{
            "src/core/platform.c",
        },
        .flags = &.{"-std=c11"},
    });

    // Include directories
    const io = b.graph.io;
    dong_core.root_module.addIncludePath(b.path("include"));
    dong_core.root_module.addIncludePath(b.path("src"));
    dong_core.root_module.addIncludePath(b.path("third_party/quickjs"));
    dong_core.root_module.addIncludePath(b.path("third_party/lexbor/source"));
    dong_core.root_module.addIncludePath(b.path("third_party/yoga"));
    dong_core.root_module.addIncludePath(b.path("third_party/freetype/include"));
    if (fileExists(io, "third_party/harfbuzz/src/harfbuzz.cc")) {
        dong_core.root_module.addIncludePath(b.path("third_party/harfbuzz/src"));
    } else {
        dong_core.root_module.addIncludePath(b.path("zig-out/harfbuzz/include/harfbuzz"));
        dong_core.root_module.addIncludePath(b.path("zig-out/harfbuzz/include"));
    }

    if (fileExists(io, "third_party/msdfgen/core/Contour.cpp")) {
        dong_core.root_module.addIncludePath(b.path("third_party")); // For msdfgen/msdfgen.h
    } else {
        dong_core.root_module.addIncludePath(b.path("zig-out/msdfgen/include"));
    }

    linkLibCpp(dong_core);
    linkLibC(dong_core);

    // Platform-specific settings
    if (platform.is_windows) {
        dong_core.root_module.addCMacro("_CRT_SECURE_NO_WARNINGS", "");
        dong_core.root_module.addCMacro("NOMINMAX", "");
        linkSystemLib(dong_core, "user32");
        linkSystemLib(dong_core, "gdi32");
        linkSystemLib(dong_core, "ole32");
        linkSystemLib(dong_core, "shell32");
        linkSystemLib(dong_core, "advapi32");
        linkSystemLib(dong_core, "winhttp");
    }

    // Dependencies
    if (quickjs_artifact) |qj| {
        dong_core.step.dependOn(&qj.step);
    }
    dong_core.step.dependOn(&lexbor_artifact.step);
    dong_core.step.dependOn(&yoga_artifact.step);
    dong_core.step.dependOn(&freetype_artifact.step);
    dong_core.step.dependOn(harfbuzz_step);
    dong_core.step.dependOn(msdfgen_step);

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
    const sdl_backend = b.addLibrary(.{
        .name = "dong_sdl_backend",
        .linkage = .static,
        .root_module = createRootModule(b, target, optimize),
    });

    // SDL backend sources
    sdl_backend.root_module.addCSourceFiles(.{
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
        },
        .flags = &.{
            "-std=c++20",
            "-DDONG_SDL_BUILDING_DLL",
        },
    });

    // SDL_shadercross
    sdl_backend.root_module.addCSourceFiles(.{
        .files = &.{
            "third_party/SDL_shadercross/src/SDL_shadercross.c",
        },
        .flags = &.{
            "-DSDL_SHADERCROSS_DXC",
            "-DSDL_SHADERCROSS_SPIRVCROSS",
        },
    });

    // Include directories
    sdl_backend.root_module.addIncludePath(b.path("include"));
    sdl_backend.root_module.addIncludePath(b.path("src"));
    sdl_backend.root_module.addIncludePath(b.path("backends/sdl"));
    sdl_backend.root_module.addIncludePath(b.path("third_party/sdl/include"));
    sdl_backend.root_module.addIncludePath(b.path("third_party/SDL_shadercross/include"));
    sdl_backend.root_module.addIncludePath(b.path("third_party/quickjs"));
    sdl_backend.root_module.addIncludePath(b.path("third_party/lexbor/source"));
    sdl_backend.root_module.addIncludePath(b.path("third_party/yoga"));
    sdl_backend.root_module.addIncludePath(b.path("third_party/freetype/include"));
    const io = b.graph.io;
    if (fileExists(io, "third_party/harfbuzz/src/harfbuzz.cc")) {
        sdl_backend.root_module.addIncludePath(b.path("third_party/harfbuzz/src"));
    } else {
        sdl_backend.root_module.addIncludePath(b.path("zig-out/harfbuzz/include"));
    }

    if (fileExists(io, "third_party/msdfgen/core/Contour.cpp")) {
        sdl_backend.root_module.addIncludePath(b.path("third_party")); // For msdfgen/msdfgen.h
    } else {
        sdl_backend.root_module.addIncludePath(b.path("zig-out/msdfgen/include"));
    }

    // DXC include path (can be absolute or relative)
    if (std.fs.path.isAbsolute(config.dxc_include_path)) {
        sdl_backend.root_module.addIncludePath(.{ .cwd_relative = config.dxc_include_path });
    } else {
        sdl_backend.root_module.addIncludePath(b.path(config.dxc_include_path));
    }

    // Vulkan SDK include for spirv_cross_c.h
    if (config.vulkan_sdk_path) |vk_path| {
        if (platform.is_windows) {
            const vk_include = b.fmt("{s}/Include/spirv_cross", .{vk_path});
            sdl_backend.root_module.addIncludePath(.{ .cwd_relative = vk_include });
        } else {
            const vk_include = b.fmt("{s}/include/spirv_cross", .{vk_path});
            sdl_backend.root_module.addIncludePath(.{ .cwd_relative = vk_include });
        }
    }

    linkLibCpp(sdl_backend);
    linkLibC(sdl_backend);

    // Platform-specific settings
    if (platform.is_windows) {
        sdl_backend.root_module.addCMacro("_CRT_SECURE_NO_WARNINGS", "");
        sdl_backend.root_module.addCMacro("NOMINMAX", "");
        linkSystemLib(sdl_backend, "user32");
        linkSystemLib(sdl_backend, "gdi32");
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
    // DXC is needed for Windows/Linux desktop builds with SDL backend
    // macOS uses Metal shaders directly, doesn't need DXC
    const needs_dxc = (platform.is_windows or platform.is_linux) and platform.is_native;
    if (!needs_dxc) {
        return null;
    }

    // Platform-specific paths
    const dxc_lib_path = if (platform.is_windows)
        config.dxc_lib_path orelse (DXC_DIR ++ "/lib/x64")
    else
        config.dxc_lib_path orelse (DXC_DIR ++ "/lib");
    
    const dxc_check_path = if (platform.is_windows)
        DXC_DIR ++ "/bin/x64/dxcompiler.dll"
    else
        DXC_DIR ++ "/lib/libdxcompiler.so";

    // Check if DXC is already downloaded
    const io = b.graph.io;
    if (IoDir.cwd().access(io, dxc_check_path, .{})) |_| {
        std.debug.print("DXC found at: {s}\n", .{DXC_DIR});
        config.dxc_lib_path = dxc_lib_path;
        return null;
    } else |_| {
        std.debug.print("DXC not found, will download...\n", .{});
    }

    // Create platform-specific download step
    const download_step = if (platform.is_windows) blk: {
        // Windows: Use PowerShell
        break :blk b.addSystemCommand(&.{
            "powershell",
            "-NoProfile",
            "-ExecutionPolicy",
            "Bypass",
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
    } else blk: {
        // Linux: Use bash/curl/tar
        break :blk b.addSystemCommand(&.{
            "/bin/bash",
            "-c",
            b.fmt(
                \\set -e
                \\VERSION='{s}'
                \\DATE='{s}'
                \\OUT_DIR='{s}'
                \\TEMP_FILE="/tmp/dxc_linux.tar.gz"
                \\URL="https://github.com/microsoft/DirectXShaderCompiler/releases/download/$VERSION/linux_dxc_$DATE.x86_64.tar.gz"
                \\
                \\echo "Downloading DXC $VERSION for Linux..."
                \\echo "URL: $URL"
                \\
                \\# Download
                \\curl -L -o "$TEMP_FILE" "$URL"
                \\
                \\# Extract
                \\echo "Extracting to $OUT_DIR..."
                \\rm -rf "$OUT_DIR"
                \\mkdir -p "$OUT_DIR"
                \\tar -xzf "$TEMP_FILE" -C "$OUT_DIR"
                \\
                \\# Cleanup
                \\rm -f "$TEMP_FILE"
                \\
                \\echo "DXC $VERSION installed to $OUT_DIR"
                \\echo "Note: You may need to add $OUT_DIR/lib to LD_LIBRARY_PATH"
            , .{ DXC_VERSION, DXC_DATE, DXC_DIR }),
        });
    };

    config.dxc_lib_path = dxc_lib_path;

    return &download_step.step;
}
