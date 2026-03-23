---
name: headless-interaction-regression
description: >-
  Turns “manual click / selection / native UI” bugs into deterministic capture pipelines
  for Dong: multi-frame html_render_test, JS snippets after frame 0, stitched BMPs, and
  log-based DOM checks. Use when a defect only appears after human interaction in
  dong_app or similar, when Cursor cannot drive the GUI, or when repro steps are flaky.
---

# Headless Interaction Regression (Dong)

## Overview

无法由 Agent 直接操作本机窗口时，把**交互序列编译成脚本 + 离屏多帧渲染**，产出可比对、可归档、可 CI 的**静态产物**（BMP、拼接图、日志），再据此改引擎或对比回归。

核心思想：**交互与渲染解耦**——用 `document.execCommand` / `eval_script` / 合成鼠标注入代替手点，用**操作前/后两帧**（或更多）固定视觉证据。

## When to Use

- 问题依赖：**选区、焦点、按钮点击顺序、contenteditable、表单** 等，纯静态 HTML 单帧渲染无法复现；
- `dong_app` / 带 SDL 窗口的手动路径能复现，`run-html-test` 单页加载不能；
- 需要给 issue / PR 附**可重复**的截图或 diff，而不是口述操作步骤。

## Dong 仓库内建能力（优先使用）

1. **`html_render_test`**（`zig build run-html-test -- ...`）  
   - 多帧：`--frames 2`（或更多）。  
   - 在第 0 帧截图**之后**执行 JS：`--eval-after-frame0-file <path.js>`（相对 HTML 所在目录、`cwd` 或 `exe/data/` 解析，见 `resolveScriptPath`）。  
   - 横向拼接前后两帧：`--stitch-horizontal`，默认额外生成 `*_stitched.bmp`；可选 `--stitch-output`。

2. **Snippet 目录**：`dong/examples/data/tests/snippets/`  
   - 放可版本管理的短脚本（focus、Range、`execCommand`、模拟「点按钮失焦」等）。

3. **合成输入（可选）**：环境变量 `DONG_TEST_CLICK`、`DONG_TEST_CLICKS`、`DONG_TEST_CLICK_FRAME`（见 `html_render_test` 帮助输出）。用于必须先走引擎鼠标路径才能设置的内部状态（例如 `last_editable_root_`）。

4. **日志**：`DONG_LOG_LEVEL`、`DONG_LOG_TO_STDOUT=1`；关注与编辑相关的 WARN（如 `[JS] execCommand`、`[BOLD-DOM]` 等，以当前代码为准）。

## Recommended Workflow

1. **最小 HTML**：从能手动复现的页面抽一页测试文件（或专用 `test_*.html`），去掉与 bug 无关的干扰。  
2. **先定「操作后」目标**：明确操作完成后 DOM/视觉上应出现的特征（加粗、多一行、错位等）。  
3. **写 snippet.js**：在 IIFE 里顺序调用 DOM API；若模拟「点工具栏」，在 `execCommand` 前把焦点移到 `button`（与手操一致）。  
4. **两帧 + 拼接**：  
   `zig build run-html-test -- <html> zig-out/tmp/case.bmp 800 600 2 --eval-after-frame0-file snippets/xxx.js --stitch-horizontal`  
5. **核对产物**：`*_f000.bmp`（前）、`*_f001.bmp`（后）、`*_stitched.bmp`（并排对比）。  
6. **若仍不复现**：逐项排除  
   - **视口**：与 `dong_app` 同宽高；  
   - **选区实现**：JS `getSelection()` / `addRange` 是否与引擎内 `Selection` 为同一实例（曾出现「改了 JS 选区但 `execCommand` 读另一份」导致无操作）；  
   - **焦点分支**：`execCommand` 在「焦点在 editor」vs「焦点在 button」时是否走不同 C++ 路径；  
   - **是否必须先 `mousedown`**：必要时用 `DONG_TEST_CLICK` 命中 contenteditable。

## Pitfalls (Lesson Learned)

- **JS 暴露的 Selection 必须与 `execCommand` / 布局 / 绘制共用同一 C++ `Selection`**；否则脚本里选中了文字，格式化仍可能对折叠选区 no-op。  
- **`selectNodeContents` 与鼠标拖选**边界常不一致；复现布局类 bug 时若仍失败，可改为 `setBaseAndExtent` 式选区或合成点击+拖移（若引擎支持）。  
- **`eval` 不在 `tick()` 内**：正常由**下一帧** `dong_engine_tick` 做 style/layout/command；若怀疑多帧稳态，可临时加第三帧纯渲染对比 f001/f002。

## Complementary Options

- **页内自动执行**：在 HTML 末尾内联脚本（同步 IIFE）做单帧 `run-html-test`，适合无需「真·前一帧」对比的场景。  
- **外壳自动化**（PyAutoGUI、pywinauto 等）：仅在脚本化无法对齐内部状态时作为最后手段，脆弱且难进 CI。  
- **与本仓库其他 skill 配合**：产出 stitched BMP 后，可用 `baseline-render-diff-analysis` 做左/右语义分区解读（若一侧为浏览器基线）。

## Example Command (Template)

```bash
cd dong
zig build run-html-test -- examples/data/tests/test_contenteditable_basic.html \
  zig-out/tmp/case.bmp 800 600 2 \
  --eval-after-frame0-file snippets/ce_bold_after_frame0.js \
  --stitch-horizontal
```

按场景替换 HTML、输出前缀与 snippet 路径即可。
