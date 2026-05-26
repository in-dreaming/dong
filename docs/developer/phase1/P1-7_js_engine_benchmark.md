# P1-7 — JS 引擎 Benchmark 报告

> 上游：[`docs/roadmap.md`](../roadmap.md) Phase 1 P1-7
> 决策上文：[`docs/overview/positioning.md`](../positioning.md) § 8（"JS 引擎暂不换"，本任务产出报告后再决策）
>
> 状态：方案 v1 / 2026-04-17

---

## 1. 目标

为"是否切换 JS 引擎"提供**可复现的定量数据**：

- 在 dong 真实场景下对比 QuickJS / QuickJS-NG / Hermes 的性能与体积。
- 输出 `docs/developer/reports/js_engine_bench_2026Q3.md`（或当时月份）。
- 报告结论同步到 [`docs/overview/positioning.md`](../positioning.md) § 8 决策记录。

不是为了换引擎；是为了**得到换或不换的依据**。结论可能是"继续 QuickJS"。

---

## 2. 现状

- JS 引擎：QuickJS（经典版本，源码在 `dong/third_party/quickjs/`）。
- React/Preact 已实测：bundle eval 100–500 ms（[`docs/developer/arch/react-reconciler-spec.md`](../arch/react.md) § 8）。
- reconciler diff 在大型 UI 偶尔卡顿。
- QuickJS 优势：易嵌入 / 体积小（~600KB）/ MIT 协议 / 无 JIT 安全风险；劣势：解释器，~10–50× 慢于 V8。

候选对比：

| 引擎 | 位置 | 协议 | JIT | 嵌入难度 | 备注 |
|---|---|---|---|---|---|
| **QuickJS** | 当前 | MIT | 否 | ★★★★★ | baseline |
| **QuickJS-NG** | https://github.com/quickjs-ng | MIT | 否 | ★★★★★ | 社区 fork，性能 / 兼容修复 |
| **Hermes** | https://github.com/facebook/hermes | MIT | 可选 | ★★★ | RN 用，AOT 字节码 / 启动快 |
| **MuJS** | — | ISC | 否 | ★★★★ | 体积更小但更慢；不在主候选 |
| ~~V8~~ | — | BSD | 是 | ★ | 体积大、嵌入复杂、JIT 内存执行权限要求；本任务**不**评估（明确放弃） |
| ~~JavaScriptCore~~ | — | LGPL | 是 | ★★ | macOS / iOS only；跨平台不适用 |

**主候选**：QuickJS-NG 与 Hermes，与 baseline QuickJS 三者对比。

---

## 3. 设计

### 3.1 度量维度

报告必须覆盖：

| 维度 | 测量方法 | 单位 |
|---|---|---|
| **Bundle eval（冷启动）** | `eval(react+reconciler+app bundle)` 端到端 | ms |
| **Bundle eval（热）** | 第二次 eval 同 bundle | ms |
| **react-reconciler diff（小树）** | 100 节点 setState → commit | ms |
| **react-reconciler diff（大树）** | 1000 节点 setState | ms |
| **JSON parse / stringify** | 1MB JSON 数据 | ms |
| **Property access (object hot path)** | `obj.x = 1; obj.x + 1` 1M 次 | ns / 次 |
| **Function call overhead** | 空函数 1M 次调用 | ns / 次 |
| **Closure 创建** | 1M 次创建闭包 | ns / 次 |
| **Promise / microtask** | resolve + then 链 1k | ms |
| **setTimeout / rAF 调度** | 1000 个 timer 注册 + fire | ms |
| **GC 暂停** | 长时间运行触发 major GC，最大暂停 | ms |
| **Heap 内存占用** | 100 节点 React app idle 时 RSS 增量 | MB |
| **二进制体积（增量）** | dong.dll size with vs without | KB |
| **冷启动延迟（dong_app 启动到首帧）** | 端到端 | ms |

### 3.2 测试脚本：`dong/scripts/tools/js_bench.py`

```bash
python js_bench.py --engines quickjs,quickjs-ng,hermes \
                   --machine ref \
                   --iters 5 \
                   --report docs/developer/reports/js_engine_bench_2026Q3.md
```

实施：

1. 为每个引擎构建一个独立的 `dong.dll`（`zig build -Djs-engine=quickjs-ng` 等）。
2. 跑一组固定 benchmark HTML：
   - `bench_bundle_eval.html`（含一个真实 react bundle 的 eval 计时器）
   - `bench_reconciler_small.html` / `_large.html`
   - `bench_json_parse.html`
   - `bench_property_access.html`
   - `bench_promise_chain.html`
   - `bench_setTimeout_storm.html`
   - `bench_gc_pause.html`
