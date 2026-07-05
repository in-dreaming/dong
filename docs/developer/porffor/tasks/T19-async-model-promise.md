# T19 — 异步模型定稿与 Promise 可用性评估

- **Area**: porffor-fork + dong-host
- **性质**: **调研/决策型** — 产出《Porffor 异步约定》+ Promise 评估报告 + 决策记录，**本任务不含实现**；若决策开放 Promise 子集，修复工作另立任务（以本任务的评估矩阵为规格）
- **优先级**: P2
- **依赖**: T10（callback fetch 的落地经验）
- **预估**: 2–3 人天（评估）；若决定修 Promise 另立任务
- **前置阅读**: `docs/developer/porffor/tasks/setup.md` §4；`dong/third_party/porffor/compiler/builtins/promise.ts`

## 背景

全面切换后没有 QuickJS 兜底，异步只有两条路：**callback（现行官方模型）**，或者把 Porffor 的 Promise 子集修到可用。README 明言 Promise/async 有 known bugs，但上游 `builtins/promise.ts` 有完整实现——「有多坏、修到能用要多少成本」一直没有量化，导致 T10/T13 的测试改写决策缺依据。本任务给出定论。

## 工作内容

### 1. callback 异步约定收口成文档

把 T08（事件槽）/ T09（timer/rAF）/ T10（fetch 结果槽）三处已经形成的模式统一成一份《Porffor 异步约定》：

- 回调 = export 名字符串，C++ callExport；
- 结果槽生命周期 = 回调执行期间，槽外读取返回空 + debug 日志；
- 失败路径与成功路径共用回调（ok/error 字段区分）；
- re-entrancy 规则（引用 T08 定稿的栈式约定）。

### 2. Promise 现状评估（fork 侧）

- 用例矩阵：`then` / `catch` / `finally` / 链式 / `Promise.all` / `race` / `resolve` 已决值 / 微任务时序（与 setTimeout 交错）；每例三条路径（node wasm、2c+clang、native）跑一遍；
- 产出失败清单 + 根因初判 + 修复成本估计（人天）；
- 明确 `async` / `await` 单独评估（预期：不开放）。

### 3. 决策记录

- 若「Promise 最小子集（then/catch，无 all/race）」在 2c 下修复成本 ≤ 5 人天且稳定 → 立后续 fork 任务开放该子集（fetch 可提供 `.then` 形式的薄包装）；
- 否则**正式禁用**：prelude 不暴露 Promise，T12/T18 生成器与 T13 迁移 checklist 的 lint 卡口把 `Promise` / `async` / `await` 列为构建错误。

## 验收标准

1. 《Porffor 异步约定》文档合入并被 T08/T09/T10 任务的完成记录引用。
2. Promise 评估矩阵（用例 × 路径 × 结果）与决策记录合入。
3. 决策对应的执行项落地：开放子集 → 后续任务已立；禁用 → lint 卡口进 CI。
4. setup §4 的 Promise 行按决策更新。

## 完成记录

- **日期**: 2026-07-05
- **决策**: **禁用 Promise**（修复成本 8–12+ 人天，超 5 人天阈值）
- **文档**: `docs/developer/porffor/async-convention.md`（统一 T08/T09/T10 callback 模型）、`T19-promise-evaluation.md`（评估矩阵）
- **Lint**: `dong/scripts/porffor_lint.mjs`；接入 `porffor_compile.mjs`、`porffor_inline_handlers.mjs`
- **Repro**: `docs/developer/porffor/tasks/repro/t19/t19_verify.mjs`
- **遗留**: Porffor fork 内 Promise 实现未删除（仅 Dong 产品路径禁用）；微任务泵未做
