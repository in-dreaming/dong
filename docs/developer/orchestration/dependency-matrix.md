# Dong 任务依赖矩阵 与 并行 Wave 调度

> 适用范围：`docs/developer/phase0/` + `docs/developer/phase1/` + `docs/developer/phase2/` 全部 27 个工作项。
>
> 目的：给"主管理者 (orchestrator) + N 个 cli worker"模式提供一个**确定的依赖图**与**冲突可控的并行批次**。
>
> 集成分支：`dev_next`（首次启动时由 orchestrator 创建于 `main`）。所有 feature 分支从 `dev_next`（或上游依赖 feature 分支）派生，完成后合并回 `dev_next`。

---

## 1. 工作项清单（27 项）

| ID | 名称 | 方案 | 估时 |
|---|---|---|---|
| P0-1 | Uber Quad Pipeline | [P0-1](../phase0/P0-1_uber_quad_pipeline.md) | 3 周 |
| P0-2 | 9-slice 面板 | [P0-2](../phase0/P0-2_nineslice_panel.md) | 1 周 |
| P0-3 | mask + conic-gradient | [P0-3](../phase0/P0-3_mask_and_conic_gradient.md) | 2 周 |
| P0-4 | 手柄空间导航 | [P0-4](../phase0/P0-4_gamepad_spatial_navigation.md) | 2 周 |
| P0-5 | IME 组合输入 | [P0-5](../phase0/P0-5_ime_composition.md) | 1.5 周 |
| P0-6 | 局部 damage rect | [P0-6](../phase0/P0-6_partial_damage_rect.md) | 3 周 |
| P0-7 | 性能基准脚本 | [P0-7](../phase0/P0-7_perf_baseline.md) | 1.5 周 |
| P0-8 | gap_analysis P0 清账 | [P0-8](../phase0/P0-8_gap_analysis_p0_cleanup.md) | 2 周 |
| P1-1 | DongDrawList C ABI | [P1-1](../phase1/P1-1_drawlist_c_abi.md) | 4 周 |
| P1-2 | `<host-view>` 嵌入 | [P1-2](../phase1/P1-2_host_view_embed.md) | 3 周 |
| P1-3 | DevTools v1 | [P1-3](../phase1/P1-3_devtools_v1.md) | 4 周 |
| P1-4 | Live reload | [P1-4](../phase1/P1-4_live_reload.md) | 2 周 |
| P1-5 | CSS Grid 子集 | [P1-5](../phase1/P1-5_css_grid_subset.md) | 4 周 |
| P1-6 | HDR 输出（基础） | [P1-6](../phase1/P1-6_hdr_output.md) | 2 周 |
| P1-7 | JS 引擎 benchmark | [P1-7](../phase1/P1-7_js_engine_benchmark.md) | 2 周 |
| P1-8 | Async layout/shaping (实验) | [P1-8](../phase1/P1-8_async_layout_shaping.md) | 4 周 |
| P1-9 | gap_analysis P1 清账 | [P1-9](../phase1/P1-9_gap_analysis_p1_cleanup.md) | 4 周 |
| P2-1 | World text (billboard) | [P2-1](../phase2/P2-1_world_text.md) | 3 周 |
| P2-2 | UI Decal | [P2-2](../phase2/P2-2_decal.md) | 3 周 |
| P2-3 | World overlay quad | [P2-3](../phase2/P2-3_world_overlay.md) | 3 周 |
| P2-4 | Lottie / Rive | [P2-4](../phase2/P2-4_lottie_rive.md) | 4 周 |
| P2-5 | dong-ui 组件库 | [P2-5](../phase2/P2-5_dong_ui_library.md) | 8 周 |
| P2-6 | TS 类型 + npm 发布 | [P2-6](../phase2/P2-6_typescript_npm.md) | 3 周 |
| P2-7 | 可视化编辑器 dogfood | [P2-7](../phase2/P2-7_visual_editor.md) | 6 周 |
| P2-8 | `.dpkg` 资源包 | [P2-8](../phase2/P2-8_dpkg_resource_pack.md) | 4 周 |
| P2-9 | Async 默认开启 | [P2-9](../phase2/P2-9_async_default_on.md) | 1 周 |
| P2-10 | gap_analysis P2 清账 | [P2-10](../phase2/P2-10_gap_analysis_p2_cleanup.md) | 6 周 |

