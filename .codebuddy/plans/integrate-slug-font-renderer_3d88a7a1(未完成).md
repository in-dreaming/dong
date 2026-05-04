---
name: integrate-slug-font-renderer
overview: 研究 `task.md` 中“接入 slug 字体方案”的落地路径，梳理现有文本渲染链路、可改造接入点、构建与 shader 影响范围，并形成可执行实施计划。
todos:
  - id: define-slug-boundary
    content: 使用[skill:brainstorming]明确并行接入、受控字体与MSDF回退边界
    status: pending
  - id: build-slug-runtime
    content: 修改dong/build.zig与dong/CMakeLists.txt，接入Slug运行时与构建
    status: pending
    dependencies:
      - define-slug-boundary
  - id: add-slug-resource-layer
    content: 新增dong/src/render/slug/*，实现字体资源、字形缓存与查询
    status: pending
    dependencies:
      - build-slug-runtime
  - id: wire-sdl-slug-pipeline
    content: 修改sdl_gpu_driver与slug_text_*.hlsl，接通Slug绘制管线
    status: pending
    dependencies:
      - add-slug-resource-layer
  - id: add-fallback-and-tests
    content: 使用[skill:test-driven-development]补充HTML用例并实现MSDF安全回退
    status: pending
    dependencies:
      - wire-sdl-slug-pipeline
  - id: debug-and-verify
    content: 使用[skill:systematic-debugging][skill:baseline-render-diff-analysis][skill:verification-before-completion]完成构建与渲染验收
    status: pending
    dependencies:
      - add-fallback-and-tests
---

## User Requirements

- 研究并规划在 Dong 中接入 Slug 字体方案，基于仓库内现有代码与本地参考资料形成可执行落地路径。
- 范围聚焦文本渲染链路，不要求同步改造无关模块。
- 结果需要明确改动链路、验证方式、回退边界与主要风险。

## Product Overview

- 当前引擎已经具备字体解析、字形 shaping、布局、DisplayList 和 GPU 文本绘制能力。
- 新方案应在不破坏现有文本功能的前提下，引入 Slug 矢量字体渲染能力，使小字号、大缩放、变换场景下的文字边缘更稳定、更平滑。

## Core Features

- 复用现有字体选择与 shaping 结果，新增 Slug 字体资源和字形缓存通路。
- 在 SDL 渲染后端增加 Slug 专用绘制路径，并支持批量准备资源。
- 保留现有 MSDF 文本路径作为兼容回退，便于逐步切换和对比验证。

## Tech Stack Selection

- 复用现有项目栈：C++20、Zig 构建、CMake 桌面构建、SDL3 GPU、HLSL、FreeType、HarfBuzz。
- 现有文本主链路已验证可用：`src/render/text_shaper.cpp` 负责 shaping，`src/render/painter/painter_text.cpp` 产出 `DrawGlyphRunData`，`backends/sdl/sdl_gpu_driver_execute.cpp` 走 MSDF 绘制。
- 当前仓库没有已接入的 Slug 运行时代码，`tmp/slug/**` 仅为参考资料来源。

## Implementation Approach

- 最优方案是“并行接入 Slug，保留 MSDF 回退”，而不是直接替换现有文本链路。现有 `ShapedText` 已携带 `font_paths`、`font_path_index`、`glyph_id`、`units_per_em`，足够支撑后端按字形切换不同字体资源，无需先重写布局与 shaping。
- 具体做法是保留 `font_resolver + TextShaper + Painter` 输出，新增一个独立的 Slug 资源层，把 `font_path + glyph_id` 映射为 Slug 字形数据、纹理与绘制元数据；SDL 后端在 `prepareResources()` 中预热缓存，在 `executeDrawText()` 中优先走 Slug，失败时自动回退到现有 MSDF。
- 不建议直接拷贝 `tmp/slug/C4-Engine/**` 源码入库。已检查到 `C4Slug.h` 带专有许可声明，实施时应仅参考公开资料与 `tmp/slug/Slug-main/*.hlsl` 的 MIT Shader 思路，自行实现运行时封装。

## Implementation Notes

- 优先复用 `src/render/text_shaper.cpp` 与 `src/render/display_list.hpp` 的现有数据结构，避免把 Slug 逻辑扩散进 DOM、Layout、Painter。
- `backends/sdl/sdl_gpu_driver_resources.cpp` 已有批量去重与预热模式，Slug 资源准备应沿用该节奏，避免在激活 render pass 后临时建资源。
- 当前 `backends/sdl/shaders/text_vs.hlsl`、`text_fs.hlsl` 是 MSDF 专用，Slug 应新增独立 shader 与 pipeline，不要混改现有 MSDF shader。
- 纯离线 `.slug` 资源方案与当前“运行时按系统字体路径解析字体”的模式存在天然张力。首阶段应允许“受控字体走 Slug、其它字体回退 MSDF”，后续再考虑扩大覆盖面。
- 性能上，shaping 复杂度保持 O(n glyphs)；Slug 资源命中后查找应为 O(1) 平均；真正昂贵的是首次字体/字形准备，因此必须做跨帧缓存、批量预热和按字体聚合提交。
- 需要重点防回归的点：字形回退字体、文字阴影、裁剪、变换矩阵、GPU 资源生命周期与上传同步。

## Architecture Design

- `src/render/text_shaper.cpp`：继续负责字体解析、HarfBuzz shaping、回退字体选择。
- `src/render/slug/*`：新增 Slug 字体资源层，负责字体级资源、字形网格/元数据缓存、纹理装载与查询。
- `backends/sdl/sdl_gpu_driver_resources.cpp`：在绘制前批量准备 Slug 字形资源。
- `backends/sdl/sdl_gpu_driver_execute.cpp`：按能力选择 Slug 或 MSDF 路径，维持现有 `DrawText` 命令接口不变。
- `backends/sdl/shaders/slug_text_*.hlsl`：实现 Slug 覆盖率计算与最终着色。
- 回退策略：任一字形资源缺失、字体未受支持或渲染路径初始化失败时，直接落回当前 MSDF。

## Directory Structure

### Directory Structure Summary

本方案以“最小侵入并行接入”为原则，主要改动集中在构建系统、SDL 文本后端和新增的 Slug 资源层。

- `d:/mix/agents/game/indr/dong/dong/build.zig`  `[MODIFY]`  
构建编排入口。接入 Slug 运行时依赖或自研模块的编译、安装与链接规则，保持现有 FreeType/HarfBuzz/msdfgen 构建不受影响。

- `d:/mix/agents/game/indr/dong/dong/CMakeLists.txt`  `[MODIFY]`  
桌面构建入口。补齐 Slug 相关 include/link 规则，并保证现有 `dong` / `dong_render` 目标继续可编译。

- `d:/mix/agents/game/indr/dong/dong/src/render/slug/slug_types.hpp`  `[NEW]`  
Slug 通用数据结构。定义字体资源、字形资源、纹理句柄和缓存键等共享类型。

- `d:/mix/agents/game/indr/dong/dong/src/render/slug/slug_font_cache.hpp`  `[NEW]`  
Slug 资源层接口。声明字体级缓存、字形准备、查询和释放接口，供 SDL 后端调用。

- `d:/mix/agents/game/indr/dong/dong/src/render/slug/slug_font_cache.cpp`  `[NEW]`  
Slug 资源层实现。负责按 `font_path + glyph_id` 准备数据、做缓存/LRU、组织纹理与字形元数据，并处理回退失败场景。

- `d:/mix/agents/game/indr/dong/dong/backends/sdl/sdl_gpu_driver.hpp`  `[MODIFY]`  
SDL 后端主类声明。新增 Slug shader、pipeline、缓存成员和辅助方法，维持现有 MSDF 成员并存。

- `d:/mix/agents/game/indr/dong/dong/backends/sdl/sdl_gpu_driver.cpp`  `[MODIFY]`  
SDL 后端生命周期管理。释放 Slug pipeline、shader、纹理和缓存资源，避免资源泄漏。

- `d:/mix/agents/game/indr/dong/dong/backends/sdl/sdl_gpu_driver_init.cpp`  `[MODIFY]`  
初始化 GPU 资源。编译 `slug_text_*.hlsl`、创建 Slug 管线，初始化后端级字体资源缓存。

- `d:/mix/agents/game/indr/dong/dong/backends/sdl/sdl_gpu_driver_resources.cpp`  `[MODIFY]`  
资源预热入口。扩展现有批量资源准备逻辑，先去重并准备 Slug 字体/字形资源，再保留 MSDF 兼容路径。

- `d:/mix/agents/game/indr/dong/dong/backends/sdl/sdl_gpu_driver_execute.cpp`  `[MODIFY]`  
文本执行入口。新增 Slug 绘制分支、按字体聚合提交、缺失资源回退 MSDF，并控制状态切换与批处理。

- `d:/mix/agents/game/indr/dong/dong/backends/sdl/shaders/slug_text_vs.hlsl`  `[NEW]`  
Slug 顶点着色器。基于公开 Slug shader 思路适配 SDL_GPU 的 uniform/binding 约定。

- `d:/mix/agents/game/indr/dong/dong/backends/sdl/shaders/slug_text_fs.hlsl`  `[NEW]`  
Slug 片元着色器。实现 band/curve 采样与 coverage 计算，输出最终字形透明度与颜色。

- `d:/mix/agents/game/indr/dong/dong/examples/data/tests/test_slug_text_scaling.html`  `[NEW]`  
回归用例。验证小字号、大字号和高倍缩放下的文字质量与回退稳定性。

- `d:/mix/agents/game/indr/dong/dong/examples/data/tests/test_slug_text_transform.html`  `[NEW]`  
回归用例。验证旋转、缩放等变换场景下的文本一致性与边缘表现。

## Agent Extensions

### Skill

- **brainstorming**  
- Purpose: 在正式编码前收敛 Slug 接入边界，明确“并行接入、受控字体、MSDF 回退”的实现范围。  
- Expected outcome: 得到稳定的首阶段范围，避免一开始就试图替换全部文本系统。

- **test-driven-development**  
- Purpose: 先补充 Slug 专项 HTML 回归用例，再围绕用例接入后端路径。  
- Expected outcome: 新增能力有明确验收标准，且不破坏现有文本行为。

- **systematic-debugging**  
- Purpose: 当 Slug 路径出现字体错位、裁剪异常、闪烁或资源同步问题时，按步骤定位根因。  
- Expected outcome: 以证据驱动方式收敛到缓存、几何、shader 或 GPU 生命周期问题。

- **baseline-render-diff-analysis**  
- Purpose: 对比基线浏览器截图与 Dong 渲染图，分析 Slug 接入后的具体视觉差异。  
- Expected outcome: 输出带证据区域的问题清单，指导修正优先级。

- **verification-before-completion**  
- Purpose: 在宣称接入完成前，统一执行构建、单例渲染和基线对比验证。  
- Expected outcome: 只有在 `zig build` 与渲染回归都通过后才结束任务。