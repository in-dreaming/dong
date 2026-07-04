# T01 — 修复 2c 多参数 wasm import 代码生成

- **Area**: porffor-fork（`dong/third_party/porffor` 子模块，分支 `enjin_porffor`）
- **性质**: 纯实现（修复）型 — 对标物：node/wasm 路径行为（`porf` 直跑同一脚本的结果）
- **优先级**: P0
- **依赖**: 无
- **预估**: 0.5–1 人天
- **前置阅读**: `docs/developer/porffor/tasks/setup.md` §3 §5

## 背景

Dong 的所有 host 交互都靠 wasm import。2c（wasm → C）在 `--2c-wasm-imports` 开启时会为 import 生成 extern 声明和调用，但**调用生成只弹出一个参数**，多参 import 生成的 C 代码错误。这是 Dong 被迫使用 stage/commit 样板（多次单参 import + commit）的根本原因。

## 现状（bug 位置）

`compiler/2c.js` `Opcodes.call` 分支中，import 调用只 pop 了栈顶一个值：

```626:628:compiler/2c.js
              const call = `${name}(${importFunc.params.length > 0 ? vals.pop() : ''})`;
              if (importFunc.returns.length > 0) vals.push(call);
                else line(call);
```

而同文件普通（非 import）函数调用是正确的按参数个数逆序弹栈：

```675:676:compiler/2c.js
          let args = [];
          for (let j = 0; j < func.params.length; j++) args.unshift(removeBrackets(vals.pop()));
```

extern 声明部分（L621–623）已经正确输出全部参数类型，只有调用处错。

另外注意：`createImport(name, params, returns, js, c)`（`compiler/builtins.js` L27）本身支持任意参数个数，wasm 路径（node 运行）多参 import 是正常的——问题仅在 2c。

## 修复方案

把 import 调用与普通调用统一：按 `importFunc.params.length` 逆序 `vals.pop()` 收集参数后 join。注意保持 `returns.length > 0` 时 push 表达式、否则 `line()` 的现有行为。

## 验收标准

1. 新增最小复现 `docs/developer/porffor/tasks/repro/t01_multiarg_import.js`：

```js
// 通过 createImport('dong_test3', 3, 1, ...) 注册一个 3 参 import 并调用
```

   配套一个小 node 脚本（可参考 `compiler/wrap.js` 用法）：`createImport` 注册 0/1/2/3 参 import → `porf c` 生成 C → 人工/脚本断言生成的调用形如 `__porf_import_dong_test3(a, b, c)` 且参数顺序正确（第一个 JS 实参对应 C 第一个形参）。
2. 用 clang 编译生成的 C（extern 由测试桩 C 文件提供实现），运行结果正确。
3. 0 参、1 参 import 行为不回归（`porf c` 编译 `bench/hello.js` 输出与修改前一致）。
4. 现有 wasm 路径不受影响（`node runtime/index.js docs/developer/porffor/tasks/repro/t01_multiarg_import.js` 正常）。

## 风险

- 参数顺序易错：wasm 栈顶是**最后一个**参数，必须 `unshift`（逆序恢复）。
- 修完后 Dong 侧可逐步退役 stage/commit，但那是 Dong 仓库的后续工作，本任务不涉及。
