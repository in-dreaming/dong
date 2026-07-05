# T13 盘点 — T20 负责测试去向

**日期**: 2026-07-05  
**来源**: T20 决策矩阵评审  
**标记约定**: `blocked(Txx)` / `dropped(原因)` / `ready`（见 T13）

| 测试文件 | T20 决策 | T13 标记 | 迁移去向 |
|----------|----------|----------|----------|
| `test_selection_api.html` | (b)+最小(a) | `blocked(T14)` | C++ 模拟选区 + `selectionText()` 断言；不全量移植 Range API |
| `test_selection_mouse.html` | (b) | `blocked(T14)` | 多帧鼠标拖拽由 C++ 驱动 |
| `test_selection_pseudo.html` | (b) | `blocked(T14)` | 伪元素选区 C++ 断言 |
| `test_contenteditable_basic.html` | (b) | `blocked(T14)` | C++ 键盘输入 + 多帧截图 |
| `test_contenteditable_typing.html` | (b) | `blocked(T14)` | 同上 |
| `test_contenteditable_features.html` | (b) | `blocked(T14)` | 同上 |
| `test_contenteditable_auto.html` | (b) | `blocked(T14)` | 同上 |
| `test_contenteditable_bold_auto.html` | (b) | `blocked(T14)` | 同上 |
| `test_contenteditable_prebolded.html` | (b) | `blocked(T14)` | 同上 |
| `test_ime_composition.html` | (b) | `blocked(T14)` | C++ composition 事件序列 |
| `innerhtml_test.html` | (a) | `blocked(T06)` | 改用 `parseHtml` + DOM 读断言 |
| `innerhtml_simple_test.html` | (a) | `blocked(T06)` | 同上 |
| `innerhtml_auto_test.html` | (a) | `blocked(T06)` | 同上 |
| `innerhtml_baseline.html` | (a) | `pending` | 静态为主；script 段用 `parseHtml` |
| `innerhtml_original.html` | (a) | `blocked(T06)` | 同上 |
| `test_formdata_api.html` | (a) | `blocked(T06)` | `formSerialize` + JSON 解析 |
| `test_formdata_name_serialization.html` | (a) | `blocked(T06)` | 同上 |

**说明**: 本表为 T13 全量盘点的 T20 切片；全表随 T13 阶段 0 继续扩充。
