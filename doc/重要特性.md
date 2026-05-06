# Dong 重要特性一览

## 渲染架构

### Scene Graph 模式

跳过 DOM/CSS/Yoga 全流程，HTML 编译为扁平场景节点，O(1) 属性更新 + AABB hit-test。适用于游戏 UI（所有元素 `position: absolute` + 显式像素坐标）。

- 启用：`<meta name="dong-render-mode" content="scene">`
- JS API：`dong.scene.find(name)`, `dong.scene.set(id, prop, val)`, `dong.scene.on(id, event, fn)`
- 实现：`src/dom/scene_compiler.cpp`, `src/render/scene_graph.cpp`, `src/script/js_scene_bindings.cpp`

| Case | 说明 |
|:-----|:-----|
| `test_scene_compiler.html` | Scene Compiler 基本编译 |
| `test_scene_graph.html` | Scene Graph 渲染 + JS 交互 |
| `game_ui2.html` | 完整游戏 UI (gamelikeui/) |

### 双模渲染 (Retained + Immediate)

DOM retained mode (结构化 UI) 与 overlay immediate mode (高频动态内容) 共享同一 GPU 管线。

- Immediate API：`dong.drawRect()`, `dong.drawCircle()`, `dong.renderText()`, `dong.clearOverlay()`
- 实现：`src/render/overlay_draw.hpp`, `src/script/js_text_layout_bindings.cpp`

| Case | 说明 |
|:-----|:-----|
| `test_dual_mode.html` | 静态 DOM 框架 + 动态 overlay (粒子 + 文字流 + 障碍物) |
| `test_dual_mode_domonly.html` | DOM-only 基线对照 |

### GPU 性能优化

- **Uber Quad 合批**：多个 rect/rounded/shadow 合并为一次 GPU instanced draw call
- **Display List skip-if-clean**：DOM 未变时复用缓存的 display list
- **Present-only 快速路径**：静态帧仅 blit intermediate → swapchain（~15x 提升）
- **局部重绘 (Partial Repaint)**：paint-only 样式变更仅重绘受影响子树
- 环境变量：`DONG_GPU_STATS=1`, `DONG_FORCE_FULL_REPAINT=1`
- 实现：`src/core/engine_view.cpp`, `backends/sdl/sdl_gpu_driver_execute.cpp`

| Case | 说明 |
|:-----|:-----|
| `test_partial_repaint.html` | 局部重绘验证 |

### GlobalShared (多 View 资源共享)

同进程多 View 共享 GlyphAtlas，23 屏幕场景节省 ~936MB GPU 内存。

- C API：`include/dong_global_shared.h`
- 实现：`src/core/global_shared.cpp`

### 9-Slice GPU 渲染

`border-image` 属性通过 9-slice shader 单次 draw call 渲染，四角不变形、中间拉伸。

- 实现：`backends/sdl/shaders/nineslice_fs.hlsl`, `src/render/painter.cpp`

| Case | 说明 |
|:-----|:-----|
| `test_nineslice_basic.html` | 基本 9-slice 渲染 |

### CSS Mask

- **快速路径**：conic-gradient mask + solid background 单 pass 合成
- **通用路径**：isolated layer + multiply blend 合成任意 mask

| Case | 说明 |
|:-----|:-----|
| `test_mask_conic_cooldown.html` | conic mask 快速路径 |
| `test_mask_general.html` | 通用 mask 路径 |

---

## 3D / 世界空间 API

### World Text Billboard

3D 世界空间文字渲染，支持距离衰减 alpha。

- C API：`include/dong_world_text.h`
- 实现：`src/render/world_text.cpp`

### World Decal

3D 投影贴花（地面标记等）。

- C API：`include/dong_decal.h` (`dong_decal_create`, `dong_decal_set_position`)

### World Overlay

世界空间中嵌入的 HTML overlay（如 3D 场景中的 UI 面板）。

- C API：`dong_world_overlay_create()`, `dong_world_overlay_load_html()`

### 3D Scene + HTML Screens

多个 HTML 渲染表面在 3D 场景中排列，支持第一人称摄像机漫游。

| Case | 说明 |
|:-----|:-----|
| `3d_screens_simple/main.c` | 3D Gallery (Preact + tests + emoji) |

---

## 资源打包 (.dpkg)

GPU 友好的资源包格式，支持字体/图片/HTML 打包为单文件，运行时一次加载。

- 打包工具：`python scripts/tools/pack_resources.py --font-file x.ttf -o out.dpkg`
- C API：`dong_resource_pack_load("file.dpkg")`, `dong_resource_pack_get_entry()`
- 实现：`include/dong_resource_pack.h`

---

## 向量动画 (Lottie / Rive)

### Lottie

JSON 动画格式加载与播放。

- C API：`dong_vector_load_file(engine, DONG_VECTOR_LOTTIE, "anim.json")`
- 播放：`dong_vector_play()`, `dong_vector_tick(va, dt)`

### Rive

Rive 二进制格式 + State Machine 驱动。

- C API：`dong_vector_load(engine, DONG_VECTOR_RIVE, data, len)`
- State Machine：`dong_vector_rive_play_state_machine(va, "Walk")`

