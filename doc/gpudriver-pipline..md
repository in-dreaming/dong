# Dong GPU Driver Pipeline vNext 设计草案

状态：草案（面向实现），不考虑历史兼容性

目标：为 Dong 引擎设计一条专用 UI 的 GPU 渲染管线，整体思路参考 Ultralight：
- 只针对 HTML/CSS UI 图元（rect / rounded-rect / text / image）
- 所有几何降维为 quad，CPU 不做像素级栅格化
- 有清晰的多层 IR：DisplayList → LayerTree → GPUCommandList
- 强化 layer 缓存与 atlas，最大化批处理与最小 state change

---

## 1. 总体架构

自上而下的数据流（一次 frame）：

1) DOM/Layout 层
- 已有：Lexbor + Yoga + Dong layout 引擎
- 输出：每个节点的布局结果（x, y, width, height，样式属性）

2) DisplayList 生成
- 新增：`render::DisplayListBuilder`
- 针对布局树遍历，生成 UI 原语级 DisplayList：
  - DrawRect / DrawRoundedRect / DrawBorder
  - DrawGlyphRun (文本行)
  - DrawImage
  - ClipRect / ClipRRect
  - SaveLayer / RestoreLayer

3) LayerTree & 缓存
- 新增：`render::LayerTree`
- 将 DisplayList 投影到合成层（Layer）
  - SurfaceLayer（滚动区域、复杂 subtree）
  - OpacityLayer
  - TransformLayer
  - ClipLayer
  - TextLayer
- 每个 Layer 对应一个 GPU offscreen 纹理（render target），内容只在 dirty 时重 raster

4) GPU IR 编译：DisplayList/LayerTree → GPUCommandList
- 新增：`render::GPUCompiler`
- 职责：
  - 遍历 LayerTree
  - 通过 atlas（glyph atlas / image atlas）为文本和图片分配纹理空间
  - 根据 pipeline / 纹理 / blend 状态构建 batch
  - 输出高层 GPUCommandList：
    - BeginPass(target)
    - SetPipeline
    - BindTexture/Atlas
    - DrawInstancedQuad
    - CompositeLayer

5) GPU 后端执行
- 新增抽象：`render::GPUDriver`（Dong 内部接口）
  - 目前基于 SDL_gpu 实现一个后端（后续可扩展 OpenGL/Metal）
- 输入：GPUCommandList
- 输出：实际 SDL_gpu 调用（创建/绑定纹理、render pass、draw call、present）

最终，`View::update()` 的 GPU 路径将变为：

- JS/DOM → layout → DisplayList → LayerTree → GPUCommandList → GPUDriver(基于 SDL_gpu) → 窗口 swapchain

---

## 2. IR 设计概览

### 2.1 DisplayList（UI 原语级指令）

特点：高层语义，直接来自 layout + style，面向 UI，而不是面向 GPU。

候选指令集合：
- `DrawRect { rect, color }`
- `DrawRoundedRect { rect, color, radius }`
- `DrawBorder { rect, border_widths, border_colors, radius }`
- `DrawGlyphRun { glyph_buffer, font, color }`
- `DrawImage { rect, image_handle, opacity }`
- `ClipRect { rect }`
- `ClipRRect { rect, radius }`
- `SaveLayer { opacity, blend_mode }`
- `RestoreLayer`

这些指令在 C++ 中会以轻量 POD 结构表达，内部不暴露具体 GPU 资源。

### 2.2 LayerTree（合成层）

目标：
- 将 DisplayList 分配到若干可复用的 GPU 纹理层
- 只在 dirty 的 layer 上重新跑 DisplayList → GPU raster
- 滚动/透明度/transform 通过变换层和合成实现，无需重绘

典型 Layer 类型：
- SurfaceLayer：滚动容器或大型区域
- OpacityLayer：实现不透明/半透明子树（包含多个 DisplayItem）
- TransformLayer：CSS transform
- ClipLayer：剪裁区域
- TextLayer：专门为大段文本做 batch

每个 Layer 至少包含：
- `Rect bounds`（逻辑坐标）
- `GPUTextureHandle render_target`（可空：直接渲染到父 layer / swapchain）
- `bool dirty`（内容是否需要重 raster）
- `children`（子 layer 列表）

### 2.3 GPUCommandList（面向 GPU 的 IR）

特点：
- 与具体后端无关
- 每个命令基本对应 1 个 GPU pipeline 或一个 batch draw

示意命令：
- `BeginFrame()` / `EndFrame()`
- `BeginPass(target)` / `EndPass()`
- `SetViewport(rect)`
- `SetPipeline(pipeline_id)`
- `BindTexture(slot, texture_id)`
- `BindUniforms(block_id, data)`
- `DrawInstancedQuad(instance_buffer, count)`
- `CompositeLayer(src_layer, dst_target, transform, opacity)`

这些命令最后由 `GPUDriver` 实现转译为 SDL_gpu 调用。

---

## 3. GPU primitive & shader 策略

### 3.1 几何统一为 Quad

所有绘制 primitive 最终都映射为一批 instanced quad：
- rect / rounded-rect / border / shadow → quad + SDF shader
- glyph run → 每个 glyph 一个 quad，纹理坐标指向 glyph atlas
- image → quad + 采样 image atlas 或独立纹理

CPU 生成的数据：
- 每个实例的：位置、尺寸、颜色、圆角半径、边框参数、glyph UV 等
- 统一写入 instance buffer，由 GPU 通过 instancing 批量绘制。

### 3.2 SDF / MSDF shader

- RoundedRect / Border / Shadow：
  - fragment shader 中使用 analytic SDF：
    - 输入：localPos、corner_radius、border_width 等
    - 输出：alpha，通过 smoothstep 做抗锯齿
