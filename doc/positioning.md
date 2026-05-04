# Dong 定位与边界（Ultralight 路线）

> 目的：把"dong 是什么、不是什么、谁是目标用户、能力边界在哪"一次写清，避免后续讨论反复回到原点。
>
> 决策日期：2026-04-17
> 状态：approved（核心决策已锁定，细节随 roadmap 推进迭代）

---

## 1. 一句话定位

**dong 是一套用 HTML/CSS/DOM 子集描述、GPU 直接渲染的游戏 UI 工具，面向游戏 editor 与 runtime（含 3D HUD/世界空间 UI），不是浏览器。**

---

## 2. 路线选择：Ultralight，不是 Coherent

行业里"游戏内 web-like UI"基本只有两条路：

| 路线 | 代表 | 思路 | 取舍 |
|---|---|---|---|
| Coherent (Gameface) | 改造 Chromium / Blink | HTML 完整兼容 | 内存大、依赖重、构建复杂 |
| Ultralight | 自研裁剪引擎 | 强裁剪 + 嵌入友好 + 极致性能 | 牺牲冷门 web 特性，换可控、可嵌入 |

**dong 选 Ultralight 路线**：

- 不追求 web 标准完整性。`gap_analysis.md` 列出的"web 缺口"按"游戏 UI 是否需要"重新排序，不是按 P0/P1/P2。
- 不追求"任意 web 页面能跑"。dong 接受的输入是"为 dong 写的 HTML"，不是"任意从浏览器搬过来的 HTML"。
- 不追求 Web 平台 API 完整性（无 Service Worker / WebRTC / WebGL / IndexedDB / Web Worker / Shadow DOM 等）。
- 追求：**单 frame budget 可控**、**嵌入宿主引擎不带 RT**、**首屏延迟低**、**多 view 共享资源**、**字体在任意字号下都清晰（Slug）**。

后续凡涉及"要不要补这个 web 特性"的讨论，统一用本路线裁决：**只补游戏 editor / runtime 真正需要的，其它一律不做。**

---

## 3. 目标用户与场景矩阵

| 场景 | 主要用户 | dong 必须做好 |
|---|---|---|
| **游戏 Editor**（关卡编辑器、属性 inspector、资源浏览器、节点图） | 引擎团队、TA、内部工具开发 | 复杂 panel 布局、高频局部更新、可嵌入引擎主程序、可挂载引擎 viewport |
| **2D 游戏 HUD**（血条、技能、背包、聊天、minimap） | 游戏 UI / 客户端程序 | 高密度静态结构、低 draw call、Slug 文字、手柄导航 |
| **3D 中的 UI**（屏幕电视、终端、菜单 quad） | 游戏 UI / 关卡设计 | 多 view 共享 atlas、纹理输出可被宿主采样 |
| **世界空间 UI**（伤害浮字、名字版、3D HUD、贴花） | 游戏战斗 / 渲染 | 高频 instanced 绘制、与宿主 3D 渲染管线协同（深度、occlusion） |

**非目标场景**（不做）：

- 浏览页面、跑第三方 web 应用、嵌入网页广告、运行 Chrome 扩展。
- 复杂富文本编辑器（Quill / Tiptap 那种带协同编辑/嵌入媒体的）。
- 完整的 web 平台兼容（File API / WebSocket / WebGL / Service Worker）。
- 移动浏览器替代品。

---

## 4. 三轨渲染模式（正式定档）

dong 长期保留三种渲染模式，**通过 `<meta name="dong-render-mode">` 与 JS API 显式选择**。三者共享 GPU pipeline 与字体系统，互斥的是"DOM 树/布局/事件"路径。

### 4.1 DOM 模式（默认）

- 用法：什么都不写，默认即 DOM 模式。也可显式 `<meta name="dong-render-mode" content="dom">`。
- 适用：**editor、复杂业务面板、设置界面、React/Preact 应用**。
- 特征：完整 DOM/CSS/Yoga/事件冒泡/伪类/animation；可对接 react-reconciler。
- 性能预期：见 `doc/perf_budget.md` 的 Editor / 业务面板栏。
- 文档：`doc/arch/architecture.md`、`doc/arch/react.md`。

### 4.2 Scene Graph 模式

- 用法：`<meta name="dong-render-mode" content="scene">`。
- 适用：**高密度静态结构的游戏 HUD**（典型：背包格子矩阵、装备槽、技能栏、minimap 框架）。
- 特征：跳过 CSS 选择器匹配 / Yoga / 事件冒泡，扁平节点 + 显式像素坐标 + AABB hit-test，O(1) 属性更新。
- 限制：要求所有元素 `position: absolute` + 显式像素 / `transform: translate`；不支持 `width:100%`、`margin`、`display:inline-block` 等 Scene Compiler 处理不了的特性。
- 文档：`doc/重要特性.md` § Scene Graph 模式、`src/dom/scene_compiler.cpp`、`src/render/scene_graph.cpp`。

### 4.3 Direct Draw 模式（overlay / immediate）

- 用法：JS 调用 `dong.drawRect / drawCircle / renderText / clearOverlay`，可与 DOM/Scene Graph 叠加。
- 适用：**高频动态绘制**（典型：伤害浮字、粒子文字、雷达扫描线、debug overlay）。
- 特征：无 DOM 节点，纯命令 → DisplayList 注入 → 单 GPU draw call（合批后）。
- 性能基线：40 行动态文字合批 → 480 FPS（详见 `doc/arch/arch_font.md`）。
- 文档：`src/render/overlay_draw.hpp`、`src/script/js_text_layout_bindings.cpp`。

### 4.4 三轨边界规则

