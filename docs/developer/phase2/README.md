# Phase 2 方案集

> Phase 2 主线：让 dong "成为游戏 UI 的可选事实标准之一"。聚焦点：**3D 一等公民 + 生态与组件库 + 持续优化**。
>
> 上游决策：[`docs/overview/positioning.md`](../positioning.md) · 三阶段路线：[`docs/roadmap.md`](../roadmap.md) · 性能门槛：[`docs/developer/perf-budget.md`](../perf_budget.md)
>
> 时间窗口：9–18 个月（Phase 1 完成之后）。
>
> 状态：方案 v1 / 2026-04-17。每个工作项独立 PR / 独立验收。

---

## 1. 工作项总览

| ID | 名称 | 类型 | 估时 | 关键依赖 | 方案 |
|---|---|---|---|---|---|
| **P2-1** | `dong_world_text_t`（世界空间 billboard 文字） | 3D primitive | 3 周 | Phase 0 / 1 完成 | [P2-1](./P2-1_world_text.md) |
| **P2-2** | `dong_decal_t`（UI 贴花到世界几何） | 3D primitive | 3 周 | — | [P2-2](./P2-2_decal.md) |
| **P2-3** | `dong_world_overlay_t`（任意 world space quad） | 3D primitive | 3 周 | P1-2 host-view | [P2-3](./P2-3_world_overlay.md) |
| **P2-4** | Lottie / Rive 兼容路径 | 渲染特性 | 4 周 | Slug pipeline | [P2-4](./P2-4_lottie_rive.md) |
| **P2-5** | `dong-ui` 官方组件库 | 生态 | 8 周 | P0-3 / P0-4 / P2-1 | [P2-5](./P2-5_dong_ui_library.md) |
| **P2-6** | TypeScript 类型 + npm 发布 | 生态 | 3 周 | — | [P2-6](./P2-6_typescript_npm.md) |
| **P2-7** | 可视化编辑器（dogfood） | 工程化 | 6 周 | P1-3 / P1-4 / P2-5 | [P2-7](./P2-7_visual_editor.md) |
| **P2-8** | GPU 资源打包 `.dpkg` | 工程化 | 4 周 | — | [P2-8](./P2-8_dpkg_resource_pack.md) |
| **P2-9** | Async layout / shaping 默认开启 | 性能 | 1 周 | P1-8 实验报告 | [P2-9](./P2-9_async_default_on.md) |
| **P2-10** | `gap_analysis.md` P2 清账 | 标准对齐 | 6 周（多人并行 9 批） | — | [P2-10](./P2-10_gap_analysis_p2_cleanup.md) |

---

## 2. 推荐执行顺序与并行度

```
Month 1-2 (启动期)
  ├── P2-1 (World Text)                  ← 优先做，给 dong-ui DamageNumber 做底
  ├── P2-6 (npm/TS) S1-S2 core-types     ← 独立可早做
  ├── P2-8 (dpkg) S1-S2 格式 + reader    ← 独立可早做
  └── P2-10 B1/B2/B6 (RTL / Pointer / scroll-snap) ← 多人并行

Month 3-5 (主力期)
  ├── P2-2 (Decal)
  ├── P2-3 (World Overlay)
  ├── P2-5 (dong-ui) S1-S5 核心 + 反馈 + 表单
  ├── P2-4 (Lottie/Rive) 接入
  ├── P2-6 完工 (npm 包发布)
  ├── P2-8 (dpkg) S3-S7 字体/图片/shader/JS bytecode
  ├── P2-9 (Async 默认开启) ← 等 P1-8 报告 1 个月稳定后
  └── P2-10 续 (B3/B4/B5/B7)

Month 6-9 (生态期)
  ├── P2-5 (dong-ui) S6-S9 游戏组件 + 主题 + npm
  ├── P2-7 (dong-editor)                ← 等 P2-5 完成
  ├── P2-4 完工 (rive state machine + 调优)
  ├── P2-8 完工 (multi-pack overlay)
  └── P2-10 完工 (B8/B9 长尾 + dong 扩展整理)

Buffer
  └── 整合 + Phase 2 完成判定全部跑通
```

并行节点：

- **P2-1 / P2-2 / P2-3** 三个 3D primitive 可分给同人 / 同组顺次（共享 WorldRenderContext）。
- **P2-4 / P2-5 / P2-6 / P2-8 / P2-10** 互相独立，可大规模并行。
- **P2-7** 强依赖 P1-3 / P1-4 / P2-5 / P2-6；最后做。
- **P2-9** 仅需 P1-8 完成且稳定 1 个月。

