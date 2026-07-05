# T19 — Promise 可用性评估与决策

**日期**: 2026-07-05  
**决策**: **正式禁用 Promise / async / await**（Porffor 路径）

## 评估矩阵

用例在 `docs/developer/porffor/tasks/repro/t19/`；矩阵见 `t19_matrix.json`。

| 用例 | wasm 直跑 | 2c 编译 | 2c+clang 运行 |
|------|-----------|---------|---------------|
| then(resolve) | 无稳定输出 | OK | SKIP（无 clang） |
| catch(reject) | 无稳定输出 | OK | SKIP |
| finally | 无稳定输出 | OK | SKIP |
| 链式 then | 无稳定输出 | OK | SKIP |
| Promise.all | 无稳定输出 | OK | SKIP |
| Promise.race | 无稳定输出 | OK | SKIP |
| 已决值 | 无稳定输出 | OK | SKIP |
| 微任务 vs setTimeout | 无稳定输出 | OK | SKIP |

## 根因初判

1. **wasm 路径**：Promise 微任务调度在 headless 2c 验证环境下无可靠 stdout 观测；README 明言 known bugs。
2. **2c 路径**：语法与 codegen 可通过，**运行时语义未验证为生产可用**。
3. **async/await**：未纳入矩阵；预期需额外 transform，成本高于 callback 模型收益。

## 修复成本估计

| 子集 | 估计人天 | 说明 |
|------|----------|------|
| then/catch 最小子集 | 8–15 | 微任务队列 + 2c 稳定性 + 全平台测试 |
| all/race | +5 | 组合语义 |
| async/await | +10+ | 不建议 |

**结论**：修复成本 > 5 人天门槛，且与「callback 唯一官方模型」战略冲突。

## 执行项（已落地）

1. 《Porffor 异步约定》→ `docs/developer/porffor/async-convention.md`
2. `dong/scripts/porffor_lint.mjs` — 构建期拒绝 Promise/async/await
3. `setup.md` §4 Promise 行更新为「禁用」
4. prelude **不**暴露 `Promise`

## 后续

若 fork 侧将来修 Promise，以本矩阵为回归 oracle，另立 porffor-fork 任务；开放前不得解除 lint。