| 规则 | 说明 |
|---|---|
| **同一 view 内可叠加 DOM + Direct Draw** | 已支持（`test_dual_mode.html`）。常用：DOM 框架 + Direct Draw 文字流。 |
| **DOM 模式与 Scene Graph 模式互斥** | 一个 view 只能选一种树结构。 |
| **三轨共用 GlyphAtlas / ImageAtlas / Pipeline** | 不同模式的 GPU 命令最终走同一条 execute 链路。 |
| **每个模式都必须有 perf 基线** | 见 `doc/perf_budget.md`。 |

后续若要加第四轨（如 ImGui-style retained），必须先证明现有三轨不能解决问题，再讨论。

---

## 5. 3D 集成策略

### 5.1 `dong_scene3d_*` 的定位

`dong_scene3d_*`（自带相机 + ray hit-test + WASD）**正式定位为 demo / 工具 API**：

- 用途：示例代码、纯 dong 应用、原型验证、内部工具。
- **不**作为生产级游戏的 3D 渲染入口。
- 后续维护：保持可用，但不再扩展功能（如阴影、PBR、骨骼动画）。

### 5.2 一等公民：宿主可调用的 World Space UI Primitives

游戏 runtime 真正需要的是"宿主 3D 引擎渲染管线 + dong 提供 UI 内容"。Phase 2 起，dong 把这部分上升为一等 API（详见 `roadmap.md` Phase 2）：

| Primitive | 用途 | 与宿主的契约 |
|---|---|---|
| **HTML → GPU Texture**（已有） | 把整个 view 渲染到一张 SDL_GPUTexture | 宿主自己采样、贴到任意 quad |
| **`dong_world_text_t`**（待做） | 世界空间 billboard 文字（伤害字、名字版） | 宿主提供 view/proj 矩阵；dong 出 instanced 顶点 / draw |
| **`dong_decal_t`**（待做） | UI 投影到世界几何（标记、地面提示） | 宿主提供深度 + 表面法线；dong 出 RGBA + UV |
| **`dong_world_overlay_t`**（待做） | 世界空间任意 quad UI（终端、屏幕） | 宿主提供 quad transform + occlusion；dong 出 GPU 命令 |
| **`DongDrawList` C ABI**（待做） | 把 view 输出从"GPU 像素"改为"可合成 draw command" | 宿主把命令翻译为自己的 UI mesh，无 RT |

**核心原则**：dong 不假设自己拥有相机和 swapchain；这些由宿主控制。

---

## 6. 能力边界（明确不做的事）

| 类别 | 不做 | 原因 |
|---|---|---|
| HTML | `<iframe>` `<embed>` `<object>` `<portal>` `<map>` | 浏览器特有 |
| CSS | `:visited` `@page` 多列 column-* | 无浏览历史 / 无打印需求 |
| Layout | 自研 BFC 完整 float（基础已有，不再深挖） | 用 flex / grid 替代 |
| 文本 | Ruby `<rt>` `<rp>` `<bdi>` 完整 BiDi | 复杂度高，按需求再说 |
| Web 平台 | Service Worker / Web Worker / WebGL / WebRTC / WebXR / IndexedDB / Cookie / History API | 不属于游戏 UI 工具范畴 |
| 媒体 | 完整 MSE / EME / DRM / 字幕完整渲染（VTT 复杂特性） | 看 P 级别按需做（`doc/specific/wip_video.md`）|
| 安全 | 无沙箱、无 CSP、无同源策略 | 资产是游戏自带，不需要浏览器安全模型 |

**例外通道**：若特定客户场景必须用上面的某项，走 plugin 注入（见 `dong_plugin_api.h`），不进 Core。

---

## 7. 与已有方向文档的关系

| 文档 | 作用 |
|---|---|
| **本文档（positioning.md）** | 定位与边界总纲。其他文档与本文档冲突时，以本文档为准。 |
| `doc/roadmap.md` | 三阶段交付计划，落实本定位。 |
| `doc/perf_budget.md` | 各场景性能预算，验证本定位的可行性。 |
| `doc/arch/architecture.md` `doc/summary/architecture.md` | 当前实现快照。 |
| `doc/arch/render_pipeline.md` | Uber Quad Pipeline 优化方案。 |
| `doc/arch/react.md` | React 适配现状。 |
| `doc/ideal/引擎适配.md` | RT-free DrawList 思路（Phase 1 落地）。 |
| `doc/integration_ue.md` `doc/integration_unity.md` | UE/Unity 适配草案（**低优先级**，方向参考）。 |
| `doc/specific/html_css_dom_gap_analysis.md` | Web 标准缺口清单（参考价值，不是优先级依据）。 |
| `doc/specific/html_css_dom_草案.md` | Web 标准支持清单（同上）。 |

---

## 8. 决策记录（Decision Log）

| 日期 | 决策 | 替代方案及放弃原因 |
|---|---|---|
| 2026-04-17 | 走 Ultralight 路线 | Coherent (改 Chromium) 太重，与"嵌入游戏"冲突 |
| 2026-04-17 | JS 引擎暂不换（继续 QuickJS） | Hermes / V8 评估留 Phase 1 benchmark；现阶段优化空间还够 |
| 2026-04-17 | `dong_scene3d_*` 仅定位 demo / 工具 | 不与宿主 3D 渲染管线竞争 |
| 2026-04-17 | 三轨（DOM / Scene Graph / Direct Draw）正式定档 | 不引入第四轨；不收敛到单轨 |
| 2026-04-17 | UE/Unity 适配定为低优先级 | 先把 Core / RT-free DrawList 做扎实，再谈 adapter |

后续重大决策追加到本表，并在 PR 中引用本文件。
