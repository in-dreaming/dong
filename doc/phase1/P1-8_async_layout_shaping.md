# P1-8 — Async Layout / Shaping（实验）

> 上游：[`doc/roadmap.md`](../roadmap.md) Phase 1 P1-8
> 性能门槛：[`doc/perf_budget.md`](../perf_budget.md) § 3.2（首屏阻塞 < 50 ms）
>
> 状态：方案 v1（**实验性**）/ 2026-04-17

---

## 1. 目标

把 dong 当前**单线程**的 layout / 文字 shaping 切到 worker thread，让主线程只跑命令编译与提交，降低主线程阻塞时间：

- 首次 React bundle eval 后的首屏布局阻塞从 100–500 ms → ≤ 50 ms。
- 大型 DOM 树（≥ 1000 节点）resize / 字体变更时的 reflow 不冻结输入响应。
- 文字 shaping（HarfBuzz）耗时占比高的页面（中文长文 / Slug 路径）输入延迟下降。

**实验性**：本任务在 Phase 1 内交付 v1 实验版本（env var 启用）；Phase 2 P2-9 才默认开启。

---

## 2. 现状

| 项 | 现状 |
|---|---|
| Layout 线程 | 主线程（Yoga 同步） |
| Shaping 线程 | 主线程（HarfBuzz 同步） |
| 缓存 | TextShaper 有 static cache（[`doc/arch/arch_font.md`](../arch/arch_font.md) § 9） |
| QuickJS | 严格单线程（不允许跨线程访问 JS context） |
| `prepare_resources / execute` | 已分离阶段；准备阶段可异步 |

代码索引：
- Layout：`dong/src/layout/layout_engine.cpp`
- Text shaper：`dong/src/render/text_shaper.cpp`
- Painter / Engine View：`dong/src/render/painter.cpp` / `src/core/engine_view.cpp`

---

## 3. 设计

### 3.1 关键约束

- **JS context 严格主线程**：QuickJS / DOM 树操作只能在主线程。
- **Layout 输入快照可并行**：把节点的 ComputedStyle + 子树结构快照传到 worker，worker 内调 Yoga 算 box；返回结果合并回 DOM。
- **Shaping 输入快照可并行**：text run 的 (字符串, font, font_size, letter-spacing) 是纯输入，可后台 shape。
- **Painter / GPUCompiler / Driver 仍在主线程**：渲染序列必须在主线程提交（GPU plugin 不要求线程安全）。

### 3.2 双队列架构

```
主线程
 │  DOM 操作 / JS / 输入 / paint+compile+submit
 │
 ├─ submit LayoutTask(snapshot) ────────► [LayoutWorker]
 │                                       Yoga calculate
 │  ◄───────── result + dirty rect ──────│
 │
 ├─ submit ShapeTask(text run) ─────────► [ShaperWorker]
 │                                       HarfBuzz shape + cache fill
 │  ◄────── glyphs + advances ───────────│
 │
 (同帧无可用结果时：用上一帧布局/形状继续渲染；下一帧用新结果；
  或者降级到 sync 模式：直接在主线程算)
```

**两个 worker thread**（不是 thread pool）：
- 1 × LayoutWorker（Yoga 大概率 CPU-bound）
- 1 × ShaperWorker（HarfBuzz / FreeType IO）

**双队列实现**：
- `concurrentqueue.h`（moodycamel 的无锁队列；MIT；header-only），或简单 `mutex + condvar` 队列。

### 3.3 任务设计

#### LayoutTask

```cpp
struct LayoutTask {
    uint64_t  generation;       // 主线程递增
    DOMNode*  root;             // 失效 root（与 P0-6 协同）
    LayoutSnapshot snapshot;    // 子树结构 + 各节点 computed style 副本
    Size      available_size;
    // 完成回调（在主线程消费）
};

struct LayoutResult {
    uint64_t  generation;
    std::vector<NodeBoxResult> boxes;   // {node_id, x, y, w, h}
};
```

**LayoutSnapshot**：必须是不引用 DOM 节点裸指针的纯数据结构。设计上把 ComputedStyle 复制（小，~200 bytes / 节点）、子树边 vector、Yoga 节点 ID 数组。

#### ShapeTask

```cpp
struct ShapeTask {
    uint64_t generation;
    TextShapeKey key;       // 已有 cache key
    std::string  text;
    FontHandle   font;
    float        size, letter_spacing;
};

struct ShapeResult {
    uint64_t generation;
    TextShapeKey key;
    ShapedText shaped;
};
```

