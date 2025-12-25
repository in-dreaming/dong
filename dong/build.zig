const std = @import("std");
const builtin = @import("builtin");

const skia_root_dir = "third_party/skia/Skia-m124";

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
// Helper Functions
// =============================================================================

fn addCSourcesRecursive(
    b: *std.Build,
    comp: *std.Build.Step.Compile,
    base_dir: []const u8,
    flags: []const []const u8,
    target: std.Build.ResolvedTarget,
) void {
    var dir = std.fs.cwd().openDir(base_dir, .{ .iterate = true }) catch |err| std.debug.panic("failed to open {s}: {s}", .{ base_dir, @errorName(err) });
    defer dir.close();

    var walker = dir.walk(b.allocator) catch |err| std.debug.panic("failed to walk {s}: {s}", .{ base_dir, @errorName(err) });
    defer walker.deinit();

    var files = std.ArrayList([]const u8).init(b.allocator);

    const is_windows = target.result.os.tag == .windows;

    while (walker.next() catch unreachable) |entry| {
        if (entry.kind == .file and std.mem.endsWith(u8, entry.path, ".c")) {
            // Platform-specific file filtering
            const has_windows_path =
                std.mem.indexOf(u8, entry.path, "ports/windows") != null or
                std.mem.indexOf(u8, entry.path, "ports\\windows") != null or
                std.mem.indexOf(u8, entry.path, "ports/windows_nt") != null or
                std.mem.indexOf(u8, entry.path, "ports\\windows_nt") != null or
                std.mem.indexOf(u8, entry.path, "/windows/") != null or
                std.mem.indexOf(u8, entry.path, "\\windows\\") != null;

            const has_posix_path =
                std.mem.indexOf(u8, entry.path, "ports/posix") != null or
                std.mem.indexOf(u8, entry.path, "ports\\posix") != null or
                std.mem.indexOf(u8, entry.path, "/posix/") != null or
                std.mem.indexOf(u8, entry.path, "\\posix\\") != null;

            // Skip platform-specific files for the wrong platform
            if (is_windows and has_posix_path) continue;
            if (!is_windows and has_windows_path) continue;

            const full = std.fs.path.join(b.allocator, &.{ base_dir, entry.path }) catch unreachable;
            files.append(full) catch unreachable;
        }
    }

    if (files.items.len == 0) {
        std.debug.panic("no .c files discovered under {s}", .{base_dir});
    }

    const owned = files.toOwnedSlice() catch unreachable;
    comp.addCSourceFiles(.{ .files = owned, .flags = flags });
}

fn addSkiaIncludePaths(step: *std.Build.Step.Compile, b: *std.Build) void {
    const root = skia_root_dir;
    step.addIncludePath(b.path(root));
    step.addIncludePath(b.path(b.fmt("{s}/include", .{root})));
    step.addIncludePath(b.path(b.fmt("{s}/include/core", .{root})));
    step.addIncludePath(b.path(b.fmt("{s}/include/gpu", .{root})));
    step.addIncludePath(b.path(b.fmt("{s}/modules/skshaper/include", .{root})));
    step.addIncludePath(b.path(b.fmt("{s}/modules/skparagraph/include", .{root})));
}

fn resolveSkiaOutDir(
    b: *std.Build,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
) []const u8 {
    const flavor = if (optimize == .Debug) "Debug" else "Release";
    const platform = switch (target.result.os.tag) {
        .windows => "win",
        .macos => "mac",
        else => std.debug.panic("Skia-m124 currently supports only macOS/Windows (got {s})", .{@tagName(target.result.os.tag)}),
    };
    const arch = switch (target.result.cpu.arch) {
        .x86_64 => "x64",
        .aarch64 => "arm64",
        else => std.debug.panic("Skia-m124 has no prebuilt artifacts for {s}", .{@tagName(target.result.cpu.arch)}),
    };

    return b.fmt("{s}/out/{s}-{s}-{s}", .{ skia_root_dir, flavor, platform, arch });
}

fn configureSkia(
    step: *std.Build.Step.Compile,
    b: *std.Build,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
) void {
    addSkiaIncludePaths(step, b);

    switch (target.result.os.tag) {
        .windows => {
            const skia_lib_dir = resolveSkiaOutDir(b, target, optimize);
            const libs = &.{
                "skia.lib",
                "skottie.lib",
                "skparagraph.lib",
                "skresources.lib",
                "sksg.lib",
                "skshaper.lib",
                "skunicode.lib",
                "svg.lib",
                "bentleyottmann.lib",
                "harfbuzz.lib",
                "icu.lib",
                "freetype2.lib",
                "libpng.lib",
                "libjpeg.lib",
                "libwebp.lib",
                "libwebp_sse41.lib",
                "expat.lib",
                "zlib.lib",
                "skcms.lib",
                "wuffs.lib",
                "spvtools.lib",
                "spvtools_val.lib",
            };

            inline for (libs) |lib_name| {
                const lib_path = b.fmt("{s}/{s}", .{ skia_lib_dir, lib_name });
                step.addObjectFile(b.path(lib_path));
            }

            step.linkSystemLibrary("opengl32");
            step.linkSystemLibrary("user32");
            step.linkSystemLibrary("gdi32");
            step.linkSystemLibrary("ole32");
            step.linkSystemLibrary("oleaut32");
            step.linkSystemLibrary("kernel32");
        },
        .macos => {
            const skia_lib_dir = resolveSkiaOutDir(b, target, optimize);
            const archives = .{
                "libskia.a",
                "libskottie.a",
                "libskparagraph.a",
                "libskshaper.a",
                "libskunicode.a",
                "libsksg.a",
                "libskresources.a",
                "libharfbuzz.a",
                "libfreetype2.a",
                "libicu.a",
                "libskcms.a",
                "libwuffs.a",
                "libsvg.a",
                "libwebp.a",
                "libwebp_sse41.a",
                "libpng.a",
                "libjpeg.a",
                "libzlib.a",
                "libexpat.a",
                "libpiex.a",
                "libbentleyottmann.a",
            };

            inline for (archives) |name| {
                const archive_path = b.fmt("{s}/{s}", .{ skia_lib_dir, name });
                step.addObjectFile(b.path(archive_path));
            }

            step.linkFramework("CoreFoundation");
            step.linkFramework("CoreGraphics");
            step.linkFramework("CoreText");
            step.linkFramework("Metal");
            step.linkFramework("ApplicationServices");
            step.linkSystemLibrary("z");
        },
        else => {
            std.debug.panic("Skia-m124 currently supports only macOS/Windows targets", .{});
        },
    }
}

