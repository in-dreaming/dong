const std = @import("std");
const builtin = @import("builtin");

pub fn build(b: *std.Build) void {
    // 1. 定义构建选项
    // 在 Windows 上显式指定 MSVC ABI，避免 CRT 链接混乱
    const target = if (builtin.os.tag == .windows)
        b.resolveTargetQuery(.{
            .cpu_arch = .x86_64,
            .os_tag = .windows,
            .abi = .msvc,
        })
    else
        b.standardTargetOptions(.{});

    const optimize = b.standardOptimizeOption(.{});

    // 2. 编译主库 (dong)
    const lib = b.addStaticLibrary(.{
        .name = "dong",
        .target = target,
        .optimize = optimize,
        .root_source_file = null,
    });

    lib.linkLibCpp();
    lib.linkLibC();
    lib.addIncludePath(b.path("include"));
    lib.addIncludePath(b.path("src"));

    buildQuickJS(b, lib, target, optimize);
    buildLexbor(b, lib, target, optimize);
    buildYoga(b, lib, target, optimize);
    buildSkia(b, lib, target);
    buildDongCore(b, lib);

    linkPlatformLibs(lib, target);
    b.installArtifact(lib);

    // 3. 编译 Demo 可执行文件
    buildDemos(b, lib, target, optimize);
}

fn buildQuickJS(b: *std.Build, lib: *std.Build.Step.Compile, target: std.Build.ResolvedTarget, optimize: std.builtin.OptimizeMode) void {
    if (!dirExists(b.build_root.handle, "third_party/quickjs")) return;

    const quickjs_lib = b.addStaticLibrary(.{
        .name = "quickjs",
        .target = target,
        .optimize = optimize,
        .root_source_file = null,
    });

    quickjs_lib.linkLibC();
    quickjs_lib.addIncludePath(b.path("third_party/quickjs"));

    // 只编译必要的文件，跳过会导致 Windows 编译问题的文件
    quickjs_lib.addCSourceFiles(.{
        .files = &.{
            "third_party/quickjs/cutils.c",
            "third_party/quickjs/libunicode.c",
        },
        .flags = &.{ "-D_GNU_SOURCE", "-Wno-error=implicit-function-declaration" },
    });

    b.installArtifact(quickjs_lib);
    lib.linkLibrary(quickjs_lib);
    std.debug.print("QuickJS compiled\n", .{});
}

