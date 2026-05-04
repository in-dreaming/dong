---
name: integrate-slug-font-renderer
overview: 更新 `task.md` 对应的 Slug 接入方案：明确 Slug 与现有 MSDF 两套文本方案需要长期共存且可选，并允许围绕文本渲染链路做合理抽象重构，但实现必须基于自研封装与算法，不能直接引入 `tmp/` 中参考代码。
todos:
  - id: clarify-renderer-boundary
    content: 使用[skill:brainstorming]明确Auto/MSDF/Slug模式与整run回退边界
    status: completed
  - id: add-renderer-selection-api
    content: 修改dong.h、api_bindings.cpp、engine_view.cpp接入文本渲染模式配置
    status: completed
    dependencies:
      - clarify-renderer-boundary
  - id: refactor-text-command-abstraction
    content: 修改display_list与painter_text，补齐统一文本后端选择元数据
    status: completed
    dependencies:
      - add-renderer-selection-api
  - id: implement-slug-runtime
    content: 新增src/render/slug实现轮廓提取、curve编码、band构建与缓存
    status: completed
    dependencies:
      - refactor-text-command-abstraction
  - id: wire-dual-sdl-renderers
    content: 重构SDL文本执行为MSDF与Slug双实现并接入独立shader
    status: completed
    dependencies:
      - implement-slug-runtime
  - id: verify-dual-renderers
    content: 使用[skill:test-driven-development][skill:systematic-debugging][skill:baseline-render-diff-analysis][skill:verification-before-completion][skill:requesting-code-review]完成用例、构建与验收
    status: completed
    dependencies:
      - wire-dual-sdl-renderers
---

## User Requirements

- 在 Dong 中让现有 MSDF 文本渲染与新 Slug 文本渲染长期共存，并且可显式选择，不是单向替换。
- 允许围绕文本渲染链路做结构重构与抽象，让选择逻辑、资源准备和执行路径更清晰。
- 不允许直接引入、拷贝或依赖 `tmp/slug/**` 中的源码实现，只能参考资料，自行实现对应工具、算法和运行时封装。
- 结果需要明确改动链路、选择方式、回退边界、验证方法与主要风险。

## Product Overview

- 现有文本功能、布局和绘制能力保持可用，新增后同一引擎可在不同场景选择 MSDF 或 Slug 文本方案。
- 视觉上，MSDF 保持当前表现；Slug 重点提升小字号、高倍缩放、旋转与变换场景下的文字边缘稳定性与平滑度。

## Core Features

- 提供 `Auto / MSDF / Slug` 可选文本渲染模式。
- 复用现有字体解析、shaping、布局和 DisplayList 输出，只在文本渲染后段增加可切换实现。
- 新增自研 Slug 字体资源、曲线数据和 band 数据生成链路。
- 当 Slug 字体、字形或算法不适用时，安全回退到 MSDF，保证功能连续性。

## Tech Stack Selection

- 继续复用现有项目栈：C++20、Zig、CMake、SDL3 GPU、HLSL、FreeType、HarfBuzz、msdfgen。
- 已验证的现有主链路保持不变：`src/render/text_shaper.cpp` 负责 shaping，`src/render/painter/painter_text.cpp` 产出 `DrawGlyphRunData`，`backends/sdl/sdl_gpu_driver_execute.cpp` 执行当前 MSDF 路径。
- `tmp/slug/**` 仅作为参考资料来源，不作为编译输入、源码依赖或运行时资源来源。

## Implementation Approach

### 总体策略

采用“共享前半段、分叉后半段”的方案：保留当前 `font_resolver + TextShaper + Painter + DrawGlyphRunData` 链路，在文本执行阶段引入统一的“文本渲染后端选择层”，让 MSDF 与 Slug 都挂在同一选择面下运行。

### 关键技术决策

1. **显式选择优先，失败回退次之**  
不再只做“Slug 失败再退回 MSDF”，而是先引入明确的渲染模式：

- `Auto`：按默认策略优先选择 Slug 或 MSDF；
- `MSDF`：强制使用当前路径；
- `Slug`：优先使用 Slug，若整段文本不满足条件则整段回退 MSDF。  
这样“共存且可选”是架构能力，而不是异常处理副作用。

