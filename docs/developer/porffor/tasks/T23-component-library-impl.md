# T23 — 组件库与 React 示例复刻

- **Area**: toolchain
- **性质**: **纯实现型** — 对标物：**T18 规格 §7 组件清单 + §9 复刻 checklist**；渲染/交互行为以现有 React 示例（截图与交互录制）为 baseline
- **优先级**: P2
- **依赖**: T22（框架编译器可用）
- **预估**: 5–8 人天（组件可多人并行认领）
- **前置阅读**: T18《framework-spec.md》§7 §9；`docs/developer/porffor/tasks/setup.md` §7.0

## 范围

按 T18 规格 §7 的组件清单逐个实现（button / label / list / progress / dialog / input 等，以清单为准），并完成第三个对标示例的复刻：

1. **组件实现**：每个组件按规格给定的 props 表、DOM 结构契约、样式契约实现为框架组件；每个组件配一个最小用例页（进 Dong 测试目录，标 `porffor: ready`）。
2. **`react-game-ui` 示例复刻**：用框架 + 组件库重写，按规格 §9 checklist 逐项验收。
3. **组件文档**：每个组件的 props / 事件 / 用法示例进 `docs/developer/porffor/components.md`（或规格书附录）。

## 验收标准

1. 规格 §7 清单内组件全部实现，各自最小用例页通过。
2. `react-game-ui` 复刻通过：checklist 勾选完 + 多帧截图与 React 版 baseline 比对一致 + 交互（点击/键盘）行为一致。
3. T13 盘点表中 `blocked(T18)` 的测试全部转 `ready` 或给出剩余阻塞说明。
4. 组件文档合入。

## 风险

- 组件样式契约若在复刻中发现与引擎 CSS 子集冲突（如某组件依赖未实现的 CSS 特性），走「回 T18 修订」流程调整契约，不在组件里堆 workaround；
- game-ui 含动画/帧率敏感交互，性能问题回溯到 T22 store 热路径或 T09 rAF，不在组件层硬扛。
