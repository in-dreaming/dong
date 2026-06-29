# Dong

GPU-accelerated HTML/CSS UI engine for games and interactive applications.

**Dong** 是一套用 HTML/CSS/DOM 子集描述、GPU 直接渲染的游戏 UI 引擎，面向游戏 editor 与 runtime（含 3D HUD / 世界空间 UI），不是浏览器。

[快速开始](./docs/getting-started/quickstart.md) · [文档索引](./docs/README.md) · [路线图](./docs/roadmap.md) · [贡献指南](./CONTRIBUTING.md)

---

## 特性

- **三轨渲染**：DOM（默认）、Scene Graph（高密度 HUD）、Direct Draw（即时 overlay）
- **现代 CSS 子集**：Flexbox、Grid、sticky/fixed、gradient、mask、animation 等
- **JavaScript**：QuickJS 引擎，支持 React / Preact 示例
- **文本渲染**：MSDF + Slug 双渲染器，Color Emoji 支持
- **嵌入友好**：平台无关 Core（`dong.dll`）+ 可插拔 Backend
- **3D 集成**：多 HTML 屏幕、World Text / Decal / Overlay C API
- **开发工具**：DevTools (F12)、Live Reload、Chrome Trace Profiler

## 快速开始

前置条件：[Zig](https://ziglang.org/) 0.16、CMake、Ninja、Python 3.10+（Windows 另需 Visual Studio Build Tools）。

```bash
cd dong
zig build examples   # 默认全量示例（首次会重配 build-cmake-sdl）

# Windows
.\zig-out\bin\3d_screens_simple.exe
.\zig-out\bin\dong_app.exe --html data\preact-counter\index.html

# Linux / macOS
./zig-out/bin/dong_app --html data/preact-counter/index.html
```

React/Preact  bundle 较大，建议设置脚本超时：

```bash
# Windows
set DONG_SCRIPT_TIMEOUT_MS=10000

# Linux / macOS
export DONG_SCRIPT_TIMEOUT_MS=10000
```

更多示例见 [快速开始指南](./docs/getting-started/quickstart.md)。

backend
```bash
zig build -Dbackend=sdl|gpu|none
zig build dong-core -Dbackend=none    # 已通过
zig build gpu-backend -Dbackend=gpu   # 已通过（stub DLL）
zig build examples -Dbackend=sdl   
```

## 示例程序

| 程序 | 说明 |
|------|------|
| `dong_app` | 通用 HTML 查看器（`--html` / `--watch`） |
| `minimal_dong_demo` | 最小 AppCore 示例（~50 行） |
| `interactive_demo_new` | 交互式 demo（~200 行） |
| `3d_screens_simple` | 3D 场景中嵌入多个 HTML 屏幕 |
| `html_render_test` | 无头渲染，输出 BMP 截图 |

## 文档

| 文档 | 说明 |
|------|------|
| [docs/README.md](./docs/README.md) | 文档总索引 |
| [定位与边界](./docs/overview/positioning.md) | Dong 是什么、不是什么 |
| [渲染模式](./docs/guide/render-modes.md) | DOM / Scene / Overlay 三轨 |
| [架构概览](./docs/architecture/overview.md) | 库分层与渲染管线 |
| [JS API 参考](./docs/reference/js-api.md) | `dong.*` 与 DOM 扩展 |
| [CSS 子集](./docs/reference/css-subset.md) | 支持的 CSS 属性与限制 |
| [构建指南](./docs/getting-started/build-from-source.md) | 完整构建说明 |

维护者文档（Phase spec、编排、性能预算）见 [docs/developer/](./docs/developer/)。

## 项目状态

**Alpha** — API 与行为仍可能变动。详见 [路线图](./docs/roadmap.md)。

## 许可证

[Dong 本体](./LICENSE) 采用 MIT 许可证。第三方依赖各自遵循其原始许可证，见 [docs/reference/third-party-licenses.md](./docs/reference/third-party-licenses.md)。

## 贡献

欢迎提交 Issue 与 Pull Request。请先阅读 [CONTRIBUTING.md](./CONTRIBUTING.md)。

AI 编码助手请参阅 [AGENTS.md](./AGENTS.md)（与 CONTRIBUTING 互补，面向自动化工具）。
