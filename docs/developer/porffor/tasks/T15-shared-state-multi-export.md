# T15 — 跨模块状态共享：2c 多 export（主）/ host 状态槽（兜底）

- **Area**: porffor-fork（`dong/third_party/porffor` 子模块）+ dong-host
- **性质**: **两段式** — 阶段 0：spike（2c export 初始化语义）+ 方案 A/B 选型，产出**决策记录**并评审；阶段 1：按决策实现。gate 未过不写生产代码。实现对标物：node/wasm 路径行为（`porf` 直跑同一脚本的结果）
- **优先级**: **P0**（T08/T12/T22 的架构前置；不解决则「状态放全局变量」的对策不成立）
- **依赖**: 无（与 T01/T03 并行安全，均改 2c，认领时相互知会）
- **预估**: 方案 B 1 人天；方案 A 3–5 人天
- **前置阅读**: `docs/developer/porffor/tasks/setup.md` §3（事实 F1/F2）§5

## 背景（事实 F1/F2）

现状：每个事件/定时器 handler 被编译为**独立 Porffor 模块**，拥有独立 `_memory`；`callExport` 实际是「跑另一个模块的 main」（`generated/porffor/registry.h`、`porffor_script_registry.cpp`）。因此**主模块的全局变量与 handler 里的全局变量是两块互不可见的内存**——「回调用具名 export + 状态放全局变量」的模型只剩一半成立。现有示例 `hello_dom` 靠 handler 内重新 `getElementById` 回避了状态共享；任何计数器 / todo / 框架（T18）级别的交互都绕不开这个问题。

根因是 2c 的产物只暴露一个 `main` 入口，Dong 只能用「每 handler 一个模块」模拟多入口。

## 方案 A（主攻）：2c 多 export 支持

Porffor 的 wasm 路径本身支持 `export function`（`wrap.js` 实例化后 exports 可调）。缺口在 2c 没有为非 main 的 export 生成 C 入口。目标：

1. **fork 侧**：2c 为每个 wasm export 生成独立 C 入口函数（与 main 共享同一份 memory / 全局变量），命名可控（如 `--2c-export-prefix`）；确认非 main 入口被调用时数据段 / globals 的初始化语义（是否假设 main 已先跑过一次，写清约定）。
2. **dong-host 侧**：`porffor_compile.mjs` / manifest / registry 改造——handler 不再单独编译；manifest 条目声明 `exports: ["onBtnClick", ...]`；`dong_porf_handler_t` 直接指向同模块的 export 符号；`callExport` 顺带支持带参调用（`f64 × N`，解决 F2，供 T09 rAF 时间戳等使用）。

收益：状态天然共享、消灭每 handler 一份 memory 与 prelude 重复编译、callExport 可带参。**这是对整体架构收益最大的 fork 任务，优先做。**

## 方案 B（兜底 / 过渡）：host 状态槽

与方案 A 不冲突，可先行落地并长期保留（也是跨模块通信手段）：

| import | 说明 |
|--------|------|
| `dong_state_set_num(slotId, v)` / `dong_state_get_num(slotId) -> f64` | 数字槽 |
| `dong_state_set_str(slotId, str)` / `dong_state_get_str(slotId) -> str` | 字符串槽（依赖 T16 通道） |

slotId 由脚本约定常量（prelude 可给命名 helper）。缺点：脚本样板重、状态散落 host。

## 验收标准

1. 复现 `docs/developer/porffor/tasks/repro/t15_shared_state.js`：主模块 `let count = 0` + `export function onClick()` 自增并 `setTextContent`；2c 产物中两个入口共享全局；模拟点击两次后文本为 "2"（node wasm 路径与 2c+clang 路径结果一致）。
2. `hello_dom` 示例重写为单模块多 export 版本，行为不变；`porffor_manifest.json` 新语法有文档。
3. 旧的「独立 handler 模块」机制去留有明确决策记录（保留过渡 or 一次性切换并迁移现有示例）。
4. 方案 B 状态槽 import 落地（无论 A 进度），数字槽不依赖 T16 可先交付。
5. callExport 带参路径有用例（如 export 接收一个 f64 参数并回显）。
6. 完成后更新 setup §3 事实清单 F1/F2。

## 风险

- 2c 非 main 入口的初始化顺序是最大不确定点：数据段初始化、全局变量初值可能都在 main 序言里——需要先读 2c 产物确认，必要时把初始化拆成独立 `__init` 供 Dong 在模块注册时调用。
- 与 T01/T02/T03 都改 2c / prepend 区域，改动冲突要协调。