fn configureExample(
    exe: *std.Build.Step.Compile,
    b: *std.Build,
    dong: *std.Build.Step.Compile,
    quickjs: *std.Build.Step.Compile,
    lexbor: *std.Build.Step.Compile,
    yoga: *std.Build.Step.Compile,
    target: std.Build.ResolvedTarget,
    config: BuildConfig,
) void {
    // This function is only called on non-Windows platforms
    const lib_ext = ".a";
    const lib_prefix = "lib";

    // Common include paths
    exe.addIncludePath(b.path("include"));
    exe.addIncludePath(b.path("src"));
    exe.addIncludePath(b.path("third_party/quickjs"));
    exe.addIncludePath(b.path("third_party/sdl/include"));

    // Link libraries
    exe.linkLibrary(dong);
    exe.linkLibrary(quickjs);
    exe.linkLibrary(lexbor);
    exe.linkLibrary(yoga);

    // Link C/C++ runtime
    exe.linkLibC();
    exe.linkLibCpp();

    // Link FreeType, HarfBuzz and msdfgen (static libraries)
    exe.addObjectFile(.{ .cwd_relative = b.fmt("zig-out/freetype/lib/{s}freetype{s}", .{ lib_prefix, lib_ext }) });
    exe.addObjectFile(.{ .cwd_relative = b.fmt("zig-out/harfbuzz/lib/{s}harfbuzz{s}", .{ lib_prefix, lib_ext }) });
    exe.addObjectFile(.{ .cwd_relative = b.fmt("zig-out/msdfgen/lib/{s}msdfgen-core{s}", .{ lib_prefix, lib_ext }) });
    exe.addObjectFile(.{ .cwd_relative = b.fmt("zig-out/msdfgen/lib/{s}msdfgen-ext{s}", .{ lib_prefix, lib_ext }) });

    // Platform-specific configuration (macOS only since Windows uses CMake)
    switch (target.result.os.tag) {
        .macos => {
            exe.linkFramework("CoreFoundation");
            exe.linkFramework("CoreGraphics");
            exe.linkFramework("CoreText");

            // Shader compilation dependencies (HLSL -> SPIR-V -> MSL)
            if (config.vulkan_sdk_path) |vulkan_path| {
                const vulkan_lib = b.fmt("{s}/lib", .{vulkan_path});
                exe.addLibraryPath(.{ .cwd_relative = vulkan_lib });
                exe.addRPath(.{ .cwd_relative = vulkan_lib });
            }
            exe.linkSystemLibrary("spirv-cross-c-shared");

            if (config.dxc_lib_path) |dxc_path| {
                exe.addLibraryPath(.{ .cwd_relative = dxc_path });
                exe.addRPath(.{ .cwd_relative = dxc_path });
            }
            exe.linkSystemLibrary("dxcompiler");
        },
        .linux => {
            // Linux configuration
            if (config.vulkan_sdk_path) |vulkan_path| {
                const vulkan_lib = b.fmt("{s}/lib", .{vulkan_path});
                exe.addLibraryPath(.{ .cwd_relative = vulkan_lib });
                exe.addRPath(.{ .cwd_relative = vulkan_lib });
            }
            exe.linkSystemLibrary("spirv-cross-c-shared");

            if (config.dxc_lib_path) |dxc_path| {
                exe.addLibraryPath(.{ .cwd_relative = dxc_path });
                exe.addRPath(.{ .cwd_relative = dxc_path });
            }
            exe.linkSystemLibrary("dxcompiler");
        },
        else => {
            std.debug.panic("Unsupported target OS: {s}", .{@tagName(target.result.os.tag)});
        },
    }
}

