---
name: multi-frame-render-debug
overview: 构建多帧渲染导出 + Python 基线渲染 + 多帧视觉对比工具链，用于定位并修复 HTML 用例的低频闪烁/不一致问题。
todos:
  - id: repo-scan
    content: 使用[subagent:code-explorer]梳理渲染导出入口与用例结构
    status: completed
  - id: cpp-multiframe-export
    content: 改造html_render_test.cpp支持n帧渲染并逐帧导出
    status: completed
    dependencies:
      - repo-scan
  - id: python-baseline-render
    content: 新增html_baseline_render.py生成base图片并对齐视口参数
    status: completed
    dependencies:
      - repo-scan
  - id: multi-frame-compare
    content: 新增vl_tool_multi.py合并大图并输出逐帧diff与摘要
    status: completed
    dependencies:
      - python-baseline-render
      - cpp-multiframe-export
  - id: debug-docs
    content: 编写doc/debug文档：命令、产物示例、定位与回归步骤
    status: completed
    dependencies:
      - multi-frame-compare
  - id: batch-validate-fix
    content: 批量校验examples/data/tests并修复发现的不一致问题
    status: completed
    dependencies:
      - debug-docs
---

## Product Overview

“multi-frame-render-debug”是一套用于排查 HTML 渲染低频闪烁/不一致问题的工具链：将同一 HTML 连续渲染多帧导出，生成基线渲染结果，并对多帧与基线做可视化合并与逐帧差异输出，帮助快速定位异常帧与异常区域。

## Core Features

- **多帧渲染导出**：对单个 HTML 用例连续渲染 n 帧，导出每一帧独立图片/贴图文件；输出命名清晰、可批处理复现。
- **基线渲染产出**：对同一 HTML 生成稳定的“base”图片，用于对齐对比与回归检查；输出目录结构与用例一一对应。
- **多帧合并对比**：将 base + n 帧拼接合成为单张对比大图（网格/条带），便于快速肉眼发现闪烁、跳变、缺失、锯齿变化；同时输出逐帧差异图与差异统计摘要（例如差异像素占比/最大差异帧）。
- **排查文档**：以步骤化方式描述“生成多帧→生成 base→合并与差异→定位→修复→回归”的完整流程，并给出输出示例图的观感说明（正常/异常对比）。

## Tech Stack

- C++：改造 `examples/html_render_test.cpp` 支持多帧渲染与逐帧导出
- Python：在 `scripts/tools` 实现基线渲染与多帧对比工具（建议使用 Pillow / OpenCV 做图像读写、拼接与差分）
- 文档：在 `doc/debug` 编写 Markdown 流程文档与示例

## System Architecture

```mermaid
flowchart LR
A[HTML 用例] --> B[C++ 多帧渲染导出]
B --> C[frame_000..frame_n 图片集]
A --> D[Python 基线渲染]
D --> E[base 图片]
C --> F[vl_tool_multi.py 合并与差异]
E --> F
F --> G[合并大图]
F --> H[逐帧 diff 图/统计]
G --> I[人工/MLLM 分析定位]
H --> I
I --> J[修复用例/渲染问题并回归]
```

## Module Division

- **Multi-Frame Export (C++)**
- Responsibility：渲染 n 帧、逐帧 dump 贴图/图片、统一命名与元信息输出
- Interfaces：CLI 参数（输入 html、帧数 n、输出目录、可选固定随机种子/时间步进）
- **Baseline Render (Python)**
- Responsibility：不用 dong 的常规渲染路径生成 base；与用例路径/尺寸对齐；稳定可复现
- Interfaces：输入 html、输出 base 路径、视口/缩放参数
- **Visual Compare Tool (Python)**
- Responsibility：读取 base + 多帧；生成合并大图与逐帧差分结果；输出摘要（最异常帧、差异阈值过滤）
- Interfaces：`vl_tool_multi.py --html ... --base ... --frames_dir ... --n ... --out ...`
- **Debug Docs**
- Responsibility：一页式流程 + 常见问题 + 输出产物说明 + 回归检查清单

## Implementation Details

### Relevant Directory Changes

```text
project-root/
├── examples/
│   └── html_render_test.cpp              # 修改：支持渲染 n 帧并逐帧导出
├── scripts/
│   └── tools/
│       ├── html_baseline_render.py       # 新增：Python 基线渲染产出 base
│       └── vl_tool_multi.py              # 新增：多帧合并 + 逐帧差异输出
└── doc/
    └── debug/
        └── multi-frame-render-debug.md   # 新增：工具链文档与排查流程
```

### Key Interfaces (suggested)

- C++ CLI（示例）
- `--html <path>` `--frames <n>` `--out <dir>` `--width <w>` `--height <h>` `--seed <int>`
- Python
- `html_baseline_render.py --html ... --out ... --width ... --height ...`
- `vl_tool_multi.py --html ... --base ... --frames_dir ... --n ... --out ... --tile_cols ... --diff_threshold ...`

### Testing Strategy

- 选取 3-5 个已知稳定用例：多次运行应产生一致的 base 与近零 diff
- 选取 1-2 个疑似闪烁用例：多帧导出应能稳定复现“异常帧 index”
- 批量跑 `examples/data/tests`：输出汇总报告（失败用例列表、最大差异帧与截图路径）

## Agent Extensions

- **subagent:code-explorer**
- Purpose: 全仓扫描定位相关渲染入口、现有导出逻辑、脚本目录约定与用例组织方式
- Expected outcome: 输出可改造点清单（文件/函数/参数链路）与最小改动路径，避免破坏现有结构