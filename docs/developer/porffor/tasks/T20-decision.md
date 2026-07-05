# T20 — Selection / CE / IME / DOMParser / FormData 决策矩阵

**日期**: 2026-07-05  
**评审**: Wave 3 gate 通过（实现 (a) 类最小子集）

| 能力 | 相关测试 | 决策 | API / 去向 |
|------|----------|------|------------|
| Selection / Range 查询 | `test_selection_api.html` 等 | **(b) 裁剪 + 最小 (a)** | `selectionText()` import；完整 Range API **dropped**（游戏 UI 用引擎选区） |
| contentEditable 交互 | `test_contenteditable_*` | **(b)** | C++ 输入驱动 + T14 `call-export-after-frame0` 或 headless 键盘注入；不暴露 `execCommand` |
| IME composition | `test_ime_composition.html` | **(b)** | C++ composition 状态机；JS 仅 UI 反馈（T08 事件槽可扩展 `eventData`） |
| DOMParser | `innerhtml_*` 部分 | **(a)** | `parseHtml(html) -> nodeId`（游离根，lexbor） |
| FormData | `test_formdata_api.html` | **(a) 子集** | `formSerialize(formId) -> JSON`；无 `FormData` 对象 |
| MutationObserver | — | **(c)** | 引擎内部 callExport；不做通用 observer |
| `execCommand` | CE bold 等 | **(c)** | 多帧测试改 C++ 驱动或 snippet export（T14） |

## (a) 类已实现 import

| import | prelude |
|--------|---------|
| `dong_parse_html` | `parseHtml(s)` |
| `dong_form_serialize` | `formSerialize(formId)` |
| `dong_selection_text` | `selectionText()` |

## (b) 类测试去向

见 `T13-test-inventory.md` 中 `blocked(T20)` 行；迁移时标 `<!-- porffor: blocked(T20) -->` 或改 C++ 驱动。

## (c) 类 dropped 候选

- 完整 Selection/Range DOM API
- `document.execCommand`
- MutationObserver 通用 API

理由：游戏 UI 产品面由引擎 + 扁平 import 覆盖；成本/收益不成比例。
