# Setup — Dong × Porffor 全面切换 共享上下文

> 所有任务 agent 必读。读完本文档 + 你认领的任务文件（`docs/developer/porffor/tasks/Txx-*.md`）即可开工。

## 1. 项目背景与总目标

**Dong** 是一个游戏 UI 引擎，实现了 HTML + DOM + CSS 的一个子集，原先内嵌 **QuickJS** 作为 JS 引擎。现在的目标是**把 JS 引擎完全切换为 Porffor**（AOT 的 JS → Wasm → C 编译器），并**最终移除 QuickJS**。动机：

- 100% AOT 编译，无 JIT / 解释器，适合游戏主机等禁止 JIT 的平台；
- 编译产物是 C，可与引擎静态链接，体积小、启动快；
- 沙箱性好（Wasm 语义），host 交互面极小（只有显式 import）。

**战略（2026-07 定稿，取代 `调研1.md` 的「双引擎并存」结论）：**

1. **不做兼容 / 回退**：不保留 QuickJS 作为「高级特性兜底」。一切能力要么在 Porffor + host import 上重建，要么**显式裁剪**并记录原因（见 T20 决策矩阵）。不允许出现「这个走 QuickJS」的方案。
2. **框架自研**：不迁移 React / Preact。参考其组件 / 状态模型，搭建 **AOT 友好的自研框架与组件库**（编译期模板展开 + 全局 store + 具名 export handler，见 T18），重写全部 React/Preact 示例。
3. **三线并进**：
   - **porffor-fork**：修 2c 正确性与能力（多参 import、属性 hash、字符串编码、多 export、可重入…）；
   - **dong-host**：host import 扁平 API 重建 DOM / Web 子集；
   - **toolchain / framework**：build 期工具链（内联 handler 提取、snippet 预编译、测试迁移）与自研框架。
4. **终点**：T21 移除 QuickJS（代码、submodule、构建目标），全部测试跑在 Porffor 上；无法迁移的测试显式标 `dropped` 并记录原因。

## 2. 仓库与路径

| 内容 | 位置 |
|------|------|
| 任务文档（本文件所在） | **Dong 仓库** `docs/developer/porffor/tasks/` |
| 复现用例 | Dong 仓库 `docs/developer/porffor/tasks/repro/` |
| **Porffor fork** | Dong 仓库子模块 `dong/third_party/porffor`（`in-dreaming/porffor`，工作分支 `enjin_porffor`，基线 ≈ 上游 `CanadaHonk/porffor` v0.61.13） |
| Porffor 集成层 | `dong/src/script/porffor/`（host / prelude / registry）、`dong/scripts/porffor_*.mjs`（编译管线）、`dong/generated/porffor/`（产物） |
| QuickJS 旧路径（**迁移参照物**，T21 移除） | `dong/src/script/js_*bindings*`、`dong/third_party/quickjs` |

porffor-fork 任务开工前先初始化子模块（`scripts/porffor_paths.mjs` 有对应报错提示）：

```bash
git submodule update --init dong/third_party/porffor
cd dong/third_party/porffor && npm install
```

任务分三类，认领前看任务文件头部 **Area** 字段：

- `Area: porffor-fork` — 改 `dong/third_party/porffor` 里的 Porffor 编译器本身（fork 内提交）；
- `Area: dong-host` — 改 Dong 的 C++ host / prelude / 绑定层；
- `Area: toolchain` — 改 Dong 构建工具链 / 自研框架 / 测试基建。

## 3. 集成现状（P0 已完成）与已核实的代码事实

Dong 不在浏览器里跑 Porffor wasm，而是 `porf c`（2c 路径）生成 C 后静态链接进引擎。已实现：

| 能力 | 实现方式 |
|------|----------|
| AOT 模块启动 | manifest + HTML 中 `data-porffor-module` 属性 |
| `getElementById` | host import，返回 `nodeId`（数字句柄，**不是** DOM 对象） |
| `setTextContent` / 读文本 | stage/commit import（见下） |
| `addEventListener` | handler 为 **export 名字符串**；优先同模块 `exports[]`（T15），过渡仍支持独立 `handlers` 子模块 |
| `setTimeout` | 同上，回调 = export 名 |
| 时间 / 日志 | `dong_time_now`、`dongLog` host import |
| 定时器驱动 | C++ `PorfforHost::processTimers` 每帧 tick |