2. **避免 ABI 破坏，优先新增 setter**
`dong/include/dong.h` 当前 `dong_engine_desc_t` 已稳定使用。为了减少创建路径的兼容风险，优先新增文本渲染模式枚举和 setter API，而不是直接扩展创建参数结构体。

3. **自研 Slug 运行时，而非引入参考源码**
基于 FreeType 轮廓数据自行实现：

- 轮廓提取；
- 曲线编码；
- band 数据构建；
- 纹理上传与缓存；
- shader 采样与覆盖率计算。  
参考 `tmp/slug/Slug-main/*.hlsl` 的公开思路，但实现必须在项目内自研。

4. **先支持稳定轮廓，再扩展复杂字体**
首阶段优先保证 TrueType 二次曲线路径稳定；对复杂 CFF/OTF 三次曲线字体，采用“确定性转二次曲线”或整段回退 MSDF，避免为了全覆盖把首版系统做得过重。

### Performance and Reliability

- shaping 复杂度保持 `O(n glyphs)`，不改变现有瓶颈。
- Slug 首次字形准备主要成本在轮廓遍历、曲线编码和 band 构建，单字形约为 `O(curves + bands)`。
- 运行时查找应基于 `font_path + glyph_id` 缓存，命中期望为平均 `O(1)`。
- 资源准备必须放在预热阶段完成，避免在 render pass 激活后临时创建纹理或上传数据。
- 为保证渲染一致性，Slug 不支持的情况优先采用“整 run 回退”，避免同一文本段混用两种算法导致视觉不一致。

### Avoiding Technical Debt

- 不把 Slug 逻辑扩散到 DOM、CSS、Layout。
- 不直接在 `executeDrawText()` 中继续堆叠分支，而是把 MSDF 与 Slug 执行拆成独立小模块。
- 控制函数长度，采用小函数分层，符合仓库“不写长函数”的约束。

## Implementation Notes

- 复用 `DrawGlyphRunData` 作为两种方案的共同输入，只新增“首选渲染模式”和“解析后的实际渲染模式”元数据。
- `backends/sdl/sdl_gpu_driver_resources.cpp` 已有资源预热节奏，Slug 资源准备应并入这里，避免生命周期与同步问题。
- `backends/sdl/shaders/text_*.hlsl` 保持 MSDF 专用；Slug 使用独立 shader 与 pipeline。
- 若新增公共 API，优先新增枚举和 setter，避免直接扩大 `dong_engine_desc_t`。
- 若 Slug 初始化失败，默认不影响现有 MSDF 功能。
- 日志应复用现有日志体系，只打印模式解析、资源缺失、回退原因等可行动信息，避免逐字形刷屏。

## Architecture Design

### 核心结构

- **配置层**：公开 `Auto / MSDF / Slug` 模式，挂到 engine/view 级别。
- **选择层**：根据用户选择、字体支持情况和运行时能力，为每个 `DrawGlyphRunData` 解析出实际渲染器。
- **MSDF 实现层**：保留当前 atlas 与 shader 路径。
- **Slug 实现层**：新增轮廓提取、曲线编码、band 构建、纹理缓存与专用 shader。
- **SDL 执行层**：统一走一个文本入口，但内部委托到两个实现模块。

### 组件关系

- `src/render/text_shaper.cpp`：继续输出统一字形数据。
- `src/render/display_list.hpp`：承载文本渲染模式元数据。
- `src/render/text_renderer_selector.*`：决定每个 glyph run 走哪种实现。
- `src/render/slug/*`：负责 Slug 数据生产和缓存。
- `backends/sdl/sdl_text_renderer_msdf.cpp`：承接现有 MSDF 执行逻辑。
- `backends/sdl/sdl_text_renderer_slug.cpp`：执行 Slug 专用绘制。
- `backends/sdl/sdl_gpu_driver_*`：负责初始化、预热、调度与状态管理。

## Directory Structure

### Directory Structure Summary

本次方案以“前段共享、后段可切换”为主线，核心改动集中在公共配置、DisplayList 元数据、SDL 文本执行重构和新增 Slug 运行时。

- `d:/mix/agents/game/indr/dong/dong/include/dong.h` `[MODIFY]`  
公共 C API 入口。新增文本渲染模式枚举与 setter 声明，保持现有创建接口兼容。

