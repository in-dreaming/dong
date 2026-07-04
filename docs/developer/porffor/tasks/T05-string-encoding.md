# T05 — 字符串拼接乱码与非 ASCII 输出（上游 #337 / #274）

- **Area**: porffor-fork（`dong/third_party/porffor` 子模块，分支 `enjin_porffor`）
- **性质**: 纯实现（修复）型 — 对标物：JS 标准语义（node 直跑普通 V8 的结果）；含少量根因分辨工作（#274 是数据坏还是输出通道坏），分辨结论进完成记录
- **优先级**: P0（游戏 UI 大量中文文本，属阻塞级）
- **依赖**: 无
- **预估**: 2–4 人天
- **前置阅读**: `docs/developer/porffor/tasks/setup.md` §5 §6

## 背景

两个上游 issue 直接威胁 Dong 的文本渲染：

1. [#337](https://github.com/CanadaHonk/porffor/issues/337)（v0.61.13，恰为 fork 基线）：wasm 模式下

```js
let frame = 0;
const label = "frame:" + frame;
print(label);   // 期望 frame:0，实际乱码
```

   数字单独打印正常，字符串字面量单独打印正常，**拼接**后乱码。疑似编码/字节序问题。

2. [#274](https://github.com/CanadaHonk/porffor/issues/274)：native（2c）编译后非 ASCII 字符（issue 里是 box-drawing 字符）输出损坏，wasm 路径正常。对中文 UI 是致命的。

## 背景知识

Porffor 有两种字符串类型：`bytestring`（latin1/单字节，快路径）与 `string`（UTF-16 双字节）。拼接、数字转字符串（`compiler/builtins/stringtonumber.ts`、`string.ts`、`string_f64.ts`）、以及 2c 对内存中字符串数据段的输出（`compiler/2c.js` 数据段 / `_memory` 初始化，~L232）都涉及两类型间转换。乱码高发点：

- bytestring 与 string 混拼时类型/步长搞错（1 字节 vs 2 字节步长读写）；
- 数字 → 字符串产物的类型标注与实际编码不一致；
- 2c 把 data 段字节写入 `_memory` 时的转义/编码（#274 更像这一类，或 Windows 控制台输出编码问题——需先分辨是**数据错**还是**打印错**）。

## 工作内容

1. 复现矩阵 `docs/developer/porffor/tasks/repro/t05/`：
   - `"str" + number`、`number + "str"`、模板字符串、`String(n)`；
   - 纯 ASCII、中文（"帧率"）、emoji；
   - 三条路径各跑一遍：node wasm（`porf x.js`）、`porf c` + clang、Windows 下 native。
   - 每例记录 期望/实际，形成矩阵表格提交。
2. 对 #337：从 wasm 路径入手（与 2c 无关），定位拼接 builtin 中 bytestring/string 的类型判定或拷贝步长错误；先查上游 0.61.13 之后的提交是否已修（若已修，评估 rebase / cherry-pick，优先 cherry-pick 保持基线稳定）。
3. 对 #274：先区分「内存里就是坏的」vs「printChar 输出通道坏的」（在 C 产物里 dump 字节判断）。若是 Windows 控制台 codepage 问题，对 Dong 无影响（Dong 走自己的文本渲染），在完成记录中说明并降级；若是 data 段/拼接产物损坏，则修 2c。
4. 明确并记录 **Dong 文本交换编码约定**：建议 host import 边界统一 UTF-8（Porffor 侧 bytestring 对非 ASCII 不够用，需确认 string→UTF-8 转换 helper 是否存在，缺则在 prelude/builtin 补一个 `__Porffor_toUtf8` 类 helper）——这条产出是 **T16（字符串通道）** 的接口依据，进而支撑 T06/T08。注意 import 边界丢失类型信息的问题（setup §3 事实 F4）由 T16 负责，本任务只需给出编码结论。

## 验收标准

1. repro 矩阵中所有 ASCII 用例三条路径全部正确。
2. 中文用例：wasm 路径正确；2c 路径正确或有明确根因记录 + Dong 侧约定（见上）。
3. 不回归：`bench/strcat.js`、`bench/string_methods.js` 在 wasm 与 2c 下行为与修改前一致（乱码修复除外）。
4. 完成记录注明上游修复状态与 cherry-pick 的 commit。
