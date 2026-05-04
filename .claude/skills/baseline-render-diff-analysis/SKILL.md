---
name: baseline-render-diff-analysis
description: Use when comparing a side-by-side render image (HTML baseline on left, dong render on right) with its source HTML to identify rendering anomalies and explain likely engine-side causes.
---

# Baseline Render Diff Analysis

## Overview

用于分析 `run_baseline_compare.py` 产物中的单张对比图（左基线、右 dong）并结合 HTML 语义，输出可落地的差异诊断。

核心原则：
1. 先做客观“视觉差异定位”，再做“可能根因推断”；
2. 所有结论必须绑定证据区域；
3. 区分“规范允许差异”和“明显渲染异常”。

## When to Use

- 输入是并排图：左侧浏览器基线，右侧 dong 渲染；
- 同时提供对应 HTML 文件；
- 需要说明“哪里不一致、严重程度、可能是哪类引擎问题”。

## Required Inputs

- `comparison_image`：单张并排图（左 baseline / 右 dong）
- `html_file`：该图对应的 HTML

## Analysis Workflow

1. **建立语义锚点**：从 HTML 提取关键结构（标题、按钮、输入框、文本块、边框/背景块、图标等）。
2. **分区比对**：按“从上到下、从左到右”扫描图像，定位元素级差异。
3. **差异归类**（至少覆盖）：
   - 几何类：位置偏移、尺寸错误、裁切/溢出；
   - 样式类：颜色、圆角、边框、阴影、透明度；
   - 文本类：字体、字重、字距、换行、基线对齐；
   - 资源类：图片缺失、渐变异常、背景重复；
   - 时序类：疑似状态帧错误、局部未更新。
4. **严重度评估**：`critical` / `major` / `minor`。
5. **根因方向**：给出 1~3 个最可能的引擎侧方向（layout、style cascade、text shaping、paint order、clip/blend、resource lifetime、GPU sync）。

## Output Format

按以下结构输出：

1. `总体结论`：一句话总结当前 dong 渲染质量。  
2. `差异清单`：逐条列出
   - `区域`（如“右侧按钮区”）
   - `现象`（看到了什么）
   - `期望`（基线应是什么）
   - `严重度`
   - `可能根因`
3. `优先修复建议`：按优先级给出 3 条以内建议（每条对应一个可验证方向）。

## Guardrails

- 不要把“抗锯齿细微差异”误判为重大问题；
- 不要在无证据时给出唯一根因；
- 若图片分辨率、缩放比例不一致，先声明“可能存在比较偏差”。