总周期：**~6–9 个月**（典型 6 人小队）。

---

## 3. Phase 2 完成判定（与 [`roadmap.md`](../roadmap.md) § 3.2 对齐）

- [ ] World Space 3 个 primitive 都有可跑 demo，且 perf budget 达标（[`perf_budget.md`](../perf_budget.md) § 3.5）。
- [ ] `dong-ui` 在 npm 上可装；`npm create dong-app` 起手 ≤ 1 分钟。
- [ ] dong-editor 至少能完成"编辑一个 game_ui2 类页面"的全流程（拖控件 / 改属性 / 预览 / 保存）。
- [ ] Lottie / Rive 各跑通一个真实设计稿。
- [ ] dong_app `--pkg app.dpkg` 启动 ≤ 200 ms 首帧。
- [ ] Async layout 默认开启，回归测试稳定。
- [ ] `gap_analysis.md` 中本批次条目状态全部更新，剩余条目明确分类（"待 P3" / "Won't Do"）。
- [ ] 所有 10 个工作项的 § 5 Hard 验证规则全部通过。
- [ ] 所有方案文档"完成后更新清单"全部执行。

---

## 4. 文档约定

继承 [Phase 0 README](../phase0/README.md) § 4 / § 5 的八节结构与共用工程约定。

新增约定：

| 项 | 约定 |
|---|---|
| **PR 标题前缀** | `[P2-X]` |
| **3D primitive ABI** | 必须经 review 进入稳定 ABI；与 P1-1 DrawList 同等待遇 |
| **dong 私有扩展** | 必须 `dong-` / `--dong-` / `data-dong-` 前缀；进 `docs/reference/extensions.md` |
| **npm 发布** | 跟随 dong git tag 同版本；CI 自动 publish |
| **dong-ui 组件 PR** | 必须含 unit + pixel baseline + a11y 检查 |

---

## 5. 与 Phase 0 / Phase 1 / Phase 3 的关系

| 关系 | 说明 |
|---|---|
| **Phase 0 → Phase 2** | P0-3 conic-gradient 是 dong-ui HealthBar 必需；P0-4 spatial nav 是所有 dong-ui 交互组件默认 |
| **Phase 1 → Phase 2** | P1-1 DrawList ABI / P1-2 host-view 是 World Overlay 的前置；P1-3 DevTools 是 dong-editor 的前置；P1-4 live reload 是 dong-editor 的前置；P1-7 JS 引擎决策若切则在 Phase 2 落地（不在本路线图）；P1-8 async 是 P2-9 的前置 |
| **Phase 2 → Phase 3+** | P2-10 留下的"待 P3"清单；JS 引擎切换（如有）；mobile / 主机平台调优；dong-editor 高级功能（JSX / 协同）|

---

## 6. 与既有文档的索引

| 文档 | 与本目录关系 |
|---|---|
| [`docs/overview/positioning.md`](../positioning.md) | 上位决策；本目录方案不得违背 |
| [`docs/roadmap.md`](../roadmap.md) | Phase 2 概览来自此 |
| [`docs/developer/perf-budget.md`](../perf_budget.md) | 验证规则的指标依据 |
| [`docs/developer/phase0/README.md`](../phase0/README.md) | 前置 |
| [`docs/developer/phase1/README.md`](../phase1/README.md) | 前置 |
| [`docs/developer/arch/text-rendering-spec.md`](../arch/arch_font.md) | P2-4 Lottie/Rive 复用 Slug pipeline 的依据 |
| [`docs/developer/design/frame.md`](../ideal/frame.md) | P2-5 dong-ui 灵感来源；任务完成后替换 |
| [`docs/developer/gap-analysis.md`](../specific/html_css_dom_gap_analysis.md) | P2-10 来源 |
| [`docs/guide/integrations/unreal-engine.md`](../integration_ue.md) [`integration_unity.md`](../integration_unity.md) | World Space primitives + npm 包后宿主集成更顺；adapter 仍长期低优先级 |

---

## 7. 决策记录

| 日期 | 决策 |
|---|---|
| 2026-04-17 | Phase 2 方案集 v1 创建 |
| 2026-04-17 | World Space 3D primitives 在 Phase 2 一次性交付（不拆三轮） |
| 2026-04-17 | dong-editor 完全用 dong + dong-ui 自身实现（dogfood） |
| 2026-04-17 | dpkg 资源包 v1 仅做基础打包 + mmap；增量 patch 留 v2 |
| 2026-04-17 | Async layout 默认开启需 P1-8 实验稳定 ≥ 1 个月 |

后续 Phase 2 内重大调整在此处追加。
