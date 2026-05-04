# Dong 性能预算（Perf Budget）

> 目的：把"什么场景下、必须在多少 ms / 多少 draw / 多少 MB 内跑完"写死，作为优化方向、PR 验收、CI 回归的统一标尺。
>
> 与 [`doc/positioning.md`](./positioning.md) 的能力边界、[`doc/roadmap.md`](./roadmap.md) 的阶段目标一一对应。
>
> 状态：v1 草案 2026-04-17。指标基于当前已实测数据 + Ultralight 路线合理推断；每个 Phase 结束后回填实测，必要时调整。

---

## 1. 总原则

1. **Frame Budget 倒推。** 先定每帧总预算，再分摊给 layout / paint / GPU / app。超的部分要么砍特性、要么换轨道（DOM → Scene Graph → Direct Draw）。
2. **三轨各有基线。** 同一类场景在不同轨道上的预算不同，不混在一起评。
3. **预算分两档：Soft / Hard。**
   - Soft = 期望值；超过触发 perf 警告，需要立项优化。
   - Hard = 红线；超过 PR 不准合，CI 报错。
4. **测试机型分两档：Reference / Min。**
   - Reference = 主流开发机（桌面 RTX 3060 / Apple M1）。基线 perf 跑这台。
   - Min = 最低支持配置（集显笔记本 / 中端手机骁龙 7 系级）。可降帧但不许卡顿。
5. **指标必须可自动测。** 见 § 6 测量工具与脚本。

---

## 2. Frame Budget 总账

| 目标帧率 | 总预算 / 帧 | 典型分配（Reference 机型） |
|---|---|---|
| **60 FPS** (大多数 PC / 主机游戏 UI) | 16.6 ms | App 4 / Layout+Paint 4 / GPU 6 / 留 2 |
| **120 FPS** (高刷主机 / 电竞) | 8.3 ms | App 2 / Layout+Paint 2 / GPU 3 / 留 1 |
| **240 FPS** (静态 UI 上限) | 4.1 ms | 只允许 present-only 路径，无完整重建 |

`App` = JS 执行 + react-reconciler diff + 业务逻辑；`Layout+Paint` = StyleEngine + Yoga + Painter + GPUCompiler；`GPU` = 实际 SDL_GPU 提交 + GPU 处理。

> Min 机型可降到 30 FPS，预算翻倍（33.3 ms），分摊比例同上。

---

## 3. 场景预算

### 3.1 场景 S1：游戏 HUD（Scene Graph 模式）

**典型代表**：`game_ui2.html`（血条 / 技能栏 / 背包格 / minimap 框 / 分数面板，~80 节点，~30 文字 run）

| 指标 | Soft | Hard | 当前实测 (Ref) |
|---|---|---|---|
| FPS（present-only 静态帧）| ≥ 240 | ≥ 200 | ~2300 |
| FPS（每帧改 1 个属性，scene graph O(1) 更新）| ≥ 144 | ≥ 120 | 待测 |
| GPU draw call / 帧 | ≤ 30 | ≤ 50 | **68** ⚠️ |
| GPU 时间 / 帧 (Ref) | ≤ 2 ms | ≤ 4 ms | ~6.5 ms（无 present-only） / ~0.4 ms（present-only） |
| Layout+Paint / 帧 | ≤ 1 ms | ≤ 3 ms | 待测 |
| Atlas 内存（GlyphAtlas） | ≤ 64 MB | ≤ 128 MB | ~40 MB（单字号） |
| 启动到首帧 | ≤ 200 ms | ≤ 500 ms | ~150 ms（无 React） |

**到期目标**：Phase 0 P0-1（Uber Quad）后 draw call 进入 Soft 区间。

---

### 3.2 场景 S2：DOM 业务面板（editor / 设置界面 / React app）

**典型代表**：`dock_demo`、`react-game-ui`、`react-todo-classic`（~200–500 节点，含 React reconciler）