---

## dong-ui 组件库

纯 CSS 游戏 UI 组件库，含三套主题。

- 主题：`data-theme="cyberpunk"` (默认暗色), `"medieval"` (暖棕), `"clean"` (浅色)
- 组件：Button, Card, Badge, Progress, Tabs, Menu, Modal, Toast, Tooltip...
- 实现：`examples/data/dong-ui/dong-ui.css`

| Case | 说明 |
|:-----|:-----|
| `dong-ui/demo.html` | 全组件 showcase |

---

## 开发者工具

### DevTools (F12)

内嵌 DOM 树 inspector overlay，支持节点高亮选中。

- 快捷键：F12 打开/关闭
- 实现：`src/devtools/`

### Live Reload (--watch)

文件修改后 ≤500ms 自动重载 HTML。

- CLI：`dong_app --html test.html --watch`
- 实现：`src/core/hot_reload.cpp`

### Profiler

Chrome Trace 格式性能分析。

- 环境变量：`DONG_PROFILER=1`, `DONG_PROFILER_OUTPUT=trace.json`
- 性能基准：`python scripts/tools/perf_baseline.py`

### JS Benchmark

QuickJS 引擎微基准测试套件。

- 工具：`python scripts/tools/js_bench.py`
- 12 项基准（DOM ops, event dispatch, style recalc 等）

---

## CSS 布局特性

### CSS Grid

支持 `fr`/`px`/`%`/`auto`/`repeat()`/`gap`/`grid-column`/`grid-row` 等现代 Grid 布局。

- 实现：`src/layout/grid_layout.cpp`

| Case | 说明 |
|:-----|:-----|
| `test_grid_basic.html` | 基本 Grid 布局 |

### position: sticky

布局时占位 + 渲染时视觉调整，支持嵌套 sticky、containing block 夹紧。

| Case | 说明 |
|:-----|:-----|
| `test_sticky_scroll_top.html` | 顶部吸附 |
| `test_sticky_scroll_bottom.html` | 底部吸附 |
| `test_sticky_parent_clamp.html` | 父容器夹紧 |
| `test_sticky_nested.html` | 嵌套 sticky |

### position: fixed

视口语义定位，滚动时固定不动。

| Case | 说明 |
|:-----|:-----|
| `test_position_fixed.html` | header/footer/FAB 固定 |

### aspect-ratio

Pre-layout 维度解析 (Yoga 之前)，支持 min/max 约束、replaced elements、flex 约束模式。

| Case | 说明 |
|:-----|:-----|
| `test_aspect_ratio_width_auto_height.html` | width + auto height |
| `test_aspect_ratio_min_max.html` | min/max 约束交互 |
| `test_aspect_ratio_flex.html` | flex 容器中的 aspect-ratio |

### display: contents / flow-root

- `contents`：Yoga 树手术，子节点附着到祖父节点
- `flow-root`：BFC margin collapse 阻断

| Case | 说明 |
|:-----|:-----|
| `test_display_contents_layout.html` | contents 布局 |
| `test_flow_root_margin_collapse.html` | flow-root margin collapse |

---

## CSS 样式特性

### 列表样式 & ::marker

支持 disc/circle/square/decimal/alpha/roman，`list-style-position: outside|inside`，`::marker` 独立样式。

| Case | 说明 |
|:-----|:-----|
| `test_list_markers_basic.html` | 基本标记渲染 |
| `test_list_style_types.html` | 各类型标记 |
| `test_marker_pseudo.html` | ::marker 伪元素样式 |
| `test_list_nested.html` | 嵌套列表计数器 |

### Gradient

| Case | 说明 |
|:-----|:-----|
| `test_conic_from_at.html` | conic-gradient 渲染 |
| `gradient_test.html` | linear-gradient |

### 其他 CSS

| Case | 说明 |
|:-----|:-----|
| `test_text_overflow_ellipsis.html` | text-overflow: ellipsis |
| `test_border_radius_percent.html` | 百分比圆角 |
| `text_shadow_test.html` | 文本阴影 |
| `transform_test.html` | CSS transform |
| `test_css_layer_basic.html` | @layer 层叠层 |
| `test_css_supports.html` | @supports 查询 |
| `test_logical_properties.html` | 逻辑属性 (inline/block) |
| `test_oklab_color.html` | oklab() 颜色 |
| `test_oklch_color.html` | oklch() 颜色 |
| `test_color_mix.html` | color-mix() |
| `test_css_hyphens_soft_hyphen.html` | 连字符断词 |

---

## 表单元素

### `<select>` 下拉列表

点击展开/收起、选项选择、键盘导航 (Arrow/Enter/Escape)、`change` 事件、JS API。

| Case | 说明 |
|:-----|:-----|
| `test_select_keyboard.html` | 键盘导航 |
| `test_form_element_bindings.html` | 表单绑定 |
| `test_textarea_element_bindings.html` | textarea 绑定 |

### `<input>` / `<textarea>`

光标定位、文本选择、编辑、maxlength、placeholder、readonly/disabled。

### 其他表单

disabled 元素不可交互（事件拦截 + 60% opacity），autofocus 自动聚焦。