### 3.4 同步策略：相邻两帧

```
Frame N (主线程):
  1. JS / DOM 修改
  2. 收集 invalidations (P0-6)
  3. 提交 LayoutTask 给 worker
  4. 用 frame N-1 的 layout 结果继续渲染 (paint + submit)
  5. 在 paint 中遇到未 ready 的 text run → 提交 ShapeTask；本帧文字用 placeholder（透明 / 上一帧 cache）

Frame N+1 (主线程):
  1. 取 worker 已完成结果
  2. 把 box / shape 写回 DOM 节点
  3. paint with frame N+1 results
```

**视觉效果**：业务方可能感知 1 帧延迟（layout 后下帧才看到效果）。对游戏 UI 这通常可接受（16 ms）。

业务侧可强制同步：

```c
// 强制同步：阻塞等 worker 完成；用于初始化或不允许延迟的场景
int dong_engine_force_sync_layout(dong_engine_t* eng);
```

### 3.5 Fallback：worker 不存在或满

| 触发 | 行为 |
|---|---|
| `DONG_ASYNC_LAYOUT=0`（默认）| 主线程同步走（与现状一致） |
| `DONG_ASYNC_LAYOUT=1` 但 worker 队列长度 > N | 主线程同步处理本任务；log warn |
| Worker 异常（捕获到 Yoga assert）| 该任务回主线程同步重做；下一帧禁用 worker；log error |
| 节点结构在 snapshot 与回写之间被 JS 删除 | 回写时按 generation 检查；过期结果丢弃 |

### 3.6 一致性边界

下列场景 dong **强制同步**（暂存接口禁止异步）：

| 场景 | 原因 |
|---|---|
| `getBoundingClientRect` / `offsetWidth` 等几何 query | 用户期望同步可见 |
| `scrollIntoView` | 同上 |
| 焦点变化 / hit-test | 必须基于最新 box |
| Live reload / first paint | 用户期望立即看到 |
| 业务调 `dong_engine_force_sync_layout` | 显式 |

实现：在异步路径中维护"async-pending generation"；任何同步 query 触发 `force_sync_layout()`，等当前异步任务完成后才返回结果。

### 3.7 与 P0-6（damage rect）协同

- Invalidation 收集仍在主线程。
- LayoutTask 的 `root` 来自 P0-6 的 LCA 计算结果；只对该子树做异步 layout。
- 子树越小，async 收益越小（< 10 节点直接同步）。
- 阈值：节点数 < 10 → 同步；≥ 10 → 异步。可调。

### 3.8 数据冲突保护

```
主线程修改 ComputedStyle 时：
  if (该节点正在 async layout):
    → wait 完成 + 重新提交（保证一致性）
    → 或者 flag "pending re-layout"，当前结果回来后丢弃
```

简化：dong 维护**单个 inflight LayoutTask**（不允许并发多任务），新提交先等旧任务结束。Yoga 单次 calculate 通常 < 5 ms，对 60 FPS 帧而言几乎不抖。

ShapeTask 可并发（多 text run 互不影响），但每 key 只一份 inflight。

### 3.9 内存与 lifetime

- LayoutSnapshot 用 `unique_ptr` 持有；worker 完成后释放。
- ShapeResult 直接写入 TextShaper static cache（线程安全 cache，参考 [`doc/arch/arch_font.md`](../arch/arch_font.md)）。
- Worker 析构：dong_engine_destroy 时给 worker 发 stop signal + join。

---

## 4. 实施步骤

| Step | 范围 |
|---|---|
| **S1 — Worker thread 骨架** | LayoutWorker / ShaperWorker 启停；空任务循环 |
| **S2 — LayoutSnapshot + 同步双向** | snapshot 拷贝；worker 内 Yoga 计算；结果回写；`DONG_ASYNC_LAYOUT=1` 启用；既有 baseline 不变 |
| **S3 — ShapeTask 异步** | text shaper 提交后台；本帧 placeholder；下帧 cache 命中 |
| **S4 — Fallback / force_sync API** | 几何 query 强制同步；测试用例验证 |
| **S5 — Perf 度量 + DevTools 显示** | DevTools 显示当前 inflight tasks；首屏阻塞 metric |
| **S6 — 实验报告** | 产出 `doc/perf/async_layout_report.md` 决定 Phase 2 是否默认开启 |

---

## 5. 通过验证规则

### 5.1 Hard

