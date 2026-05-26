# React / Preact 集成

Dong 内嵌 QuickJS，提供浏览器级 DOM/CSS/Layout。通过 **react-reconciler**（或 Preact 兼容层）可将 JSX 组件直接映射到 Dong DOM。

## 架构

```
用户 JSX → react / preact → Host Config 适配层 → Dong DOM API → Layout → GPU
```

- **不用 ReactDOM**：跳过浏览器 quirks，直接操作 Dong DOM
- **Bundle 外置构建**：esbuild 打成 IIFE，`<script src>` 加载到 QuickJS
- **Core 零修改**：适配在 JS 层；缺失 binding 按需补齐

完整 Host Config 规格见 [react-reconciler-spec.md](../developer/arch/react-reconciler-spec.md)。

## 快速运行

```bash
cd dong
zig build examples

# 提高脚本超时（bundle 较大）
set DONG_SCRIPT_TIMEOUT_MS=10000

cd zig-out/bin
dong_app.exe --html data/preact-counter/index.html
dong_app.exe --html data/react-counter/index.html
```

## 从源码构建 Bundle

需要 Node.js：

```bash
zig build preact   # 或 zig build react
```

源码位于 `dong/preact/`、`dong/react/`。

## 关键依赖 API

Preact/React 在 Dong 上运行需要以下 DOM/调度 API（均已实现或持续补齐）：

| API | 用途 |
|-----|------|
| `queueMicrotask` | Fiber scheduler |
| `MessageChannel` | scheduler 通信 |
| `document.createElement` / `appendChild` | 树操作 |
| `nodeValue` / `data` setter | 文本节点更新 |
| `addEventListener` | 事件（含大小写归一化） |
| `Element.animate()` | Web Animations |

## 调试技巧

- 点击无反应：检查 `nodeValue` 更新、事件名大小写、脚本超时
- 首屏慢：提高 `DONG_SCRIPT_TIMEOUT_MS`；考虑 P1-8 async shaping
- 渲染异常：用 `html_render_test` 多帧截图对比

## 相关文档

- [JavaScript API](../reference/js-api.md)
- [CSS 子集](../reference/css-subset.md)
- [调试指南](./debugging.md)
