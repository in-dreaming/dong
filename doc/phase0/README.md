# Phase 0 方案集

> Phase 0 主线：让 dong "在游戏里真的跑得动"。聚焦点：**性能 + 游戏交互**。
>
> 上游决策：[`doc/positioning.md`](../positioning.md) · 三阶段路线：[`doc/roadmap.md`](../roadmap.md) · 性能门槛：[`doc/perf_budget.md`](../perf_budget.md)
>
> 时间窗口：0–3 个月（2026 Q2–Q3）。
>
> 状态：方案 v1 / 2026-04-17。每个工作项独立 PR / 独立验收。

---

## 1. 工作项总览

| ID | 名称 | 类型 | 估时 | 关键依赖 | 方案 |
|---|---|---|---|---|---|
| **P0-1** | Uber Quad Pipeline | 性能 | 2–3 周 | — | [P0-1](./P0-1_uber_quad_pipeline.md) |
| **P0-2** | 9-slice / nine-patch panel | 渲染特性 | 1–2 周 | P0-1（复用 material 5）| [P0-2](./P0-2_nineslice_panel.md) |
| **P0-3** | CSS mask + conic-gradient | 渲染特性 | 2 周 | P0-1（复用 material 6 fast path）| [P0-3](./P0-3_mask_and_conic_gradient.md) |
| **P0-4** | Gamepad Spatial Navigation | 交互 | 1.5 周 | — | [P0-4](./P0-4_gamepad_spatial_navigation.md) |
| **P0-5** | IME composition 三件套 | 交互 | 1.5 周 | — | [P0-5](./P0-5_ime_composition.md) |
| **P0-6** | Display List 局部失效（damage rect） | 性能 | 2–3 周 | 与 P0-1 协同 | [P0-6](./P0-6_partial_damage_rect.md) |
| **P0-7** | 三轨性能基线脚本 | 工程化 | 1.5 周 | 其他任务的验证依赖；优先做 | [P0-7](./P0-7_perf_baseline.md) |
| **P0-8** | `gap_analysis.md` 真·P0 清账 | 标准对齐 | 1.5 周 | — | [P0-8](./P0-8_gap_analysis_p0_cleanup.md) |

---

## 2. 推荐执行顺序与并行度

```
Week 0-1
  └── P0-7 (基线脚本 v0)   ← 必须先于其他任务的验证
  └── P0-8 (启动 B1/B2 批) ← 与 P0-7 并行

Week 2-4
  ├── P0-1 (Uber Quad S1-S2)
  ├── P0-4 (Gamepad)         ← 完全独立，可并行
  ├── P0-5 (IME)             ← 完全独立，可并行
  └── P0-8 (B3-B7 续)

Week 5-7
  ├── P0-1 (S3-S4)
  ├── P0-6 (damage rect S1-S4)
  └── P0-2 (依赖 P0-1 的 material 扩展)

Week 8-10
  ├── P0-1 (S5 切默认 + 删旧)
  ├── P0-3 (mask + conic-gradient)
  └── P0-6 (S5-S7)

Week 11-12
  └── 整合 + Phase 0 完成判定全部跑通
  └── 所有方案文档"完成后更新清单"全部勾掉
```

并行节点：

- **P0-4 / P0-5 / P0-8** 完全独立，可三个人各拿一项并行。
- **P0-1 / P0-2 / P0-3** 是渲染流水线 + material 系列，建议同一人或同组顺次推进。
- **P0-6** 与 P0-1 在最后阶段会触及同样代码，需要 careful merge；建议 P0-1 S4 完成后 P0-6 才进 S4。
- **P0-7** 是验证依赖，要 Week 1 内交付 v0（至少能跑 S1/S3/S4）。

---

## 3. Phase 0 完成判定（与 [`roadmap.md`](../roadmap.md) § 1.2 对齐）

- [ ] `game_ui2` 在 1080p / 60Hz 下稳定 240+ FPS（present-only）+ 全交互路径不掉 60 FPS。
- [ ] `test_dual_mode`（DOM + overlay 文字流 + 粒子）≥ 200 FPS。
- [ ] `dock_demo` editor 场景拖动 panel 时不出现整树重建。
- [ ] 中文 / 日文输入在所有 `<input>` `<textarea>` `<contenteditable>` 上 IME 候选框位置正确。
- [ ] 手柄 D-pad 在 `game_ui2.html` 内可完成"开始 → 选择项 → 确认 → 返回"完整路径。
- [ ] CI 跑 `perf_baseline.py` 不掉队。
- [ ] 所有 8 个工作项的 § 5 Hard 验证规则全部通过。
- [ ] 所有方案文档底部的"完成后更新清单"全部执行。

---

## 4. 文档约定

每份方案文档统一结构：

| 章节 | 内容 |
|---|---|
| 1. 目标 | 一段话讲清"做什么、不做什么的边界" |
| 2. 现状 | 起点；引用现有代码路径 |
| 3. 设计 | 数据结构、API、shader、算法 |
| 4. 实施步骤 | 拆 PR / Step；标注并行度 |
| 5. 通过验证规则 | Hard / Soft / 必须新增的测试 / 验证脚本 |
| 6. 风险与回退 | 已知风险 + env var 回退路径 |
| 7. 不在本方案范围 | 显式排除项，避免 scope creep |
| 8. 完成后更新 | 必须更新的文档清单 |

