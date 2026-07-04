# T02 — 2c 支持 `createImport` 的内联 C 实现

- **Area**: porffor-fork（`dong/third_party/porffor` 子模块，分支 `enjin_porffor`）
- **性质**: 纯实现型 — 对标物：node/wasm 路径行为（`js` 实现与 `c` 实现结果一致）
- **优先级**: P1
- **依赖**: T01（先保证多参调用正确）
- **预估**: 1–2 人天
- **前置阅读**: `docs/developer/porffor/tasks/setup.md` §3 §5

## 背景

`createImport(name, params, returns, js, c)`（`compiler/builtins.js` L27）签名里有 `c` 参数——设计意图是"以 C 源码提供 import 实现"，但 **2c（`compiler/2c.js`）完全没有消费 `importFunc.c`**。当前 2c 对 import 只有两条路：

1. `--2c-wasm-imports`：生成 `__attribute__((import_module(...)))` 的 extern 声明，交给链接期解决；
2. 默认：只硬编码了 `print`/`printChar`/`time`/`timeOrigin` 等少数内置，其余打 warning `unimplemented import`，调用被**静默丢弃**（见 2c.js ~L633–666）。

Dong 是静态链接场景：host 函数就是 C++ 函数。若 2c 能把 `importFunc.c` 内联/声明进产物，Dong 就不需要 wasm import attribute，也少一层胶水。

## 目标

在 2c 的 import 调用分支（`Opcodes.call`，`compiler/2c.js` ~L614）增加第三条路径：

- 若 `importFunc.c` 存在：将其作为函数体生成一个 C 函数（放入 `prepend`，注意去重与 `static`），调用点直接调它。参数命名约定需在实现时定义清楚（建议：`p0, p1, ...`，返回值直接 `return`）。
- 优先级：`importFunc.c` > `--2c-wasm-imports` extern > 内置硬编码 > warning。
- 对**既无 `c` 又未开 `2cWasmImports`** 的未知 import，把 warning 升级为可选的硬错误（`--2c-strict-imports`），避免静默丢调用（Dong 曾因此浪费排查时间）。

## 验收标准

1. 复现/示例 `docs/developer/porffor/tasks/repro/t02_inline_c_import.js` + 说明文件：注册

```js
createImport('dong_add', 2, 1, (a, b) => a + b, 'return p0 + p1;');
```

   `porf c` 产物中含该 C 函数且调用正确；clang 编译后运行输出与 node wasm 路径一致。
2. 多个模块（两次编译产物链接到一起）不产生 duplicate symbol（配合 static，见 T03）。
3. `--2c-strict-imports` 开启时未知 import 编译报错并指明 import 名；默认行为不变。
4. 现有 `--2c-wasm-imports` 路径不回归。

## 风险 / 说明

- `c` 的类型约定：params/returns 是 wasm valtype（`f64`/`i32`），生成的 C 形参类型用 2c 现成的 `CValtype` 映射。
- 若实现中发现 `c` 参数在上游有其它隐含约定（搜索上游历史），以兼容上游为先。
