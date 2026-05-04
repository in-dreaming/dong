# Dong 当前架构速览（给 vibecode/新同学）

> 目标：把一段 HTML/CSS/JS 在宿主（独立 App / 游戏引擎 / 3D Screen）里渲染成 2D UI，并支持输入事件、脚本与离屏渲染。

## 1. 顶层分层（从宿主往里看）

- **宿主/应用层**：examples、`dong_app`、或外部引擎。
- **公共 C ABI（推荐入口）**：`dong/include/dong.h`
  - `dong_engine_*` 系列（插件化方向，长期目标是 `dong.dll` 不依赖 SDL）。
- **Legacy View ABI（大量 demo/工具仍在用）**：`dong/include/dong_legacy_view.h`
  - `dong_view_*` 系列，内部直接封装 `dong::View`。

> 现实情况：当前仓库里很多 demo（包括 3D screen）仍通过 legacy API 驱动；新 ABI 主要用于“插件化重构”的过渡。

## 2. 核心模块与职责（代码目录导览）

### 2.1 核心调度：`dong::View`
- **位置**：`dong/src/core/view.hpp` / `dong/src/core/view.cpp`
- **职责**：
  - 维护 DOM、样式、布局、渲染管线对象（`dom::Manager`、`StyleEngine`、`layout::Engine`、`render::Painter`、GPU 相关对象等）。
  - 接收宿主输入（mouse/key/text/wheel），做 hit-test、事件分发、焦点管理。
  - 驱动渲染：swapchain（`update()`）或离屏（`renderToGPUTexture()` / `renderOffscreen()`）。

### 2.2 DOM/HTML：`dom::Manager` + 解析器
- **位置**：`dong/src/dom/dom_manager.*`、`dong/src/dom/html/*`
- **职责**：
  - `loadHTML()` 解析 HTML → 构建 DOM 树（底层依赖 lexbor 等）。
  - 提供简单查询/修改接口（by id/class/tag）。
  - 暴露 `StyleEngine` 用于样式计算。

### 2.3 CSS：`StyleEngine` / `SelectorMatcher` / `CSSParser`
- **位置**：`dong/src/dom/css/*`
- **职责**：
  - 解析 CSS rule、计算 specificity/cascade。
  - `computeStyles(node)`：从 root 开始递归计算 computed style。
  - 支持部分伪类：`:hover/:active/:focus`（依赖运行时状态，由 `View` 维护命中元素状态）。
  - 内置了一个“最小 UA 样式”（例如 `button` 的 `outset/inset`），用于让表单控件更接近浏览器。

### 2.4 布局：`layout::Engine`
- **位置**：`dong/src/layout/layout_engine.*`
- **职责**：
  - 以 Yoga 为主的布局计算。
  - 产出每个 DOMNode 的 layout box（x/y/w/h），供 hit-test 与绘制使用。

### 2.5 渲染：DisplayList / LayerTree / GPU IR
- **DisplayList 构建（DOM/Layout → 绘制序列）**
  - **位置**：`dong/src/render/painter.*`、`dong/src/render/display_list.hpp`
  - **职责**：遍历 DOM + layout，生成 DisplayList；同时维护 LayerTree（隔离层/合成相关）。
- **GPU 中立命令（DisplayList → GPUCommandList）**
  - **位置**：`dong/src/render/gpu_ir.hpp`
  - **职责**：`GPUCompiler` 把 DisplayList 编译成 `GPUCommandList`（包含文本/图片/圆角/阴影/裁剪/隔离层等命令）。

### 2.6 GPU 后端：`GPUDriverSDL`
- **位置**：`dong/src/render/gpu_driver_sdl.*` + `..._execute.cpp` / `..._resources.cpp`
- **职责**：
  - 基于 SDL3 `SDL_gpu` 执行 `GPUCommandList`。
  - 管线：rect / rounded-rect / shadow / image / text。
  - 资源管理：图片 atlas、GlyphAtlas（FreeType / HarfBuzz / MSDF 等）、shader 管理。
  - 支持离屏渲染（render-to-texture）。

### 2.7 脚本：QuickJS + JSBindings
- **位置**：`dong/src/script/*`
- **职责**：
  - `ScriptEngine` 封装 QuickJS：eval、call、task pump。
  - `JSBindings`：把 DOM/事件系统桥接到 JS（`document`/`element`/`addEventListener`/`console` 等）。

