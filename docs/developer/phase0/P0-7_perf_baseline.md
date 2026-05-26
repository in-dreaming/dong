# P0-7 — 三轨性能基线脚本

> 上游：[`docs/roadmap.md`](../roadmap.md) Phase 0 P0-7
> 性能预算：[`docs/developer/perf-budget.md`](../perf_budget.md)
>
> 状态：方案 v1 / 2026-04-17

---

## 1. 目标

把 [`perf_budget.md`](../perf_budget.md) 6 个场景的指标做成**一键可跑、CI 可阻断、可时序对比**的脚本。具体：

- 一条命令跑全集 / 单场景。
- 输出结构化 `results_baseline_*.json`（与 `auto_profile_loop.py` 对齐）。
- 自动判定 Soft / Hard 阈值，超 Hard 退非 0 退出码（CI 阻断）。
- 与上次 main 对比，输出趋势报表（Markdown）。
- DOM / Scene Graph / Direct Draw 三轨各自有独立场景，单独度量。

---

## 2. 现状

已有：
- `dong/scripts/tools/auto_profile_loop.py` — sweep + trace 解析（见 `docs/developer/optimization/profile-loop.md`）。
- `dong/tmp/trace_summary.py` — Top N self time。
- `dong/scripts/tools/run_baseline_compare.py` — 像素 baseline。
- `dong/scripts/tools/run_multiframe_regress.py` — 非确定性回归。

缺口：
- 没有"按场景跑 + 自动判定阈值 + 与历史对比"的统一入口。
- 没有 Direct Draw / Scene Graph 模式的固定基线场景。
- 没有 CI 集成。

---

## 3. 设计

### 3.1 新增脚本：`dong/scripts/tools/perf_baseline.py`

```bash
# 跑所有场景一次（Reference 机型）
python perf_baseline.py --machine ref

# 单场景多轮取中位
python perf_baseline.py --scene S1 --iters 3

# 与上次 main 对比
python perf_baseline.py --diff main --report report.md

# CI 模式：超 Hard 阈值 exit 1
python perf_baseline.py --ci

# 仅检查 draw call（不跑性能）
python perf_baseline.py --metric draw --scene S1
```

### 3.2 场景定义（`dong/scripts/tools/perf_scenes.yaml`）

```yaml
S1:
  name: "Game HUD (Scene Graph)"
  track: scene_graph
  driver: dong_app
  args: ["--html", "data/gamelikeui/game_ui2.html"]
  warmup_ms: 2000
  run_ms: 5000
  metrics:
    fps_static:        { soft: 240,  hard: 200 }       # present-only
    fps_attr_change:   { soft: 144,  hard: 120 }
    draw_call:         { soft: 30,   hard: 50 }
    gpu_time_us:       { soft: 2000, hard: 4000 }
    layout_paint_us:   { soft: 1000, hard: 3000 }
    glyph_atlas_mb:    { soft: 64,   hard: 128 }
  scripted_actions: # 可选：每帧执行
    - frame: 100
      action: "scene_set_attr id=hp value=0.7"

S2:
  name: "DOM Business Panel (React)"
  track: dom
  driver: dong_app
  args: ["--html", "data/react-game-ui/index.html"]
  env:
    DONG_SCRIPT_TIMEOUT_MS: "10000"
  warmup_ms: 3000
  run_ms: 5000
  metrics:
    fps_idle:               { soft: 60,   hard: 30 }
    fps_interactive:        { soft: 60,   hard: 30 }
    app_ms:                 { soft: 4,    hard: 10 }
    layout_paint_ms:        { soft: 4,    hard: 8 }
    draw_call:              { soft: 80,   hard: 150 }
    full_repaint_per_frame: { soft: 0,    hard: 1 }    # P0-6 关键指标

S3:
  name: "Direct Draw (Text Flow)"
  track: direct_draw
  driver: dong_app
  args: ["--html", "data/tests/test_text_flow_dynamic.html"]
  warmup_ms: 1000
  run_ms: 5000
  metrics:
    fps_dynamic:        { soft: 300, hard: 144 }
    glyph_run_count:    { soft: 5,   hard: 10 }
    text_cache_hit:     { soft: 0.95, hard: 0.80 }

S4:
  name: "3D Multi-Screen (GlobalShared)"
  track: multi
  driver: 3d_screens_simple
  args: []
  warmup_ms: 3000
  run_ms: 5000
  metrics:
    fps_static:          { soft: 60,  hard: 30 }
    gpu_mem_saving_pct:  { soft: 80,  hard: 50 }
    total_mem_mb:        { soft: 800, hard: 1500 }

S6:
  name: "Editor Workflow (dock_demo)"
  track: dom
  driver: dong_app
  args: ["--html", "data/dock_demo_inspector.html"]
  scripted_actions:
    - frame: 0
      action: "mouse_down 100 30"     # 抓 panel header
    - { frame: 60..120, action: "mouse_move +1 0 each" }   # 拖动 60 帧
    - frame: 121
      action: "mouse_up"
  metrics:
    fps_during_drag:                  { soft: 60,  hard: 30 }
    full_repaint_count_during_drag:   { soft: 0,   hard: 1 }
    keystroke_to_pixel_ms:            { soft: 16,  hard: 33 }
```