fn configureDong(
    dong: *std.Build.Step.Compile,
    b: *std.Build,
    target: std.Build.ResolvedTarget,
    config: BuildConfig,
) void {
    // This function is only called on non-Windows platforms
    dong.addIncludePath(b.path("include"));
    dong.addIncludePath(b.path("src"));
    dong.addIncludePath(b.path("third_party/quickjs"));
    dong.addIncludePath(b.path("third_party/lexbor/source"));
    dong.addIncludePath(b.path("third_party/yoga"));
    dong.addIncludePath(b.path("third_party/sdl/include"));
    dong.addIncludePath(b.path("third_party/SDL_shadercross/include"));

    // VulkanSDK spirv_cross include
    if (config.vulkan_sdk_path) |vulkan_path| {
        const spirv_include = switch (target.result.os.tag) {
            .macos => b.fmt("{s}/include/spirv_cross", .{vulkan_path}),
            else => b.fmt("{s}/include/spirv_cross", .{vulkan_path}),
        };
        dong.addIncludePath(.{ .cwd_relative = spirv_include });
    }

    // DXC include
    dong.addIncludePath(.{ .cwd_relative = config.dxc_include_path });

    // FreeType, HarfBuzz and msdfgen includes
    dong.addIncludePath(.{ .cwd_relative = "zig-out/freetype/include/freetype2" });
    dong.addIncludePath(.{ .cwd_relative = "zig-out/harfbuzz/include/harfbuzz" });
    dong.addIncludePath(.{ .cwd_relative = "zig-out/msdfgen/include" });

    dong.addCSourceFiles(.{
        .files = &.{
            "src/api_bindings.cpp",
            "src/core/context.cpp",
            "src/core/view.cpp",
            "src/dom/dom_manager.cpp",
            "src/dom/dom_node.cpp",
            "src/dom/event_system.cpp",
            "src/dom/parser.cpp",
            "src/dom/style_engine.cpp",
            "src/layout/layout_engine.cpp",
            "src/script/script_engine.cpp",
            "src/script/js_bindings.cpp",
            "src/render/painter.cpp",
            "src/render/render_surface.cpp",
            "src/render/resource_manager.cpp",
            "src/render/gpu_device.cpp",
            "src/render/gpu_surface.cpp",
            "src/render/font_resolver.cpp",
            "src/render/font_finder.cpp",
            "src/render/text_shaper.cpp",
            "src/render/font_metrics.cpp",
            "src/render/gpu_painter.cpp",
            "src/render/gpu_driver_sdl.cpp",
            "src/render/shader_manager.cpp",
            "src/render/glyph_atlas.cpp",
            "src/platform/sdl3_window.cpp",
            "src/input/input_adapter_sdl3.cpp",
            "src/dom/focus_manager.cpp",
            "src/dom/input_element.cpp",
        },
        .flags = &.{"-std=c++17"},
    });

    dong.addCSourceFiles(.{
        .files = &.{
            "third_party/SDL_shadercross/src/SDL_shadercross.c",
        },
        .flags = &.{"-DSDL_SHADERCROSS_DXC"},
    });

    dong.linkLibC();
    dong.linkLibCpp();
}

fn getStaticLibName(base_name: []const u8, target: std.Build.ResolvedTarget) []const u8 {
    return switch (target.result.os.tag) {
        .windows => base_name, // e.g., "freetype.lib"
        else => base_name, // e.g., "libfreetype.a"
    };
}

fn getFreetypeLibPath(b: *std.Build, target: std.Build.ResolvedTarget) []const u8 {
    return switch (target.result.os.tag) {
        .windows => b.fmt("zig-out/freetype/lib/freetype.lib", .{}),
        else => b.fmt("zig-out/freetype/lib/libfreetype.a", .{}),
    };
}

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // Load build configuration
    const config = loadBuildConfig(b.allocator);

    const is_windows = target.result.os.tag == .windows;

    // ==========================================================================
    // On Windows, use CMake for all C++ code due to zig 0.15 libcxxabi issues
    // ==========================================================================
    if (is_windows) {
        buildWindows(b, target, config);
        return;
    }

    // ==========================================================================
    // Non-Windows build (macOS, Linux) - use zig for everything
    // ==========================================================================
    buildNonWindows(b, target, optimize, config);
}