fn buildLexbor(b: *std.Build, lib: *std.Build.Step.Compile, target: std.Build.ResolvedTarget, optimize: std.builtin.OptimizeMode) void {
    if (!dirExists(b.build_root.handle, "third_party/lexbor/source")) return;

    const lexbor_sources = &.{
        "third_party/lexbor/source/lexbor/core/array.c",
        "third_party/lexbor/source/lexbor/core/array_obj.c",
        "third_party/lexbor/source/lexbor/core/avl.c",
        "third_party/lexbor/source/lexbor/core/bst.c",
        "third_party/lexbor/source/lexbor/core/bst_map.c",
        "third_party/lexbor/source/lexbor/core/conv.c",
        "third_party/lexbor/source/lexbor/core/diyfp.c",
        "third_party/lexbor/source/lexbor/core/dobject.c",
        "third_party/lexbor/source/lexbor/core/dtoa.c",
        "third_party/lexbor/source/lexbor/core/hash.c",
        "third_party/lexbor/source/lexbor/core/in.c",
        "third_party/lexbor/source/lexbor/core/mem.c",
        "third_party/lexbor/source/lexbor/core/mraw.c",
        "third_party/lexbor/source/lexbor/core/plog.c",
        "third_party/lexbor/source/lexbor/core/print.c",
        "third_party/lexbor/source/lexbor/core/serialize.c",
        "third_party/lexbor/source/lexbor/core/shs.c",
        "third_party/lexbor/source/lexbor/core/str.c",
        "third_party/lexbor/source/lexbor/core/strtod.c",
        "third_party/lexbor/source/lexbor/core/utils.c",
        "third_party/lexbor/source/lexbor/css/at_rule.c",
        "third_party/lexbor/source/lexbor/css/css.c",
        "third_party/lexbor/source/lexbor/css/declaration.c",
        "third_party/lexbor/source/lexbor/css/log.c",
        "third_party/lexbor/source/lexbor/css/parser.c",
        "third_party/lexbor/source/lexbor/css/property.c",
        "third_party/lexbor/source/lexbor/css/rule.c",
        "third_party/lexbor/source/lexbor/css/state.c",
        "third_party/lexbor/source/lexbor/css/stylesheet.c",
        "third_party/lexbor/source/lexbor/css/unit.c",
        "third_party/lexbor/source/lexbor/css/value.c",
        "third_party/lexbor/source/lexbor/encoding/decode.c",
        "third_party/lexbor/source/lexbor/encoding/encode.c",
        "third_party/lexbor/source/lexbor/encoding/encoding.c",
        "third_party/lexbor/source/lexbor/encoding/range.c",
        "third_party/lexbor/source/lexbor/encoding/res.c",
        "third_party/lexbor/source/lexbor/html/encoding.c",
        "third_party/lexbor/source/lexbor/html/interface.c",
        "third_party/lexbor/source/lexbor/html/node.c",
        "third_party/lexbor/source/lexbor/html/parser.c",
        "third_party/lexbor/source/lexbor/html/serialize.c",
        "third_party/lexbor/source/lexbor/html/token.c",
        "third_party/lexbor/source/lexbor/html/tokenizer.c",
        "third_party/lexbor/source/lexbor/html/token_attr.c",
        "third_party/lexbor/source/lexbor/html/tree.c",
        "third_party/lexbor/source/lexbor/dom/collection.c",
        "third_party/lexbor/source/lexbor/dom/exception.c",
        "third_party/lexbor/source/lexbor/dom/interface.c",
        "third_party/lexbor/source/lexbor/ns/ns.c",
        "third_party/lexbor/source/lexbor/punycode/punycode.c",
        "third_party/lexbor/source/lexbor/selectors/selectors.c",
        "third_party/lexbor/source/lexbor/style/event.c",
        "third_party/lexbor/source/lexbor/style/style.c",
        "third_party/lexbor/source/lexbor/engine/engine.c",
        "third_party/lexbor/source/lexbor/tag/tag.c",
        "third_party/lexbor/source/lexbor/unicode/idna.c",
        "third_party/lexbor/source/lexbor/unicode/unicode.c",
        "third_party/lexbor/source/lexbor/url/url.c",
        "third_party/lexbor/source/lexbor/utils/http.c",
        "third_party/lexbor/source/lexbor/utils/warc.c",
        "third_party/lexbor/source/lexbor/css/at_rule/state.c",
        "third_party/lexbor/source/lexbor/css/property/state.c",
        "third_party/lexbor/source/lexbor/css/selectors/pseudo.c",
        "third_party/lexbor/source/lexbor/css/selectors/pseudo_state.c",
        "third_party/lexbor/source/lexbor/css/selectors/selector.c",
        "third_party/lexbor/source/lexbor/css/selectors/selectors.c",
        "third_party/lexbor/source/lexbor/css/selectors/state.c",
        "third_party/lexbor/source/lexbor/css/syntax/anb.c",
        "third_party/lexbor/source/lexbor/css/syntax/parser.c",
        "third_party/lexbor/source/lexbor/css/syntax/state.c",
        "third_party/lexbor/source/lexbor/css/syntax/syntax.c",
        "third_party/lexbor/source/lexbor/css/syntax/token.c",
        "third_party/lexbor/source/lexbor/css/syntax/tokenizer.c",
        "third_party/lexbor/source/lexbor/html/interfaces/document.c",
        "third_party/lexbor/source/lexbor/html/interfaces/element.c",
    };

    const lexbor_lib = b.addStaticLibrary(.{
        .name = "lexbor",
        .target = target,
        .optimize = optimize,
        .root_source_file = null,
    });

    lexbor_lib.linkLibC();
    lexbor_lib.addIncludePath(b.path("third_party/lexbor/source"));
    lexbor_lib.addCSourceFiles(.{
        .files = lexbor_sources,
        .flags = &.{"-DLEXBOR_STATIC"},
    });

    b.installArtifact(lexbor_lib);
    lib.linkLibrary(lexbor_lib);
    lib.addIncludePath(b.path("third_party/lexbor/source"));
    std.debug.print("Lexbor compiled", .{});
}