| 指标 | Soft | Hard | 当前实测 (Ref) |
|---|---|---|---|
| FPS（idle / hover）| ≥ 60 | ≥ 30 | 60 |
| FPS（频繁交互：拖动 / 输入） | ≥ 60 | ≥ 30 | 待测 |
| App / 帧（React reconciler diff） | ≤ 4 ms | ≤ 10 ms | ~5–20 ms（QuickJS） |
| Layout+Paint / 帧 | ≤ 4 ms | ≤ 8 ms | 待测 |
| GPU draw call / 帧 | ≤ 80 | ≤ 150 | 待测 |
| GPU 时间 / 帧 (Ref) | ≤ 4 ms | ≤ 8 ms | 待测 |
| 拖动 panel 时**整树重建次数** | 0 | 1 | 当前 = 1（每帧）⚠️ |
| Live reload 端到端 (P1 后) | ≤ 500 ms | ≤ 1500 ms | 当前不支持 |
| 首屏（含 React bundle eval） | ≤ 500 ms | ≤ 1500 ms | ~100–500 ms（仅 eval） |
| JS heap | ≤ 30 MB | ≤ 80 MB | 待测 |

**到期目标**：
- Phase 0 P0-6（damage rect）后"整树重建"指标进入 0。
- Phase 1 P1-8（async shaping）后首屏阻塞 < 50 ms。

---

### 3.3 场景 S3：Direct Draw 高频动态（伤害字 / 粒子文字 / 雷达）

**典型代表**：`test_dual_mode.html`、`test_text_flow_dynamic.html`（DOM 静态背景 + Direct Draw 高频）

| 指标 | Soft | Hard | 当前实测 (Ref) |
|---|---|---|---|
| FPS（40 行动态文字流，每帧重生成）| ≥ 300 | ≥ 144 | 300–480 |
| FPS（500 个伤害浮字 instanced）| ≥ 144 | ≥ 60 | 待测（Phase 2 目标） |
| GPU draw call（合批后）| ≤ 5 | ≤ 10 | 1（合并 GlyphRun）|
| Direct Draw API 调用 / 帧 | ≤ 1000 | ≤ 5000 | — |
| 文字 cache 命中率 | ≥ 95% | ≥ 80% | 待测 |

---

### 3.4 场景 S4：3D 多 HTML 屏幕（GlobalShared）

**典型代表**：`3d_screens_simple`，23 个 800×600 HTML 屏幕在 3D 场景中

| 指标 | Soft | Hard | 当前实测 (Ref) |
|---|---|---|---|
| FPS（所有屏幕静态）| ≥ 60 | ≥ 30 | 待测 |
| FPS（3 个屏幕动画 / 帧）| ≥ 60 | ≥ 30 | 待测 |
| GlobalShared GPU 内存节省 | ≥ 80% | ≥ 50% | ~936 MB / 23 屏（实测 ~80%） |
| 单屏幕 update 时间 | ≤ 1 ms | ≤ 3 ms | 待测 |
| 总内存（23 屏幕） | ≤ 800 MB | ≤ 1.5 GB | 待测 |

---

### 3.5 场景 S5：World Space UI（Phase 2 起）

**典型代表**（Phase 2 设计）：500 个伤害浮字 + 30 个名字版 + 5 个 3D 终端 quad

| 指标 | Soft | Hard | 当前 |
|---|---|---|---|
| FPS（500 浮字 + 30 名字版）| ≥ 144 | ≥ 60 | — |
| 浮字 instanced draw call | ≤ 2 | ≤ 5 | — |
| 名字版 draw call | ≤ 5 | ≤ 30 | — |
| 与宿主深度 buffer 协同延迟 | ≤ 0.2 ms | ≤ 1 ms | — |

---

### 3.6 场景 S6：Editor 复杂工作流

**典型代表**（Phase 1 起）：dock_demo + 100 个 inspector field + 10 个嵌入 viewport

| 指标 | Soft | Hard | 当前 |
|---|---|---|---|
| 拖动一个 panel header 时 FPS | ≥ 60 | ≥ 30 | 待测 |
| 输入一个 field 字符 → 像素更新延迟 | ≤ 16 ms | ≤ 33 ms | 待测 |
| 切换 tab 端到端 | ≤ 50 ms | ≤ 200 ms | 待测 |
| DevTools 打开（F12）端到端（P1 后） | ≤ 200 ms | ≤ 500 ms | 不支持 |