| 项 | 阈值 |
|---|---|
| `DONG_ASYNC_LAYOUT=1` 下既有 baseline 全集像素一致 | ≥ 99% 通过（允许下帧延迟视觉，但稳定后像素一致） |
| 几何 query API 仍同步可用（`getBoundingClientRect` 即时返回正确） | 必须 |
| `dong_engine_destroy` 不泄漏 worker thread | 必须 |
| Worker 异常不影响主线程崩溃 | 必须 |
| `DONG_ASYNC_LAYOUT=0`（默认）行为与现状完全一致 | 必须 |
| 首次 React bundle eval 后的首屏布局阻塞 ≤ 50 ms | 必须（[`perf_budget.md`](../perf_budget.md) § 3.2） |
| 1000 节点 resize 时主线程帧不掉到 30 FPS 以下 | 必须 |

### 5.2 Soft

| 项 | 阈值 |
|---|---|
| 异步路径带来的视觉延迟 | ≤ 1 帧（16ms） |
| Layout snapshot 拷贝开销 | ≤ 0.5 ms（1000 节点） |
| Worker 唤醒延迟 | < 0.1 ms |
| Async layout vs sync layout（同样树）总时间 | ≥ 1.3× 提升（即并行确实有效） |
| Force sync 调用阻塞 | < 5 ms（典型场景） |

### 5.3 必须新增的测试

| 文件 | 验证 |
|---|---|
| `tests/async/test_async_layout_basic.cpp` | snapshot → worker → 写回 |
| `tests/async/test_async_layout_invalidation.cpp` | snapshot 与 JS 修改竞态：generation check |
| `tests/async/test_force_sync_query.cpp` | `getBoundingClientRect` 在异步路径下仍正确 |
| `tests/async/test_async_shape.cpp` | text shape 异步 |
| `tests/async/test_worker_lifetime.cpp` | destroy 不泄漏 |
| `examples/data/tests/perf_async_resize.html` | 1000 节点频繁 resize |
| `dong/scripts/test_async_perf.py` | sync vs async 对比报告 |

### 5.4 验证命令

```bash
# 单元
zig build run-feature-tests --filter Async

# Perf
python dong/scripts/test_async_perf.py
# 期望输出：
#   sync layout p50 = 8.2ms, p99 = 15ms
#   async layout p50 = 1.1ms (主线程), worker p50 = 7.5ms (并行)
#   首屏阻塞 sync=320ms, async=42ms

# 像素稳态
DONG_ASYNC_LAYOUT=1 python dong/scripts/tools/run_baseline_compare.py --iters-per-test 3
# 第一帧可能不一致（延迟），稳定后必须一致
```

---

## 6. 风险与回退

| 风险 | 缓解 |
|---|---|
| 视觉延迟感知（业务方报"1 帧延迟"觉得不稳定） | 业务侧可一直跑同步路径（默认即关）；force_sync API |
| Snapshot 拷贝开销超过节省 | 阈值：< 10 节点同步；监控开销 |
| Yoga 不是线程安全 → 多 worker 同时调用崩 | 单 LayoutWorker；Yoga node 仅本 worker 访问 |
| HarfBuzz 与 FreeType 线程安全 | Shaper 内每 worker 独立 hb_buffer；FreeType 全局 lock |
| Slug 字体的曲线纹理上传必须主线程 | shaping 只算 glyph index/metrics；上传仍主线程 |
| QuickJS 误调用 → 崩 | 显式禁止 worker 内访问 JS；assert |
| Async 与 Live Reload (P1-4) 时序混 | reload 时 force_sync_layout |

---

## 7. 不在本方案范围

- ❌ Painter / GPU compile / DriverSubmit 异步化（仍主线程）
- ❌ JS 异步执行（QuickJS 单线程不动）
- ❌ Style cascade 异步（在主线程，与 ComputedStyle 紧耦合）
- ❌ Worker thread pool（v1 双 worker 即可）
- ❌ SIMD layout 加速（与异步正交，独立优化）
- ❌ Phase 2 默认开启（v1 仅 env var；P2-9 评估）

---

## 8. 完成后更新

- [ ] `doc/roadmap.md` Phase 1 P1-8 状态 ✅；Phase 2 P2-9 标"由 P1-8 实验报告决定"
- [ ] `doc/perf/async_layout_report.md` 入库
- [ ] `doc/perf_budget.md` § 3.2 首屏阻塞实测填入
- [ ] `doc/重要特性.md` 新增 § "Async Layout / Shaping（实验）"
- [ ] `dong/CLAUDE.md` 与 `AGENTS.md` 加 `DONG_ASYNC_LAYOUT` 环境变量条目