---

## 2. 依赖矩阵（who-depends-on-whom）

### 2.1 硬依赖（"上游未 merge 不能开工"）

| Feature | 直接上游 | 备注 |
|---|---|---|
| P0-1 | — | 根节点 |
| P0-2 | **P0-1** | 复用 Uber 路径 material=5 |
| P0-3 | **P0-1** | 共享 material=6 fast path；conic 单独 pipeline 不阻塞 |
| P0-4 | — | 独立 |
| P0-5 | — | 独立 |
| P0-6 | **P0-1** | 与 Uber 路径同改 `painter.cpp` / `gpu_compiler.cpp` |
| P0-7 | — | 越早越好；其他任务的验收依赖它 |
| P0-8 | — | 散点修补，互不冲突 |
| P1-1 | **P0-1, P0-2, P0-3, P0-6** | 命令集要稳定再冻结 ABI |
| P1-2 | **P1-1** | host-view 命令是 DrawList 的一种 |
| P1-3 | **P0-6, P0-7** | DevTools 用 invalidation + perf counter |
| P1-4 | — | 独立 |
| P1-5 | — | 独立（Yoga 协同 / 自研 GridEngine） |
| P1-6 | — | 独立（shader / swapchain 改动） |
| P1-7 | — | 独立（benchmark 脚本） |
| P1-8 | **P0-6** | 复用 invalidation 体系 |
| P1-9 | — | 散点修补 |
| P2-1 | **P0-1**（Uber 文字材质 fast path 可选）；Slug pipeline 已有 | 也建议晚于 P1-1 以共享 GlobalShared |
| P2-2 | — | 独立 3D pass |
| P2-3 | **P1-2** | 内部复用 host-view 投影机制 |
| P2-4 | — | 独立（接 rlottie / rive-cpp） |
| P2-5 | **P0-3, P0-4, P2-1** | HealthBar / focus / DamageNumber |
| P2-6 | **P1-1**（类型源）；其余可早做 | core-types 子包要等 ABI 稳 |
| P2-7 | **P1-3, P1-4, P2-5** | dogfood 需 DevTools + reload + ui 库 |
| P2-8 | — | 独立 |
| P2-9 | **P1-8 + ≥1 月稳定窗** | 时间窗依赖，不只是 merge 依赖 |
| P2-10 | — | 散点修补 |

### 2.2 软依赖（"建议错峰，否则易冲突"）

物理冲突（同一文件 / 同一系统）需要串行或仔细 rebase：

| 群组 | 共享改动区域 | 建议排队顺序 |
|---|---|---|
| **Render core** P0-1, P0-2, P0-3, P0-6, P1-1, P1-6 | `src/render/painter*.cpp`, `src/render/gpu_ir.hpp`, shaders | P0-1 → (P0-2/P0-3 并行) → P0-6 → P1-1 → P1-6 |
| **Layout core** P0-6, P1-5, P1-8 | `src/layout/*` | P0-6 → P1-5、P1-8 并行 |
| **DOM/CSS 散点** P0-8, P1-9, P2-10 | `src/dom/css/*`, `src/dom/*_element.cpp` | 每批内部按子项细分；不同批避免同周做 |
| **C ABI 头** P1-1, P1-2, P0-4, P0-5, P1-6, P2-1/2/3, P2-8 | `dong/include/*.h` | 各 feature 各自加自己的头；P1-1 是 ABI 主版本号闸门 |
| **AppCore** P1-3, P1-4, P2-7 | `dong/appcore/*` | P1-3、P1-4 可并行；P2-7 等两者 |
| **JS 引擎层** P1-7, P1-8, P2-9 | `src/script/*` | P1-7 是只读 benchmark 不冲突；P1-8/P2-9 串行 |

