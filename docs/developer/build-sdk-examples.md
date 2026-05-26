# 通过 Zig 构建 Dong SDK + Examples（并打包）

## 前置条件

- **所有命令必须在 `dong/` 目录下执行**
- Windows 推荐使用 PowerShell
- 已安装 Zig（本仓库当前验证通过：`zig 0.15.2`）

> Windows 下如果你习惯用 `&&` 串联命令：PowerShell 不支持 `&&`，请用 `;` 分隔，或分两行执行。

## 一键构建（SDK + examples）

在 `dong/` 下执行：

```bash
zig build examples
```

该命令会构建并把产物安装到 `dong/zig-out/`：

- `zig-out/include/`：SDK 头文件（`dong.h`、`dong_global_shared.h`、`dong/appcore/*.h` 等）
- `zig-out/lib/`：链接库（`.lib`/`.a` 等）
- `zig-out/bin/`：examples 可执行文件 + 运行时依赖（例如 `dong.dll`、`dong_sdl_backend.dll`、`SDL3.dll`、可选的 FFmpeg dll）以及 `data/`

常用的 examples（Windows）通常在：

- `zig-out/bin/minimal_dong_demo.exe`
- `zig-out/bin/interactive_demo_new.exe`
- `zig-out/bin/html_render_test.exe`（headless 渲染）

## 打包成可分发目录

在 `dong/` 下执行：

```bash
zig build package
```

输出目录：

- `dong/zig-out/dist/dong_sdk_examples_<target-triple>/`

目录结构固定为：

- `include/`
- `lib/`
- `bin/`（包含 examples、运行时 dll、`data/`）

## 生成 zip 包

在 `dong/` 下执行：

```bash
zig build package-zip
```

输出文件：

- `dong/zig-out/dist/dong_sdk_examples_<target-triple>.zip`

## 常见选项

- **禁用 FFmpeg（减少依赖/体积）**：

```bash
zig build examples -Dffmpeg=false
```

- **仅构建第三方依赖**：

```bash
zig build deps
```

## Cross Compile（交叉编译）

### 支持范围（重要）

- **Windows 主机交叉到 Linux / Android / iOS 等目标时**，目前推荐使用 `-Dlibs-only=true`（只产出静态库/导入库），用于集成到你的游戏/引擎工程。
- 完整的 `examples`（含 SDL3/CMake、可执行文件、运行时 dll）通常要求 **native** 构建；跨到非宿主平台时不保证可用。

### Linux x64 (glibc) 示例

```bash
zig build -Dtarget=x86_64-linux-gnu -Dlibs-only=true -p zig-out-linux-x64 deps
```

产物在：
- `zig-out-linux-x64/include/`
- `zig-out-linux-x64/lib/`

### Linux ARM64 (glibc) 示例

```bash
zig build -Dtarget=aarch64-linux-gnu -Dlibs-only=true -p zig-out-linux-arm64 deps
```

### musl（静态链接更友好）示例

```bash
zig build -Dtarget=x86_64-linux-musl -Dlibs-only=true -p zig-out-linux-musl-x64 deps
zig build -Dtarget=aarch64-linux-musl -Dlibs-only=true -p zig-out-linux-musl-arm64 deps
```

## Cross Package（交叉打包）

`package/package-zip` 已支持 **跟随 `-p/--prefix`**，因此 cross compile 时可以直接打包对应前缀的 SDK。

例如：把 Linux x64 的 libs-only SDK 打包成目录 + zip：

```bash
zig build -Dtarget=x86_64-linux-gnu -Dlibs-only=true -p zig-out-linux-x64 package
zig build -Dtarget=x86_64-linux-gnu -Dlibs-only=true -p zig-out-linux-x64 package-zip
```

输出：
- `zig-out-linux-x64/dist/dong_sdk_x86_64-linux-gnu/`（只包含 `include/` + `lib/`）
- `zig-out-linux-x64/dist/dong_sdk_x86_64-linux-gnu.zip`

## 运行示例

构建完成后，一般在 `dong/zig-out/bin/` 里直接运行：

```bash
./interactive_demo_new.exe
```

指定 HTML（示例）：

```bash
./dong_app.exe --html data\tests\test_dual_mode.html
```

## 备注：`build.env`

仓库支持通过 `dong/build.env` 配置 Vulkan SDK、DXC 等路径。若没有该文件，会使用默认行为并打印提示。

