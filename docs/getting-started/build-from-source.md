# 从源码构建

所有命令在 `dong/` 子目录执行（Zig 构建根目录）。

## 基本命令

```bash
cd dong

zig build                    # 构建全部（deps + core + examples）
zig build examples           # 构建示例到 zig-out/bin
zig build deps               # 仅第三方依赖
```

### 图形后端（`-Dbackend`）

| 值 | 说明 |
|----|------|
| `sdl`（桌面默认） | SDL3 + `dong_sdl_backend` + 全部 examples |
| `gpu` | `dong_gpu_backend` stub + Core（**无** SDL examples） |
| `none` | 仅 `dong_core` 与第三方静态库 |

```bash
zig build examples -Dbackend=sdl          # 默认
zig build dong-core -Dbackend=none
zig build gpu-backend -Dbackend=gpu
```

`-Dsdl=false` 已废弃，请改用 `-Dbackend=none`。

外部 Zig 项目集成见 [Zig Package 集成](../guide/integrations/zig-package.md)。

## 子模块

```bash
git submodule update --init --recursive
```

- SDL：`dong/third_party/sdl` 跟踪分支 **`enjin/gpu/main`**
- GPU：`dong/third_party/gpu` 跟踪分支 **`main`**（仓库无 `master` 分支）

`gpu` 含嵌套子模块，必须 `--recursive`。

优化级别：

```bash
zig build examples -Doptimize=Debug
zig build examples -Doptimize=ReleaseSmall
zig build examples -Doptimize-size=true   # 二进制体积优化
```

## 单独构建依赖

| 命令 | 说明 |
|------|------|
| `zig build quickjs` | QuickJS |
| `zig build lexbor` | HTML 解析 |
| `zig build yoga` | Flexbox 布局 |
| `zig build freetype` | 字体 |
| `zig build harfbuzz` | 文本整形 |
| `zig build msdfgen` | MSDF 字体 |
| `zig build sdl3` | SDL3（CMake） |
| `zig build dong-core` | Dong Core 库 |
| `zig build sdl-backend` | SDL 后端（`-Dbackend=sdl`） |
| `zig build gpu-backend` | GPU 后端 stub（`-Dbackend=gpu`） |

## React / Preact

预构建 bundle 在 `zig build examples` 时自动复制。从源码构建需 Node.js：

```bash
zig build react    # React 示例 bundle
zig build preact   # Preact 示例 bundle
```

## 配置文件

复制 `build.env.example` 为 `build.env`，配置平台路径：

| 文件 | 平台 |
|------|------|
| `build.env.windows.example` | Windows |
| `build.env.macos.example` | macOS |
| `build.env.android.example` | Android 交叉编译 |
| `build.env.ios.example` | iOS |
| `build.env.ohos.example` | HarmonyOS |

常用变量：`VULKAN_SDK_PATH`、`DXC_LIB_PATH` 等。

## Windows 注意事项

- 使用 `clang-cl` + Ninja 进行 CMake 构建
- WSL 中若找不到 zig，使用 `cmd.exe /c zig build`
- Release 模式构建以避免 `_ITERATOR_DEBUG_LEVEL` 不匹配
- DXC 库默认路径：`third_party/dxc_2025_07_14/lib/x64`

下载 DXC：

```bash
python scripts/download_dxc.py --version 2025_07_14
```

## 交叉编译

从 Windows 交叉编译 Linux：

```bash
zig build -Dtarget=x86_64-linux-gnu -Dlibs-only=true -p zig-out-linux-x64 deps
zig build -Dtarget=aarch64-linux-gnu -Dlibs-only=true -p zig-out-linux-arm64 deps
zig build -Dtarget=x86_64-linux-musl -Dlibs-only=true -p zig-out-linux-musl-x64 deps
```

辅助脚本：

```bash
python scripts/cross_compile.py all
python scripts/cross_compile.py linux-arm64
python scripts/cross_compile.py list
```

macOS/iOS 交叉编译需要 macOS SDK（仅在 macOS 主机可用）。

移动目标自动进入 `libs-only` 模式，输出静态库供原生 App 集成。

## 着色器预编译

移动平台 SPIRV 预编译：

```bash
python scripts/precompile_shaders.py --vulkan
python scripts/precompile_shaders.py --vulkan --header
python scripts/precompile_shaders.py --metal   # 需要 spirv-cross
```

## 开发工具

```bash
zig build run-feature-tests     # 功能测试套件
zig build render-all-tests      # 批量渲染 examples/data/tests/
zig build run-html-test -- <html> [out.bmp] [w] [h]
```

## 依赖构建方式

| 依赖 | 构建方式 |
|------|----------|
| QuickJS, Lexbor, Yoga, FreeType, HarfBuzz, msdfgen | Pure Zig |
| Dong Core, SDL Backend | Pure Zig (C++20) |
| SDL3 | CMake |

## 输出路径

使用绝对路径指定输出目录，避免嵌套 `zig-out/zig-out/` 路径。
