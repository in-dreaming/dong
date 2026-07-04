# T17 决策记录 — 多 View / 模块多实例

**日期**: 2026-07-04  
**结论**: **Plan B — 实例快照 / swap（dong-host）**

## Spike 发现（2c 产物）

对 `t15_shared_state.js` / `t17/t17_counter.js` 执行 `porf c --module --2c-prefix=...`：

1. **Wasm linear memory** 对应模块级 C 指针 `char* {prefix}memory` + `{prefix}memory_pages`（全局变量，非 `_memory` 栈分配）。
2. **JS 模块级 `let` / 全局状态** 落在 **static C 标量**（如 `static f64 t15_count`、`static i32 t15_countjjtype`），与 memory 指针分离；仅 swap memory **不够**隔离实例。
3. **`__porf_init`** 在 `memory != NULL` 时早退；per-instance 初始化需 host 先清空指针、调用 init、再把 calloc 结果复制进 instance buffer（见 `ensureInstanceInitialized`）。
4. Porffor 运行时 internal 全局（`jjporfjjcurrentPtr`、prelude 常量等）同样是 static 标量，需一并纳入 snapshot。

## 方案选型

| 方案 | 决策 | 理由 |
|------|------|------|
| **A — 2c reentrant ctx*** | 不采用 | fork 面大，T17 时间盒内不划算 |
| **B — 实例 snapshot/swap** | **采用** | 复用 T08 active-module 模式；compile 期解析 static 全局生成 `state_capture`/`state_apply`；每 View 独立 memory + 快照 |
| **C — 链接期复制** | 垫底 | 二进制膨胀、屏数 build 期锁死 |

## 实现约定

1. **`PorfforHost` / `PorfforScriptRegistry` per-View**：`ScriptEngine` 各持一份；C import 经 `thread_local g_active_host` + `ActiveHostScope` 路由。
2. **每 (Registry, module) 一份 `ModuleInstance`**：独立 memory buffer + `state_snapshot`；`callExport`/`run` 前 push 激活栈，swap memory 指针 + `state_apply`，退出 `state_capture` 并恢复外层实例。
3. **`porffor_compile.mjs`**：解析 `static f64|i32|... {prefix}*` 生成 `{prefix}state_t` + capture/apply；`dong_porf_module_t` 增 `init_fn` / `state_*` / `state_size`。
4. **legacy handler 子模块**：仍过渡支持；in-module export 共享 parent 的 state ops 与 instance key。

## 不采用 / 延后

- 多线程并发 swap（当前单线程主循环；`thread_local` 仅服务 import 重入）。
- Plan A reentrant 2c（长期可选）。
- `3d_screens` Porffor smoke（验收项 3）：待示例迁移 Porffor 后补跑。

## 评审 gate

- [x] Spike 完成：JS 全局 = static C 变量 + 独立 memory 指针
- [x] Plan B 可行，进入 Phase 1 实现