新增 Phase 0 任务时，复用本结构；新增 Phase 1/2 任务时，分别建 `doc/phase1/` `doc/phase2/`。

---

## 5. 共用工程约定

| 项 | 约定 |
|---|---|
| **测试用例位置** | `dong/examples/data/tests/test_*.html` |
| **Baseline 入库** | `scripts/tools/html_baseline_render.py` 一次性入；提交到 `tests_data/baselines/` |
| **像素回归** | `scripts/tools/run_baseline_compare.py` |
| **性能回归** | `scripts/tools/perf_baseline.py`（P0-7 后可用） |
| **JS 单元测试** | `dong/tests/`，`zig build run-feature-tests` |
| **PR 标题前缀** | `[P0-X]` 便于跟踪（例：`[P0-1] S2 接入 Uber Quad solid + rounded`） |
| **PR 必须** | 列出对应方案文档的"§ 5 验证规则"哪些已达标 |
| **环境变量回退** | 每个改动渲染路径的任务必须留 `DONG_RENDER_PATH=legacy` 或类似 fallback；S5 / 类似清理 step 才删 |

---

## 6. 与既有文档的索引

| 文档 | 与本目录关系 |
|---|---|
| [`doc/positioning.md`](../positioning.md) | 上位决策；本目录所有方案不得违背 |
| [`doc/roadmap.md`](../roadmap.md) | 三阶段总图；本目录 = Phase 0 详情 |
| [`doc/perf_budget.md`](../perf_budget.md) | 验证规则的指标依据 |
| [`doc/arch/render_pipeline.md`](../arch/render_pipeline.md) | P0-1 的设计原案 |
| [`doc/arch/arch_font.md`](../arch/arch_font.md) | P0-3 mask 与 Slug 协调的依据 |
| [`doc/arch/react.md`](../arch/react.md) | P0-8 react 生态对接清单的依据 |
| [`doc/specific/html_css_dom_gap_analysis.md`](../specific/html_css_dom_gap_analysis.md) | P0-8 来源清单 |
| [`doc/specific/html_css_dom_草案.md`](../specific/html_css_dom_草案.md) | 完成后状态需同步更新 |
| [`doc/重要特性.md`](../重要特性.md) | 完成后能力清单需同步更新 |
| [`doc/summary/opt.md`](../summary/opt.md) | P0-7 工具链上位文档 |
| [`doc/summary/architecture.md`](../summary/architecture.md) | P0-6 完成后一帧数据流需更新 |

---

## 7. 决策记录

| 日期 | 决策 |
|---|---|
| 2026-04-17 | Phase 0 方案集 v1 创建 |
| 2026-04-17 | UE/Unity adapter 不在 Phase 0；详见 [`integration_ue.md`](../integration_ue.md) `integration_unity.md` |
| 2026-04-17 | World Space 3D primitives 不在 Phase 0；归 Phase 2 |
| 2026-04-17 | DevTools / live reload 不在 Phase 0；归 Phase 1 |
| 2026-05-01 | P0-4/P0-5/P0-7/P0-8 实现完成确认；P0-1 已在 dev_next |
| 2026-05-01 | P0-2 CSS border-image 解析+painter emit 完成；GPU material 5 待做 |
| 2026-05-01 | P0-3 conic-gradient 完成；mask fast-path（conic+solid bg）完成；通用 mask RT 待做 |
| 2026-05-01 | P0-6 S1 invalidation 基础设施完成；46/48 调用点已分类 |

后续 Phase 0 内重大调整在此处追加。

---

## 8. 实施进度（2026-05-01 更新）

| ID | 状态 | 完成项 | 剩余 |
|---|---|---|---|
| **P0-1** | ✅ Done | uber quad pipeline 全量落地 | — |
| **P0-2** | 🔶 75% | CSS border-image 解析 ✅ · painter 9 quad emit ✅ | GPU material 5 shader |
| **P0-3** | 🔶 70% | conic-gradient 解析+渲染 ✅ · mask CSS 解析 ✅ · mask fast-path(conic+solid) ✅ | 通用 mask offscreen RT |
| **P0-4** | ✅ Done | spatial nav 算法 ✅ · C ABI ✅ · SDL gamepad ✅ · JS API ✅ · CSS nav-\* ✅ | — |
| **P0-5** | ✅ Done | composition 三件套 ✅ · SDL_SetTextInputArea ✅ | — |
| **P0-6** | 🔶 40% | InvalidationKind 数据结构 ✅ · 46/48 调用点分类 ✅ | sub-tree display list/GPU compile |
| **P0-7** | ✅ Done | perf_baseline.py ✅ · perf_scenes.yaml (4 scenes) ✅ · BENCH_AUTOSTOP ✅ · profiler auto-dump ✅ | — |
| **P0-8** | ✅ Done | DOM API/遍历/Input/CSS 级联/事件/UA 样式 全部已在 dev_next | — |
