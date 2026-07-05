# T14 — eval snippet 盘点

> 自动生成：`node dong/scripts/porffor_snippet_inventory.mjs`

## 摘要

| 指标 | 值 |
|------|-----|
| snippet 文件数 | 5 |
| 方案 A（可预编译 export） | 0 |
| 方案 B（C++ 驱动 / T20） | 5 |
| 待评审 | 0 |

## snippet 文件分类

| 文件 | 方案 | 说明 |
|------|------|------|
| snippets/ce_bold_after_frame0.js | B | execCommand + Selection — 待 T20 |
| snippets/ce_bold_like_button_click.js | B | execCommand + button focus |
| snippets/ce_bold_try_typing_after_frame0.js | B | execCommand + text node walk |
| snippets/ce_bold_try_typing_caret_before_t.js | B | execCommand + range |
| snippets/ce_underline_try_typing_after_frame0.js | B | execCommand underline |

## 引用位置（--eval-after-frame0-file）

| 来源 | snippet 路径 |
|------|-------------|
| scripts/tools/verify_ce_bold_headless.py | snippets/ce_bold_after_frame0.js |
| scripts/tools/verify_ce_enter_cursor_headless.py | snippets/ce_bold_try_typing_after_frame0.js |
| examples/data/tests/test_contenteditable_bold_auto.html | snippets/ce_bold_after_frame0.js（文档引用） |

## 试点（Porffor ready，方案 A）

| 测试 HTML | 模块 | export | runner flag |
|-----------|------|--------|-------------|
| test_porffor_mf_text.html | test_mf_text | afterFrame0 | `--call-export-after-frame0 test_mf_text::afterFrame0` |
| test_porffor_mf_class.html | test_mf_class | afterFrame0 | `--call-export-after-frame0 test_mf_class::afterFrame0` |
| test_porffor_mf_style.html | test_mf_style | afterFrame0 | `--call-export-after-frame0 test_mf_style::afterFrame0` |

CE 类 snippet 归方案 B，去向：T20 Selection/CE 或 C++ `DONG_TEST_*` 输入注入；`porffor_snippet_compile.mjs` 为 QuickJS snippet 生成 plan_b_stub 占位。