- Text：
  - 预生成 MSDF glyph atlas（可用外部工具或运行时生成）
  - fragment shader 解码 MSDF，得到高质量 glyph 边缘
- Image：
  - 普通采样 + 可选的 color transform（例如 gamma correction、tint）

---

## 4. Layer & caching 策略

原则：
- 能提升为 layer 的 subtree 尽量提升
- 滚动/transform/opacity 通过 layer 合成实现，不重复 raster

策略要点：
- DOM/layout 阶段：根据节点类型和样式，决定是否创建新 Layer
  - 例如：`overflow: scroll` → SurfaceLayer
  - `opacity < 1` → OpacityLayer
  - `transform` → TransformLayer
- 每个 layer 维护 dirty 标记：
  - DOM 结构/样式变化 → 标记相关 layer dirty
  - 滚动/transform/opacity 变化 → 仅更新 LayerTree 中的 transform/opacity，不标 dirty
- 渲染阶段：
  - 对 dirty layer：
    - 创建/复用 offscreen 纹理
    - 执行对应 DisplayList → GPUCommandList → raster
  - 对非 dirty layer：
    - 直接在最终合成 pass 中 composite 其缓存纹理

---

## 5. GPU 后端与 SDL_gpu 的关系

短期：
- 继续基于 SDL_gpu 设备与 swapchain，实现一个 `GPUDriver_SDLGPU`：
  - 负责：命令缓冲 acquire/submit、swapchain 纹理获取、render pass 创建、纹理/采样器/缓冲的 SDL_* 调用

`GPUDriver` 抽象示意：
- `beginFrame()` / `endFrame()`
- `createTexture()` / `destroyTexture()`
- `createSampler()` / `destroySampler()`
- `createPipeline()` / `destroyPipeline()`
- `execute(const GPUCommandList&)`

长期：
- 可增加 OpenGL/Metal/D3D/Vulkan 后端，GPUCommandList 不变，只更换底层 driver。

---

## 6. 与现有 Dong 代码的对接策略（高层）

目标：逐步替换当前“Skia CPU 渲染 → RGBA buffer → 上传到单张纹理 → 全屏三角形”的方案。

阶段性步骤：

1) 引入 DisplayList 层
- 在 `render::Painter` 内部将“直接 Skia 绘制”拆为：
  - 先生成 DisplayList（不直接调用 Skia）
  - 短期可以有一个 SkiaBackendCompiler：DisplayList → Skia 调用（保持 CPU 路径兼容）

2) 引入基础 GPUCompiler + GPUDriver 接口
- 实现一条最小可用路径：
  - DisplayList → 单 layer → 简化版 GPUCommandList → GPUDriver_SDLGPU
  - 支持 rect + image 渲染

3) 逐步用 GPU rect/text/image 替换 Skia 对应逻辑
- 文本先走 Skia 渲染，GPU 只做 rect/image
- 之后接入 MSDF atlas 和 GPU text pipeline

4) 在 DOM/Layout 中引入 LayerTree 构建与 caching 策略

---

## 7. TODO 列表（实现路线）

优先级从上到下（P0 > P1 > P2）：

### P0：建立最小可运行的新 GPU 管线
- [ ] 定义 C++ 级 DisplayList IR（Rect / RoundedRect / Image / GlyphRun 基本结构）
- [ ] 在 `render::Painter` 中增加 DisplayListBuilder，保证 CPU 路径可同时从 DisplayList 渲染
- [ ] 设计并实现 `render::GPUCommandList` 和 `GPUCompiler` 的基本骨架
- [ ] 基于 SDL_gpu 实现 `GPUDriver` 抽象（命令缓冲、纹理/采样器/pipeline 管理）
- [ ] 用 GPU rect + image 渲染替换当前“全屏纹理采样”方案（仍允许文本走 Skia）

### P1：引入 LayerTree 与缓存
- [ ] 设计 Layer 类型和数据结构（Surface/Opacity/Transform/Clip/Text）
- [ ] 在 DOM/layout 阶段添加 LayerTree 构建逻辑
- [ ] 为 LayerTree 增加 dirty 标记策略（结构、样式、滚动、transform、opacity）
- [ ] 在 GPUCompiler 中支持对 LayerTree 的遍历与 per-layer raster/composite

### P1：atlas 与批处理
- [ ] 设计 glyph atlas 与 image atlas 数据结构与管理器
- [ ] 在 GPUCompiler 中将 glyph/image 映射到 atlas，并为 DrawInstancedQuad 准备 instance buffer
- [ ] 实现按 pipeline / 纹理 / blend key 的批处理排序与合并

### P2：文本与 SDF 完整 GPU 化
- [ ] 集成 MSDF 生成工具链（离线或运行时）
- [ ] 实现 MSDF 文本 shader pipeline（包含 gamma/hinting 调整）
- [ ] 将 DOM 文本布局结果映射为 glyph run + atlas entry
- [ ] 在 GPUCompiler 中用 instanced quad 渲染文本

### P2：多后端 & 高级特性
- [ ] 在 `GPUDriver` 抽象下增加 OpenGL/Metal 后端实现的接口定义（不必立即实现）
- [ ] 预留多 RT / MSAA / 高精度合成等扩展点
- [ ] 为后续 dirty rect / partial repaint 预留接口（根据 Layout/DOM 的脏区域限制 raster 范围）

---

后续如果你需要，我可以基于本规范直接给出：
- DisplayList / LayerTree / GPUCommandList 的 C++ 结构草案
- 第一版 GPUCompiler + GPUDriver_SDLGPU 的最小实现骨架
- MSDF/SDF shader 的具体 HLSL/GLSL 示例
