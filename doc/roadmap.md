# Dong 路线图（Roadmap）

> 目的：把"接下来 0.5 / 1 / 2 年做什么、不做什么"固化为可追踪的里程碑，避免一边补 web 标准一边追性能优化、最后哪边都没做透。
>
> 上游决策见 [`doc/positioning.md`](./positioning.md)（Ultralight 路线、三轨、3D 策略、JS 引擎暂不换）。
> 性能门槛见 [`doc/perf_budget.md`](./perf_budget.md)。
>
> 状态：approved 2026-04-17。后续大幅调整需要在本文件 § 5 决策记录里追加条目。

---

## 0. 路线图原则

1. **先性能、再可嵌入、再生态。** 顺序不可换。性能不达标，可嵌入性谈不上；可嵌入做不通，生态没人来。
2. **每个 Phase 必须有"完成判定" (Definition of Done)。** 不靠感觉，靠可测的指标 + 可跑的 demo。
3. **三轨（DOM / Scene Graph / Direct Draw）每个 Phase 都要被基线覆盖。** 一个轨道掉出 perf budget 就回滚或修复，不允许"反正还有另外两轨"。
4. **HTML/CSS 缺口按"游戏 UI 是否需要"补，不按 web P0/P1 补。** Web 标准对齐是副产物，不是目标。
5. **UE/Unity 适配是低优先级。** Phase 0/1 不做 adapter；Phase 1 末做完 `DongDrawList` C ABI 后，由社区/客户驱动 adapter（见 `doc/integration_ue.md` `doc/integration_unity.md`）。

---

## 1. Phase 0 — 让 dong "在游戏里真的跑得动"（0–3 个月）

聚焦点：**游戏运行时性能** + **游戏交互必备**。

### 1.1 工作项

| # | 工作项 | 类型 | 负责模块 | 完成判定 |
|---|---|---|---|---|
| P0-1 | **Uber Quad Pipeline 落地** | 性能 | `backends/sdl/sdl_gpu_driver_*`、`src/render/gpu_ir.hpp` | `game_ui2` ≤ 30 draw, `test_select_keyboard` ≤ 50 draw（对比当前 68/91，详见 `doc/arch/render_pipeline.md`） |
| P0-2 | **9-slice / nine-patch panel** | 渲染特性 | 新增 `src/render/painter/painter_nineslice.cpp` + `border-image` CSS | 测试用例：4 角圆角不变形 + 中间区域 tile/stretch；`test_nineslice_panel.html` 对 baseline 像素一致 |
| P0-3 | **CSS mask + conic-gradient** | 渲染特性 | `src/dom/css/css_value_parser.cpp` + 新 shader | 圆形血条 demo (`test_circular_health.html`) 跑通 |
| P0-4 | **Gamepad Spatial Navigation** | 交互 | 新增 `src/input/spatial_nav.cpp` + JS API `dong.focusNav('up'\|'down'\|'left'\|'right')` | 用 SDL gamepad 在 `game_ui2.html` 内方向键跳焦点；支持 CSS `nav-up/down/left/right` 显式覆盖 |
| P0-5 | **IME composition 三件套** | 交互 | `src/dom/event_system.cpp` + `backends/sdl/sdl_input_adapter.cpp` | 中文/日文输入 candidate 框定位正确；`compositionstart/update/end` 事件可被 JS 监听；上报 caret rect 给 IME |
| P0-6 | **Display List 局部失效（damage rect）** | 性能 | `src/core/engine_view.cpp`、`src/render/painter.cpp` | editor 拖动一个 panel header 时，其他 panel 不重建 display list；perf budget 的 "Editor 拖动" 场景达标 |
| P0-7 | **三轨性能基线脚本** | 工程化 | `dong/scripts/tools/perf_baseline.py` | 一键跑 DOM / Scene Graph / Direct Draw 三个固定 demo，输出 `results_baseline.json`；CI 可对比 |
| P0-8 | **`gap_analysis.md` 真·P0 清账** | 标准对齐 | `src/script/js_*_bindings.cpp`、`src/dom/css/style_engine_*.cpp` | 补：`dataset` / `matches` / `closest` / 元素级 `querySelector` / `getBoundingClientRect` / `inherit/initial/unset` / `!important`（详细子项见 `gap_analysis.md`，但仅清"游戏/React 生态强依赖"项） |

### 1.2 Phase 0 完成判定（必须全部满足）

- [ ] `game_ui2` 在 1080p / 60Hz 下稳定 240+ FPS（present-only 路径）+ 全交互路径不掉 60 FPS。
- [ ] `test_dual_mode`（DOM + overlay 文字流 + 粒子）≥ 200 FPS。
- [ ] `dock_demo` editor 场景拖动 panel 时不出现整树重建（profiler 无 `commands_regenerated_this_tick_=true`）。
- [ ] 中文输入在所有 `<input>` `<textarea>` `<contenteditable>` 上 IME 候选框位置正确。
- [ ] 手柄 D-pad 在 `game_ui2.html` 内可完成"开始 → 选择项 → 确认 → 返回"完整路径。
- [ ] CI 跑 `perf_baseline.py` 不掉队（与上一周对比阈值见 `perf_budget.md`）。

