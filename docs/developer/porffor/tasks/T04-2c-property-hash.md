# T04 — 修复 2c 属性访问 hash 不一致（上游 #333）

- **Area**: porffor-fork（`dong/third_party/porffor` 子模块，分支 `enjin_porffor`）
- **性质**: 纯实现（修复）型 — 对标物：node/wasm 路径行为（同一脚本 `porf` 直跑的结果）
- **优先级**: P0
- **依赖**: 无
- **预估**: 1–3 人天
- **前置阅读**: `docs/developer/porffor/tasks/setup.md` §5 §6

## 背景

上游 issue [#333](https://github.com/CanadaHonk/porffor/issues/333)：2c/native 下

- 普通对象字面量 `obj.prop`（点访问）返回 `undefined`，`obj["prop"]` 正常；
- class 实例字段恰好相反：点访问正常、bracket 返回 `undefined`；
- `porf file.js`（wasm/node 路径）全部正常。

社区已定位 root cause（issue 内 @arv 评论）：**`o.a` 走编译期属性名 hash，`o["a"]` 走运行期 hash，两个 hash 函数在 2c 产物中结果不一致**。修复方向是对齐两个 hash 实现（而不是禁用编译期 hash）。

这很可能也是 Dong 调研中「对象方法调用不稳定（`el.addEventListener(...)` 时好时坏）」的根源——方法查找同样经属性 hash。修复后 Dong prelude 有机会从"纯扁平函数"放宽为允许简单对象字面量做命名空间。

## 工作内容

1. 在本仓库最小复现（`docs/developer/porffor/tasks/repro/t04_prop_hash.js`）：对象字面量 + class 实例，点/括号四种组合，`porf c` + clang 运行对比 node 路径。
2. 定位两个 hash 实现：编译期在 `compiler/codegen.js`（属性名 → hash 常量折叠处），运行期在 builtins（`compiler/builtins/_internal_object.ts` / `object.ts` 一带）与 2c 对其的翻译。重点排查 2c 对 hash 所用整数运算（乘法溢出、`>>>`、i32/u32 转换）的 C 翻译是否与 wasm 语义一致——上游 PR [#338](https://github.com/CanadaHonk/porffor/pull/338)（修 `>>>` 无符号右移）高度相关，**先 cherry-pick #338 再复测**，可能直接修好。
3. 若 #338 不够：对齐两个 hash（改编译期侧以匹配运行期，或反之），保持 wasm 路径不回归。
4. 顺带验证「对象方法调用」场景：`const o = { f(x) { return x + 1 } }; o.f(1)` 在 2c 下正确。

## 验收标准

1. repro 四种组合在 2c/native 下结果与 node 路径一致。
2. 对象方法调用 repro 通过。
3. 跑一遍上游测试的对象相关子集（`node test262/index.js` 太重的话，至少 `bench/object_get.js` 等现有样例 2c 前后对比）无回归。
4. 完成记录中注明是否 cherry-pick 了 #338、上游是否已有官方修复（若有，评估直接同步）。

## 风险

- hash 对齐若改运行期侧，会影响 wasm 路径已有对象布局——优先改 2c 翻译或编译期侧。
- 溢出语义：JS 数字运算在 C 里用 f64/i32 混合实现时，`imul`/移位边界行为最容易错。

## 完成记录

- **日期**: 2026-07-04
- **commit**: `78cb0a6`（Dong）；porffor fork `ab4ca68e`（enjin_porffor，cherry-pick 上游 PR #338）
- **摘要**: cherry-pick 上游 PR [#338](https://github.com/CanadaHonk/porffor/pull/338)（`6ec69590` + `84999b31`）：修复 2c/native 对 `>>>` 无符号右移的 C 翻译（`(u32)` 逻辑移位）及 wasm 侧 `f64ifyUSHR`/位运算 ToInt32  coercion。编译期 `ctHash`（codegen.js）与运行期 `__Porffor_object_hash`（builtins/_internal_object.ts）的 hash 在 2c 产物中重新对齐，修复 #333 点/括号属性访问不一致及对象方法查找。
- **变更文件**:
  - `dong/third_party/porffor/compiler/2c.js`
  - `dong/third_party/porffor/compiler/expression.js`
  - `docs/developer/porffor/tasks/repro/t04_prop_hash.js`
  - `docs/developer/porffor/tasks/repro/t04_verify.mjs`
- **验收命令**（在 `dong/third_party/porffor` 下）:
  - `E:\ws\infra\dong\.tools\node-v22.16.0-win-x64\node.exe ..\..\..\docs\developer\porffor\tasks\repro\t04_verify.mjs`
  - `node ..\..\..\docs\developer\porffor\tasks\repro\t01_verify.mjs`（回归）
  - `node ..\..\..\docs\developer\porffor\tasks\repro\t03\t03_verify.mjs`（回归）
- **上游状态**: PR #338 尚未合入 upstream main（2026-07-04）；本 fork 已 cherry-pick。
- **验收**: wasm 路径 repro 通过（exit 0）；2c 编译通过且生成 C 含 `(u32)` 逻辑移位；clang native 运行本机不可用（SKIP）。
