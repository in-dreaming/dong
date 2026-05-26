# P2-9 — Async Layout / Shaping 默认开启

> 上游：[`docs/roadmap.md`](../roadmap.md) Phase 2 P2-9
> 前置：[`P1-8 Async Layout / Shaping（实验）`](../phase1/P1-8_async_layout_shaping.md)
>
> 状态：方案 v1 / 2026-04-17

---

## 1. 目标

把 P1-8 的实验性异步路径升级为**默认行为**：

- `DONG_ASYNC_LAYOUT=1` → 默认开启；env var 仅作"显式关闭"。
- 业务方无需任何配置即可享受异步收益。
- 兼容 P1-8 已建立的所有边界（force_sync、几何 query、live reload）。
- 持续维护回归门槛；新 PR 不能让异步路径退化。

---

## 2. 现状（P1-8 完成后）

| 项 | 现状 |
|---|---|
| `DONG_ASYNC_LAYOUT=0`（默认）| 同步路径 |
| `DONG_ASYNC_LAYOUT=1` | 异步路径，需 env var |
| LayoutWorker / ShaperWorker | ✅ |
| force_sync API | ✅ |
| 实验报告 `docs/developer/reports/async_layout_report.md` | ✅ |

---

## 3. 设计

### 3.1 决策门槛

P1-8 实验报告必须满足以下条件，本任务才进入实施；否则推迟到 Phase 3：

| 条件 | 阈值 |
|---|---|
| 像素回归（异步路径稳态后） | ≥ 99.5% 通过 |
| 1000 节点 resize 主线程不掉到 30 FPS | 必须 |
| 几何 query 无可观察延迟（业务方反馈）| 必须 |
| Worker 异常崩溃率（实验期）| < 0.01%（每万次任务） |
| 与 P0-6 / P1-4 / Live Reload 协同无 bug 报告 | ≥ 1 个月稳定运行 |

如果某条件不达，本任务延后；P1-8 仍可作 opt-in 提供。

### 3.2 切换实施

```cpp
// engine_view.cpp 默认值
bool async_layout_enabled_ = true;   // 改默认

void EngineView::init() {
    const char* env = getenv("DONG_ASYNC_LAYOUT");
    if (env && (strcmp(env, "0") == 0 || strcasecmp(env, "false") == 0)) {
        async_layout_enabled_ = false;
        DONG_LOG_INFO("Async layout disabled by env");
    }
    if (async_layout_enabled_) {
        layout_worker_ = std::make_unique<LayoutWorker>(...);
        shape_worker_ = std::make_unique<ShaperWorker>(...);
    }
}
```

### 3.3 Per-View 控制

提供细粒度 opt-out：

```c
// dong_engine.h 扩展
void dong_engine_set_async_layout(dong_engine_t* eng, int enabled);
int  dong_engine_get_async_layout(dong_engine_t* eng);
```

某个 view 显式关闭异步（如对延迟敏感的 IME 输入条等极端场景）。

### 3.4 默认 fallback 行为

异步开启但出现以下情况自动 fallback 到同步本任务：

| 触发 | 行为 |
|---|---|
| 节点数 < 10 | 直接同步（worker 唤醒成本高于收益） |
| force_sync 请求频繁（> 60/秒） | 临时禁用异步本帧 |
| Worker 队列积压（> 5 任务） | 主线程同步处理新任务 |
| Worker 连续异常 N 次 | view 内禁用异步直到下次 reload |

### 3.5 与 P0-6 协同（确认）

P0-6 invalidation kind = Layout 时：
- 异步：snapshot + LayoutTask
- 同步：原路径

invalidation kind = Style：
- 异步：style 重算仍主线程；不进 worker
- 同步：同上

invalidation kind = Paint：完全不进 worker（不涉及 layout）。

### 3.6 与 Live Reload (P1-4) 协同

Live reload 触发 → force_sync_layout 一次（确保新内容立即可见）→ 异步路径在下帧自动恢复。

### 3.7 与 DevTools (P1-3) 协同

DevTools Perf panel 显示：
- inflight LayoutTask 数 / inflight ShapeTask 数
- async vs sync 路径比例（可视化）
- 帧延迟分布（async 是否引入 1 frame lag）

业务方调试时一目了然。

### 3.8 监控与回归

新增 perf counter：

```c
typedef struct {
    uint64_t async_layout_dispatched;
    uint64_t async_layout_completed;
    uint64_t async_layout_force_sync_count;
    uint64_t async_layout_fallback_count;
    uint64_t worker_exception_count;
} DongAsyncStats;

void dong_engine_get_async_stats(dong_engine_t* eng, DongAsyncStats* out);
```

CI 跑 perf_baseline.py（P0-7）时校验：
- `force_sync_count` 增长合理
- `worker_exception_count` ≤ 阈值
- async vs sync 收益符合预期