### 2.3 冲突等级（cli 并行决策依据）

| 等级 | 含义 | 调度规则 |
|---|---|---|
| **R**（Red） | 同一群组、同一周内必冲突 | 一组只能一个 in_progress |
| **Y**（Yellow） | 同一群组、不同子模块 | 允许并行，merge 时强制 rebase + verify |
| **G**（Green） | 跨群组、跨目录 | 自由并行 |

> 实操：每个 feature 在 `state-ledger.jsonl` 里登记 `groups: [...]`；调度器在派发新 worker 前查"当前 in_progress 的 groups 与候选是否同 R 群组"。

---

## 3. Wave 划分（推荐执行批次）

> 假设并发 cap = **N**（默认 4）。每个 Wave 内任务尽量都是 **G/Y**，串行的 **R** 跨 Wave 拉开。

### Wave 0 — 启动期（启动后立刻可全发）

| Feature | 群组 | 备注 |
|---|---|---|
| **P0-7** Perf baseline | tools | **必须最先跑 schema 部分**，给后续验收提供 `perf_baseline.py` |
| P0-1 Uber Quad | render-core (R) | render-core 群组的"独占占位" |
| P0-4 Gamepad nav | input | G |
| P0-5 IME | input | G |
| P0-8 gap P0 cleanup | dom-scatter | Y（按子批拆，不要一次合 8 批） |
| P1-4 Live reload | appcore | G |
| P1-7 JS bench | tools-readonly | G |

并行度：建议 **4–6 同时**（取 N=4 即可，多余的排队）。

### Wave 1 — render-core 后续（等 P0-1 主干 merge 后）

触发条件：`P0-1` 状态 = `merged`。

| Feature | 群组 | 备注 |
|---|---|---|
| P0-2 9-slice | render-core | Y vs P0-3，需各自加 material 互不交叉，merge 顺序由 orchestrator 串 |
| P0-3 mask + conic | render-core | Y vs P0-2 |
| P1-5 CSS Grid | layout | G |
| P1-6 HDR | render-core / shader | Y vs P0-2/P0-3，建议 Wave 1 末尾再上 |
| P1-9 gap P1 cleanup | dom-scatter | 按子批 |

并行度：4 同时；同时 in_progress 中的 render-core 任务 ≤ 2。

### Wave 2 — invalidation + ABI 冻结

触发条件：`P0-2 / P0-3 / P0-6` 全部 ready。

| Feature | 群组 | 备注 |
|---|---|---|
| P0-6 Damage rect | render-core + layout-core | R 占位 |
| P1-8 Async layout | layout / script | 等 P0-6 拿到 invalidation API |
| P1-1 DrawList ABI | render-core / abi | 等所有命令稳定才冻结 |

并行度：3 同时；**P0-6 与 P1-1 不允许并行 in_progress**（都改 painter→gpu_ir 主链）。建议先 P0-6 → 再 P1-1。

### Wave 3 — DevTools / 嵌入

触发条件：`P1-1, P0-6, P0-7` 全 merged。

| Feature | 群组 | 备注 |
|---|---|---|
| P1-2 host-view | abi / appcore | G |
| P1-3 DevTools | appcore | G vs P1-2 |
| P2-6 npm core-types | js-tooling | 可启动 core-types 子包（绑 P1-1 ABI 版本） |

### Wave 4 — Phase 2 大爆发（独立分支为主）

触发条件：Phase 1 主干 (P1-1/2/3/4) merged。