> S5（World Space UI）暂不加入，到 Phase 2 再补。

### 3.3 Driver 模式

`perf_baseline.py` 用既有环境变量驱动：

```
DONG_BENCH_AUTOSTOP=1
DONG_BENCH_WARMUP_MS=<warmup>
DONG_BENCH_RUN_MS=<run>
DONG_BENCH_SCENARIO_SCRIPT=<scripted_actions 编译后的字节序列>
```

`dong_app` 已有部分支持（见 `docs/developer/optimization/profile-loop.md`），缺的部分按需扩展（注入脚本动作）。

### 3.4 输入合成

scripted_actions 通过 `dong_app` 内置脚本解释器执行，每条动作转换为 `dong_engine_send_*`。**不**走系统鼠标/键盘（避免外部干扰）。

支持动作（v1）：

| 动作 | 参数 |
|---|---|
| `mouse_move x y` | 绝对坐标 |
| `mouse_move +dx +dy each` | 相对，配合 `frame: a..b` 每帧执行 |
| `mouse_down x y` / `mouse_up` | 默认 button 0 |
| `key_down keycode` / `key_up keycode` | SDL keycode |
| `text "abc"` | UTF-8 |
| `scene_set_attr id=... value=...` | Scene Graph 模式 |
| `eval_js "..."` | 任意 JS |

### 3.5 指标采集

- **FPS**：从 trace 中 `Frame` scope 计数 / 采样秒数（已有 `auto_profile_loop` 算法）。
- **draw_call**：trace 中所有 `Draw*` scope 计数 / Frame 数；或新增 driver-side counter。
- **gpu_time_us / layout_paint_us / app_ms**：trace scope self time 求和。
- **full_repaint_per_frame**：新增 `commands_regenerated_this_tick_` counter（已有变量）/ Frame 数。
- **glyph_atlas_mb / total_mem_mb**：driver-side 直接查询 `dong_engine_query_atlas_stats` / `dong_engine_query_mem`（需新增 ABI）。
- **text_cache_hit**：TextShaper 内部 counter，新增导出。
- **gpu_mem_saving_pct**：GlobalShared 已有 metric。
- **keystroke_to_pixel_ms**：scripted action 注入 keystroke 后，下一帧 swapchain present 时间戳 - keystroke 时间戳。

新增 ABI（最小集）：

```c
typedef struct {
    uint64_t frames;
    uint64_t draw_calls;
    uint64_t commands_regenerated;
    uint64_t glyph_atlas_bytes;
    uint64_t image_atlas_bytes;
    uint64_t text_cache_hits;
    uint64_t text_cache_misses;
    double   accumulated_layout_paint_us;
    double   accumulated_app_us;
} dong_perf_counters_t;

void dong_engine_get_perf_counters(dong_engine_t* eng, dong_perf_counters_t* out);
void dong_engine_reset_perf_counters(dong_engine_t* eng);
```

### 3.6 输出格式

```
tmp/perf/
  results_S1_ref_20260420_103000.json
  results_S2_ref_20260420_103045.json
  ...
  report_20260420_103000.md          # 汇总
```

`results_*.json` 示例：

```json
{
  "scene": "S1",
  "track": "scene_graph",
  "machine": "ref",
  "iters": [
    { "fps_static": 2310, "draw_call": 68, "gpu_time_us": 6500, ... },
    { ... }
  ],
  "median": { "fps_static": 2305, "draw_call": 68, ... },
  "verdict": {
    "fps_static": "PASS_SOFT",
    "draw_call": "FAIL_HARD",
    "overall": "FAIL"
  }
}
```

报表（Markdown）：

```
| Scene | Metric | Soft | Hard | Current | vs Last | Verdict |
|---|---|---|---|---|---|---|
| S1 | draw_call | 30 | 50 | 68 | -3 | ❌ HARD |
| S1 | fps_static | 240 | 200 | 2305 | +45 | ✅ |
```