- `d:/mix/agents/game/indr/dong/dong/src/api_bindings.cpp` `[MODIFY]`  
实现新的文本渲染模式设置接口，把外部配置传递到内部 engine/view。

- `d:/mix/agents/game/indr/dong/dong/src/core/engine_view.cpp` `[MODIFY]`  
保存当前 view 的文本渲染模式默认值，并把模式传给 painter 或命令生成阶段。

- `d:/mix/agents/game/indr/dong/dong/src/render/text_renderer_mode.hpp` `[NEW]`  
定义 `Auto / MSDF / Slug` 枚举、字符串转换与默认策略，供 core/render/backend 共用。

- `d:/mix/agents/game/indr/dong/dong/src/render/text_renderer_selector.hpp` `[NEW]`  
声明文本渲染后端选择器接口，负责从“用户期望模式”解析到“实际执行模式”。

- `d:/mix/agents/game/indr/dong/dong/src/render/text_renderer_selector.cpp` `[NEW]`  
实现选择策略、支持性检测结果、整 run 回退规则与原因分类。

- `d:/mix/agents/game/indr/dong/dong/src/render/display_list.hpp` `[MODIFY]`  
为 `DrawGlyphRunData` 增加文本渲染模式元数据，保持现有字形结构与布局信息不变。

- `d:/mix/agents/game/indr/dong/dong/src/render/painter.hpp` `[MODIFY]`  
增加文本模式输入或读取接口，使文本命令生成时携带渲染偏好。

- `d:/mix/agents/game/indr/dong/dong/src/render/painter/painter_text.cpp` `[MODIFY]`  
在产出 `DrawGlyphRunData` 时附带文本渲染偏好，不改动 shaping 与排版逻辑。

- `d:/mix/agents/game/indr/dong/dong/src/render/slug/slug_types.hpp` `[NEW]`  
定义 Slug 字体、字形、曲线纹理、band 纹理、缓存键与运行时结果类型。

- `d:/mix/agents/game/indr/dong/dong/src/render/slug/slug_outline_loader.hpp` `[NEW]`  
声明基于 FreeType 的轮廓提取接口，负责把字体轮廓转成内部统一曲线表示。

- `d:/mix/agents/game/indr/dong/dong/src/render/slug/slug_outline_loader.cpp` `[NEW]`  
实现轮廓读取、二次曲线归一化和必要的三次转二次预处理。

- `d:/mix/agents/game/indr/dong/dong/src/render/slug/slug_curve_encoder.hpp` `[NEW]`  
声明曲线数据编码接口，把内部曲线结构编码为 shader 可采样的数据布局。

- `d:/mix/agents/game/indr/dong/dong/src/render/slug/slug_curve_encoder.cpp` `[NEW]`  
实现曲线纹理数据布局、边界盒计算、采样空间映射与编码结果生成。

- `d:/mix/agents/game/indr/dong/dong/src/render/slug/slug_band_builder.hpp` `[NEW]`  
声明 band 数据构建接口，用于生成横向/纵向 band 索引。

- `d:/mix/agents/game/indr/dong/dong/src/render/slug/slug_band_builder.cpp` `[NEW]`  
实现 band 划分、索引写入、排序与 shader 所需纹理布局。

- `d:/mix/agents/game/indr/dong/dong/src/render/slug/slug_font_cache.hpp` `[NEW]`  
声明 Slug 字体与字形缓存接口，按 `font_path + glyph_id` 查询或构建资源。

- `d:/mix/agents/game/indr/dong/dong/src/render/slug/slug_font_cache.cpp` `[NEW]`  
实现 Slug 资源缓存、批量预热、纹理上传与失败回退所需状态管理。

- `d:/mix/agents/game/indr/dong/dong/backends/sdl/sdl_gpu_driver.hpp` `[MODIFY]`  
新增文本渲染器调度成员、Slug shader/pipeline 成员与公共辅助方法声明。

- `d:/mix/agents/game/indr/dong/dong/backends/sdl/sdl_gpu_driver_init.cpp` `[MODIFY]`  
初始化 Slug shader、pipeline、采样器和缓存对象，同时保留现有 MSDF 初始化。

