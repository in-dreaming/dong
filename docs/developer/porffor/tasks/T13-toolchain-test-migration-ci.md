# T13 — 全量测试迁移盘点 + Porffor CI

- **Area**: toolchain
- **性质**: **两段式** — 阶段 0：全量盘点分类 + 构建组织选型（产出分类表与选型记录，评审）；阶段 1：标记约定 / runner / CI 基建实现。分类表是 T18/T20/T21 的工作依据，先出表再动基建
- **优先级**: P1
- **依赖**: 无硬依赖（迁移覆盖率随 T06–T12/T18/T20 落地递增）；是 T21（QuickJS 退役）的门槛
- **预估**: 3–5 人天（基建与首次盘点）+ 迁移长尾（随其他任务推进）
- **前置阅读**: `docs/developer/porffor/tasks/setup.md` §1 §4 §7

## 背景

Dong 有 **279** 个 HTML 测试（事实 F15）：约 129 个无 `<script>`（静态渲染，Porffor 路径直接可跑），约 150 个带 `<script>`（当前依赖 QuickJS）。

**目标不是双引擎并存**——终态是全部测试跑在 Porffor 上（T21 移除 QuickJS）。因此本任务做的是：迁移状态标记、单引擎 CI、全量分类盘点、迁移模板。QuickJS 在退役前仅作为迁移期间的行为 baseline（对比渲染结果用），不进合入门槛。

## 工作内容

### 1. 标记约定（定稿后写进 Dong 测试文档）

HTML 头部注释标记迁移状态：

- `<!-- porffor: ready -->` — 已可在 Porffor 路径运行（CI 必须过）；
- `<!-- porffor: pending -->` — 待迁移（CI 记数、不阻塞）；
- `<!-- porffor: blocked(Txx) -->` — 被某任务阻塞（如 blocked(T18) = 等框架）；
- `<!-- porffor: dropped(原因) -->` — 显式裁剪，永久跳过，必须写原因。

默认值：无 script → `ready`；有 script → `pending`。

### 2. 测试 runner 改造

- Porffor 单引擎 job：`ready` 全绿为合入门槛；`pending`/`blocked` 产出覆盖率数字（ready 数 / 总数）进 CI 摘要；`dropped` 跳过。
- 迁移期间保留一个**非门槛**的 QuickJS baseline job（仅用于渲染对比），T21 时移除。

### 3. 基线盘点（首次产出）

- 全部无 script 测试跑一遍 Porffor 静态渲染 → 通过/失败清单（失败的开列表跟踪，不阻塞本任务）。
- 带 script 的 ~150 个逐个分类，产出表格（CSV/Markdown）合入 docs：
  - **直迁**：简单 DOM 读写 / console / performance，对照 T06–T11 API 清单即可改写；
  - **需 T12**（内联 handler）/ **T14**（eval snippet）/ **框架 T18→T22/T23**（React/Preact 示例与复杂状态逻辑）/ **T20**（Selection/CE/IME/DOMParser/FormData 决策）；
  - **dropped 候选**：与游戏 UI 产品面无关或成本过高的，给出裁剪理由待评审。

  这张表是 T18/T20/T21 的工作依据。

### 4. 构建组织方案（关键设计点，任务内定稿）

Porffor 模块是**静态链接**进引擎的（事实 F13：manifest → `porffor_compile.mjs` → `generated/porffor/*.c` + `registry.c`）。150 个测试全部迁移意味着几百个模块 .c。定夺构建形态并实测构建时间：

- a) 单个 test runner 链接全部测试模块（registry 几百条，链接产物大但简单）；
- b) 按测试目录分组生成多个 registry / 二进制；
- c) 每测试独立编译 + 产物缓存（增量友好，管线复杂）。

### 5. 迁移试点（模板）

从「直迁」类挑 5–10 个最简单的（`console.log`、`performance.now`、简单 DOM 读写），迁移为 Porffor manifest 版并标 `ready`，作为批量迁移模板。迁移判定 checklist（同时进 Dong 文档「如何迁移一个测试到 Porffor」）：

- 无跨作用域闭包，或已按 T15 模型（同模块多 export / 状态槽）改写；
- 无 eval / 动态 import / Promise / async（Promise 待 T19 决策）；
- 用到的 API 均已有 host import（对照 T06–T11 清单）；
- 无 React/Preact（此类归 T18 重写）。

## 验收标准

1. CI 出现 Porffor job 且 `ready` 集全绿；覆盖率数字进 CI 摘要。
2. 标记解析 + 默认规则有单测。
3. 全量分类盘点表合入（含 dropped 候选与理由）。
4. 构建组织选型记录（含实测数据）合入。
5. 5–10 个试点迁移合入 + 迁移 checklist 文档。

## 完成记录

- **Commit**: `8d34b91`
- 标记解析：`porffor_test_tags.mjs` + `porffor_test_tags.test.mjs`。
- 全量盘点：`T13-test-inventory.md`（285 文件，ready 138 = 48.4%）。
- 构建选型：方案 a) 记入盘点 §构建组织选型。
- 试点：test_hidden_attr、test_checkbox_toggle、test_dom_content_loaded、test_porffor_greeting、test_class_demo、test_input_value、test_title_tooltip、test_list_markers_basic + 3× mf pilot。
- CI：`zig build run-porffor-tests`；`porffor-migration-checklist.md`。
- 验证：PS1 生成盘点；node 单测待本机 node 环境执行。
