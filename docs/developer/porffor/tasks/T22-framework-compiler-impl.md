# T22 — 框架编译器与 store 运行时实现

- **Area**: toolchain + dong-host
- **性质**: **纯实现型** — 对标物：**T18《framework-spec.md》**（黄金样例 + 复刻 checklist）。规格不可行处回 T18 修订评审，不得擅自改语义
- **优先级**: P1
- **依赖**: **T18 规格评审通过**（硬门槛）、T15（状态模型）、T16（字符串协议）；端到端跑通需 T06 T07 T08
- **预估**: 5–8 人天
- **前置阅读**: T18 产出的《framework-spec.md》全篇；`docs/developer/porffor/tasks/setup.md` §7.0

## 范围

按 T18 规格实现框架的编译器与运行时基座，**不含组件库**（T23）：

1. **模板编译器**（node 工具）：Svelte 子集模板 → 扁平 Porffor JS（初始 DOM 交给引擎 HTML 解析；更新逻辑生成为具名函数；事件绑定生成 `addEventListener(export 名)`）。接入 `porffor_compile.mjs` 管线上游，产物布局按规格 §6。
2. **store 运行时**：按规格 §2 的状态形态（T15 方案 A 的模块全局 / 方案 B 的 host 槽）实现 `setState` → 脏标记 → 更新函数调度。
3. **lint 卡口**：规格 §8 的禁止面检查（无闭包捕获 / eval / Promise / 动态 import），编译器出口强制执行，进 CI。
4. **快照测试**：规格 §1 全部黄金样例落成「模板输入 → 生成 JS」快照测试；规格新增样例时测试同步扩。

## 验收标准

1. 黄金样例快照测试全绿（规格 §1 每条构造至少一例）。
2. **`react-counter` 与 `react-todo-classic` 复刻通过**：按规格 §9 checklist 逐项勾选；多帧截图与 React 版 baseline 比对一致（QuickJS 退役前用其渲染结果作 baseline）。
3. lint 卡口进 CI 且有反例测试（塞一段含闭包的生成物应报错）。
4. 编译器有独立单测与错误信息质量要求（模板语法错误报文件/行号）。
5. 实现中触发的规格修订（若有）在 T18 完成记录中留痕，规格与实现无漂移。

## 完成记录

- **Commit**: (见 feature/porffor Wave 4)
- **编译器**: `porffor_framework_compile.mjs`、`porffor_framework_lint.mjs`
- **单测**: `porffor_framework_compile.test.mjs`（bind/event/if + 反例）
- **管线**: `porffor_manifest.json` `framework[]` → `porffor_compile.mjs`
- **示例**: `porf_counter`（.porf 编译）；`porf_todo` / `porf_game_ui`（手写 store 模式）
- **遗留**: todo `{#each}` 编译器化、Preact baseline 截图 — 见 `WAVE4-LEFTOVER.md`

## 风险

- T15 若只落地方案 B（状态槽），store 热路径经 host import，性能要实测（对标 checklist 里 game-ui 的帧率项提前关注）；
- 与 T12（内联 handler 生成器）的代码复用边界按规格 §6 执行，避免两套模板处理。