- `d:/mix/agents/game/indr/dong/dong/backends/sdl/sdl_gpu_driver_resources.cpp` `[MODIFY]`  
在现有资源预热阶段加入 Slug 字体/字形去重与批量准备。

- `d:/mix/agents/game/indr/dong/dong/backends/sdl/sdl_gpu_driver_execute.cpp` `[MODIFY]`  
保留统一 `DrawText` 入口，改为调用解析后的具体文本渲染器，不再堆叠大分支。

- `d:/mix/agents/game/indr/dong/dong/backends/sdl/sdl_text_renderer_shared.hpp` `[NEW]`  
抽出 MSDF 与 Slug 共用的批处理、uniform 写入、clip 与变换辅助逻辑。

- `d:/mix/agents/game/indr/dong/dong/backends/sdl/sdl_text_renderer_msdf.cpp` `[NEW]`  
从当前 `executeDrawText()` 中拆出 MSDF 执行逻辑，保持现有行为并缩小函数规模。

- `d:/mix/agents/game/indr/dong/dong/backends/sdl/sdl_text_renderer_slug.cpp` `[NEW]`  
实现 Slug 专用绘制、纹理绑定、批处理和失败回退接入。

- `d:/mix/agents/game/indr/dong/dong/backends/sdl/shaders/slug_text_vs.hlsl` `[NEW]`  
Slug 顶点着色器，自研适配 SDL_GPU 的输入和 uniform 约定。

- `d:/mix/agents/game/indr/dong/dong/backends/sdl/shaders/slug_text_fs.hlsl` `[NEW]`  
Slug 片元着色器，自研覆盖率计算与颜色输出逻辑。

- `d:/mix/agents/game/indr/dong/dong/build.zig` `[MODIFY]`  
纳入新增 Slug 自研模块与 shader 编译输入，保持现有依赖编排稳定。

- `d:/mix/agents/game/indr/dong/dong/CMakeLists.txt` `[MODIFY]`  
把新增源码与 shader 相关构建规则接入现有目标。

- `d:/mix/agents/game/indr/dong/dong/examples/data/tests/test_text_renderer_slug_scale.html` `[NEW]`  
验证小字号、大字号与缩放场景下 Slug 和 MSDF 的可选性与结果稳定性。

- `d:/mix/agents/game/indr/dong/dong/examples/data/tests/test_text_renderer_slug_transform.html` `[NEW]`  
验证旋转、缩放、裁剪场景下 Slug 路径与回退逻辑的可用性。

## Key Code Structures

```cpp
enum class TextRendererMode : uint8_t {
    Auto,
    Msdf,
    Slug
};

struct TextRendererSelection {
    TextRendererMode requested;
    TextRendererMode resolved;
    bool fallback_used;
    const char* reason;
};
```

```cpp
class TextRendererSelector {
public:
    TextRendererSelection resolve(const DrawGlyphRunData& run) const;
};
```

## Agent Extensions

### Skill

- **brainstorming**
- Purpose: 收敛“共存、可选、可回退”的抽象边界与首阶段支持范围。
- Expected outcome: 明确配置入口、回退粒度和模块拆分原则。

- **test-driven-development**
- Purpose: 先补回归用例，再围绕选择层、Slug 资源层和 SDL 执行层推进实现。
- Expected outcome: 每个阶段都有稳定验收点，减少重构回归。

- **systematic-debugging**
- Purpose: 排查 Slug 路径中的轮廓编码、纹理上传、shader 计算和资源同步问题。
- Expected outcome: 用证据区分算法错误、缓存错误和 GPU 生命周期问题。

- **baseline-render-diff-analysis**
- Purpose: 对比浏览器基线与 Dong 渲染图，分析 Slug 与 MSDF 的具体视觉差异。
- Expected outcome: 产出区域化问题清单，指导修正优先级。

- **verification-before-completion**
- Purpose: 在结束前统一执行构建、单例渲染和基线对比验证。
- Expected outcome: 只有在构建和渲染回归通过后才宣称完成。

- **requesting-code-review**
- Purpose: 在核心抽象层与双实现接通后做一次代码审查。
- Expected outcome: 提前发现接口泄漏、职责混乱和回退边界问题。