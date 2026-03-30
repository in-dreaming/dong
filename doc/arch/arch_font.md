# 字体渲染方案 (Font Rendering Architecture)

## 1. 整体架构

Dong 引擎的字体渲染采用**双渲染器并行架构**，支持两种不同的文字渲染后端：

```
HTML/CSS 文本
     │
     ▼
Text Shaping (HarfBuzz + FreeType)
     │
     ▼
Glyph Run（字形序列）
     │
     ▼
┌────────────────────────────────────┐
│     TextRendererSelector          │
│  (选择 MSDF 或 Slug)              │
└────────────────────────────────────┘
     │
     ├──▶ MSDF Renderer ──▶ GlyphAtlas ──▶ GPU Texture ──▶ MSDF Shader
     │
     └──▶ Slug Renderer ──▶ SlugFontCache ──▶ Curve/Band Textures ──▶ Slug Shader
```

### 渲染器对比

| 特性 | MSDF | Slug |
|------|------|------|
| 原理 | 多通道距离场纹理 | 解析式二次贝塞尔曲线覆盖率 |
| 纹理需求 | 每字形一张 MSDF 图 | 共享 Curve + Band 纹理 |
| 抗锯齿 | 亚像素（纹理插值） | 解析式像素覆盖率 |
| 质量 | 中等（依赖 MSDF 分辨率） | 高（无限分辨率） |
| 初始化开销 | 中（离线生成） | 低（即时计算） |
| 状态 | 稳定 | 需验证 |

## 2. 核心组件

### 2.1 TextRendererSelector

负责在运行时选择具体的渲染器。

**文件**: `src/render/text_renderer_selector.cpp`

```cpp
enum class TextRendererMode : uint8_t {
    Auto,   // 自动选择（默认 Slug，若不可用则 MSDF）
    Msdf,   // 强制 MSDF
    Slug,   // 强制 Slug（不可用时降级到 MSDF）
};

TextRendererSelection resolve(TextRendererMode requested) const;
```

**选择逻辑**:
- `Auto`: Slug 可用时用 Slug，否则用 MSDF
- `Msdf`: 始终用 MSDF
- `Slug`: Slug 可用时用 Slug，否则降级到 MSDF

### 2.2 MSDF 渲染路径

**数据流**:
```
FreeType (设计单位)
    │
    ▼
msdfgen (生成多通道 SDF)
    │
    ▼
GlyphAtlas (GPU Atlas 管理)
    │
    ▼
MSDF Shader (纹理采样 + 距离场解析)
```

**关键类**:
- `GlyphAtlas`: 管理字形位图缓存，支持多页 (128/192/256/384px 四档)
- `GlyphMetrics`: 字形度量信息（EM 空间）
- `AtlasEntry`: Atlas 条目 (UV 坐标 + 度量)

**Shader**: `backends/sdl/shaders/text_fs.hlsl`
- 从 Atlas 采样 SDF
- 计算像素到轮廓的距离
- 输出平滑的 alpha 覆盖

### 2.3 Slug 渲染路径

Slug 是基于二次贝塞尔曲线解析式覆盖率计算的文字渲染算法（MIT License, Eric Lengyel 2017）。

**数据流**:
```
FreeType Outline (EM 空间)
    │
    ▼
loadGlyphOutline() ──▶ QuadraticBezier[]
    │
    ▼
encodeCurves() ──▶ CurveTexel[] (4096×2048)
    │
    ▼
buildBands() ──▶ BandTexel[] (4096×8192)
    │
    ▼
SlugFontCache (CPU 缓存)
    │
    ▼
upload to GPU (R32G32B32A32_FLOAT)
    │
    ▼
Slug Shader (覆盖率计算)
```

**关键类**:
- `SlugFontCache`: 线程安全的 Slug 字形数据缓存
- `SlugVertexBuilder`: 构建 Slug 顶点缓冲 (TRIANGLELIST, 5×float4/vertex)

**Shader**: `backends/sdl/shaders/slug_text_*.hlsl`
- `CurveTexel`: 存储贝塞尔曲线控制点 (p0, p1, p2)
- `BandTexel`: 存储 band 索引（水平/垂直方向的曲线列表）
- `SlugRender()`: 解析式覆盖率计算函数

**GPU 纹理**:
| 纹理 | 格式 | 尺寸 | 用途 |
|------|------|------|------|
| Curve | R32G32B32A32_FLOAT | 4096×2048 | 贝塞尔曲线控制点 |
| Band | R32G32B32A32_FLOAT | 4096×动态 | Band 索引（曲线位置） |

## 3. 渲染调度

### 3.1 DisplayList 构建

`Painter::paintTextAndInput()` 调用 `TextShaper` 生成字形序列，然后通过 `DisplayListBuilder::addGlyphRun()` 添加到 DisplayList。

每条 GlyphRun 携带 `text_renderer_mode` 字段，指定该字形序列的渲染器选择。

### 3.2 GPU 命令编译

`GPUCompiler::compile()` 将 DisplayList 转换为 GPUCommandList，`DrawGlyphRun` → `GPUCommand::DrawText`，携带字形数据和渲染器模式。

