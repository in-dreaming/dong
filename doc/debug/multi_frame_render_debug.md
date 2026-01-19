## 多帧渲染不一致/闪烁排查工具链

这套工具用于把“低频闪烁 / 大多数帧错误、偶尔帧正确”的现象**稳定采样为多帧图片**，并用“浏览器基线渲染”作为对照，最后把所有帧合成一张大图交给多模态模型分析每帧差异。

### 目录与工具

- **Dong 多帧导出**：`examples/html_render_test.cpp` → `html_render_test.exe`
- **浏览器基线渲染（不使用 dong）**：`scripts/tools/html_baseline_render.py`
- **多帧拼图 + 差异分析（可选 LLM）**：`scripts/tools/vl_tool_multi.py`
- **单图验收（可选 LLM）**：`scripts/tools/vl_tool.py`

---

## 1) 用 dong 导出 n 帧

### 用法

```bash
html_render_test <html_file> [output.bmp] [width] [height] [frames]
html_render_test <html_file> [output.bmp] [width] [height] --frames N [--frame-ms MS] [--no-update]
```

### 关键参数

- **`--frames N`**：导出 N 帧（每帧输出独立 BMP）
- **`--frame-ms MS`**：每帧之间 sleep（用于让基于真实时间的动画/状态推进更明显）
- **`--no-update`**：不调用 `dong_view_update()`，只重复 render（用于判断“不一致是否来自渲染链路本身”）

### 输出规则

当 `frames > 1`：
- 如果 `output` 以 `.bmp` 结尾：写出 `xxx_f0.bmp ... xxx_fN.bmp`
- 如果 `output` 是目录：写出 `<html_stem>_f0.bmp ...`

> 建议使用**绝对路径**作为输出目录，避免在 `zig-out/bin` 下运行时出现 `zig-out/bin/zig-out/tmp/...` 这种“二次嵌套”。

---

## 2) 用浏览器渲染基线（base）

`html_baseline_render.py` 使用 Playwright + Chromium 做 headless 渲染截图，作为对照基线。

### 安装依赖

```bash
pip install playwright
playwright install chromium
```

### 用法

```bash
python scripts/tools/html_baseline_render.py <path/to/test.html> --out <base.png> --width 800 --height 600
```

建议：
- 用固定 `width/height/device-scale-factor` 对齐 dong 输出
- 如遇到字体/动画导致的抖动，可调大 `--wait-ms`

---

## 3) 合并多帧并生成差异报告

`vl_tool_multi.py` 会把 `BASE` 与每一帧 `FRAME_i` 以及 `DIFF(BASE vs FRAME_i)` 拼成一张大图，并输出 JSON 报告（包含每帧 diff bbox）。

### 仅做拼图+像素差异（不调用 LLM）

```bash
python scripts/tools/vl_tool_multi.py <html_file> \
  --base <base.png> \
  --glob "<frames_dir>/*_f*.bmp" \
  --out-image <merged.png> \
  --out-json <report.json> \
  --no-llm
```

### 调用多模态模型（可选）

配置环境变量：

```bash
set OPENROUTER_API_KEY=... 
set OPENROUTER_MODEL=x-ai/grok-4.1-fast
```

然后去掉 `--no-llm`：

```bash
python scripts/tools/vl_tool_multi.py <html_file> --base <base.png> --glob "..." --out-image merged.png --out-json report.json
```

输出：
- `merged.png`：每行一个 frame，三列 `BASE | FRAME_i | DIFF`
- `report.json`：逐帧 `diff_bbox` +（可选）LLM 的逐帧描述/根因方向

---

## 4) 单张图验收（可选）

`vl_tool.py` 用于“单张截图 + HTML”的一致性分析（适合快速验收某一帧）。

```bash
set OPENROUTER_API_KEY=...
python scripts/tools/vl_tool.py <image_path> <html_path>
```

---

## 5) 如何用这套工具定位“低频闪烁”

### 建议流程

1. 先用 `html_baseline_render.py` 生成 `base.png`
2. 用 `html_render_test` 导出足够多帧（例如 120 帧，`--frame-ms 16`）
3. 用 `vl_tool_multi.py` 合并并查看：
   - 哪些帧与 base 一致
   - 哪些帧出现明显差异（bbox 会指示差异区域）
4. 如果差异呈现“偶尔正确、经常错误”，优先怀疑：
   - GPU 提交/同步边界不稳
   - 资源生命周期（pipeline/shader/texture）与帧循环竞争
   - pass 切换时的状态污染（blend/scissor/viewport/uniform）

### 引擎侧重点排查点（经验清单）

- **Swapchain acquire 语义**：SDL_gpu 后端可能出现“acquire 成功但 texture==NULL”的跳帧语义；上层若没显式处理，用户侧观感会像闪烁。
- **读回同步**：读回 GPUTexture 必须 fence 等待完成；否则可能采到上一帧/未完成 copy 的内容。
- **TransferBuffer cycle**：大量多帧读回时，cycle 策略/等待策略不一致会造成偶发现象。

---

## 6) 校验 `examples/data/tests` 用例（建议）

### A. 仅做“多帧自一致性”检查（不依赖浏览器）

用于确认是否存在“同一 HTML、同一参数，多帧输出不一致”的问题：

```bash
python scripts/tools/run_multiframe_regress.py --frames 60 --frame-ms 0
```

它会对每个用例输出：
- `OK: all frames identical`：多帧一致
- `NONDETERMINISTIC`：存在帧间差异，并给出 `*_nondet.png/json`

### B. 浏览器 baseline 对比（推荐）

```bash
python scripts/tools/run_baseline_compare.py --frames 60 --frame-ms 16
```

会对每个用例产出：
- `<case>_base.png`：浏览器渲染基线
- `<case>_f000.bmp ...`：dong 多帧输出
- `<case>_merged.png`：三列拼图 `BASE | FRAME_i | DIFF`
- `<case>_report.json`：逐帧 diff bbox +（可选）LLM 分析结果

如需只跑单个用例：

```bash
python scripts/tools/run_baseline_compare.py --case cursor_test --frames 120 --frame-ms 16
```

> 如要启用 LLM 分析：设置 `OPENROUTER_API_KEY` 并加 `--llm`。

