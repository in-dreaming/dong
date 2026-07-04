# T20 — Selection/Range、CE/IME、DOMParser、FormData：host 化或裁剪决策

- **Area**: dong-host
- **性质**: **两段式** — 阶段 0：决策矩阵填完并**评审**（gate）；阶段 1：仅 (a) 类的 import 实现（对标物：QuickJS 对应 bindings 的行为）。(b)/(c) 类只产出去向条目，不在本任务实现
- **优先级**: P2（决策先行；实现量视决策）
- **依赖**: T06（import 脚手架）、T08（事件槽）
- **预估**: 决策 1–2 人天 + 实现视决策（可拆子任务）
- **前置阅读**: `docs/developer/porffor/tasks/setup.md` §1 §4 §7

## 背景

这几类能力原计划「暂留 QuickJS」。全面切换后没有这个选项，每一项必须三选一并记录：

- **(a) host import 实现子集** —— 值得为产品面保留的；
- **(b) 逻辑下沉 C++** —— 引擎内部本来就有实现、JS 只是查询/驱动面的（测试改由 C++ 驱动，衔接 T14 方案 B）；
- **(c) 显式裁剪（dropped）** —— 与游戏 UI 产品面无关或成本严重不成比例的，删测试并在 T13 盘点表记录原因。

## 决策矩阵（任务内填完并评审；下表为初判倾向）

| 能力 | 相关测试 | 初判 | 依据 |
|------|----------|------|------|
| Selection / Range 查询 API | `test_selection_api.html` 等 | b / c | 选区逻辑引擎已有 C++ 实现（`src/dom` Selection、`js_selection_bindings.cpp` 只是包装）；JS 面按游戏 UI 需求裁到最小（如只留 `dong_selection_text() -> str`） |
| contentEditable 交互驱动 | `test_contenteditable_*` 系列 | b | CE 编辑逻辑全在 C++；多帧测试经 T14 迁移为 callExport / C++ 模拟输入 |
| IME composition / beforeinput | `test_ime_composition.html` 等 | b | 事件与状态机在 C++；JS 只做 UI 反馈——需要时在 T08 事件槽补 composition 字段（`dong_event_data() -> str`） |
| DOMParser | `innerhtml_*` 系列部分 | a | 一个 import 即可：`dong_parse_html(html) -> nodeId`（游离树根），复用引擎 lexbor 解析 |
| FormData | `test_formdata_api.html` | a / c | `dong_form_serialize(formNodeId) -> str`（JSON）一个 import；若产品面不需要则裁剪 |

## 工作内容

1. 逐项过一遍相关测试的实际断言，填完矩阵（含每项的 API 清单或裁剪理由），评审定稿；
2. 决定 (a) 的项：import 清单 + 最小用例（走 T06 脚手架）；
3. 决定 (b) 的项：与 T14 的 snippet 分类表对齐，给出每个测试的 C++ 驱动改造去向；
4. 决定 (c) 的项：T13 盘点表标 `dropped(原因)`。

## 验收标准

1. 决策矩阵评审合入（每项有决策 + 依据 + 去向）。
2. (a) 类实现合入并有用例；(b) 类在 T14/T13 有对应条目；(c) 类标记完成。
3. T13 盘点表中本任务负责的测试全部有去向，无悬空。