### 2.8 事件系统：`EventDispatcher` / Focus
- **位置**：`dong/src/dom/event_system.*`、`dong/src/dom/focus_manager.*`
- **职责**：
  - C++ 侧事件派发与 bubbling。
  - `JSBindings` 按需把 C++ 事件转发到 JS listener。

## 3. 一帧内的数据流（最常用的“从输入到像素”路径）

### 3.1 Swapchain 渲染（典型窗口内 UI）
1) 宿主把输入喂给 `dong_view_send_*`（legacy）或 `dong_engine_send_*`（新 ABI）。
2) `dong::View::handle_*`：
   - hit-test 找到 DOM 节点
   - 更新运行时伪类状态（`:hover/:active/:focus`）
   - `markNeedsRepaint()` → 标记需要重建命令
   - 通过 `EventDispatcher` 分发事件（并可能触发 JS 回调）
3) `dong::View::update()`：
   - 若 `commands_dirty_`，会先 `computeStyles()` → `layout_engine->calculateLayout()` → `Painter::buildDisplayList()`
   - `GPUCompiler` 编译成 `GPUCommandList`
   - `GPUDriverSDL` 执行，最终提交到 swapchain。

### 3.2 离屏渲染（3D Screen / 截图对比 / 多帧采样）
- 入口：`dong::View::renderToGPUTexture()`（以及 legacy 的 `dong_view_render_to_gpu_texture`）。
- 关键差异：离屏渲染会强制做一次完整布局；并且需要 **确保样式也被重新计算**（尤其是 `:active` 这类运行态依赖）。
- 产物：一个 `SDL_GPUTexture*`（调用方负责释放）。

## 4. 3D Screen 集成（你最近在用的那条链路）

- **示例**：`dong/examples/3d_screen_script.cpp`
- **核心做法**：
  - 在 3D 场景里对 screen quad 做 ray/UV hit-test
  - UV → 像素坐标
  - 调用 `dong_view_send_mouse_move/down/up` 把交互送进 `View`
  - 每帧调用 `dong_view_render_to_gpu_texture` 生成 UI 纹理贴到 3D quad 上

> 易踩坑：mouse-down/mouse-up 需要稳定地发给同一个 screen；同帧 press+release 会让 `:active` 很难“被看见”。

## 5. 关键入口与“去哪改”索引

- **输入 / 伪类状态**：`View::handle_mouse_move/down/up()`（`core/view.cpp`）
- **选择器匹配**：`SelectorMatcher`（`dom/css/selector_matcher.*`）
- **样式重算策略**：`StyleEngine::computeStyles()`（`dom/css/style_engine.*`）
- **布局与 hit-test**：`layout_engine.*` + `hitTestElementAt()`（`core/view.cpp` 内部 helper）
- **绘制序列生成**：`render::Painter::buildDisplayList()`（`render/painter.*`）
- **GPU 命令定义**：`render/gpu_ir.hpp`
- **GPU 执行**：`render/gpu_driver_sdl_*`（execute/resources）
- **字体/文本**：`render/text_shaper.*`、`render/glyph_atlas.*`
- **JS/DOM 桥**：`script/js_bindings.*`

## 6. 调试/一致性工具链（定位“偶发闪烁/不一致”）

- **Dong 多帧导出**：`examples/html_render_test.cpp` → `html_render_test.exe`
- **浏览器 baseline**：`scripts/tools/html_baseline_render.py`（Playwright + Chromium）
- **拼图 + 像素 diff**：`scripts/tools/vl_tool_multi.py`
- **用例回归**：`scripts/tools/run_multiframe_regress.py`、`scripts/tools/run_baseline_compare.py`

## 7. 读代码建议（vibecode 快速上手路线）

1) 先读 `doc/architecture.md`（已有的总体分层）
2) 再读 `core/view.hpp/.cpp`：理解“宿主→引擎”的所有入口
3) 跟一次 `renderToGPUTexture()`：样式→布局→DisplayList→GPUCommandList→GPUDriverSDL
4) 遇到 UI 不一致：优先用 `run_baseline_compare.py` 固化成可复现 diff

---

更新时间：2026-01-12
