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

| 文件 | 方案 | 说明 | 大小 |
|------|------|------|------|
| snippets/ce_bold_after_frame0.js | B | CE/selection — C++ input or T20 | 782B |
| snippets/ce_bold_like_button_click.js | B | CE/selection — C++ input or T20 | 835B |
| snippets/ce_bold_try_typing_after_frame0.js | B | CE/selection — C++ input or T20 | 857B |
| snippets/ce_bold_try_typing_caret_before_t.js | B | CE/selection — C++ input or T20 | 1639B |
| snippets/ce_underline_try_typing_after_frame0.js | B | CE/selection — C++ input or T20 | 862B |

## 引用位置（--eval-after-frame0-file）

| 来源 | snippet 路径 |
|------|-------------|
| scripts/tools/verify_ce_bold_headless.py | snippets/ce_bold_after_frame0.js |
| examples/data/tests/test_contenteditable_bold_auto.html | snippets/ce_bold_after_frame0.js |

## 试点（Porffor ready）

| 测试 HTML | 模块 | export | 说明 |
|-----------|------|--------|------|
| test_porffor_mf_text.html | test_mf_text | afterFrame0 | 帧后改 textContent |
| test_porffor_mf_class.html | test_mf_class | afterFrame0 | 帧后 classAdd |
| test_porffor_mf_style.html | test_mf_style | afterFrame0 | 帧后 setStyle |

CE 类 snippet（`ce_bold_*`）归方案 B，待 T20；不阻塞 T14 runner 基建验收。