| Feature | 群组 | 备注 |
|---|---|---|
| P2-1 World text | render-3d | G |
| P2-2 Decal | render-3d | Y vs P2-1（共享 3D pass 但不同 shader） |
| P2-4 Lottie/Rive | render / vector | Y vs P2-1（共享 Slug 输入） |
| P2-8 dpkg | tooling / runtime-loader | G |
| P2-10 gap P2 cleanup | dom-scatter | 按子批 |

并行度：4–5 同时；render-3d 群组内同时 ≤ 2。

### Wave 5 — 依赖前面

触发条件：见每行。

| Feature | 上游 | 备注 |
|---|---|---|
| P2-3 World overlay | P1-2 merged | render-3d + appcore |
| P2-5 dong-ui | P0-3, P0-4, P2-1 merged | 大块组件库；内部仍可拆并行 PR |
| P2-9 Async default-on | P1-8 merged + ≥30 天 perf 无回归 | 仅 1 周代码，但要等观察窗 |
| P2-6 npm 完整 | P1-1, P2-5 merged | core-types/react/preact/ui 全发 |

### Wave 6 — 收尾

触发条件：`P1-3, P1-4, P2-5` merged。

| Feature | 上游 | 备注 |
|---|---|---|
| P2-7 dong-editor dogfood | P1-3, P1-4, P2-5 | 唯一一个真正的"全栈集成"任务；最后做 |

---

## 4. DAG 摘要

```
P0-7 ─────────────────────────────────────────────► (供给所有验收)
P0-4, P0-5, P0-8, P1-4, P1-7 ────────────────────► (独立)

P0-1 ─┬─► P0-2 ─┐
      ├─► P0-3 ─┤
      ├─► P0-6 ─┼─► P1-1 ─► P1-2 ─► P2-3
      └─────────┘                ╲
                                  ╲
P0-6 ─► P1-8 ──────────────► P2-9
P0-7, P0-6 ─► P1-3 ────────► P2-7
P1-4 ─────────────────────► P2-7
P0-3, P0-4 ─┐
P2-1 ───────┴─► P2-5 ─────► P2-7
P1-1 ─► P2-6 (core-types) ─┐
P2-5 ──────────────────────┴─► P2-6 (full npm)

P1-5, P1-6, P1-9, P2-2, P2-4, P2-8, P2-10  独立
```

---

## 5. 调度参数（喂给 orchestrator）

| 参数 | 默认 | 说明 |
|---|---|---|
| `MAX_PARALLEL_WORKERS` | 4 | cli 同时跑的 feature 上限 |
| `MAX_PER_GROUP_R` | 1 | 同一 R 群组同时 in_progress 上限 |
| `MAX_PER_GROUP_Y` | 2 | 同一 Y 群组同时 in_progress 上限 |
| `MERGE_SERIAL` | true | 合并到 `dev_next` 强制串行（避免 ff 竞态） |
| `RETRY_VERIFY` | 2 | verify 失败自动重跑 cli 的最大次数 |
| `OBSERVE_WINDOW_DAYS` | 30 | P2-9 类"观察期"任务最低等待天数 |

---

## 6. 与 orchestrator 的接口

orchestrator 每次 tick 时：

1. 读取 `state-ledger.jsonl`（[schema](./state-ledger-schema.md)）。
2. 遍历未完成 feature，对照本文件的 § 2.1 / § 2.2 / § 2.3 计算 `eligible = true|false`。
3. 在 `eligible` 子集里，按 § 5 调度参数派发新 worker，最多 `MAX_PARALLEL_WORKERS - 当前活跃数` 个。
4. 工作流详见 [`orchestrator-prompt.md`](./orchestrator-prompt.md) § 3。

---

## 7. 决策记录

| 日期 | 决策 |
|---|---|
| 2026-04-17 | 依赖矩阵 v1；以 `dev_next` 为单一集成分支 |
| 2026-04-17 | render-core 群组同时 in_progress 硬限 = 2 |
| 2026-04-17 | merge 串行（不并行 ff），避免显示历史交错 |
