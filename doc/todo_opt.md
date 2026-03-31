# Dong 双模渲染架构 — Pretext 思想在 Native 引擎的实现

## Pretext 宣言的本质与 dong 的映射

Pretext 解决的问题是：**Web 浏览器把 DOM 当作唯一的布局/测量通道**，导致任何文本操作都触发 style → layout → paint 全链路。Pretext 的做法：用 Canvas API 测量，用纯算术布局，DOM 只在最终渲染时才碰。

Dong 作为一个**自己实现了 DOM/CSS/Layout/Paint 的 native 引擎**，面临完全平行的问题：

| 浏览器的 DOM 税 | Dong 的等价开销 |
|:--|:--|
| DOM 操作触发 style recalc | 任何 `markStyleDirty` → 子树重算 |
| Style 触发 reflow | **任何节点脏 → 全量 Yoga 树重建** (最大问题) |
| Reflow 触发 repaint | **`commands_dirty` → 全量 `buildDisplayList` O(N)** |
| Repaint 触发 composite | **全量 `GPUCompiler::compile`** |

---

## 三层改进架构

### 层次 1: ✅ 已完成 — Bypass DOM (Direct Draw)

`dong.renderText()` + `dong.clearOverlay()` — 效果：从 6 FPS 到 400+ FPS。

### 层次 2: ✅ 已完成 — 减轻 DOM 税

**A. Immediate Mode API 扩展**

| API | 功能 | 参数 |
|:----|:-----|:-----|
| `dong.clearOverlay()` | 清除所有 overlay 项 | — |
| `dong.renderText({lines, font, color})` | 直接渲染文本（合批） | lines: [{x,y,text}] |
| `dong.drawRect({x,y,w,h,color,radius?,strokeWidth?})` | 矩形/圆角矩形 | 0 radius = 填充矩形 |
| `dong.drawCircle({cx,cy,r,color,strokeWidth?})` | 圆形 | strokeWidth>0 = 空心 |

实现文件:
- `src/render/overlay_draw.hpp` — OverlayDraw 单例
- `src/script/js_text_layout_bindings.cpp` — JS 绑定

**B. Display List skip-if-clean**

当 DOM 未变化（只有 overlay 变了）时，复用上一帧的 DOM display list，跳过整个 DOM 遍历 + paint 过程。

机制:
1. `tickGenerateCommandsIfNeeded()` 检查 `overlay_dirty` 和 `dom_dirty` 两个独立标志
2. 如果 `overlay_dirty && !dom_dirty && dom_display_list_valid_`:
   - 走**快速路径** `Render::overlayOnly`：直接 `restoreDisplayList` + `appendOverlayItems`
   - 跳过 `buildDisplayList(root, layout_engine)` 的 O(N) DOM 遍历
3. `markNeedsRepaint()` 同时 invalidate `dom_display_list_valid_`
4. 完整 DOM 渲染时自动缓存 display list items

实现文件:
- `src/core/engine_view.cpp` — skip-if-clean 快速路径
- `src/render/painter.hpp` — `restoreDisplayList()` 方法
- `src/render/display_list.hpp` — `appendItems()`, `clear()`

### 层次 3: ✅ 已完成 — 双模渲染引擎架构

```
dong engine (当前架构)
├── Retained Mode (DOM 层, 适用于结构化/交互性 UI)
│   ├── HTML/CSS → DOM → Style → Yoga Layout → Display List
│   ├── skip-if-clean: DOM 未变时复用缓存的 display list
│   └── 适用: 菜单、对话框、表单、静态 HUD
│
├── Immediate Mode (Overlay 层, 适用于高频动态内容)
│   ├── dong.drawRect()   — 矩形/圆角矩形
│   ├── dong.drawCircle() — 圆形
│   ├── dong.renderText() — 合批文本渲染（1 draw call）
│   ├── dong.clearOverlay() — 帧清除
│   └── 适用: 粒子效果、动画文字、实时图表
│
└── GPU Pipeline (共享)
    ├── Retained display list (缓存) + Immediate overlay (每帧重建)
    ├── GPUCompiler::compile → 统一 GPU command list
    └── GPU 执行
```

核心原则:
- **DOM 管结构，不管帧率** — DOM 用于定义 UI 的结构和交互
- **Immediate 管帧率，不管结构** — 高频更新内容直接走 GPU
- **两者共享一个 GPU 管线** — 统一的 display list + command list

---

## 测试用例

| 文件 | 用途 |
|:-----|:-----|
| `test_dual_mode.html` | **双模渲染**: 静态 DOM 框架 + 动态 overlay (粒子 + 文字流 + 障碍物) |
| `test_dual_mode_domonly.html` | **DOM-only 基线**: 同样的视觉效果，全部通过 DOM 操作实现 |

运行方式:
```bash
cd zig-out/bin
# 双模渲染 (Retained + Immediate)
dong_app.exe --html data\tests\test_dual_mode.html --width 960 --height 600 --no-vsync

# DOM-only 基线对照
dong_app.exe --html data\tests\test_dual_mode_domonly.html --width 960 --height 600 --no-vsync
```

---

## 未来改进方向

| 优先级 | 项目 | 说明 |
|:-------|:-----|:-----|
| 高 | Yoga 树持久化 | 不再每帧销毁重建 Yoga 树，用 `YGNodeMarkDirty` 增量更新 |
| 中 | `dong.drawImage()` | 补全图片绘制 API |
| 中 | draw call 合批通用化 | 将 overlay 的 glyph run 合并策略推广到 DOM 侧 |
| 低 | `dong.drawPath()` | 补全路径绘制 API |