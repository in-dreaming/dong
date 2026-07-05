# T08 — 事件系统扩展：event 槽 + 生命周期事件 + 分发路由修复

- **Area**: dong-host
- **性质**: 纯实现型（含两个小定稿点：生命周期事件约定、re-entrancy 规则——定稿即评审，不单独成阶段）— 对标物：QuickJS 事件行为（`test_select_keyboard.html` 等测试断言）+ T16 协议
- **优先级**: P0（交互测试全靠它；T10/T12 的前置）
- **依赖**: T16（`-> str` 字段的字符串通道）；建议 T15（跨模块状态/多 export）决策后动工，handler 组织方式受其影响
- **预估**: 3–5 人天
- **前置阅读**: `docs/developer/porffor/tasks/setup.md` §3 §8 §9

## 背景

现状：`addEventListener` 已可用（export 名回调模型），但 handler 拿不到 event 对象，且只覆盖少数事件类型。Porffor 无法把 MouseEvent 等宿主对象传进 wasm，评估结论：**「全局 event 槽」模式，高可行，纯 Dong 侧**。

模型：C++ 在调用 handler export **之前**，把当前事件字段写入 host 侧"当前事件槽"；handler 内通过同步 import 读取。事件是同步分发的，槽在 handler 执行期间稳定。

## API 清单

### 8.1 事件槽读取 import

| import | 说明 |
|--------|------|
| `dong_event_type() -> str` | `"click"` / `"input"` / `"keydown"` … |
| `dong_event_target() -> nodeId` | |
| `dong_event_key() -> str` / `dong_event_key_code() -> i32` | 键盘 |
| `dong_event_x() -> f64` / `dong_event_y() -> f64` / `dong_event_button() -> i32` | 鼠标/指针 |
| `dong_event_modifiers() -> i32` | bitmask: shift=1, ctrl=2, alt=4, meta=8 |
| `dong_event_value() -> str` | input/change 时目标当前值（省一次 `dong_get_value`） |
| `dong_event_prevent_default()` / `dong_event_stop_propagation()` | 写操作，标记回 C++ 事件分发器 |

### 8.2 事件类型扩展

现有 bridge 上补齐：`input`、`change`、`keydown`、`keyup`、`focus`、`blur`、`mousedown`、`mouseup`、`mousemove`、`pointerdown/up/move`（按引擎现有事件命名对齐）。覆盖 `test_select_keyboard.html` 所需全部类型。

**生命周期事件（事实 F7）**：`engine_view.cpp` 的 Porffor 分支目前把 `DOMContentLoaded` / `load` 的分发整段 `#ifndef` 跳过。本任务内二选一定稿：
- 约定「模块 main 即 DOMContentLoaded」（写入 setup §3 与框架文档），并补 `load` 的 dispatch；
- 或补齐两个事件的完整分发（document 级 listener 走同一 export 名模型）。
`test_dom_content_loaded.html` 迁移时会撞上，不能悬空。

### 8.3 prelude

提供 `eventType()`、`eventKey()` 等直通包装。**不要**构造 event 对象字面量传给 handler（闭包/对象模型限制，见 setup §4 与 T04）。

### 8.4 现状实现坑（本任务必修）

- **路由错误（F5）**：`dispatchPorfforEvent` 现在按 `registry()->activeModule()`（= 最后 run 的模块）找 handler，多模块页面会路由错。修复：`registerExportHandler` 时记录注册方模块名，分发按记录的模块 callExport。
- **active module 不恢复（F6）**：`callExport` 切换 active module / memory 后不还原，嵌套调用会顶掉外层模块的内存指针。修复：callExport 栈式 save/restore active module——与验收 3 的事件槽 re-entrancy 一并处理。

## 验收标准

1. `test_select_keyboard.html` 及至少一个鼠标交互用例的 Porffor 版通过。
2. `preventDefault` 生效用例：checkbox 的 click handler 调用后勾选态不变。
3. 嵌套分发用例：handler 内再触发另一事件（若引擎允许同步 re-dispatch），事件槽需栈式保存恢复——或明确文档禁止 re-entrancy 并在 C++ 侧断言。
4. 全部事件类型有一张「类型 × 可读字段」覆盖表进 Dong 文档。
5. F5/F6 修复各有回归用例：两个模块分别监听各自元素、点击路由正确；handler 内（若引擎允许）同步 re-dispatch 后外层模块内存指针未被顶掉。
6. 生命周期事件约定定稿并更新 setup §3 事实清单。

## 风险

- 事件槽 re-entrancy 是主要设计坑，验收 3 必须二选一明确（并与 F6 的 active module 栈一起处理）。
- `dong_event_value` 的字符串编码/通道依赖 T05/T16 约定（中文输入法场景归 T20 的 IME 决策，本任务不含 composition 事件）。

## 完成记录

- **日期**: 2026-07-04
- **Commit**: e273b9d
- **事件类型 × 可读字段覆盖表**:

| 事件类型 | type | target | key/keyCode | x/y/button | modifiers | value | preventDefault |
|----------|------|--------|-------------|------------|-----------|-------|----------------|
| click, dblclick | ✓ | ✓ | | ✓ | ✓ | | ✓ |
| mousedown, mouseup, mousemove | ✓ | ✓ | | ✓ | ✓ | | ✓ |
| pointerdown/up/move | ✓ | ✓ | | ✓ | ✓ | | ✓ |
| keydown, keyup, keypress | ✓ | ✓ | ✓ | | ✓ | | ✓ |
| focus, blur | ✓ | ✓ | | | | | ✓ |
| input, change | ✓ | ✓ | | | | ✓ | ✓ |
| DOMContentLoaded, load | ✓ | ✓ | | | | | ✓ |

- **Lifecycle 约定**: Porffor module `main()` 在 HTML 解析后同步执行（等同内联脚本）；`DOMContentLoaded` 与 `load` 在 main 返回后对 `<body>`（或 root）上已注册的 export 名 listener 分发。
- **Re-entrancy**: 事件槽栈式 save/restore（`pushEventSlot`/`popEventSlot`）；`callExport` 同时 save/restore active module 与 `result_slot`。
- **异步约定**: 见 `docs/developer/porffor/async-convention.md`（T19 定稿）。
- **遗留**: composition/IME 事件未覆盖（T20）；`load` 未入 `EventDispatcher` enum（Porffor 走 `dispatchSimpleEvent` 字符串匹配，足够 lifecycle listener）。