### 3.7 CI 集成

`.github/workflows/perf.yml`（草案）：

```yaml
on: [pull_request]
jobs:
  perf:
    runs-on: self-hosted-gpu  # 必须真实 GPU
    steps:
      - run: zig build
      - run: python dong/scripts/tools/perf_baseline.py --ci --machine ref \
              --scenes S1,S2,S3 --diff main --report perf.md
      - uses: actions/upload-artifact@v4
        with: { path: perf.md }
      - if: failure()
        run: echo "::error::Perf budget violated, see perf.md"
```

CI 不需要 self-hosted GPU 时（开发阶段）：用 software renderer 跑，仅校验 draw_call / repaint_count 等不依赖 GPU 性能的指标。

---

## 4. 实施步骤

| Step | 范围 |
|---|---|
| **S1 — 新增 ABI: `dong_engine_get_perf_counters`** | core 内部已有半数 counter；汇总并暴露 |
| **S2 — `perf_baseline.py` v0** | 仅 S1 / S3 / S4（无 scripted action 的场景），跑通 + 输出 JSON |
| **S3 — Scripted actions 驱动** | dong_app 加 `--bench-script` 参数；S2 / S6 跑通 |
| **S4 — 阈值判定 + Markdown report** | verdict 计算 + report 生成 |
| **S5 — `--diff main` + 历史对比** | 从 git tag 拉上一次 main 的 results JSON 对比 |
| **S6 — CI 接入** | self-hosted runner / software fallback |

S2–S6 可与 P0-1 / P0-6 并行。S1 是其它 Phase 0 任务的验证依赖，**优先做**。

---

## 5. 通过验证规则

### 5.1 Hard

| 项 | 阈值 |
|---|---|
| `perf_baseline.py --scene S1` 无任何报错跑出 results.json | 必须 |
| 6 个场景中至少 4 个（S1/S2/S3/S4）跑通 | 必须 |
| `--ci` 在所有 Phase 0 任务未完成时仍可识别已实测的指标超 Hard 并 exit 1 | 必须 |
| 像素回归脚本 `run_baseline_compare.py` 仍可用 | 不破坏既有 |

### 5.2 Soft

| 项 | 阈值 |
|---|---|
| 单场景一次跑完时间 | ≤ 30s |
| 全场景一次跑完时间 | ≤ 5 min |
| 多次跑同一场景指标方差 | < 5% |
| Markdown report 可读性 | 表格、颜色（emoji）、链接 |

### 5.3 Self-test

`perf_baseline.py` 自身必须有：

```bash
# 不依赖真实运行，只测脚本逻辑
python -m pytest dong/scripts/tools/test_perf_baseline.py
```

测试内容：
- yaml 解析正确
- 阈值判定正确（Pass/Fail/SoftOnly）
- diff main 时找不到 baseline 友好降级（warn 而非崩）

---

## 6. 风险与回退

| 风险 | 缓解 |
|---|---|
| 不同机型 perf 抖动大 | machine 维度独立 baseline；`--machine ref/min` |
| 真实 GPU CI runner 难维护 | 提供 software fallback（仅校验非 GPU 指标） |
| scripted_actions 时序不稳 | 用 `frame: N` 而非 `time: ms`；dong_app 帧驱动确定性高 |
| trace 解析与 dong 内部 scope 名变更耦合 | scope 名集中在 `src/core/profiler.h`；改名时同步更新 perf_scenes.yaml |
| CI 红的太频繁 | 区分 Soft/Hard：Soft 仅 warn，Hard 才阻断；新增 metric 默认仅 Soft |

---

## 7. 不在本方案范围

- ❌ GPU profiling（per-shader 时间）—— 用 RenderDoc 手动
- ❌ 内存 leak 检测 —— 用 ASAN 手动
- ❌ JS heap 详细分析 —— 用 QuickJS GC 自带 stats
- ❌ Cross-platform mobile perf —— Phase 1 起做
- ❌ 跨 git 分支 long-term trend graph（持久化数据库）—— v1 只对比相邻两次

---

## 8. 完成后更新

- [ ] `docs/developer/perf-budget.md` § 6 测量工具与脚本表，把 `perf_baseline.py` 标 ✅
- [ ] `docs/developer/optimization/profile-loop.md` 末尾追加"perf_baseline.py 用法"
- [ ] `dong/include/dong.h` 文档化 `dong_engine_get_perf_counters`
- [ ] CI README 加 perf job 说明
