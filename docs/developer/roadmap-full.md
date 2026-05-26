# Dong 路线图（完整版）

> 对外摘要见 [../roadmap.md](../roadmap.md)。  
> 上游决策：[定位与边界](../overview/positioning.md) · 性能门槛：[perf-budget.md](./perf-budget.md)  
> 状态：approved 2026-04-17

## 原则

1. **先性能、再可嵌入、再生态**
2. 每个 Phase 有可测 Definition of Done
3. 三轨（DOM / Scene / Direct Draw）均需 perf 基线
4. HTML/CSS 缺口按「游戏 UI 是否需要」补
5. UE/Unity adapter 低优先级；Phase 1 交付 `DongDrawList` C ABI 后由社区驱动

## Phase 0 — 游戏运行时性能（0–3 个月）

| # | 工作项 | 完成判定摘要 |
|---|--------|-------------|
| P0-1 | Uber Quad Pipeline | `game_ui2` ≤ 30 draw；见 [render-pipeline](../architecture/render-pipeline.md) |
| P0-2 | 9-slice panel | `border-image` GPU；`test_nineslice_panel.html` |
| P0-3 | CSS mask + conic-gradient | `test_circular_health.html` |
| P0-4 | Gamepad 空间导航 | `dong.focusNav` + `nav-*` CSS |
| P0-5 | IME composition | 中文/日文输入 + composition 事件 |
| P0-6 | Display List 局部失效 | editor 拖动不全树重建 |
| P0-7 | 三轨 perf 基线 | `perf_baseline.py` CI |
| P0-8 | gap P0 清账 | React 强依赖 DOM API |

详细 spec → [phase0/](./phase0/)

## Phase 1 — 可嵌入与工程化（3–9 个月）

| # | 工作项 | 完成判定摘要 |
|---|--------|-------------|
| P1-1 | DongDrawList C ABI | 外部 C 程序消费 DrawList |
| P1-2 | `<host-view>` 嵌入 | 宿主渲染区域 + clip/transform |
| P1-3 | DevTools v1 | F12 DOM inspector |
| P1-4 | Live reload | `--watch` ≤500ms |
| P1-5 | CSS Grid 子集 | `test_grid_basic.html` |
| P1-6 | HDR 输出 | scRGB / PQ swapchain |
| P1-7 | JS 引擎 benchmark | QuickJS vs 备选评估报告 |
| P1-8 | Async layout/shaping | worker thread shaping |
| P1-9 | gap P1 清账 | fixed/sticky/Observer 等 |

详细 spec → [phase1/](./phase1/)

## Phase 2 — 生态与世界空间（9–18 个月）

| # | 工作项 | 完成判定摘要 |
|---|--------|-------------|
| P2-1 | `dong_world_text_t` | 500 伤害字 @ 60 FPS |
| P2-2 | `dong_decal_t` | 世界投影贴花 |
| P2-3 | `dong_world_overlay_t` | 3D 终端 / 广告牌 |
| P2-4 | Lottie / Rive | 各 1 真实设计稿 |
| P2-5 | dong-ui 组件库 | ≥20 组件 + 主题 |
| P2-6 | TypeScript + npm | `@dong/react` 等 |
| P2-7 | 可视化编辑器 | dogfood |
| P2-8 | `.dpkg` 资源打包 | 启动 <200ms |
| P2-9 | Async 默认开启 | P1-8 稳定后 |
| P2-10 | gap P2 清账 | 按需 |

详细 spec → [phase2/](./phase2/)

## 决策记录

| 日期 | 决策 |
|------|------|
| 2026-04-17 | Ultralight 路线、暂不换 JS、三轨定档、`dong_scene3d_*` 仅 demo |
| 2026-04-17 | UE/Unity adapter 仅 ABI + 草案，Core 不投入 adapter 工程 |

## 明确不做

Coherent/Chromium 路线、第四种渲染轨道、`dong_scene3d_*` PBR 扩展、Web Worker/WebGL/Service Worker、Core 自带 UE/Unity adapter。