**stage/commit 模式**：由于 2c 的多参数 import bug（见 T01），Dong 目前把多参调用拆成多次单参 import：先 `dong_stage_0(a)`、`dong_stage_1(b)`… 存入 C++ 侧槽位，最后 `commit_xxx()` 触发真正操作。T01 修复后此样板可逐步退役，但**已有接口保持兼容**。

**编译管线（F13）**：`dong/scripts/porffor_manifest.json` → `porffor_compile.mjs`（内含 NaN 替换 / 符号加前缀 / helper static 化三个文本后处理，T03 目标是让其不再必要）→ `dong/generated/porffor/*.c` + `registry.{h,c}` + `sources.cmake`，静态链接进引擎。

### 已核实的代码事实清单（开工前必读，避免重复踩坑）

以下事实均已对照源码验证过（2026-07），任务文件用 `F#` 编号引用。若你的任务推翻 / 修复了某条，**同步更新本表**。

| # | 事实 | 代码位置 | 相关任务 |
|---|------|----------|----------|
| F1 | 事件 handler 可与主模块**同编译单元**：manifest `exports[]` 指向同模块 `{prefix}export_*` shim，**共享 `_memory` 与 static 全局**；旧 `handlers` 独立子模块仍支持过渡 | `porffor_compile.mjs`、`registry.h`、`2c.js` | T08 T12 T18 |
| F2 | `callExport` 支持 **0 或 1 个 `f64` 参数**（`param_count` + `fn0`/`fn1`）；仍不支持 N>1 | `registry.h`、`porffor_script_registry.cpp` | T09、T15 |
| F3 | ~~host→wasm `makeByteString` static bump~~ **已修（T16）**：host→wasm 改 **拉取式** `result_slot_` + `dong_str_len/read/byte_at`；host 不写模块 memory | `dong_porf_host.cpp`、`dong_porffor_prelude.js` | T06 T08 T10 |
| F4 | import 边界只传 f64、**丢失类型 tag**（仍在）；**已修（T16）**：prelude 强制 `toUtf8()`；`readByteString` UTF-8 + 边界校验 | `dong_porf_host.cpp`、`dong_porffor_prelude.js` | T05 |
| F5 | ~~事件分发按 `registry()->activeModule()`~~ **已修（T08）**：`registerExportHandler` 记录 `module_name`，分发按 listener 所属模块 `callExport` | `js_bindings_porffor.cpp` | T08 |
| F6 | ~~`callExport` 切换 active module 后不恢复~~ **已修（T08）**：`callExport` save/restore active module + `result_slot` 栈；事件槽 `pushEventSlot`/`popEventSlot` 支持 re-entrancy | `porffor_script_registry.cpp`、`dong_porf_host.cpp` | T08 |
| F7 | ~~Porffor 跳过 `DOMContentLoaded`/`load`~~ **已修（T08）**：Porffor 与 QuickJS 共用 lifecycle 分发；约定 **module `main()` = 内联脚本时机**，`DOMContentLoaded`/`load` 在 main 之后对 body 监听者分发 | `engine_view.cpp` | T08 |
| F8 | `getNodeIdFor` 是对 map 的线性反查 O(n) | `js_bindings_porffor.cpp::getNodeIdFor` | T07 |
| F9 | timer id = `timers_.size() + 1`，删除后会重复，不能直接做 clearTimeout | `dong_porf_host.cpp::timerSetTimeout` | T09 |
| F10 | ~~模块 memory / host / registry 进程级单例~~ **已修（T17）**：`PorfforHost`/`PorfforScriptRegistry` per-View；每 (View,module) 独立 memory + `state_capture`/`state_apply` swap；C import 经 `thread_local g_active_host` | `porffor_script_registry.cpp`、`porffor_compile.mjs`、`dong_porf_host.cpp` | T17 |
| F11 | 引擎每帧钩子顺序已固定：`processPendingTasks`（timers）→ `tickTimers` → `tickAnimationFrames`（Porffor 下为空实现）→ flush 布局 | `engine_view.cpp::tickProcessScriptTasks` | T09 |
| F12 | 内联 handler 扫描在 Porffor 分支是打 warning 的 stub，但仍被 `engine_view` 调用 | `js_bindings_porffor.cpp::scanAndRegisterInlineEventHandlers` | T12 |
| F13 | 模块 / handler 静态注册管线见上「编译管线」 | `scripts/porffor_compile.mjs` | T03 T12 T13 |
| F14 | headless 多帧交互测试的实际 flag 是 `--eval-after-frame0-file <path>`（读文件，不是内联字符串） | `examples/html_render_test.cpp` | T14 |
| F15 | HTML 测试共 **279** 个，其中约 150 个带 `<script>` | `dong/examples/data/tests/` | T13 |