3. 每个 benchmark 跑 `--iters` 次取中位 + std。
4. 测内存：tick 100 次后调 `dong_engine_query_mem` + 进程 RSS。
5. 输出 Markdown 报告。

### 3.3 引擎接入抽象

dong 新增 `dong/src/script/script_engine_iface.hpp`（不是大改架构，是把现有 `ScriptEngine` 内的 QuickJS 调用统一收口到一个 vtable）：

```cpp
struct JsEngineVTable {
    void* (*create_runtime)(...);
    void* (*create_context)(void* rt);
    int   (*eval)(void* ctx, const char* src, size_t len, const char* name);
    void* (*get_global)(void* ctx);
    int   (*call)(void* ctx, void* fn, int argc, void** argv, void** out_ret);
    void  (*free_value)(void* ctx, void* v);
    void  (*gc)(void* rt);
    /* ... 等价于 QuickJS 当前用到的 ~30 个函数 */
};

extern const JsEngineVTable* dong_js_engine_quickjs();
extern const JsEngineVTable* dong_js_engine_quickjs_ng();   // 新
extern const JsEngineVTable* dong_js_engine_hermes();        // 新
```

ScriptEngine 持有一个 vtable 指针，其它代码通过 vtable 调用。

> 这步本身价值很大（解耦）；即便 P1-7 结论是"继续 QuickJS"，vtable 仍保留作为未来插槽。

### 3.4 公平性约束

- **同一台 reference 机型**（无后台负载）。
- **同一份 JS bundle**（用 esbuild 同配置构建）。
- 关闭 dong 的所有非必要功能（present-only off / DevTools off / live reload off）。
- 每次跑前 `dong_engine_reset_perf_counters`。
- 单次 benchmark 进程独立，避免热缓存污染。
- Hermes 启动时是否预编译 bytecode（HBC）作为单独条目（"with HBC" / "raw JS"）。
- QuickJS-NG 与 QuickJS 同样跑（对照 fork 改进幅度）。

### 3.5 报告产出格式

`docs/developer/reports/js_engine_bench_<YYYYMM>.md`：

```markdown
# JS 引擎 Benchmark 报告（2026 Q3）

## 测试机型
- CPU: ...
- OS: Windows 11 / macOS 14 / Ubuntu 24
- dong 版本: <git sha>
- 测试时间: 2026-XX-XX

## 引擎版本
| Engine | Version | Build mode |
|---|---|---|
| QuickJS | xxx | -O2 |
| QuickJS-NG | xxx | -O2 |
| Hermes (AOT HBC) | xxx | release |

## 综合结论（TL;DR）
... 一段话 ...
推荐决策: <继续 QuickJS / 切 QuickJS-NG / 切 Hermes / 待定>

## 详细数据
（每个 benchmark 一张表 + 雷达图链接）

## 体积影响
| Engine | dong.dll 增量 | 启动 RSS 增量 |
|---|---|---|

## 兼容性问题
（哪些 dong / React feature 在某引擎下不工作 / 行为不同）

## 决策建议与替代
1. ... 推荐方案 ...
2. ... 备选方案 ...
3. 风险与迁移成本
```

### 3.6 评估的"软"维度（非数字）

| 维度 | 说明 |
|---|---|
| 协议 / 法律 | dong 是 MIT 兼容必要 |
| 维护活跃度 | commit 频率、issue 响应 |
| Debugger / DevTools 协议支持 | 决定是否能接 Chrome DevTools / VS Code |
| ES feature 兼容（ES2022 / 2023 / Top-level await）| react / 第三方依赖可能用到 |
| iOS App Store 政策（JIT 限制）| Hermes 是为 RN 设计，无 JIT 默认 |

报告必须包含定性分析。

### 3.7 决策框架

报告末尾给出"决策矩阵"：

```
                            权重   QuickJS   QuickJS-NG   Hermes
冷启动 < 200ms               20%
React diff (1k 节点)         15%
GC 最大暂停 < 5ms            15%
体积增量 < 1MB               10%
集成成本                     10%
跨平台一致性                 10%
ES2023 feature 覆盖率        5%
社区维护活跃度               10%
DevTools 调试支持           5%
———————————————————————————————————————
加权得分

最终结论: 选 X
```

---

## 4. 实施步骤

