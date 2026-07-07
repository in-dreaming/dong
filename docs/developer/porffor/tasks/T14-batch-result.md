# T14 波次处理结果

> `node dong/scripts/porffor_batch_migrate_t14.mjs` 生成

## 摘要

| 动作 | 数量 |
|------|------|
| ready（frame-0 冒烟） | 8 |
| blocked(T20)（execCommand 解析期） | 4 |
| blocked(T20) + mf.json 占位 | 1 |
| 跳过 | 273 |

## 明细

| 文件 | 结果 | 说明 |
|------|------|------|
| test_ce_debug.html | ready | CE debug buttons layout |
| test_ce_enter_offset.html | ready | static CE blocks + log placeholder |
| test_ce_interactive_sim.html | blocked(T20) | simulated button+execCommand on parse |
| test_ce_mixed_multiline.html | ready | mixed CE sections layout |
| test_ce_simulate.html | blocked(T20) | simulated execCommand flow on parse |
| test_contenteditable_auto.html | blocked(T20) | execCommand suite on parse |
| test_contenteditable_basic.html | ready | CE toolbar + editor layout |
| test_contenteditable_bold_auto.html | blocked(T20)+mf | auto bold on parse; needs C++ drive or ce_bold snippet (Plan B) (test_contenteditable_bold_auto.mf.json) |
| test_contenteditable_features.html | blocked(T20) | execCommand feature matrix on parse |
| test_contenteditable_prebolded.html | ready | pre-bolded span in static HTML |
| test_contenteditable_typing.html | ready | CE editor + instruction list |
| test_ime_composition.html | ready | IME fields layout; events on user input |
| test_selection_api.html | ready | selection UI; interaction via click only |

## 后续

- **ready** 集：`run-porffor-tests.mjs` frame-0 布局回归
- **blocked(T20)**：待 C++ 输入注入或 `ce_bold_*` snippet Porffor 化（Plan B）
- `test_contenteditable_bold_auto.mf.json` 已预留 `callExportAfterFrame0`，待 `porffor_snippet_compile` 完成 prelude 改写后启用