---

## 4. Atlas / 资源预算

| 资源 | Soft | Hard | 备注 |
|---|---|---|---|
| GlyphAtlas（MSDF） | ≤ 128 MB | ≤ 256 MB | 4 档分辨率（128/192/256/384px） |
| Slug Curve + Band 纹理 | ≤ 64 MB | ≤ 128 MB | 4096×2048 + 4096×动态 |
| ImageAtlas | ≤ 256 MB | ≤ 512 MB | 单 view；多 view 走 GlobalShared |
| Shader 字节码（runtime） | ≤ 4 MB | ≤ 8 MB | SPIRV / DXIL / MSL |
| JS heap | ≤ 30 MB | ≤ 80 MB | 单 view；不含 React/Preact bundle |

---

## 5. 启动延迟预算

| 阶段 | Soft | Hard |
|---|---|---|
| dong.dll 加载 + Platform init | ≤ 100 ms | ≤ 300 ms |
| 字体加载（FreeType + Slug 预生成） | ≤ 100 ms | ≤ 300 ms |
| HTML 解析 + 首次 layout | ≤ 50 ms | ≤ 200 ms |
| React/Preact bundle eval（首次） | ≤ 100 ms | ≤ 500 ms |
| 首帧渲染 | ≤ 50 ms | ≤ 200 ms |
| **冷启动到首帧（无 React）** | ≤ 300 ms | ≤ 800 ms |
| **冷启动到首帧（含 React bundle）** | ≤ 600 ms | ≤ 1500 ms |

---

## 6. 测量工具与脚本

### 6.1 已有

| 工具 | 用途 | 文档 |
|---|---|---|
| `dong/scripts/tools/auto_profile_loop.py` | 自动 build + run + 采样 trace | `doc/summary/opt.md` |
| `dong/tmp/trace_summary.py` | 从 trace 中找 Top N 自耗时 scope | 同上 |
| `examples/html_render_test.cpp` | headless 多帧渲染 | `doc/summary/architecture.md` |
| `scripts/tools/run_baseline_compare.py` | 浏览器 baseline 像素 diff | `doc/summary/architecture.md` |

### 6.2 待建（Phase 0 P0-7）

`dong/scripts/tools/perf_baseline.py`：

- 输入：场景 ID（S1–S6）、机型档（Ref/Min）、迭代次数。
- 输出：`results_baseline_{scene}_{machine}_{stamp}.json`，包含 § 3 各场景的全部指标。
- CI 集成：每个 PR 跑 S1 / S2 / S3 三个核心场景，与上一次 main 对比，超出 Hard 阈值阻塞合入；超出 Soft 阈值发出 warning。

### 6.3 待建（Phase 1 P1-3 副产品）

DevTools 内置一个 Perf HUD overlay：实时显示 FPS / draw / atlas usage / JS heap，方便手动在游戏内验证预算。

---

## 7. 预算违反时的处理流程

1. **新功能 PR 触发预算超标 (Hard)**：必须在 PR 内修复或回退该功能。
2. **新功能 PR 触发预算超标 (Soft)**：允许合入，但必须立 issue + 在 `roadmap.md` 当前 Phase 内排期修复。
3. **历史代码 perf 回归（与上周 main 对比）**：触发 git bisect，定位首个超标 PR；必要时 revert。
4. **预算本身定低了 / 定高了**：在 `§ 8` 决策记录里追加调整，给出实测数据 + 调整原因。

---

## 8. 决策记录与变更（Change Log）

| 日期 | 变更 | 说明 |
|---|---|---|
| 2026-04-17 | v1 创建 | 基线指标主要来自 `doc/重要特性.md` § GPU 性能优化、`doc/opt/20260331.md`、`doc/arch/render_pipeline.md`、`doc/arch/arch_font.md`。S5 / S6 等 Phase 2/1 场景的指标为路线图目标，待实测后回填。 |

后续 Phase 完成后回填"当前实测"列；预算调整必须在此处留痕。
