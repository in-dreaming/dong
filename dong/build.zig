const std = @import("std");

fn addLexborFiles(lib: *std.Build.Step.Compile, _: *std.Build) void {
    const lexbor_files = &.{
        "third_party/lexbor/source/lexbor/css/at_rule.c",
        "third_party/lexbor/source/lexbor/css/css.c",
        "third_party/lexbor/source/lexbor/css/declaration.c",
        "third_party/lexbor/source/lexbor/css/parser.c",
        "third_party/lexbor/source/lexbor/css/property.c",
        "third_party/lexbor/source/lexbor/css/rule.c",
        "third_party/lexbor/source/lexbor/css/stylesheet.c",
        "third_party/lexbor/source/lexbor/css/unit.c",
        "third_party/lexbor/source/lexbor/css/value.c",
        "third_party/lexbor/source/lexbor/encoding/encoding.c",
        "third_party/lexbor/source/lexbor/html/parser.c",
        "third_party/lexbor/source/lexbor/html/tokenizer.c",
        "third_party/lexbor/source/lexbor/html/tree.c",
        "third_party/lexbor/source/lexbor/html/serialize.c",
        "third_party/lexbor/source/lexbor/html/interface.c",
        "third_party/lexbor/source/lexbor/dom/collection.c",
        "third_party/lexbor/source/lexbor/dom/interface.c",
        "third_party/lexbor/source/lexbor/selectors/selectors.c",
        "third_party/lexbor/source/lexbor/core/array.c",
        "third_party/lexbor/source/lexbor/core/array_obj.c",
        "third_party/lexbor/source/lexbor/core/avl.c",
        "third_party/lexbor/source/lexbor/core/bst.c",
        "third_party/lexbor/source/lexbor/core/hash.c",
        "third_party/lexbor/source/lexbor/core/mem.c",
        "third_party/lexbor/source/lexbor/core/mraw.c",
        "third_party/lexbor/source/lexbor/core/str.c",
        "third_party/lexbor/source/lexbor/ns/ns.c",
    };
    lib.addCSourceFiles(.{
        .files = lexbor_files,
        .flags = &.{"-DLEXBOR_STATIC"},
    });
}

fn addYogaFiles(lib: *std.Build.Step.Compile, _: *std.Build) void {
    const yoga_files = &.{
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
    };
    lib.addCSourceFiles(.{
        .files = yoga_files,
        .flags = &.{"-std=c++20", "-fno-exceptions", "-fno-rtti"},
    });
}

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    
    const lib = b.addStaticLibrary(.{
        .name = "dong",
        .target = target,
        .optimize = optimize,
        .root_source_file = null,
    });
    
    lib.linkLibCpp();
    lib.linkLibC();
    
    // Include paths
    lib.addIncludePath(b.path("include"));
    lib.addIncludePath(b.path("src"));
    lib.addIncludePath(b.path("third_party/quickjs"));
    lib.addIncludePath(b.path("third_party/lexbor/source"));
    lib.addIncludePath(b.path("third_party/yoga"));
    lib.addIncludePath(b.path("third_party/skia/include"));
    
    // ============================================================
    // QuickJS (C sources)
    // ============================================================
    lib.addCSourceFiles(.{
        .files = &.{
            "third_party/quickjs/quickjs.c",
            "third_party/quickjs/libregexp.c",
            "third_party/quickjs/libunicode.c",
            "third_party/quickjs/cutils.c",
            "third_party/quickjs/dtoa.c",
        },
        .flags = &.{"-D_GNU_SOURCE", "-DCONFIG_VERSION=\"2024-01\""},
    });
    
    // ============================================================
    // Dong main sources (C++)
    // ============================================================
    lib.addCSourceFiles(.{
        .files = &.{
            "src/api_bindings.cpp",
        },
        .flags = &.{"-std=c++17"},
    });
    
    // ============================================================
    // Lexbor (C sources)
    // ============================================================
    addLexborFiles(lib, b);
    
    // ============================================================
    // Yoga (C++ sources)
    // ============================================================
    addYogaFiles(lib, b);
    
    // ============================================================
    // Skia (prebuilt, link necessary libraries)
    // ============================================================
    const skia_lib_path = "third_party/skia/mac/out/Debug-arm64";
    lib.addLibraryPath(b.path(skia_lib_path));
    
    // Skia header includes
    lib.addIncludePath(b.path("third_party/skia/mac/include"));
    lib.addIncludePath(b.path("third_party/skia/mac/modules/skshaper/include"));
    lib.addIncludePath(b.path("third_party/skia/mac/modules/skparagraph/include"));
    lib.addIncludePath(b.path("third_party/skia/mac/modules/skottie/include"));
    lib.addIncludePath(b.path("third_party/skia/mac/modules/sksg/include"));
    lib.addIncludePath(b.path("third_party/skia/mac/modules/skunicode/include"));
    lib.addIncludePath(b.path("third_party/skia/mac/modules/skresources/include"));
    lib.addIncludePath(b.path("third_party/skia/mac/third_party/externals/freetype/include"));
    
    // Link all Skia libraries (must link in a specific order)
    lib.linkSystemLibrary("skia");
    lib.linkSystemLibrary("skottie");
    lib.linkSystemLibrary("skparagraph");
    lib.linkSystemLibrary("skshaper");
    lib.linkSystemLibrary("skunicode");
    lib.linkSystemLibrary("sksg");
    lib.linkSystemLibrary("skresources");
    lib.linkSystemLibrary("harfbuzz");
    lib.linkSystemLibrary("freetype2");
    lib.linkSystemLibrary("icu");
    lib.linkSystemLibrary("skcms");
    lib.linkSystemLibrary("wuffs");
    lib.linkSystemLibrary("svg");
    lib.linkSystemLibrary("webp");
    lib.linkSystemLibrary("webp_sse41");
    lib.linkSystemLibrary("png");
    lib.linkSystemLibrary("jpeg");
    lib.linkSystemLibrary("zlib");
    lib.linkSystemLibrary("expat");
    lib.linkSystemLibrary("piex");
    lib.linkSystemLibrary("bentleyottmann");
    lib.linkSystemLibrary("spvtools");
    lib.linkSystemLibrary("spvtools_val");
    
    // macOS frameworks required by Skia
    lib.linkFramework("CoreFoundation");
    lib.linkFramework("CoreGraphics");
    lib.linkFramework("CoreText");
    lib.linkFramework("Metal");
    lib.linkFramework("ApplicationServices");
    
    b.installArtifact(lib);
}
