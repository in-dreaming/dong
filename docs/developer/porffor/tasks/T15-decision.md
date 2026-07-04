# T15 决策记录 — 跨模块状态共享

**日期**: 2026-07-04  
**结论**: **Plan A + Plan B 同时交付**（A 为主路径，B 为兜底）

## Spike 发现（2c 产物）

对 `t15_wasm_state.js` 执行 `porf c --module --2c-prefix=t15_`：

1. **2c 已能为 wasm export 生成独立 C 函数**（`t15_onClick`、`t15_echoArg`），与 `#main` 入口共享同一份 `static` 全局变量（如 `t15_count`）。
2. **初始化仅出现在 `#main` 序言**（`calloc` + data segment `memcpy`）。非 main export 被直接调用时 memory 未初始化——**风险确认**。
3. **export 使用 Porffor 内部调用约定**（`jjnewtarget/jjthis` + `struct ReturnValue`），Dong 不能直接 `int (*)(void)` 调用原始 export 符号。

## 方案选型

| 方案 | 决策 | 理由 |
|------|------|------|
| **A — 2c 多 export** | **采用** | 初始化可拆为幂等 `{prefix}__porf_init()`；可为每个 export 生成 `{prefix}export_{name}` shim（0 参 / `f64` 参）；manifest 单模块 + `exports[]`，handler 共享 parent memory |
| **B — host 状态槽** | **同时落地** | `dong_state_set_num/get_num` 不依赖 T16；跨模块 / 过渡场景保留 |

## 约定

1. Dong 脚本使用 `export function handlerName()` + manifest `"exports": ["handlerName"]`；**不要** `export function main`（与 2c entry 符号冲突）。
2. 编译带 export 的脚本时 build 管线加 `--module`。
3. `main()` 与 export shim 均调用 `{prefix}__porf_init()`（`if (memory) return` 幂等）。
4. 旧 manifest `handlers` 独立子模块机制**保留过渡**（registry 填 `legacy_handler_module`），hello_dom 已迁移为单模块 export。
5. `callExport(module, name, args?, arg_count?)`：`param_count` 0 或 1（`f64`）。

## 不采用 / 延后

- 2c 层 `export function main` 与 entry 重名：脚本侧禁止，不在 2c 做重命名 magic。
- `callExport` 通用 `f64×N`（N>1）：registry 结构预留，当前仅 0/1 参。

## 评审 gate

- [x] Spike 完成，初始化语义有明确约定（幂等 init）
- [x] Plan A 可行，进入 Phase 1 实现
- [x] Plan B 数字槽同步交付