### 3.3 GPU 执行

`SDLGPUDriver` 根据 `selection.resolved` 选择调用:
- `executeDrawTextMsdf()`: MSDF 路径
- `executeDrawTextSlug()`: Slug 路径

```cpp
// sdl_gpu_driver_execute.cpp:1574
if (selection.resolved == TextRendererMode::Slug && slug_text_pipeline_) {
    executeDrawTextSlug(ctx, cmd);
} else {
    executeDrawTextMsdf(ctx, cmd);
}
```

## 4. 渲染器初始化

**SDL GPU 后端** (`sdl_gpu_driver_init.cpp`):

```cpp
// MSDF 初始化（始终）
text_pipeline_ = SDL_CreateGPUGraphicsPipeline(...);

// Slug 初始化（尝试）
if (initSlugPipeline()) {
    text_renderer_selector_.setSlugAvailable(true);
}
```

**Slug Pipeline 创建** (`initSlugPipeline`):
1. 编译 `slug_text_vs.hlsl` / `slug_text_fs.hlsl`
2. 创建 `slug_curve_sampler_` (NEAREST)
3. 创建 5 属性顶点缓冲描述 (TRIANGLELIST)
4. 创建 `slug_text_pipeline_`

## 5. 渲染器切换

### 5.1 API 层

```c
// dong.h
dong_engine_set_text_renderer_mode(engine, DONG_TEXT_RENDERER_SLUG);
dong_engine_get_text_renderer_mode(engine);
```

### 5.2 环境变量

| 变量 | 值 | 效果 |
|------|-----|------|
| `DONG_TEXT_RENDERER` | `slug` | 强制 Slug |
| `DONG_TEXT_RENDERER` | `msdf` | 强制 MSDF |
| `DONG_TEXT_RENDERER` | `auto` | 自动选择 |

支持的应用:
- `html_render_test`
- `dong_app`
- `3d_screens_simple`

### 5.3 应用示例

```bash
# Slug 渲染（默认，现在自动选择）
dong_app --html test.html

# 强制 MSDF
set DONG_TEXT_RENDERER=msdf
dong_app --html test.html

# 强制 Slug
set DONG_TEXT_RENDERER=slug
dong_app --html test.html
```

## 6. Slug 坐标系统

### 6.1 坐标系转换

```
FreeType (Y轴向上, EM空间[0,1])
    │
    │ scale = font_size
    ▼
EM空间 × font_size = 屏幕像素
    │
    │ screen_y = pos_y - ft_y * scale
    ▼
屏幕空间 (Y轴向下)
```

### 6.2 Slug 数据准备

**曲线数据** (`slug_outline_loader.cpp`):
- `loadGlyphOutline()`: 将 FT Outline 转换为二次贝塞尔曲线列表
- 坐标归一化到 EM 空间 (除以 `units_per_EM`)

**Band 构建** (`slug_band_builder.cpp`):
- 将字形包围盒划分为 H×V 个 band
- 每个 band 存储与其相交的曲线索引列表
- 用于 O(H+V) 复杂度的覆盖率查询

### 6.3 Vertex 构建

每个字形 quad 生成 4 个顶点 (TRIANGLELIST):

```cpp
// slug_vertex_builder.hpp
SlugVertex {
    pos[4]:   screen_x, screen_y, normal_x, normal_y
    tex[4]:   em_x, em_y, bandLocPacked, bandMaxPacked
    jac[4]:   inverse Jacobian (scale 变换)
    bnd[4]:   bandScale, bandOffset
    col[4]:   RGBA
}
```

## 7. 已知限制

1. **Slug 曲线数量限制**: 单字形曲线数受 Band 容量限制
2. **EM 空间精度**: 极端字号下可能有精度问题
3. **中文/复杂字形**: 需要验证 Slug 对 CJK 扩展区的支持

## 8. 文件索引

| 文件 | 描述 |
|------|------|
| `src/render/text_renderer_selector.cpp` | 渲染器选择逻辑 |
| `src/render/text_renderer_mode.hpp` | 模式枚举定义 |
| `src/render/glyph_atlas.hpp` | MSDF Atlas 管理 |
| `src/render/slug/slug_font_cache.hpp` | Slug 缓存管理 |
| `src/render/slug/slug_outline_loader.cpp` | FT Outline 加载 |
| `src/render/slug/slug_band_builder.cpp` | Band 构建 |
| `src/render/slug/slug_vertex_builder.hpp` | Slug 顶点构建 |
| `backends/sdl/shaders/slug_text_*.hlsl` | Slug GLSL/HLSL Shader |
| `backends/sdl/sdl_gpu_driver_execute.cpp` | 渲染执行调度 |
| `backends/sdl/sdl_gpu_driver_init.cpp` | Pipeline 初始化 |

## 9.prepared的启示
prepared → layout 两阶段缓存架构 和 完备的 Unicode 折行规则
- 浏览器很多实现多有时代局限，比如静态网页，但目前动态内容越来越多，空间换时间收益更大了。