fn buildWindows(b: *std.Build, target_in: std.Build.ResolvedTarget, config: BuildConfig) void {
    var target = target_in;
    // Force MSVC ABI
    if (target.result.abi != .msvc) {
        target = b.resolveTargetQuery(.{
            .cpu_arch = target.result.cpu.arch,
            .os_tag = .windows,
            .abi = .msvc,
        });
    }

    // Build third-party dependencies with CMake
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
    const freetype_lib_abs = b.fmt("{s}/{s}/lib/freetype.lib", .{ b.build_root.path.?, freetype_prefix });

    // ==========================================================================
    // QuickJS via CMake + MinGW64 GCC (shared DLL)
    // This avoids clang-cl's lack of GCC inline asm support
    // ==========================================================================
    const quickjs_cmake_config = b.addSystemCommand(&.{
        "cmake", "-S", "third_party/quickjs_make", "-B", quickjs_build_dir,
        "-G", "MinGW Makefiles",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_C_COMPILER=gcc",
        "-DBUILD_SHARED_LIBS=ON",
    });
    const quickjs_cmake_build = b.addSystemCommand(&.{ "cmake", "--build", quickjs_build_dir, "--config", "Release" });
    quickjs_cmake_build.step.dependOn(&quickjs_cmake_config.step);
    const quickjs_cmake_install = b.addSystemCommand(&.{ "cmake", "--install", quickjs_build_dir, "--prefix", quickjs_prefix });
    quickjs_cmake_install.step.dependOn(&quickjs_cmake_build.step);

    // FreeType
    const freetype_cmake_config = b.addSystemCommand(&.{
        "cmake", "-S", "third_party/freetype", "-B", freetype_build_dir,
        "-G", "Ninja",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_C_COMPILER=clang-cl",
        "-DCMAKE_CXX_COMPILER=clang-cl",
        "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL",
        "-DBUILD_SHARED_LIBS=OFF",
        "-DFT_DISABLE_HARFBUZZ=ON",
        "-DFT_DISABLE_PNG=ON",
        "-DFT_DISABLE_ZLIB=ON",
        "-DFT_DISABLE_BZIP2=ON",
        "-DFT_DISABLE_BROTLI=ON",
    });
    const freetype_cmake_build = b.addSystemCommand(&.{ "cmake", "--build", freetype_build_dir, "--config", "Release" });
    freetype_cmake_build.step.dependOn(&freetype_cmake_config.step);
    const freetype_cmake_install = b.addSystemCommand(&.{ "cmake", "--install", freetype_build_dir, "--prefix", freetype_prefix });
    freetype_cmake_install.step.dependOn(&freetype_cmake_build.step);

    // HarfBuzz
    const harfbuzz_cmake_config = b.addSystemCommand(&.{
        "cmake", "-S", "third_party/harfbuzz", "-B", harfbuzz_build_dir,
        "-G", "Ninja",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_C_COMPILER=clang-cl",
        "-DCMAKE_CXX_COMPILER=clang-cl",
        "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL",
        "-DBUILD_SHARED_LIBS=OFF",
        "-DHB_HAVE_FREETYPE=ON",
        b.fmt("-DFREETYPE_INCLUDE_DIRS={s}", .{freetype_include_abs}),
        b.fmt("-DFREETYPE_LIBRARY={s}", .{freetype_lib_abs}),
        "-DHB_HAVE_GLIB=OFF",
        "-DHB_HAVE_ICU=OFF",
        "-DHB_BUILD_UTILS=OFF",
        "-DHB_BUILD_SUBSET=OFF",
    });
    harfbuzz_cmake_config.step.dependOn(&freetype_cmake_install.step);
    const harfbuzz_cmake_build = b.addSystemCommand(&.{ "cmake", "--build", harfbuzz_build_dir, "--config", "Release" });
    harfbuzz_cmake_build.step.dependOn(&harfbuzz_cmake_config.step);
    const harfbuzz_cmake_install = b.addSystemCommand(&.{ "cmake", "--install", harfbuzz_build_dir, "--prefix", harfbuzz_prefix });
    harfbuzz_cmake_install.step.dependOn(&harfbuzz_cmake_build.step);

    // msdfgen
    const msdfgen_cmake_config = b.addSystemCommand(&.{
        "cmake", "-S", "third_party/msdfgen", "-B", msdfgen_build_dir,
        "-G", "Ninja",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_C_COMPILER=clang-cl",
        "-DCMAKE_CXX_COMPILER=clang-cl",
        "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL",
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
    });
    msdfgen_cmake_config.step.dependOn(&freetype_cmake_install.step);
    const msdfgen_cmake_build = b.addSystemCommand(&.{ "cmake", "--build", msdfgen_build_dir, "--config", "Release" });
    msdfgen_cmake_build.step.dependOn(&msdfgen_cmake_config.step);
    const msdfgen_cmake_install = b.addSystemCommand(&.{ "cmake", "--install", msdfgen_build_dir, "--prefix", msdfgen_prefix });
    msdfgen_cmake_install.step.dependOn(&msdfgen_cmake_build.step);

    // SDL3
    const sdl3_cmake_config = b.addSystemCommand(&.{
        "cmake", "-S", "third_party/sdl", "-B", sdl3_build_dir,
        "-G", "Ninja",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_C_COMPILER=clang-cl",
        "-DCMAKE_CXX_COMPILER=clang-cl",
        "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL",
        "-DSDL_TEST=OFF",
        "-DSDL_STATIC=OFF",
        "-DSDL_SHARED=ON",
        "-DSDL_TESTS=OFF",
        "-DSDL_EXAMPLES=OFF",
    });
    const sdl3_cmake_build = b.addSystemCommand(&.{ "cmake", "--build", sdl3_build_dir, "--config", "Release" });
    sdl3_cmake_build.step.dependOn(&sdl3_cmake_config.step);
    const sdl3_cmake_install = b.addSystemCommand(&.{ "cmake", "--install", sdl3_build_dir, "--prefix", sdl3_prefix });
    sdl3_cmake_install.step.dependOn(&sdl3_cmake_build.step);

    // Build dong and examples with CMake
    const cmake_build_dir = "build-cmake";

    const vulkan_sdk_cmake_arg = if (config.vulkan_sdk_path) |vk_path|
        b.fmt("-DVULKAN_SDK_PATH={s}", .{vk_path})
    else
        "-DVULKAN_SDK_PATH=";

    const dxc_lib_cmake_arg = if (config.dxc_lib_path) |dxc_path|
        b.fmt("-DDXC_LIB_PATH={s}", .{dxc_path})
    else
        "-DDXC_LIB_PATH=third_party/dxc_2025_07_14/lib/x64";

    const cmake_config = b.addSystemCommand(&.{
        "cmake", "-S", ".", "-B", cmake_build_dir,
        "-G", "Ninja",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_C_COMPILER=clang-cl",
        "-DCMAKE_CXX_COMPILER=clang-cl",
        "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL",
        vulkan_sdk_cmake_arg,
        dxc_lib_cmake_arg,
    });
    cmake_config.step.dependOn(&freetype_cmake_install.step);
    cmake_config.step.dependOn(&harfbuzz_cmake_install.step);
    cmake_config.step.dependOn(&msdfgen_cmake_install.step);
    cmake_config.step.dependOn(&sdl3_cmake_install.step);
    cmake_config.step.dependOn(&quickjs_cmake_install.step);

    const cmake_build = b.addSystemCommand(&.{ "cmake", "--build", cmake_build_dir, "--config", "Release" });
    cmake_build.step.dependOn(&cmake_config.step);

    const examples_step = b.step("examples", "Build all examples with CMake (Windows)");
    examples_step.dependOn(&cmake_build.step);

    // Install step
    const cmake_install = b.addSystemCommand(&.{ "cmake", "--install", cmake_build_dir, "--prefix", "zig-out" });
    cmake_install.step.dependOn(&cmake_build.step);
    b.getInstallStep().dependOn(&cmake_install.step);
}