## 4. Porffor 硬性限制与对策（全面切换版）

来自 README 与源码验证（`compiler/codegen.js`）。**语言层限制不要试图在任务里「修复」**，但对策不再允许「留 QuickJS」：

| 限制 | 源码依据 | 对策（无 QuickJS 兜底） |
|------|----------|------|
| **无跨作用域闭包**（除参数和全局变量） | `lookupName()`（codegen.js ~L545）只查 `scope.locals` 和 `globals` | 同模块内状态放全局变量；**跨 handler 状态走 T15**（2c 多 export 同模块 / host 状态槽）；框架层由 T18 在**编译期**变换规避闭包 |
| **无运行时 `eval()` / `Function()`** | AOT 根约束。注意：**编译期已知字符串**的 eval 是支持的（codegen.js ~L2242 `knownValue` 路径） | 一切脚本 build 期确定：内联 handler 提取（T12）、测试 snippet 预编译（T14）、交互逻辑必要时下沉 C++ |
| **无 ES Module 运行时 loader** | 无动态加载机制 | build 期 bundle / manifest 多模块（T13、T18 生成器） |
| **`Promise`/`async` 有 known bugs** | `builtins/promise.ts` 有实现但 README 明言不稳定 | **callback（export 名）是唯一官方异步模型**（T09/T10）；Promise 子集是否开放由 **T19** 评估定夺，评估通过前一律禁用 |
| **React / Preact 等框架** | 依赖闭包 + 动态性 | **不迁移、不兜底**：T18 自研 AOT 友好框架替代，全部 React/Preact 示例重写 |

## 5. Porffor 源码地图（`dong/third_party/porffor`）

| 文件 | 作用 | 相关任务 |
|------|------|----------|
| `compiler/index.js` | 编译总入口：源码 → wasm | |
| `compiler/codegen.js` | AST → wasm 主体（~7600 行），作用域/调用/对象逻辑都在这 | T04 |
| `compiler/2c.js` | wasm → C 编译器（2c）。**import 调用生成在 ~L614–668**；`2cWasmImports` pref 开启 extern 声明路径 | T01 T02 T03 T04 T15 |
| `compiler/builtins.js` | `createImport(name, params, returns, js, c)` 定义 host import（L27）；内置函数表 | T01 T02 |
| `compiler/wrap.js` | 编译 + 实例化封装，node 下运行入口；re-export `createImport` | |
| `compiler/assemble.js` | 拼 wasm 二进制 | T05 |
| `compiler/builtins/*.ts` | 用 TS 写的内置 API（string、promise、console…），经 `precompile.js` 生成 `builtins_precompiled.js` | T05 T19 |
| `compiler/prefs.js` | 命令行参数（`Prefs.xxx` ← `--xxx`） | T03 |
| `runtime/index.js` | `porf` CLI | |

**如何本地验证**（Windows，node ≥ 18，在 `dong/third_party/porffor` 下）：

```bash
node runtime/index.js path/to/test.js            # 直接跑（wasm 路径）
node runtime/index.js c path/to/test.js out.c    # 生成 C（2c 路径）
node runtime/index.js native test.js out.exe --compiler=clang   # 原生编译
```

修改 `compiler/builtins/*.ts` 后需要重新预编译：`node compiler/precompile.js`。

## 6. 上游已知 issue（与本项目相关）