### 1.3 不在 Phase 0 范围

- ❌ DongDrawList C ABI（Phase 1）
- ❌ DevTools / live reload（Phase 1）
- ❌ HDR（Phase 1）
- ❌ World Space 3D Primitives（Phase 2）
- ❌ UE/Unity adapter（社区驱动，无明确日程）

---

## 2. Phase 1 — 让 dong "可被宿主嵌入、可被团队上量"（3–9 个月）

聚焦点：**可嵌入性** + **工程化 / 开发体验**。

### 2.1 工作项

| # | 工作项 | 类型 | 负责模块 | 完成判定 |
|---|---|---|---|---|
| P1-1 | **`DongDrawList` C ABI v1** | 架构 | 新增 `include/dong_drawlist.h` + `src/render/drawlist_emitter.cpp` | C ABI 文档完整、版本号机制、demo：从外部 C 程序消费 DrawList 并把它打印成文本（不依赖 SDL） |
| P1-2 | **`<host-view id>` 嵌入元素 + `DrawHostView` 命令** | 架构 | `src/dom/html/html_parser.cpp` + `src/render/painter.cpp` + DrawList | demo：HTML 内含 `<host-view id="mini-map">`，宿主回调被正确触发并继承 clip/transform/opacity |
| P1-3 | **DevTools v1（嵌入式 inspector）** | 工程化 | 新增 `dong/devtools/`（用 dong 自己写） | 在任意 view 按 F12 打开 overlay，能看到 DOM 树、computed style、layout box、当前 draw call 数；不依赖外部浏览器 |
| P1-4 | **Live reload** | 工程化 | `dong_app.exe --watch` + bundler hook | HTML / CSS / JS 文件改动后 ≤ 500ms 自动重载，状态尽量保留 |
| P1-5 | **CSS Grid 子集** | 渲染特性 | `src/layout/grid_engine.cpp`（自研轻量） + 跟踪 Yoga PR #1865 | 支持 `grid-template-columns/rows`（fr / px / %）+ `grid-row/column-start/end` + `gap`；`test_grid_inventory.html` 跑通 |
| P1-6 | **HDR 输出（基础）** | 渲染特性 | `backends/sdl/sdl_gpu_driver_init.cpp` + shader color space metadata | swapchain 支持 scRGB / Rec.2100 PQ；CSS 颜色 → 线性空间正确；HDR 显示器上 `test_hdr_basic.html` 高亮值 > SDR 白点 |
| P1-7 | **JS 引擎 benchmark 报告** | 决策依据 | `dong/scripts/tools/js_bench.py` | 对比 QuickJS / QuickJS-NG / Hermes 在 react-reconciler diff、setTimeout 调度、JSON parse、property access 上的相对性能；产出报告 `doc/perf/js_engine_bench_2026Q3.md` 后再决定是否切换 |
| P1-8 | **Async layout / shaping（实验）** | 性能 | `src/layout/layout_engine.cpp` + `src/render/text_shaper.cpp` | 把 HarfBuzz shaping 切到 worker thread；首次 React bundle eval 阻塞 < 50ms（当前 100–500ms） |
| P1-9 | **`gap_analysis.md` P1 清账** | 标准对齐 | `src/dom/*` `src/script/*` | 补：position fixed 视口语义、`<details>` 完整行为、表单约束（`required` / `pattern`）、Observer JS bindings（`Mutation/Resize/Intersection`）、`performance.now()` 等。 |

### 2.2 Phase 1 完成判定

- [ ] 可在 macOS / Linux / Windows 三平台用纯 dong（无 SDL window）演示：宿主自己开窗，把 `DongDrawList` 翻译为自己的 mesh / draw 命令，UI 像素与 SDL backend 一致（容差 < 1%）。
- [ ] 嵌入元素 demo：HTML 中插入一个"被宿主控制的 minimap quad"，HTML 滚动 / clip / opacity 都正确传递给该 quad。
- [ ] DevTools 在 dong_app 中可一键打开，可改任意元素 inline style 并立即看到效果。
- [ ] React 应用 live reload 改一个组件 prop ≤ 500ms 看到画面更新。
- [ ] HDR 显示器上 dong 输出能进入 HDR 模式，亮度峰值 > 400 nits（视显示器能力）。
- [ ] JS bench 报告产出，决策"切 / 不切"已写入 `positioning.md` § 8 决策记录。

### 2.3 不在 Phase 1 范围

- ❌ World Space 3D Primitives（Phase 2）
- ❌ Lottie / Rive 兼容（Phase 2）
- ❌ 官方组件库 dong-ui（Phase 2）
- ❌ UE/Unity adapter 工程实现（**仍是低优先级**；本 Phase 仅交付 ABI + 草案文档，由外部团队/客户驱动具体 adapter）

---

## 3. Phase 2 — 让 dong "成为游戏 UI 的可选事实标准之一"（9–18 个月）

聚焦点：**3D 一等公民** + **生态与组件库** + **持续优化**。

### 3.1 工作项