### 3.9 文档与迁移指引

业务侧需要知道：
- 几何 query (`getBoundingClientRect`) 仍同步可用，无需改代码。
- 极少数对延迟极敏感的场景可调用 `dong_engine_set_async_layout(eng, 0)`。
- 实测延迟若 ≥ 1 帧明显增加，立 issue。

---

## 4. 实施步骤

| Step | 范围 |
|---|---|
| **S1 — 验收 P1-8 实验报告** | 不达 § 3.1 门槛则本任务推迟 |
| **S2 — 默认值切换 + per-view ABI** | env var 改语义；新增 set/get API |
| **S3 — Auto-fallback 逻辑（节点数 / 队列 / 异常）** | 防止异步反而拖累 |
| **S4 — perf counter + DevTools panel 集成** | 可观测 |
| **S5 — perf_baseline.py 校验项** | CI 阻断 |
| **S6 — 文档 + 迁移指引** | 业务方周知 |

---

## 5. 通过验证规则

### 5.1 Hard

| 项 | 阈值 |
|---|---|
| P1-8 § 3.1 五条门槛全部满足 | 必须 |
| 默认开启后既有 baseline 稳态像素 ≥ 99.5% 通过 | 必须 |
| `DONG_ASYNC_LAYOUT=0` 行为与 P1-8 sync 路径完全一致 | 必须 |
| `dong_engine_set_async_layout(eng, 0)` 单 view 关闭即时生效 | 必须 |
| `force_sync_layout` 仍可工作 | 必须 |
| Worker 异常率 ≤ P1-8 实验期值 | 必须 |
| DevTools Perf panel 显示 async stats | 必须 |
| CI perf_baseline 中 async stats 不超阈值 | 必须 |

### 5.2 Soft

| 项 | 阈值 |
|---|---|
| 业务方 default 启动后无需任何代码改动 | 必须 |
| 1 个月稳定无 critical bug | 期望 |
| 默认开启后用户启动延迟（首屏阻塞）较 sync 下降 ≥ 50% | 必须（即 P1-8 ≤ 50ms 目标在默认开启后达成） |
| 异步与同步切换无视觉跳变 | 期望 |

### 5.3 必须新增的测试

| 测试 | 内容 |
|---|---|
| `tests/async/test_default_on_baseline.py` | 默认开启下跑 baseline 全集 |
| `tests/async/test_per_view_disable.cpp` | per-view set 0/1 切换 |
| `tests/async/test_auto_fallback_threshold.cpp` | 队列积压 / 节点 < 10 fallback |
| `tests/async/test_worker_exception_recovery.cpp` | 模拟 worker 异常后自动禁用 |
| `dong/scripts/test_async_perf_default.py` | 默认开启的 perf 收益验证 |

### 5.4 验证命令

```bash
# 默认开启全集回归
python dong/scripts/tools/run_baseline_compare.py
# 期望：≥ 99.5% 通过

# Force off
DONG_ASYNC_LAYOUT=0 python dong/scripts/tools/run_baseline_compare.py
# 期望：与同步路径一致

# 默认开启 perf
python dong/scripts/test_async_perf_default.py
# 期望：首屏阻塞 ≤ 50ms
```

---

## 6. 风险与回退

| 风险 | 缓解 |
|---|---|
| 默认开启后某个边缘 case 像素不一致 | env var `DONG_ASYNC_LAYOUT=0` 紧急回退；业务侧立即可关 |
| Worker 在 production 设备崩溃 | 自动 fallback + log；连续异常自动禁用本 view |
| 业务方 framework 假设同步行为 | 文档明示几何 query 仍同步；除此之外 dong 内异步对 JS 透明 |
| 性能差的设备开异步反而抖（context switch 开销）| auto-fallback 节点数 < 10 已覆盖；可调阈值 |
| Phase 1 P1-8 已发布很久仍 opt-in，业务方依赖关闭| 文档明示 default change；major version bump |

---

## 7. 不在本方案范围

- ❌ Painter / GPU compile 异步化（仍主线程）
- ❌ JS 异步执行
- ❌ 多 worker 池（仍单 LayoutWorker + 单 ShaperWorker）
- ❌ 跨 view 异步合并（GlobalShared 共享 atlas 但 layout 仍 per-view 独立）
- ❌ SIMD layout 加速（独立优化任务）

---

## 8. 完成后更新

- [ ] `docs/reference/features-index.md` § "Async Layout / Shaping" 状态从"实验"改"默认"
- [ ] `docs/overview/positioning.md` § 8 决策记录追加：`<日期> Async layout 默认开启`
- [ ] `docs/developer/perf-budget.md` § 3.2 首屏阻塞实测刷新
- [ ] `dong/CLAUDE.md` `AGENTS.md` 把 `DONG_ASYNC_LAYOUT` 含义改为 "set 0 to disable"