---

## DOM / JavaScript

### 关键 API

| API | 说明 |
|:-----|:-----|
| `queueMicrotask` | React Fiber scheduler 必需 |
| `MessageChannel` / `MessagePort` | React scheduler 通信 |
| `localStorage` / `sessionStorage` | 内存实现 |
| `CustomEvent` | detail 属性支持 |
| `createDocumentFragment` | Fragment-aware appendChild |
| `performance.now()` | 高精度时间 |
| `crypto.randomUUID()` | UUID 生成 |
| `requestIdleCallback` | 空闲回调 |
| `Element.animate()` | Web Animations API |
| `Fetch API` | JSON 请求 |
| `DOMParser` | HTML 解析 |
| `MutationObserver` | DOM 变更观察 |

### ContentEditable

| Case | 说明 |
|:-----|:-----|
| `test_contenteditable_basic.html` | 基本编辑 |
| `test_contenteditable_features.html` | 综合功能 |

---

## 游戏引擎集成

### `<host-view>` 自定义元素

HTML 中的游戏引擎渲染占位符，游戏在该区域渲染自己的内容（小地图、3D 角色等）。

- 属性：`data-host-view-id="N"`
- 继承 opacity/transform
- C API：`dong_drawlist_get_commands()` 中 `DONG_DRAW_CMD_HOST_VIEW`

| Case | 说明 |
|:-----|:-----|
| `test_host_view.html` | host-view 占位渲染 |

### DongDrawList C ABI

Display list 导出为 C 结构供游戏引擎消费。

- C API：`include/dong_drawlist.h`
- 命令类型：rect, image, text, rounded_rect, shadow, gradient, host_view

### 手柄空间导航

D-pad/摇杆驱动焦点移动，支持 `data-nav-trap` 容器限定和 CSS `nav-*` 显式覆盖。

- C API：`dong_engine_send_gamepad_nav()`
- 实现：`src/dom/spatial_navigation.cpp`

| Case | 说明 |
|:-----|:-----|
| `test_spatial_nav_grid.html` | 网格导航 |
| `test_spatial_nav_explicit.html` | nav-* 覆盖 |
| `test_gamepad_full_flow.html` | nav-trap 完整流程 |

### IME 输入

支持中文/日文 IME composition 事件。

---

## 文本渲染

### 双文本渲染器 (MSDF + Slug)

- **MSDF**：Multi-Channel Signed Distance Field，atlas-based，任意缩放清晰
- **Slug**：分析覆盖率算法 (Eric Lengyel)，GPU 片段着色器内解析贝塞尔曲线
- 环境变量：`DONG_TEXT_RENDERER=slug|msdf|auto`
- 实现：`src/render/slug/`, `backends/sdl/shaders/slug_text_fs.hlsl`

### Color Emoji

支持全彩 emoji 渲染（COLR v0/v1、CBDT/CBLC、sbix），通过 `FT_LOAD_COLOR` 位图光栅化。

- shader 分支：`text_fs.hlsl` 中 `precomputedRange <= 0` 触发颜色位图直接输出
- 性能：非颜色字体 O(1) 快速拒绝
- 实现：`src/render/glyph_atlas.cpp` (`addColorGlyph`)

| Case | 说明 |
|:-----|:-----|
| `test_colr_emoji.html` | 多尺寸彩色 emoji + 混合文本 |
| `test_colr_v0_emoji.html` | COLR 层分解测试 |

### Text Shape Pre-warming

paint 前预热文字 shaping cache，减少首帧延迟。

### 文字流 & 排版

| Case | 说明 |
|:-----|:-----|
| `test_text_flow.html` | 障碍物绕排文字 |
| `chinese_font_large_test.html` | 中文大字号 |
| `test_font_weight_100_900.html` | 100-900 字重 |

---

## 多线程

### Thread-safe Text Shaping

TextShaper cache 线程安全，支持后台 worker 线程预填充 shape cache。

- 实现：`src/render/text_shaper.cpp` (mutex-protected cache)

---

## TypeScript 类型定义

`dong.d.ts` 提供完整 IDE 自动补全支持。

---

## HDR 输出

swapchain 支持 HDR 模式 (10-bit color, scRGB)。

- 配置：`enable_hdr=1`
- 需要 HDR 显示器

---

## 运行方式

```bash
cd dong/zig-out/bin

# Scene Graph 游戏 UI
dong_app.exe --html data\gamelikeui\game_ui2.html

# 双模渲染
dong_app.exe --html data\tests\test_dual_mode.html

# Color Emoji
dong_app.exe --html data\tests\test_colr_emoji.html

# Live Reload
dong_app.exe --html data\tests\test_colr_emoji.html --watch

# 3D Gallery
3d_screens_simple.exe

# headless 渲染
html_render_test.exe <html_file> [output.bmp] [width] [height]

# 资源打包
python scripts/tools/pack_resources.py --font-file font.ttf -o bundle.dpkg

# 性能基准
python scripts/tools/perf_baseline.py

# JS Benchmark
python scripts/tools/js_bench.py

# 批量渲染所有测试
zig build render-all-tests
```