| Issue | 内容 | 影响 | 任务 |
|-------|------|------|------|
| [#333](https://github.com/CanadaHonk/porffor/issues/333) | 2c 下 `obj.prop` 点访问返回 undefined（bracket 正常）；root cause：编译期属性 hash 与运行期 hash 不一致 | 任何对象属性访问、「对象方法调用不稳定」疑似同源 | T04 |
| [#337](https://github.com/CanadaHonk/porffor/issues/337) | wasm 模式 `"str" + number` 拼接输出乱码（v0.61.13） | UI 文本渲染 | T05 |
| [#274](https://github.com/CanadaHonk/porffor/issues/274) | native 编译后非 ASCII（box-drawing 等）字符输出损坏 | **中文文本** | T05 |
| [#227](https://github.com/CanadaHonk/porffor/issues/227) | host import 功能请求 → 上游已有 `createImport` 答复 | 集成参考 | T01 T02 |
| [#229](https://github.com/CanadaHonk/porffor/issues/229) | 2c 下 `__Porffor_` 函数找不到 | 2c 稳定性 | 参考 |
| PR [#338](https://github.com/CanadaHonk/porffor/pull/338) | 修 `>>>` 无符号右移（native + wasm） | cherry-pick 候选 | T04 |

## 7. 任务索引

### 7.0 任务性质与「先规格、后实现」规则

每个任务文件头部有 **性质** 字段，取值三种：

- **调研/规格型**：产出是规格书 / 评估报告 / 决策记录，验收 = 评审通过。**不合入生产代码**（允许 throwaway spike 验证可行性，代码不合入）。
- **两段式**：任务内含「阶段 0：spike / 定稿（评审 gate）→ 阶段 1：实现」。**gate 未过不得写生产代码**，阶段 0 的产出（决策记录 / 协议文档）单独可评审。
- **纯实现型**：开工前提是规格 / 对标物已存在，任务只负责按规格实现，不发明语义。

**硬规则：凡实现型工作必须有明确「对标物」（oracle）**，四选一，任务文件头部写明：

1. **QuickJS 现有实现**——`dong/src/script/js_*bindings*` 的签名与行为（T21 退役前是活参照物）；
2. **node/wasm 路径行为**——fork 修复类任务以 `porf` 直跑（非 2c）的结果为准；
3. **浏览器基线**——`scripts/tools/html_baseline_render.py` 渲染结果；
4. **已评审的规格文档**——如 T16 协议文档、T18 框架规格书。

实现中发现规格不可行 → **回规格任务修订并重新评审**，不允许实现侧擅自改语义、边实现边发明。

**分类总表**：

| 性质 | 任务 |
|------|------|
| 调研/规格型 | **T18**（框架与组件库规格）、**T19**（Promise 评估与异步决策） |
| 两段式（内含评审 gate） | **T13**（盘点选型 → CI 基建）、**T15**（方案选型 → 实现）、**T16**（协议定稿 → 实现）、**T17**（spike → 实现）、**T20**（决策矩阵 → (a) 类实现） |
| 纯实现型 | T01–T12、T14、**T22**（框架实现）、**T23**（组件库与示例复刻）、T21 |

按 Wave 组织：同 Wave 内可并行，后一 Wave 依赖前一 Wave 的产出（个别例外见依赖列）。

### Wave 1 — 地基（正确性与协议，全部无互相依赖）

| ID | 标题 | Area | 优先级 | 依赖 |
|----|------|------|--------|------|
| [T01](T01-2c-multiarg-import.md) | 修复 2c 多参数 wasm import 代码生成 | porffor-fork | **P0** | — |
| [T03](T03-2c-output-hygiene.md) | 2c 输出卫生：符号冲突 / static / NaN / --quiet | porffor-fork | P1 | — |
| [T04](T04-2c-property-hash.md) | 修复 2c 属性访问 hash 不一致（#333） | porffor-fork | **P0** | — |
| [T05](T05-string-encoding.md) | 字符串拼接乱码与非 ASCII 输出（#337/#274） | porffor-fork | **P0** | — |
| [T15](T15-shared-state-multi-export.md) | 跨模块状态共享：2c 多 export（主）/ host 状态槽（兜底） | porffor-fork + dong-host | **P0** | — |
| [T16](T16-host-string-channel.md) | host↔wasm 字符串通道与类型/编码边界 | dong-host | **P0** | T05（编码结论） |

### Wave 2 — host API 主体

| ID | 标题 | Area | 优先级 | 依赖 |
|----|------|------|--------|------|
| [T02](T02-2c-inline-c-import.md) | 2c 支持 `createImport` 的内联 C 实现 | porffor-fork | P1 | T01 |
| [T06](T06-host-dom-properties.md) | DOM 属性/查询/classList/style/几何 扁平 import 集 | dong-host | **P0** | T16 |
| [T07](T07-host-dom-mutation.md) | DOM 结构修改 import 集 | dong-host | P1 | T06 |
| [T08](T08-host-events.md) | 事件系统扩展：event 槽 + 生命周期 + 路由修复 | dong-host | **P0** | T16；建议 T15 决策后动工 |
| [T09](T09-host-timers-raf.md) | `setInterval` 与 `requestAnimationFrame` | dong-host | P1 | — |
| [T17](T17-multi-view-instances.md) | 多 View / 模块多实例支持 | dong-host + porffor-fork | P1 | 建议 T15 定稿后动工 |

### Wave 3 — 工具链、平台 API、异步

| ID | 标题 | Area | 优先级 | 依赖 |
|----|------|------|--------|------|
| [T10](T10-host-fetch-callback.md) | callback 版 `fetch` | dong-host | P2 | T08 |
| [T11](T11-host-platform-apis.md) | 平台 API + Dong 专有 API（clipboard/matchMedia/dialog/scene/textLayout） | dong-host | P2 | T06 |
| [T12](T12-toolchain-inline-handlers.md) | HTML 内联事件 build 期提取 | toolchain | P1 | T08、T15 |
| [T13](T13-toolchain-test-migration-ci.md) | 全量测试迁移盘点 + Porffor CI | toolchain | P1 | —（价值随 T06–T12 递增） |
| [T14](T14-toolchain-eval-snippets.md) | headless eval snippet 的预编译替代 | toolchain | P2 | T13 |
| [T19](T19-async-model-promise.md) | 异步模型定稿与 Promise 可用性评估 | porffor-fork + dong-host | P2 | T10 |
| [T20](T20-selection-ce-parser-decision.md) | Selection/Range、CE/IME、DOMParser、FormData：host 化或裁剪决策 | dong-host | P2 | T06 T08 |

### Wave 4 — 框架与收尾

| ID | 标题 | Area | 优先级 | 依赖 |
|----|------|------|--------|------|
| [T18](T18-porffor-ui-framework.md) | 框架与组件库：调研与规格定稿（调研/规格型） | toolchain | P1 | 无——**可立即启动**（规格不依赖实现任务） |
| [T22](T22-framework-compiler-impl.md) | 框架编译器与 store 运行时实现 | toolchain + dong-host | P1 | **T18 规格评审通过**、T15 T16（运行需 T06 T07 T08） |
| [T23](T23-component-library-impl.md) | 组件库与 React 示例复刻 | toolchain | P2 | T22 |
| [T21](T21-quickjs-retirement.md) | QuickJS 退役 | dong-host + toolchain | P3 | T06–T20、T22 T23 全部（以 T13 清单清零为准） |

**明确不做**（评估结论，勿立任务）：运行时 eval / `Function()`、运行时 ES module loader、语言级闭包支持（框架编译期规避）、在 Porffor 上直接跑 React / Preact 原包。**QuickJS 兼容 / 回退不是目标**——凡「Porffor 做不了」的能力，出路只有三条：host import 重建（T06–T11）、逻辑下沉 C++、显式裁剪（T20 决策 + T13 标记 `dropped`）。

## 8. 通用约定

- **fork 修改原则**：改动尽量小且局部，每个修复附最小复现用例（放 `docs/developer/porffor/tasks/repro/`），方便未来 rebase 上游或向上游提 PR。
- **Dong host API 命名**：全部 `dong_` 前缀 + snake_case，例如 `dong_get_value(nodeId)`。参数与返回值只用数字（f64/i32）和字符串指针（编码与传递协议以 **T16 定稿**为准；未定稿前沿用现有 stage 槽机制）。复杂结构一律 JSON 字符串。
- **回调模型**：一律「export 名字符串 + C++ callExport」，不引入函数引用；Promise 在 T19 评估通过前禁用。
- **验收**：每个任务文件都有「验收标准」小节；完成后在任务文件末尾追加 `## 完成记录`（日期、commit、遗留问题）。
- **事实清单维护**：任务修复了 §3 事实清单（F1–F15）中的某条，或发现新的关键事实，随任务 PR 更新本文件。

## 9. 术语表

| 术语 | 含义 |
|------|------|
| **2c** | Porffor 自带的 wasm → C 编译器（`compiler/2c.js`） |
| **prelude** | Dong 注入到每个 Porffor 模块前的 JS 垫片，把 `document.getElementById` 等映射为扁平 host import |
| **manifest** | Dong build 期生成的模块清单：哪个 HTML 元素/事件绑定哪个 Porffor 编译产物（`scripts/porffor_manifest.json`） |
| **nodeId** | Dong DOM 节点的整数句柄，Porffor 侧唯一的「元素引用」 |
| **stage/commit** | 多参调用拆成多个单参 import 的 workaround（§3） |
| **export 名回调** | 用 wasm export 函数名字符串代替 JS 函数引用作回调 |
| **host import** | wasm import，由 C++（`PorfforHost`）实现；经 `createImport` 声明 |
| **状态槽 / store** | 跨模块共享状态的两种形态：host 侧数字/字符串槽（T15 方案 B）、框架层全局 store（T18） |
| **dropped** | 测试/能力被显式裁剪的标记（T13 约定），必须记录原因 |
