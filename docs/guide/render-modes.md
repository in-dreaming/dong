# 渲染模式

Dong 提供三种渲染模式，共享 GPU pipeline 与字体系统。

## DOM 模式（默认）

完整 DOM / CSS / Yoga / 事件冒泡路径。适用于 editor、复杂业务面板、React/Preact 应用。

```html
<!-- 默认，可省略 -->
<meta name="dong-render-mode" content="dom">
```

**示例**：`data/preact-counter/index.html`、`data/tests/test_sticky_scroll_top.html`

## Scene Graph 模式

跳过 CSS 选择器匹配 / Yoga / 事件冒泡。HTML 编译为扁平场景节点，O(1) 属性更新 + AABB hit-test。

**要求**：所有元素 `position: absolute` + 显式像素坐标。

```html
<meta name="dong-render-mode" content="scene">
```

**JS API**：

```javascript
dong.scene.find(name)
dong.scene.set(id, prop, val)
dong.scene.on(id, event, fn)
```

| Case | 说明 |
|------|------|
| `tests/test_scene_compiler.html` | Scene Compiler 基本编译 |
| `tests/test_scene_graph.html` | 渲染 + JS 交互 |
| `gamelikeui/game_ui2.html` | 完整游戏 UI |

实现：`src/dom/scene_compiler.cpp`、`src/render/scene_graph.cpp`

## Direct Draw（Overlay / Immediate）

无 DOM 节点，纯绘制命令注入 DisplayList。可与 DOM 或 Scene Graph **叠加**。

```javascript
dong.clearOverlay()
dong.drawRect(x, y, w, h, color)
dong.drawCircle(cx, cy, r, color)
dong.renderText(text, x, y, options)
```

| Case | 说明 |
|------|------|
| `tests/test_dual_mode.html` | DOM 框架 + overlay 粒子/文字流 |
| `pretext/test_text_flow_dynamic.html` | 动态文字绕排 |

实现：`src/render/overlay_draw.hpp`、`src/script/js_text_layout_bindings.cpp`

## 模式选择指南

| 需求 | 推荐模式 |
|------|----------|
| React/Preact 应用、表单、复杂 CSS | DOM |
| 固定布局游戏 HUD（格子、技能栏） | Scene Graph |
| 高频动态文字/粒子/debug  overlay | Direct Draw |
| DOM 框架 + 动态浮字 | DOM + Direct Draw |

## GPU 性能特性

- **Uber Quad 合批**：rect/round/shadow 合并为 instanced draw
- **Display List skip-if-clean**：DOM 未变时复用缓存
- **Present-only 快速路径**：静态帧仅 blit（~15x 提升）
- **局部重绘**：paint-only 变更仅重绘子树

调试：`DONG_GPU_STATS=1`、`DONG_FORCE_FULL_REPAINT=1`

详见 [渲染管线优化](../architecture/render-pipeline.md)。

## 多 View 资源共享

同进程多 View 共享 GlyphAtlas（`dong_global_shared.h`），23 屏幕场景可节省大量 GPU 内存。

## 3D 场景中的 HTML 屏幕

`3d_screens_simple` 示例展示多个 HTML 表面排列在 3D 场景中，支持第一人称漫游。

使用 overlay API 的 screen 会以更高频率刷新（默认 30Hz，可通过 `DONG_SCENE3D_OVERLAY_TICK_HZ` 调整）。

## 世界空间 API（C）

| API | 用途 |
|-----|------|
| `dong_world_text.h` | 3D billboard 文字 |
| `dong_decal.h` | 投影贴花 |
| `dong_world_overlay.h` | 世界空间 HTML overlay |

## 相关文档

- [定位与边界](../overview/positioning.md) § 三轨定档
- [JavaScript API](../reference/js-api.md)
- [架构概览](../architecture/overview.md)
