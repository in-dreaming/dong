const std = @import("std");

const skia_root_dir = "third_party/skia/Skia-m124";

fn addCSourcesRecursive(
    b: *std.Build,
    comp: *std.Build.Step.Compile,
    base_dir: []const u8,
    flags: []const []const u8,
) void {
    var dir = std.fs.cwd().openDir(base_dir, .{ .iterate = true }) catch |err| std.debug.panic("failed to open {s}: {s}", .{ base_dir, @errorName(err) });
    defer dir.close();

    var walker = dir.walk(b.allocator) catch |err| std.debug.panic("failed to walk {s}: {s}", .{ base_dir, @errorName(err) });
    defer walker.deinit();

    var files = std.ArrayList([]const u8).init(b.allocator);

    while (walker.next() catch unreachable) |entry| {
        if (entry.kind == .file and std.mem.endsWith(u8, entry.path, ".c")) {
            const should_skip_posix = std.mem.indexOf(u8, entry.path, "ports/posix") != null or
                std.mem.indexOf(u8, entry.path, "ports\\posix") != null;
            const should_skip_windows = std.mem.indexOf(u8, entry.path, "ports/windows") != null or
                std.mem.indexOf(u8, entry.path, "ports\\windows") != null;

            if (should_skip_posix or should_skip_windows) {
                continue;
            }
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
    optimize: std.builtin.OptimizeMode,
) void {
    exe.addIncludePath(b.path("include"));
    exe.addIncludePath(b.path("src"));
    exe.addIncludePath(b.path("third_party/quickjs"));

    configureSkia(exe, b, target, optimize);

    exe.linkLibC();
    exe.linkLibCpp();
    exe.linkLibrary(dong);
    exe.linkLibrary(quickjs);
    exe.linkLibrary(lexbor);
    exe.linkLibrary(yoga);
}

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // QuickJS ---------------------------------------------------------------
    const quickjs = b.addStaticLibrary(.{
        .name = "quickjs",
        .target = target,
        .optimize = optimize,
    });
    quickjs.addIncludePath(b.path("third_party/quickjs"));
    quickjs.addCSourceFiles(.{
        .files = &.{
            "third_party/quickjs/quickjs.c",
            "third_party/quickjs/libregexp.c",
            "third_party/quickjs/libunicode.c",
            "third_party/quickjs/cutils.c",
            "third_party/quickjs/dtoa.c",
        },
        .flags = &.{"-D_GNU_SOURCE"},
    });
    quickjs.linkLibC();

    // Lexbor ----------------------------------------------------------------
    const lexbor = b.addStaticLibrary(.{
        .name = "lexbor",
        .target = target,
        .optimize = optimize,
    });
    lexbor.addIncludePath(b.path("third_party/lexbor/source"));
    addCSourcesRecursive(b, lexbor, "third_party/lexbor/source", &.{"-DLEXBOR_STATIC"});
    lexbor.linkLibC();

    // Yoga ------------------------------------------------------------------
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

    // Dong core -------------------------------------------------------------
    const dong = b.addStaticLibrary(.{
        .name = "dong",
        .target = target,
        .optimize = optimize,
    });
    dong.addIncludePath(b.path("include"));
    dong.addIncludePath(b.path("src"));
    dong.addIncludePath(b.path("third_party/quickjs"));
    dong.addIncludePath(b.path("third_party/lexbor/source"));
    dong.addIncludePath(b.path("third_party/yoga"));

    configureSkia(dong, b, target, optimize);

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
            "src/render/skia_backend.cpp",
            "src/render/resource_manager.cpp",
        },
        .flags = &.{"-std=c++17"},
    });
    dong.linkLibC();
    dong.linkLibCpp();
    dong.linkLibrary(quickjs);
    dong.linkLibrary(lexbor);
    dong.linkLibrary(yoga);

    b.installArtifact(dong);

    // ----------------------------------------------------------------------
    const examples_step = b.step("examples", "Build all examples");

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
        // 【新增】四个功能演示
        .{ .name = "dom_api_demo", .source = "examples/dom_api_demo.cpp", .flags = &.{"-std=c++17"} },
        .{ .name = "style_modification_demo", .source = "examples/style_modification_demo.cpp", .flags = &.{"-std=c++17"} },
        .{ .name = "js_return_value_demo", .source = "examples/js_return_value_demo.cpp", .flags = &.{"-std=c++17"} },
        .{ .name = "performance_demo", .source = "examples/performance_demo.cpp", .flags = &.{"-std=c++17"} },
        // 【新增】脏矩形优化演示
        .{ .name = "dirty_rect_optimization_demo", .source = "examples/dirty_rect_optimization_demo.cpp", .flags = &.{"-std=c++17"} },
        .{ .name = "skia_cpu_demo", .source = "examples/skia_cpu_demo.cpp", .flags = &.{"-std=c++17"} },
        .{ .name = "js_event_demo", .source = "examples/js_event_demo.cpp", .flags = &.{"-std=c++17"} },
        // 【新增】功能完整性演示
        .{ .name = "comprehensive_features_demo", .source = "examples/comprehensive_features_demo.cpp", .flags = &.{"-std=c++17"} },
        // 【新增】图片渲染演示
        .{ .name = "image_rendering_demo", .source = "examples/image_rendering_demo.cpp", .flags = &.{"-std=c++17"} },
    };

    inline for (example_defs) |info| {
        const exe = b.addExecutable(.{
            .name = info.name,
            .target = target,
            .optimize = optimize,
            .root_source_file = null,
        });
        exe.addCSourceFile(.{ .file = b.path(info.source), .flags = info.flags });
        configureExample(exe, b, dong, quickjs, lexbor, yoga, target, optimize);
        const install = b.addInstallArtifact(exe, .{});
        examples_step.dependOn(&install.step);
    }

    // Run shortcuts ---------------------------------------------------------
    const run_simple = b.addExecutable(.{
        .name = "run_simple_tmp",
        .target = target,
        .optimize = optimize,
        .root_source_file = null,
    });
    run_simple.addCSourceFile(.{ .file = b.path("examples/simple_demo.c"), .flags = &.{} });
    configureExample(run_simple, b, dong, quickjs, lexbor, yoga, target, optimize);
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
    configureExample(run_complete, b, dong, quickjs, lexbor, yoga, target, optimize);
    const run_complete_cmd = b.addRunArtifact(run_complete);
    const run_complete_step = b.step("run-complete", "Run complete demo");
    run_complete_step.dependOn(&run_complete_cmd.step);
}
