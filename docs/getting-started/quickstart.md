# 快速开始

本指南帮助你在 5 分钟内构建并运行 Dong 示例。

## 前置条件

- [Zig](https://ziglang.org/) 0.14+
- CMake、Ninja
- **Windows**：Visual Studio Build Tools（`clang-cl`）
- **macOS / Linux**：clang 或 gcc

可选：Node.js（从源码构建 React/Preact bundle 时需要）

## 构建

```bash
git clone <repo-url>
cd dong/dong          # 进入构建根目录
zig build examples
```

构建产物位于 `zig-out/bin/`。

首次构建会编译第三方依赖（QuickJS、Lexbor、Yoga、SDL3 等），可能需要数分钟。

## 运行 dong_app

`dong_app` 是通用 HTML 查看器：

```bash
cd zig-out/bin

# Windows
.\dong_app.exe --html data\preact-counter\index.html

# Linux / macOS
./dong_app --html data/preact-counter/index.html
```

点击 `+` / `-` 按钮验证交互。Preact bundle 较大，建议提高脚本超时：

```bash
# Windows
set DONG_SCRIPT_TIMEOUT_MS=10000

# Linux / macOS
export DONG_SCRIPT_TIMEOUT_MS=10000
```

## 推荐 Demo

| 命令 | 说明 |
|------|------|
| `dong_app --html data/preact-counter/index.html` | Preact 计数器 |
| `dong_app --html data/gamelikeui/game_ui2.html` | 游戏 UI（Scene Graph） |
| `dong_app --html data/tests/test_dual_mode.html` | DOM + Overlay 双模 |
| `dong_app --html data/video/video_play_test.html` | 视频播放 |
| `dong_app --html data/dong-ui/demo.html` | dong-ui 组件库 |
| `3d_screens_simple` | 3D 场景多 HTML 屏幕 |

## Live Reload

开发时监听文件变更并自动重载：

```bash
dong_app --html data/tests/test_colr_emoji.html --watch
```

## 无头渲染（截图）

不启动窗口，输出 BMP 截图：

```bash
# 通过 zig build 调用
cd ../../..   # 回到 dong/ 目录
zig build run-html-test -- data/tests/test_grid_basic.html output.bmp 800 600

# 或直接运行二进制
./zig-out/bin/html_render_test data/tests/test_grid_basic.html output.bmp 800 600
```

## 下一步

- [从源码构建](./build-from-source.md) — 交叉编译、build.env 配置
- [渲染模式](../guide/render-modes.md) — 选择 DOM / Scene / Overlay
- [架构概览](../architecture/overview.md) — 理解库分层