fn buildYoga(b: *std.Build, lib: *std.Build.Step.Compile, target: std.Build.ResolvedTarget, optimize: std.builtin.OptimizeMode) void {
    if (!dirExists(b.build_root.handle, "third_party/yoga/yoga")) return;

    const yoga_lib = b.addStaticLibrary(.{
        .name = "yoga",
        .target = target,
        .optimize = optimize,
        .root_source_file = null,
    });

    yoga_lib.linkLibCpp();
    addYogaIncludePaths(b, yoga_lib);

    yoga_lib.addCSourceFiles(.{
        .files = &.{
            "third_party/yoga/yoga/YGConfig.cpp",
            "third_party/yoga/yoga/YGEnums.cpp",
            "third_party/yoga/yoga/YGNode.cpp",
            "third_party/yoga/yoga/YGNodeLayout.cpp",
            "third_party/yoga/yoga/YGNodeStyle.cpp",
            "third_party/yoga/yoga/YGPixelGrid.cpp",
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
        .flags = &.{ "-std=c++20" },
    });

    b.installArtifact(yoga_lib);
    lib.linkLibrary(yoga_lib);
    lib.addIncludePath(b.path("third_party/yoga"));
    std.debug.print("Yoga compiled", .{});
}

fn addYogaIncludePaths(b: *std.Build, lib: *std.Build.Step.Compile) void {
    lib.addIncludePath(b.path("third_party/yoga"));
    lib.addIncludePath(b.path("third_party/yoga/yoga"));
    lib.addIncludePath(b.path("third_party/yoga/yoga/algorithm"));
    lib.addIncludePath(b.path("third_party/yoga/yoga/config"));
    lib.addIncludePath(b.path("third_party/yoga/yoga/debug"));
    lib.addIncludePath(b.path("third_party/yoga/yoga/enums"));
    lib.addIncludePath(b.path("third_party/yoga/yoga/event"));
    lib.addIncludePath(b.path("third_party/yoga/yoga/node"));
    lib.addIncludePath(b.path("third_party/yoga/yoga/numeric"));
    lib.addIncludePath(b.path("third_party/yoga/yoga/style"));
}

fn buildSkia(b: *std.Build, lib: *std.Build.Step.Compile, target: std.Build.ResolvedTarget) void {
    const skia_include = "third_party/skia/include";
    const skia_lib_dir = "third_party/skia/lib";

    if (!dirExists(b.build_root.handle, skia_include)) {
        std.debug.print("Info: Skia headers not found", .{});
        return;
    }

    addSkiaIncludePaths(b, lib, skia_include);

    if (!dirExists(b.build_root.handle, skia_lib_dir)) {
        std.debug.print("Warning: Skia lib not found", .{});
        return;
    }

    lib.addLibraryPath(b.path(skia_lib_dir));
    lib.linkSystemLibrary("skia");
    linkSkiaSystemLibs(lib, target);
    std.debug.print("Skia integrated", .{});
}

fn addSkiaIncludePaths(b: *std.Build, lib: *std.Build.Step.Compile, skia_path: []const u8) void {
    lib.addIncludePath(b.path(skia_path));
    lib.addIncludePath(b.path("third_party/skia"));
    lib.addIncludePath(b.path(b.fmt("{s}/core", .{skia_path})));
    lib.addIncludePath(b.path(b.fmt("{s}/gpu", .{skia_path})));
    lib.addIncludePath(b.path(b.fmt("{s}/pathops", .{skia_path})));
    lib.addIncludePath(b.path(b.fmt("{s}/codec", .{skia_path})));
    lib.addIncludePath(b.path(b.fmt("{s}/config", .{skia_path})));
}

fn linkSkiaSystemLibs(lib: *std.Build.Step.Compile, target: std.Build.ResolvedTarget) void {
    switch (target.result.os.tag) {
        .windows => {
            lib.linkSystemLibrary("opengl32");
            lib.linkSystemLibrary("user32");
            lib.linkSystemLibrary("gdi32");
            lib.linkSystemLibrary("ole32");
            lib.linkSystemLibrary("oleaut32");
            lib.linkSystemLibrary("kernel32");
        },
        .linux => {
            lib.linkSystemLibrary("GL");
            lib.linkSystemLibrary("fontconfig");
            lib.linkSystemLibrary("freetype");
        },
        .macos => {
            lib.linkFramework("CoreFoundation");
            lib.linkFramework("CoreGraphics");
            lib.linkFramework("CoreText");
            lib.linkFramework("Metal");
        },
        else => {},
    }
}

fn buildDongCore(_: *std.Build, lib: *std.Build.Step.Compile) void {
    lib.addCSourceFiles(.{
        .files = &.{
            "src/api_bindings.cpp",
            "src/core/context.cpp",
            "src/core/view.cpp",
            "src/dom/dom_node.cpp",
            "src/dom/parser.cpp",
            "src/dom/dom_manager.cpp",
            "src/dom/style_engine.cpp",
            "src/dom/event_system.cpp",
            "src/layout/layout_engine.cpp",
            "src/render/render_surface.cpp",
            "src/render/painter.cpp",
            "src/render/skia_backend.cpp",
            "src/script/script_engine.cpp",
            "src/script/js_bindings.cpp",
        },
        .flags = &.{"-std=c++17"},
    });
}

fn linkPlatformLibs(lib: *std.Build.Step.Compile, target: std.Build.ResolvedTarget) void {
    switch (target.result.os.tag) {
        .windows => {
            lib.linkSystemLibrary("kernel32");
            lib.linkSystemLibrary("user32");
            lib.linkSystemLibrary("gdi32");
        },
        .linux => {
            lib.linkSystemLibrary("c");
            lib.linkSystemLibrary("m");
        },
        .macos => {
            lib.linkFramework("CoreFoundation");
            lib.linkFramework("CoreGraphics");
        },
        else => {},
    }
}

fn dirExists(handle: std.fs.Dir, path: []const u8) bool {
    var dir = handle.openDir(path, .{ .iterate = true }) catch return false;
    defer dir.close();
    return true;
}

fn buildDemos(b: *std.Build, lib: *std.Build.Step.Compile, target: std.Build.ResolvedTarget, optimize: std.builtin.OptimizeMode) void {
    const c_demos = &[_]struct { name: []const u8, file: []const u8 }{
        .{ .name = "simple_demo", .file = "examples/simple_demo.c" },
        .{ .name = "basic_demo", .file = "examples/basic_demo.c" },
    };

    const cpp_demos = &[_]struct { name: []const u8, file: []const u8 }{
        .{ .name = "complete_demo", .file = "examples/complete_demo.cpp" },
        .{ .name = "render_and_script_demo", .file = "examples/render_and_script_demo.cpp" },
    };

    inline for (c_demos) |demo| {
        buildSingleDemo(b, lib, target, optimize, demo.name, demo.file, false);
    }

    inline for (cpp_demos) |demo| {
        buildSingleDemo(b, lib, target, optimize, demo.name, demo.file, true);
    }
}

fn buildSingleDemo(
    b: *std.Build,
    lib: *std.Build.Step.Compile,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    demo_name: []const u8,
    demo_file: []const u8,
    is_cpp: bool,
) void {
    const exe = b.addExecutable(.{
        .name = demo_name,
        .target = target,
        .optimize = optimize,
        .root_source_file = null,
    });

    exe.linkLibC();
    exe.linkLibCpp();
    exe.addIncludePath(b.path("include"));
    exe.addIncludePath(b.path("src"));
    exe.addLibraryPath(b.path("third_party/skia/lib"));

    if (is_cpp) {
        addYogaIncludePaths(b, exe);
    }

    const flags = if (is_cpp)
        &[_][]const u8{ "-std=c++17", "-D_CRT_SECURE_NO_WARNINGS" }
    else
        &[_][]const u8{ "-D_CRT_SECURE_NO_WARNINGS" };

    exe.addCSourceFile(.{
        .file = b.path(demo_file),
        .flags = flags,
    });

    exe.linkLibrary(lib);

    if (target.result.os.tag == .windows) {
        exe.subsystem = .Console;
    }

    b.installArtifact(exe);

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step(
        b.fmt("run-{s}", .{demo_name}),
        b.fmt("Run the {s} demo", .{demo_name}),
    );
    run_step.dependOn(&run_cmd.step);
}