fn buildNonWindows(b: *std.Build, target: std.Build.ResolvedTarget, optimize: std.builtin.OptimizeMode, config: BuildConfig) void {

    // Static library file extension
    const lib_ext = ".a";
    const lib_prefix = "lib";

    // ==========================================================================
    // FreeType via CMake (static)
    // ==========================================================================
    const freetype_build_dir = "third_party/freetype/build-zig";
    const freetype_prefix = "zig-out/freetype";

    var freetype_cmake_args = std.ArrayList([]const u8).init(b.allocator);
    freetype_cmake_args.appendSlice(&.{
        "cmake",
        "-S",
        "third_party/freetype",
        "-B",
        freetype_build_dir,
        "-DCMAKE_BUILD_TYPE=Release",
        "-DBUILD_SHARED_LIBS=OFF",
        "-DFT_DISABLE_HARFBUZZ=ON",
        "-DFT_DISABLE_PNG=ON",
        "-DFT_DISABLE_ZLIB=ON",
        "-DFT_DISABLE_BZIP2=ON",
        "-DFT_DISABLE_BROTLI=ON",
    }) catch unreachable;

    const freetype_cmake_config = b.addSystemCommand(freetype_cmake_args.items);
    const freetype_cmake_build = b.addSystemCommand(&.{
        "cmake",
        "--build",
        freetype_build_dir,
        "--config",
        "Release",
    });
    freetype_cmake_build.step.dependOn(&freetype_cmake_config.step);

    const freetype_cmake_install = b.addSystemCommand(&.{
        "cmake",
        "--install",
        freetype_build_dir,
        "--prefix",
        freetype_prefix,
    });
    freetype_cmake_install.step.dependOn(&freetype_cmake_build.step);

    // ==========================================================================
    // HarfBuzz via CMake (static)
    // ==========================================================================
    const harfbuzz_build_dir = "third_party/harfbuzz/build-zig";
    const harfbuzz_prefix = "zig-out/harfbuzz";

    // Use absolute paths for FreeType
    const freetype_include_abs = b.fmt("{s}/{s}/include/freetype2", .{ b.build_root.path.?, freetype_prefix });
    const freetype_lib_name = b.fmt("{s}freetype{s}", .{ lib_prefix, lib_ext });
    const freetype_lib_abs = b.fmt("{s}/{s}/lib/{s}", .{ b.build_root.path.?, freetype_prefix, freetype_lib_name });

    var harfbuzz_cmake_args = std.ArrayList([]const u8).init(b.allocator);
    harfbuzz_cmake_args.appendSlice(&.{
        "cmake",
        "-S",
        "third_party/harfbuzz",
        "-B",
        harfbuzz_build_dir,
        "-DCMAKE_BUILD_TYPE=Release",
        "-DBUILD_SHARED_LIBS=OFF",
        "-DHB_HAVE_FREETYPE=ON",
        b.fmt("-DFREETYPE_INCLUDE_DIRS={s}", .{freetype_include_abs}),
        b.fmt("-DFREETYPE_LIBRARY={s}", .{freetype_lib_abs}),
        "-DHB_HAVE_GLIB=OFF",
        "-DHB_HAVE_ICU=OFF",
        "-DHB_BUILD_UTILS=OFF",
        "-DHB_BUILD_SUBSET=OFF",
    }) catch unreachable;

    const harfbuzz_cmake_config = b.addSystemCommand(harfbuzz_cmake_args.items);
    const harfbuzz_cmake_build = b.addSystemCommand(&.{
        "cmake",
        "--build",
        harfbuzz_build_dir,
        "--config",
        "Release",
    });
    harfbuzz_cmake_build.step.dependOn(&harfbuzz_cmake_config.step);
    harfbuzz_cmake_build.step.dependOn(&freetype_cmake_install.step);

    const harfbuzz_cmake_install = b.addSystemCommand(&.{
        "cmake",
        "--install",
        harfbuzz_build_dir,
        "--prefix",
        harfbuzz_prefix,
    });
    harfbuzz_cmake_install.step.dependOn(&harfbuzz_cmake_build.step);

    // ==========================================================================
    // msdfgen via CMake (static)
    // ==========================================================================
    const msdfgen_build_dir = "third_party/msdfgen/build-zig";
    const msdfgen_prefix = "zig-out/msdfgen";

    var msdfgen_cmake_args = std.ArrayList([]const u8).init(b.allocator);
    msdfgen_cmake_args.appendSlice(&.{
        "cmake",
        "-S",
        "third_party/msdfgen",
        "-B",
        msdfgen_build_dir,
        "-DCMAKE_BUILD_TYPE=Release",
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

    const msdfgen_cmake_config = b.addSystemCommand(msdfgen_cmake_args.items);
    const msdfgen_cmake_build = b.addSystemCommand(&.{
        "cmake",
        "--build",
        msdfgen_build_dir,
        "--config",
        "Release",
    });
    msdfgen_cmake_build.step.dependOn(&msdfgen_cmake_config.step);
    msdfgen_cmake_build.step.dependOn(&freetype_cmake_install.step);

    const msdfgen_cmake_install = b.addSystemCommand(&.{
        "cmake",
        "--install",
        msdfgen_build_dir,
        "--prefix",
        msdfgen_prefix,
    });
    msdfgen_cmake_install.step.dependOn(&msdfgen_cmake_build.step);

    // ==========================================================================
    // SDL3 via CMake (shared)
    // ==========================================================================
    const sdl3_build_dir = "third_party/sdl/build-zig";
    const sdl3_prefix = "zig-out/sdl3";

    var sdl3_cmake_args = std.ArrayList([]const u8).init(b.allocator);
    sdl3_cmake_args.appendSlice(&.{
        "cmake",
        "-S",
        "third_party/sdl",
        "-B",
        sdl3_build_dir,
        "-DSDL_TEST=OFF",
        "-DSDL_STATIC=OFF",
        "-DSDL_SHARED=ON",
        "-DSDL_TESTS=OFF",
        "-DSDL_EXAMPLES=OFF",
    }) catch unreachable;


    const sdl3_cmake_config = b.addSystemCommand(sdl3_cmake_args.items);
    const sdl3_cmake_build = b.addSystemCommand(&.{
        "cmake",
        "--build",
        sdl3_build_dir,
        "--config",
        "Release",
    });
    sdl3_cmake_build.step.dependOn(&sdl3_cmake_config.step);

    const sdl3_cmake_install = b.addSystemCommand(&.{
        "cmake",
        "--install",
        sdl3_build_dir,
        "--prefix",
        sdl3_prefix,
    });
    sdl3_cmake_install.step.dependOn(&sdl3_cmake_build.step);

    // ==========================================================================
    // QuickJS
    // ==========================================================================
    const quickjs = b.addStaticLibrary(.{
        .name = "quickjs",
        .target = target,
        .optimize = optimize,
    });
    quickjs.addIncludePath(b.path("third_party/quickjs"));

    const quickjs_flags: []const []const u8 = &.{"-D_GNU_SOURCE"};

    quickjs.addCSourceFiles(.{
        .files = &.{
            "third_party/quickjs/quickjs.c",
            "third_party/quickjs/libregexp.c",
            "third_party/quickjs/libunicode.c",
            "third_party/quickjs/cutils.c",
            "third_party/quickjs/dtoa.c",
        },
        .flags = quickjs_flags,
    });
    quickjs.linkLibC();

    // ==========================================================================
    // Lexbor
    // ==========================================================================
    const lexbor = b.addStaticLibrary(.{
        .name = "lexbor",
        .target = target,
        .optimize = optimize,
    });
    lexbor.addIncludePath(b.path("third_party/lexbor/source"));
    addCSourcesRecursive(b, lexbor, "third_party/lexbor/source", &.{"-DLEXBOR_STATIC"}, target);
    lexbor.linkLibC();

    // ==========================================================================
    // Yoga
    // ==========================================================================
    const yoga = b.addStaticLibrary(.{
        .name = "yoga",
        .target = target,
        .optimize = optimize,
    });
    yoga.addIncludePath(b.path("third_party/yoga"));
    yoga.addCSourceFiles(.{
        .files = &.{
            "third_party/yoga/yoga/YGConfig.cpp",
            "third_party/yoga/yoga/YGEnums.cpp",
            "third_party/yoga/yoga/YGNode.cpp",
            "third_party/yoga/yoga/YGNodeLayout.cpp",
            "third_party/yoga/yoga/YGNodeStyle.cpp",
            "third_party/yoga/yoga/YGValue.cpp",
            "third_party/yoga/yoga/algorithm/AbsoluteLayout.cpp",
            "third_party/yoga/yoga/algorithm/Baseline.cpp",
            "third_party/yoga/yoga/algorithm/Cache.cpp",
            "third_party/yoga/yoga/algorithm/CalculateLayout.cpp",
            "third_party/yoga/yoga/algorithm/FlexLine.cpp",
            "third_party/yoga/yoga/algorithm/PixelGrid.cpp",
            "third_party/yoga/yoga/config/Config.cpp",
            "third_party/yoga/yoga/debug/AssertFatal.cpp",
            "third_party/yoga/yoga/debug/Log.cpp",
            "third_party/yoga/yoga/event/event.cpp",
            "third_party/yoga/yoga/node/LayoutResults.cpp",
            "third_party/yoga/yoga/node/Node.cpp",
        },
        .flags = &.{ "-std=c++20", "-fno-exceptions", "-fno-rtti" },
    });
    yoga.linkLibC();
    yoga.linkLibCpp();

    // ==========================================================================
    // Dong core
    // ==========================================================================
    const dong = b.addStaticLibrary(.{
        .name = "dong",
        .target = target,
        .optimize = optimize,
    });

    configureDong(dong, b, target, config);

    dong.linkLibrary(quickjs);
    dong.linkLibrary(lexbor);
    dong.linkLibrary(yoga);

    // Link FreeType, HarfBuzz and msdfgen (static libraries)
    const freetype_lib_path = b.fmt("zig-out/freetype/lib/{s}freetype{s}", .{ lib_prefix, lib_ext });
    const harfbuzz_lib_path = b.fmt("zig-out/harfbuzz/lib/{s}harfbuzz{s}", .{ lib_prefix, lib_ext });
    const msdfgen_core_path = b.fmt("zig-out/msdfgen/lib/{s}msdfgen-core{s}", .{ lib_prefix, lib_ext });
    const msdfgen_ext_path = b.fmt("zig-out/msdfgen/lib/{s}msdfgen-ext{s}", .{ lib_prefix, lib_ext });

    dong.addObjectFile(.{ .cwd_relative = freetype_lib_path });
    dong.addObjectFile(.{ .cwd_relative = harfbuzz_lib_path });
    dong.addObjectFile(.{ .cwd_relative = msdfgen_core_path });
    dong.addObjectFile(.{ .cwd_relative = msdfgen_ext_path });

    // Ensure FreeType, HarfBuzz and msdfgen are built before dong
    dong.step.dependOn(&freetype_cmake_install.step);
    dong.step.dependOn(&harfbuzz_cmake_install.step);
    dong.step.dependOn(&msdfgen_cmake_install.step);

    b.installArtifact(dong);

    // ==========================================================================
    // SDL_shadercross CLI (for shader compilation pipeline)
    // ==========================================================================
    const shadercross = b.addExecutable(.{
        .name = "shadercross",
        .target = target,
        .optimize = optimize,
        .root_source_file = null,
    });
    shadercross.addIncludePath(b.path("third_party/SDL_shadercross/include"));
    shadercross.addIncludePath(.{ .cwd_relative = "zig-out/sdl3/include" });
    shadercross.addIncludePath(.{ .cwd_relative = config.dxc_include_path });

    if (config.vulkan_sdk_path) |vulkan_path| {
        const spirv_include = switch (target.result.os.tag) {
            .macos => b.fmt("{s}/include/spirv_cross", .{vulkan_path}),
            .windows => b.fmt("{s}/Include/spirv_cross", .{vulkan_path}),
            else => vulkan_path,
        };
        shadercross.addIncludePath(.{ .cwd_relative = spirv_include });
    }

    shadercross.addCSourceFiles(.{
        .files = &.{
            "third_party/SDL_shadercross/src/SDL_shadercross.c",
            "third_party/SDL_shadercross/src/cli.c",
        },
        .flags = &.{"-DSDL_SHADERCROSS_DXC"},
    });
    shadercross.linkLibC();

    // SDL3 library path
    const sdl3_lib_path = "zig-out/sdl3/lib";
    shadercross.addLibraryPath(.{ .cwd_relative = sdl3_lib_path });
    shadercross.linkSystemLibrary("SDL3");

    // Vulkan SDK library path
    if (config.vulkan_sdk_path) |vulkan_path| {
        const vulkan_lib = switch (target.result.os.tag) {
            .macos => b.fmt("{s}/lib", .{vulkan_path}),
            else => vulkan_path,
        };
        shadercross.addLibraryPath(.{ .cwd_relative = vulkan_lib });
        shadercross.addRPath(.{ .cwd_relative = vulkan_lib });
    }
    shadercross.linkSystemLibrary("spirv-cross-c-shared");

    // DXC library path
    if (config.dxc_lib_path) |dxc_path| {
        shadercross.addLibraryPath(.{ .cwd_relative = dxc_path });
        shadercross.addRPath(.{ .cwd_relative = dxc_path });
    }
    shadercross.linkSystemLibrary("dxcompiler");

    shadercross.addRPath(.{ .cwd_relative = sdl3_lib_path });
    shadercross.step.dependOn(&sdl3_cmake_install.step);
    _ = b.addInstallArtifact(shadercross, .{});

    const shader_test_cmd = b.addRunArtifact(shadercross);
    shader_test_cmd.addArgs(&.{
        "third_party/SDL_shadercross/test/shaders/simple.vert.hlsl",
        "-o",
        "zig-out/shadercross_simple.spv",
    });
    const shader_test_step = b.step("shader-test", "Compile a test shader via SDL_shadercross");
    shader_test_step.dependOn(&shader_test_cmd.step);

    // ==========================================================================
    // Examples
    // ==========================================================================
    const examples_step = b.step("examples", "Build all examples");

    // Build all examples with zig (non-Windows only, Windows uses CMake)
    const example_defs = [_]struct {
        name: []const u8,
        source: []const u8,
        flags: []const []const u8,
    }{
        .{ .name = "simple_demo", .source = "examples/simple_demo.c", .flags = &.{} },
        .{ .name = "basic_demo", .source = "examples/basic_demo.c", .flags = &.{} },
        .{ .name = "complete_demo", .source = "examples/complete_demo.cpp", .flags = &.{"-std=c++17"} },
        .{ .name = "dom_test", .source = "examples/dom_test.cpp", .flags = &.{"-std=c++17"} },
        .{ .name = "integration_demo", .source = "examples/integration_demo.cpp", .flags = &.{"-std=c++17"} },
        .{ .name = "js_api_test", .source = "examples/js_api_test.cpp", .flags = &.{"-std=c++17"} },
        .{ .name = "render_and_script_demo", .source = "examples/render_and_script_demo.cpp", .flags = &.{"-std=c++17"} },
        .{ .name = "style_cascade_test", .source = "examples/style_cascade_test.cpp", .flags = &.{"-std=c++17"} },
        .{ .name = "dom_api_demo", .source = "examples/dom_api_demo.cpp", .flags = &.{"-std=c++17"} },
        .{ .name = "style_modification_demo", .source = "examples/style_modification_demo.cpp", .flags = &.{"-std=c++17"} },
        .{ .name = "js_return_value_demo", .source = "examples/js_return_value_demo.cpp", .flags = &.{"-std=c++17"} },
        .{ .name = "performance_demo", .source = "examples/performance_demo.cpp", .flags = &.{"-std=c++17"} },
        .{ .name = "dirty_rect_optimization_demo", .source = "examples/dirty_rect_optimization_demo.cpp", .flags = &.{"-std=c++17"} },
        .{ .name = "js_event_demo", .source = "examples/js_event_demo.cpp", .flags = &.{"-std=c++17"} },
        .{ .name = "comprehensive_features_demo", .source = "examples/comprehensive_features_demo.cpp", .flags = &.{"-std=c++17"} },
        .{ .name = "image_rendering_demo", .source = "examples/image_rendering_demo.cpp", .flags = &.{"-std=c++17"} },
        .{ .name = "gpu_view_demo", .source = "examples/gpu_view_demo.cpp", .flags = &.{"-std=c++17"} },
        .{ .name = "gpu_screenshot_demo", .source = "examples/gpu_screenshot_demo.cpp", .flags = &.{"-std=c++17"} },
        .{ .name = "gpu_screenshot_demo_basic_layout", .source = "examples/gpu_screenshot_demo_basic_layout.cpp", .flags = &.{"-std=c++17"} },
        .{ .name = "gpu_screenshot_demo_glyph_stress", .source = "examples/gpu_screenshot_demo_glyph_stress.cpp", .flags = &.{"-std=c++17"} },
        .{ .name = "gpu_screenshot_analysis", .source = "examples/gpu_screenshot_analysis.cpp", .flags = &.{"-std=c++17"} },
        .{ .name = "gpu_texture_demo", .source = "examples/gpu_texture_demo.cpp", .flags = &.{"-std=c++17"} },
        .{ .name = "interactive_demo", .source = "examples/interactive_demo.cpp", .flags = &.{"-std=c++17"} },
        .{ .name = "offscreen_test", .source = "examples/offscreen_test.cpp", .flags = &.{"-std=c++17"} },
        .{ .name = "simple_offscreen", .source = "examples/simple_offscreen.cpp", .flags = &.{"-std=c++17"} },
        .{ .name = "msdf_soft_render", .source = "examples/msdf_soft_render.cpp", .flags = &.{"-std=c++17"} },
        .{ .name = "msdf_cpu_test", .source = "examples/msdf_cpu_render_test.cpp", .flags = &.{"-std=c++17"} },
        .{ .name = "isolation_demo", .source = "examples/isolation_demo.cpp", .flags = &.{"-std=c++17"} },
        .{ .name = "sdl_gpu_demo", .source = "examples/sdl_gpu_demo.cpp", .flags = &.{"-std=c++17"} },
    };

    inline for (example_defs) |info| {
        const exe = b.addExecutable(.{
            .name = info.name,
            .target = target,
            .optimize = optimize,
            .root_source_file = null,
        });
        exe.addCSourceFile(.{ .file = b.path(info.source), .flags = info.flags });
        configureExample(exe, b, dong, quickjs, lexbor, yoga, target, config);
        exe.addLibraryPath(.{ .cwd_relative = sdl3_lib_path });
        exe.linkSystemLibrary("SDL3");
        exe.addRPath(.{ .cwd_relative = sdl3_lib_path });
        exe.step.dependOn(&sdl3_cmake_install.step);
        const install = b.addInstallArtifact(exe, .{});
        examples_step.dependOn(&install.step);
    }

    // ==========================================================================
    // Run shortcuts
    // ==========================================================================
    const run_simple = b.addExecutable(.{
        .name = "run_simple_tmp",
        .target = target,
        .optimize = optimize,
        .root_source_file = null,
    });
    run_simple.addCSourceFile(.{ .file = b.path("examples/simple_demo.c"), .flags = &.{} });
    configureExample(run_simple, b, dong, quickjs, lexbor, yoga, target, config);
    run_simple.addLibraryPath(.{ .cwd_relative = sdl3_lib_path });
    run_simple.linkSystemLibrary("SDL3");
    run_simple.addRPath(.{ .cwd_relative = sdl3_lib_path });
    run_simple.step.dependOn(&sdl3_cmake_install.step);
    const run_simple_cmd = b.addRunArtifact(run_simple);
    const run_simple_step = b.step("run-simple", "Run simple demo");
    run_simple_step.dependOn(&run_simple_cmd.step);

    const run_complete = b.addExecutable(.{
        .name = "run_complete_tmp",
        .target = target,
        .optimize = optimize,
        .root_source_file = null,
    });
    run_complete.addCSourceFile(.{ .file = b.path("examples/complete_demo.cpp"), .flags = &.{"-std=c++17"} });
    configureExample(run_complete, b, dong, quickjs, lexbor, yoga, target, config);
    run_complete.addLibraryPath(.{ .cwd_relative = sdl3_lib_path });
    run_complete.linkSystemLibrary("SDL3");
    run_complete.addRPath(.{ .cwd_relative = sdl3_lib_path });
    run_complete.step.dependOn(&sdl3_cmake_install.step);
    const run_complete_cmd = b.addRunArtifact(run_complete);
    const run_complete_step = b.step("run-complete", "Run complete demo");
    run_complete_step.dependOn(&run_complete_cmd.step);
}