| # | 工作项 | 类型 | 完成判定 |
|---|---|---|---|
| P2-1 | **`dong_world_text_t`** | 3D primitive | 世界空间 billboard 文字 API；Slug 走 instanced；与 `GlobalShared` GlyphAtlas 共享；500 个伤害浮字稳定 60 FPS |
| P2-2 | **`dong_decal_t`** | 3D primitive | UI quad 投影到世界几何；宿主提供深度+法线，dong 出 RGBA + UV |
| P2-3 | **`dong_world_overlay_t`** | 3D primitive | 任意 quad in world space + occlusion；典型场景：可交互 3D 终端、广告牌 |
| P2-4 | **Lottie / Rive 兼容路径** | 渲染特性 | 复用 Slug 贝塞尔 GPU pipeline，跑通 1 个 Lottie 与 1 个 Rive demo |
| P2-5 | **官方组件库 `dong-ui`** | 生态 | 基于 base-ui 思路，Button / Input / Select / Slider / Tabs / Tooltip / Dialog / DropdownMenu 等 ≥ 20 组件；含 game-ready 主题（`cyberpunk` / `medieval` / `clean`） |
| P2-6 | **TypeScript 类型 + npm 包** | 生态 | `@dong/react` `@dong/preact` `@dong/ui` 上 npm；含 d.ts |
| P2-7 | **可视化编辑器（dogfood）** | 工程化 | editor 用 dong 自己写；至少能编辑节点树 + 实时预览 |
| P2-8 | **GPU 资源打包 `.dpkg`** | 资源管线 | 字体/图集/shader 预打包；启动延迟 < 200ms |
| P2-9 | **多线程 layout / shaping 默认开启** | 性能 | Phase 1 实验项稳定后转默认 |
| P2-10 | **`gap_analysis.md` P2 清账** | 标准对齐 | 按需补 |

### 3.2 Phase 2 完成判定

- [ ] World Space 3 个 primitive 都有可跑 demo，且 perf budget（见 `perf_budget.md`）达标。
- [ ] `dong-ui` 在 npm 上可装，`npm create dong-app` 起手 ≤ 1 分钟。
- [ ] dong-editor 至少能完成"编辑一个 game_ui2 类页面"的全流程（拖控件、改属性、预览、保存）。
- [ ] Lottie / Rive 各跑通一个真实设计稿。

---

## 4. 跨 Phase 持续工作（Continuous）

不绑死在某个 Phase，但每个 Phase 都要分配预算：

| 项 | 频率 | 度量 |
|---|---|---|
| **Web 标准缺口清账** | 每月 1 个迭代窗口 | `gap_analysis.md` 状态变更条目数 |
| **baseline 像素回归** | 每个 PR | `run_baseline_compare.py` ≥ 99% 通过 |
| **性能回归** | 每周 | `perf_baseline.py` 与上周对比无超阈值衰退（阈值见 `perf_budget.md`）|
| **Refactoring 进度** | 持续 | `doc/重构遗留.md` |

---

## 5. 决策记录与变更（Change Log）

| 日期 | 决策 / 变更 | 说明 |
|---|---|---|
| 2026-04-17 | 路线图 v1 创建 | 基于 `positioning.md` 的 4 项决策（Ultralight / 暂不换 JS / 3D 策略 / 三轨）。 |
| 2026-04-17 | UE/Unity adapter 在 Phase 1 仅产出 ABI + 草案文档 | 实现工作由外部驱动；Core 团队不投入 adapter 工程实现。 |

后续路线变更追加到本表，并同步更新 `positioning.md` § 8。

---

## 6. 路线图与现有文档的关系

| 已有文档 | 在路线图中的位置 |
|---|---|
| `doc/arch/render_pipeline.md` (Uber Quad) | Phase 0 P0-1 的设计依据 |
| `doc/ideal/引擎适配.md` (RT-free DrawList) | Phase 1 P1-1 / P1-2 的设计依据 |
| `doc/ideal/hdr.md` | Phase 1 P1-6 的方向参考 |
| `doc/arch/react.md` | Phase 1 P1-7 / Phase 2 P2-6 的依据 |
| `doc/specific/wip_video.md` | 单独维度，按客户需求驱动；不在主路线图，但与 Phase 2 重合时复用 P2 的资源 |
| `doc/specific/wip项.md` (grid) | Phase 1 P1-5 的依据 |
| `doc/重要特性.md` | 当前已交付能力的索引；每个 Phase 完成后更新 |

---

## 7. 取消或延后的事项（Won't Do）

| 项 | 原因 |
|---|---|
| Coherent / Chromium 兼容路线 | `positioning.md` § 2 明确不走 |
| 第四种渲染轨道（如 ImGui-style） | 三轨已足够覆盖目标场景 |
| `dong_scene3d_*` 扩展（阴影 / PBR / 骨骼动画） | 仅作 demo / 工具 API，不再扩展（`positioning.md` § 5.1） |
| Web Worker / WebGL / Service Worker / WebRTC / IndexedDB | 不在游戏 UI 工具范畴（`positioning.md` § 6） |
| 自带 UE/Unity adapter 工程实现 | 长期低优先级；Core 仅负责 `DongDrawList` ABI |
