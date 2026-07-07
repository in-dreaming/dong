# 在 Zig 项目中依赖 Dong

Dong 提供正式 Zig Package（[`dong/build.zig.zon`](../../../dong/build.zig.zon)），可通过 `zig fetch` / path dependency 引入。

## 克隆与子模块

```bash
git clone --recurse-submodules <dong-repo-url>
cd dong/dong

# 若已克隆但未初始化子模块
git submodule update --init --recursive
```

`third_party/gpu` 内含嵌套子模块（slang-rhi、SDL 等），**必须** `--recursive`。

## 消费方 `build.zig.zon`

```zon
.{
    .name = .my_game,
    .version = "0.1.0",
    .dependencies = .{
        .dong = .{
            .path = "../vendor/dong/dong",
            // 或 git 依赖：
            // .url = "https://github.com/in-dreaming/dong.git",
            // .hash = "...",
        },
    },
    .paths = .{ "" },
}
```

## 消费方 `build.zig`

```zig
const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const dong = b.dependency("dong", .{
        .target = target,
        .optimize = optimize,
        .backend = .none, // 仅 Core；或 .sdl / .gpu
        .package_only = true,
    });

    const exe = b.addExecutable(.{
        .name = "my_app",
        .root_module = b.createModule(.{
            .root_source_file = b.path("src/main.zig"),
            .target = target,
            .optimize = optimize,
        }),
    });

    exe.root_module.addImport("dong", dong.module("dong"));
    exe.root_module.linkLibrary(dong.artifact("dong_core"));

    // SDL backend 时额外链接（Zig 侧静态库 + CMake 运行时 DLL）
  // exe.root_module.linkLibrary(dong.artifact("dong_sdl_backend"));

    b.installArtifact(exe);
}
```

## 构建选项

| 选项 | 值 | 说明 |
|------|-----|------|
| `-Dbackend` | `sdl` / `gpu` / `none` | 图形后端（桌面默认 `sdl`） |
| `-Dbackend=none` | | 仅 `dong_core` + 第三方静态库 |
| `-Dlibs-only=true` | | 交叉编译 / 移动平台，同 `backend=none` |
| `-Dpackage-only=true` | | 跳过 examples/apps（依赖方推荐） |
| `-Dsdl=false` | | **已废弃**，请用 `-Dbackend=none` |

## 传递依赖说明

`dong_core` 依赖 Lexbor、Yoga、FreeType、HarfBuzz、msdfgen 等静态库（脚本引擎 Porffor 的 AOT 编译产物直接编译进 `dong_core`，不是独立静态库）。作为 Zig 依赖构建时，这些库会安装到依赖包的 `zig-out/lib/`。消费方通常需要：

1. 链接 `dong.artifact("dong_core")`
2. 按目标平台补链 `lexbor`、`yoga` 等同前缀下的库（或预构建 `zig build package` SDK）

推荐引擎集成场景使用：

```bash
zig build -Dlibs-only=true -Dbackend=none package
```

得到仅含 `include/` + `lib/` 的 SDK 压缩包。

## Backend 与 SDL 子树

| backend | SDL 来源 |
|---------|----------|
| `sdl` | `dong/third_party/sdl` @ `enjin/gpu/main` |
| `gpu` | `dong/third_party/gpu/modules/3rd/sdl`（gpu 仓库自带） |
| `none` | 不构建 SDL |

`gpu` backend 当前为 **stub**（`dong_gpu_backend` 占位实现）；完整 `in-dreaming/gpu` RHI 在 `third_party/gpu` 目录独立构建，尚未嵌入 dong 主 CMake 图。

## 本地验证

```bash
cd dong
zig build dong-core -Dbackend=none
zig build gpu-backend -Dbackend=gpu
zig build examples -Dbackend=sdl
```