| Step | 范围 |
|---|---|
| **S1 — JsEngineVTable 抽象** | QuickJS 现有调用走 vtable；既有 React demo 不掉链 |
| **S2 — QuickJS-NG 接入** | `zig build -Djs-engine=quickjs-ng`；React demo 跑通 |
| **S3 — Hermes 接入** | 含 HBC 预编译 + raw JS 两个 mode；React demo 跑通 |
| **S4 — Benchmark HTML 集** | bench_*.html 7 套；各自一致脚本输出耗时 |
| **S5 — `js_bench.py` 工具** | 自动跑 + 收数 + Markdown 报告 |
| **S6 — 报告产出 + 决策记录** | 写入 `docs/developer/reports/js_engine_bench_<YM>.md`；同步 `positioning.md` § 8 |

---

## 5. 通过验证规则

### 5.1 Hard

| 项 | 阈值 |
|---|---|
| 三个引擎都能在 dong 中加载 React demo 并正常运行 | 必须 |
| 7 套 benchmark HTML 都能在三个引擎下跑出数据 | 必须 |
| 报告 Markdown 含每个维度的中位 + std + n=iters | 必须 |
| 报告含决策矩阵与最终建议（即使"维持现状"也明确写出） | 必须 |
| 数据可复现：报告含 git sha + 命令 + raw JSON 链接 | 必须 |
| `positioning.md` § 8 同步追加决策条目 | 必须 |
| 既有 dong baseline 全集在 QuickJS 路径下 100% 通过 | 必须（vtable 抽象不破坏既有） |

### 5.2 Soft

| 项 | 阈值 |
|---|---|
| 报告含可视化（雷达图 / 柱状图，简单 markdown 表也可） | 期望 |
| 三个引擎在三平台（Win/macOS/Linux）都能跑 | 期望 |
| Hermes HBC 预编译时间 / 大小 | 期望 |
| 决策矩阵权重在文档内可调整重新算 | 期望（脚本支持 `--weights`）|

### 5.3 必须新增的产物

| 文件 | 说明 |
|---|---|
| `dong/src/script/script_engine_iface.hpp` + 三个实现 | vtable 抽象 |
| `dong/scripts/tools/js_bench.py` | 报告生成器 |
| `dong/scripts/tools/bench_data/bench_*.html` | benchmark 集 |
| `docs/developer/reports/js_engine_bench_<YM>.md` | 最终报告（提交到 git） |
| `docs/overview/positioning.md` § 8 一行决策条目 | 同步追加 |

### 5.4 验证命令

```bash
# 抽象不破坏
zig build && zig build run-feature-tests

# 跑 benchmark
python dong/scripts/tools/js_bench.py --engines quickjs,quickjs-ng,hermes \
   --iters 5 --report docs/developer/reports/js_engine_bench_2026Q3.md

# 复审报告
# 至少包括 § 3.5 模板的全部 section
```

---

## 6. 风险与回退

| 风险 | 缓解 |
|---|---|
| Hermes 编译复杂、跨平台坑多 | 接入仅作为 benchmark 候选，不要求 production；接不通则报告说明、对比 QuickJS-NG 即可 |
| QuickJS-NG 与 QuickJS API 微差 | vtable 内部桥接；统一向上 |
| 不同引擎 GC 策略导致结果偏移 | 报告区分 GC pause / steady state；多 iters 取中位 |
| 报告"看似换引擎更好"但迁移成本未算 | 决策矩阵显式列"集成成本"权重 |
| 时间不足跑全 benchmark | 优先级：bundle eval > reconciler diff > GC > 其他；不全也要出 v0 报告 |
| 测试机不稳定 | dedicated benchmarking machine；对比相对值不绝对值 |

---

## 7. 不在本方案范围

- ❌ 实际切换引擎（本任务结论再决定，且会单独立任务）
- ❌ V8 / SpiderMonkey 评估（明确放弃，不在候选）
- ❌ JIT 性能评估（dong 默认无 JIT 嵌入；除非主机平台支持）
- ❌ Web Worker 评估（dong 不支持 Worker）
- ❌ TypeScript 类型检查性能（不在 runtime 范畴）

---

## 8. 完成后更新

- [ ] `docs/overview/positioning.md` § 8 决策记录追加：`<日期> 完成 JS 引擎 benchmark；决策：<结论>`
- [ ] `docs/roadmap.md` Phase 1 P1-7 状态 ✅
- [ ] `docs/developer/reports/js_engine_bench_<YM>.md` 最终入库（PR 形式，team review）
- [ ] 若决策"切换引擎"：立 P2 任务 + 迁移计划文档（不在本任务）
- [ ] 若决策"维持 QuickJS"：明确写"下次 evaluation 时间（建议 6–12 个月后）"
