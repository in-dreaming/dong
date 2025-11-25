# Dong GPU 渲染管线完整技术设计

> **状态**: 工程蓝图（V1.0）  
> **目标**: 构建专为 HTML/CSS UI 优化的 GPU-native 渲染管线  
> **参考架构**: Ultralight GPU Driver + Flutter LayerTree  
> **日期**: 2025-11-25

---

## 1. 设计原则

### 1.1 核心理念

**专用化（Specialized）而非通用化**：
- ✅ 只支持 UI 必需的图元：矩形、圆角矩形、文本、图片
- ❌ 不支持任意路径（Path）、SVG 实时栅格化
- ✅ 所有栅格化在 GPU 完成，CPU 不做像素级计算
- ✅ 几何统一为 Quad（四边形），最大化批处理

### 1.2 性能目标

- **State Change 极少**: 通过 Atlas + Batching 将 draw call 降至最低
- **Layer Caching**: 只在 dirty 时重绘，滚动/transform/opacity 通过合成实现
- **SDF/MSDF 字体**: 高质量、分辨率无关、缓存友好
- **Instancing**: 所有相同类型的图元通过 GPU instancing 批量绘制

---

## 2. 架构总览

### 2.1 数据流（单帧渲染）

```
┌─────────────────┐
│   DOM Tree      │ (Lexbor)
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Layout Tree    │ (Yoga + Dong Layout)
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  DisplayList    │ ◄─── Phase 1: 构建 UI 原语指令
└────────┬────────┘      (DrawRect, DrawGlyphRun, etc.)
         │
         ▼
┌─────────────────┐
│  LayerTree      │ ◄─── Phase 2: 分层 + 缓存策略
└────────┬────────┘      (SurfaceLayer, OpacityLayer, etc.)
         │
         ▼
┌─────────────────┐
│ GPU Compiler    │ ◄─── Phase 3: Atlas 分配 + Batching
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│GPUCommandList   │ ◄─── Phase 4: 平台无关 GPU IR
└────────┬────────┘      (SetPipeline, DrawInstanced, etc.)
         │
         ▼
┌─────────────────┐
│  GPU Driver     │ ◄─── Phase 5: 后端执行
└────────┬────────┘      (SDL_gpu / OpenGL / Metal / Vulkan)
         │
         ▼
┌─────────────────┐
│   Swapchain     │ ◄─── 呈现到窗口
└─────────────────┘
```

### 2.2 模块边界

| 模块 | 输入 | 输出 | 职责 |
|------|------|------|------|
| **Painter** | DOM + Layout | DisplayList | 遍历 DOM，生成高层渲染指令 |
| **LayerTree Builder** | DisplayList | LayerTree | 分层、缓存策略、dirty 标记 |
| **GPU Compiler** | LayerTree | GPUCommandList | Atlas 分配、批处理、实例化 |
| **GPU Driver** | GPUCommandList | 平台调用 | 抽象层，封装 SDL_gpu/GL/Metal |

---

## 3. 中间表示（IR）设计

### 3.1 DisplayList（高层语义指令）

**定位**: 直接来自 Layout，面向 UI 语义，与 GPU 无关

```cpp
// dong/src/render/display_list.hpp

namespace dong::render {

enum class DisplayItemType : uint8_t {
    DrawRect,
    DrawRoundedRect,
    DrawBorder,
    DrawGlyphRun,      // 文本（逐 glyph）
    DrawImage,
    ClipRect,
    ClipRRect,
    SaveLayer,
    RestoreLayer,
};

struct DrawRectData {
    Rect bounds;
    Color color;
};

struct DrawRoundedRectData {
    Rect bounds;
    Color color;
    float radius;      // 统一圆角，或改为 float[4]
};

struct DrawBorderData {
    Rect bounds;
    float widths[4];   // top, right, bottom, left
    Color colors[4];
    float radius[4];   // 四个角的圆角
};

// CRITICAL: GlyphRun 不存储字符串，而是布局后的 glyph 序列
struct DrawGlyphRunData {
    std::vector<GlyphInstance> glyphs;  // 每个 glyph 的位置 + ID
    Color color;
    float font_size;
    uint32_t font_id;  // 字体句柄（由 FontManager 管理）
};

struct GlyphInstance {
    uint32_t glyph_id;    // Unicode codepoint 或字形 ID
    float x, y;           // glyph 基线位置
};

struct DrawImageData {
    Rect bounds;
    uint32_t image_id;    // 图片资源句柄
    float opacity;
};

struct ClipRectData {
    Rect bounds;
};

struct SaveLayerData {
    float opacity;
    BlendMode blend_mode;
};

// 统一的 DisplayItem
struct DisplayItem {
    DisplayItemType type;
    union {
        DrawRectData rect;
        DrawRoundedRectData rounded_rect;
        DrawBorderData border;
        DrawGlyphRunData glyph_run;
        DrawImageData image;
        ClipRectData clip;
        SaveLayerData save_layer;
    };
};

struct DisplayList {
    std::vector<DisplayItem> items;
    
    // 可选：脏矩形优化
    Rect dirty_rect;
    bool has_dirty_rect = false;
};

} // namespace dong::render
```

**关键点**：
- `DrawGlyphRun` 存储的是**布局后的 glyph 序列**，不是原始字符串
- 文本 shaping/layout 在生成 DisplayList 前完成（由 HarfBuzz 或类似库处理）

---

### 3.2 LayerTree（合成层）

**定位**: 缓存与合成优化，减少重绘

```cpp
// dong/src/render/layer_tree.hpp

namespace dong::render {

enum class LayerType : uint8_t {
    Surface,      // 滚动容器、复杂子树
    Opacity,      // 透明度分组
    Transform,    // CSS transform
    Clip,         // 裁剪区域
    Text,         // 文本批处理专用层（可选）
};

class Layer {
public:
    LayerType type;
    Rect bounds;              // 逻辑坐标
    bool dirty = true;        // 是否需要重新栅格化
    
    // GPU 资源（offscreen 纹理）
    GPUTextureHandle render_target = nullptr;
    
    // 子层
    std::vector<std::shared_ptr<Layer>> children;
    
    // 该层包含的 DisplayList 片段
    std::vector<DisplayItem> items;
    
    // 变换参数（用于合成阶段）
    float opacity = 1.0f;
    Matrix4x4 transform = Matrix4x4::identity();
};

class LayerTree {
public:
    std::shared_ptr<Layer> root;
    
    // 遍历所有 dirty 的层
    void collectDirtyLayers(std::vector<Layer*>& out_layers);
    
    // 标记策略
    void markDirtyByRect(const Rect& rect);
    void markDirtyByNodeId(uint32_t node_id);
};

} // namespace dong::render
```

**关键点**：
- 每个 Layer 可能对应一个 GPU offscreen 纹理
- 只在 `dirty = true` 时重新栅格化该层
- 滚动/transform/opacity 变化只更新 `Layer` 的参数，不标记 dirty

---

### 3.3 GPUCommandList（底层 GPU IR）

**定位**: 平台无关的 GPU 指令，可翻译为 SDL_gpu/OpenGL/Metal/Vulkan

```cpp
// dong/src/render/gpu_command_list.hpp

namespace dong::render {

enum class GPUCommandType : uint8_t {
    BeginFrame,
    EndFrame,
    BeginPass,           // 开始 render pass
    EndPass,
    SetViewport,
    SetPipeline,         // 绑定 shader pipeline
    BindTexture,
    BindUniformBuffer,
    DrawInstancedQuad,   // 核心：批量绘制 quad
    CompositeLayer,      // 合成一个 layer 到目标 RT
};

struct GPUCommand {
    GPUCommandType type;
    
    union {
        struct {
            GPUTextureHandle target;
            Rect viewport;
        } begin_pass;
        
        struct {
            GPUPipelineHandle pipeline;
        } set_pipeline;
        
        struct {
            uint32_t slot;
            GPUTextureHandle texture;
            GPUSamplerHandle sampler;
        } bind_texture;
        
        struct {
            uint32_t slot;
            GPUBufferHandle buffer;
            size_t offset;
            size_t size;
        } bind_uniform;
        
        struct {
            GPUBufferHandle instance_buffer;  // 实例数据
            uint32_t instance_count;
        } draw_instanced;
        
        struct {
            GPUTextureHandle src_texture;
            GPUTextureHandle dst_target;
            Matrix4x4 transform;
            float opacity;
        } composite_layer;
    };
};

struct GPUCommandList {
    std::vector<GPUCommand> commands;
    
    void clear() { commands.clear(); }
    void addCommand(const GPUCommand& cmd) { commands.push_back(cmd); }
};

} // namespace dong::render
```

---

### 3.4 GPU Primitive Instance Data

**所有图元都通过 instancing 绘制，CPU 准备实例数据**：

```cpp
// dong/src/render/gpu_primitives.hpp

namespace dong::render {

// Rect 实例数据（每个实例 = 一个矩形）
struct RectInstance {
    float x, y, width, height;  // 位置和尺寸
    float r, g, b, a;           // 颜色
};

// RoundedRect 实例数据
struct RoundedRectInstance {
    float x, y, width, height;
    float radius;               // 圆角半径
    float r, g, b, a;
};

// Glyph 实例数据（从 atlas 采样）
struct GlyphInstance {
    float x, y, width, height;  // quad 位置
    float u0, v0, u1, v1;       // atlas UV 坐标
    float r, g, b, a;           // 文本颜色
};

// Image 实例数据
struct ImageInstance {
    float x, y, width, height;
    float u0, v0, u1, v1;       // 纹理 UV
    float opacity;
};

} // namespace dong::render
```

---

## 4. 核心子系统设计

### 4.1 Atlas 管理系统

#### 4.1.1 Glyph Atlas（MSDF 字体）

**目标**: 
- 预生成或运行时生成 MSDF 字形纹理
- 高质量、分辨率无关
- 支持多字体、多大小

```cpp
// dong/src/render/glyph_atlas.hpp

namespace dong::render {

struct GlyphMetrics {
    float advance_x;
    float bearing_x, bearing_y;
    float width, height;
};

struct AtlasEntry {
    uint32_t atlas_page;   // 第几张 atlas 纹理
    float u0, v0, u1, v1;  // UV 坐标
    GlyphMetrics metrics;
};

class GlyphAtlas {
public:
    // 查询 glyph 是否已缓存
    const AtlasEntry* getGlyph(uint32_t font_id, uint32_t glyph_id, float size);
    
    // 若未缓存，生成 MSDF 并上传
    AtlasEntry* addGlyph(uint32_t font_id, uint32_t glyph_id, float size);
    
    // 获取 atlas 纹理（用于绑定到 shader）
    GPUTextureHandle getAtlasTexture(uint32_t page_index);
    
private:
    // MSDF 生成器（封装 msdfgen 库）
    std::unique_ptr<MSDFGenerator> msdf_gen_;
    
    // 纹理装箱算法（MaxRects / Skyline）
    std::unique_ptr<AtlasPacker> packer_;
    
    // 每页 atlas 的 GPU 纹理
    std::vector<GPUTextureHandle> atlas_textures_;
    
    // 缓存：(font_id, glyph_id, size) -> AtlasEntry
    std::unordered_map<uint64_t, AtlasEntry> cache_;
};

} // namespace dong::render
```

**实现建议**：
- 使用 `msdfgen` 或 `msdf-atlas-gen` 库生成 MSDF
- Atlas 分辨率：2048x2048 或 4096x4096
- 支持动态扩展（当前页满时创建新页）

---

#### 4.1.2 Image Atlas

```cpp
// dong/src/render/image_atlas.hpp

namespace dong::render {

class ImageAtlas {
public:
    // 添加图片到 atlas（返回 UV 坐标）
    AtlasEntry addImage(const std::string& image_path);
    
    // 获取已缓存的图片
    const AtlasEntry* getImage(const std::string& image_path);
    
    GPUTextureHandle getAtlasTexture();
    
private:
    std::unique_ptr<AtlasPacker> packer_;
    GPUTextureHandle atlas_texture_;
    std::unordered_map<std::string, AtlasEntry> cache_;
};

} // namespace dong::render
```

---

### 4.2 GPU Compiler（核心编译器）

**职责**：
1. 遍历 LayerTree
2. 为每个 dirty layer 生成 GPUCommandList
3. 进行 Atlas 分配
4. 批处理优化（按 pipeline/texture/blend 排序）

```cpp
// dong/src/render/gpu_compiler.hpp

namespace dong::render {

class GPUCompiler {
public:
    GPUCompiler(GlyphAtlas* glyph_atlas, ImageAtlas* image_atlas, GPUDriver* driver);
    
    // 编译整个 LayerTree，输出 GPUCommandList
    GPUCommandList compile(const LayerTree& layer_tree);
    
private:
    GlyphAtlas* glyph_atlas_;
    ImageAtlas* image_atlas_;
    GPUDriver* driver_;
    
    // 编译单个 layer
    void compileLayer(Layer* layer, GPUCommandList& out_cmds);
    
    // 批处理优化
    struct Batch {
        GPUPipelineHandle pipeline;
        GPUTextureHandle texture;
        std::vector<void*> instances;  // RectInstance, GlyphInstance, etc.
    };
    
    void batchDisplayItems(const std::vector<DisplayItem>& items,
                          std::vector<Batch>& out_batches);
};

} // namespace dong::render
```

**批处理策略**：

```cpp
void GPUCompiler::batchDisplayItems(const std::vector<DisplayItem>& items,
                                   std::vector<Batch>& out_batches) {
    // 1. 为每个 item 生成排序键（pipeline + texture + blend）
    struct SortKey {
        uint64_t pipeline : 16;
        uint64_t texture : 32;
        uint64_t blend : 8;
        uint64_t original_index : 8;
    };
    
    std::vector<SortKey> keys;
    for (size_t i = 0; i < items.size(); ++i) {
        keys.push_back(generateSortKey(items[i], i));
    }
    
    // 2. 排序
    std::sort(keys.begin(), keys.end());
    
    // 3. 合并相邻的相同状态的 item
    Batch current_batch;
    for (const auto& key : keys) {
        const auto& item = items[key.original_index];
        
        if (needsNewBatch(current_batch, item)) {
            if (!current_batch.instances.empty()) {
                out_batches.push_back(std::move(current_batch));
            }
            current_batch = createBatch(item);
        }
        
        addInstanceToBatch(current_batch, item);
    }
    
    if (!current_batch.instances.empty()) {
        out_batches.push_back(std::move(current_batch));
    }
}
```

---

### 4.3 GPU Driver 抽象层

```cpp
// dong/src/render/gpu_driver.hpp

namespace dong::render {

class GPUDriver {
public:
    virtual ~GPUDriver() = default;
    
    // 生命周期
    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;
    
    // 资源管理
    virtual GPUTextureHandle createTexture(uint32_t width, uint32_t height, 
                                          PixelFormat format) = 0;
    virtual void destroyTexture(GPUTextureHandle handle) = 0;
    
    virtual GPUBufferHandle createBuffer(size_t size, BufferUsage usage) = 0;
    virtual void destroyBuffer(GPUBufferHandle handle) = 0;
    virtual void updateBuffer(GPUBufferHandle handle, const void* data, size_t size) = 0;
    
    virtual GPUPipelineHandle createPipeline(const PipelineDesc& desc) = 0;
    virtual void destroyPipeline(GPUPipelineHandle handle) = 0;
    
    // 执行命令列表
    virtual void execute(const GPUCommandList& commands) = 0;
    
    // 获取 swapchain 纹理
    virtual GPUTextureHandle acquireSwapchainTexture() = 0;
    virtual void present() = 0;
};

// 工厂函数
std::unique_ptr<GPUDriver> createSDLGPUDriver(SDL_GPUDevice* device, SDL_Window* window);
std::unique_ptr<GPUDriver> createOpenGLDriver();
std::unique_ptr<GPUDriver> createMetalDriver();

} // namespace dong::render
```

---

### 4.4 Shader Pipeline 设计

#### 4.4.1 Rect Shader

```hlsl
// shaders/rect.vs.hlsl

struct VSInput {
    uint vertexID : SV_VertexID;
    uint instanceID : SV_InstanceID;
};

struct VSOutput {
    float4 position : SV_Position;
    float4 color : COLOR0;
};

struct RectInstance {
    float4 rect;   // x, y, width, height
    float4 color;
};

[[vk::binding(0, 0)]] StructuredBuffer<RectInstance> instances : register(t0);
[[vk::binding(0, 1)]] cbuffer ViewUniforms : register(b0, space1) {
    float4 viewport;  // width, height, unused, unused
};

VSOutput main(VSInput input) {
    RectInstance inst = instances[input.instanceID];
    
    // 生成 quad 顶点
    float2 local = float2(
        input.vertexID == 1 || input.vertexID == 3,
        input.vertexID >= 2
    );
    
    float2 pos = inst.rect.xy + local * inst.rect.zw;
    
    // NDC 变换
    float2 ndc;
    ndc.x = (pos.x / viewport.x) * 2.0 - 1.0;
    ndc.y = 1.0 - (pos.y / viewport.y) * 2.0;
    
    VSOutput output;
    output.position = float4(ndc, 0.0, 1.0);
    output.color = inst.color;
    return output;
}
```

```hlsl
// shaders/rect.ps.hlsl

struct PSInput {
    float4 position : SV_Position;
    float4 color : COLOR0;
};

float4 main(PSInput input) : SV_Target0 {
    return input.color;
}
```

---

#### 4.4.2 Rounded Rect SDF Shader

```hlsl
// shaders/rounded_rect.vs.hlsl

struct RoundedRectInstance {
    float4 rect;
    float radius;
    float4 color;
};

[[vk::binding(0, 0)]] StructuredBuffer<RoundedRectInstance> instances : register(t0);

VSOutput main(VSInput input) {
    RoundedRectInstance inst = instances[input.instanceID];
    
    float2 local = float2(
        input.vertexID == 1 || input.vertexID == 3,
        input.vertexID >= 2
    );
    
    float2 pos = inst.rect.xy + local * inst.rect.zw;
    float2 ndc = worldToNDC(pos);
    
    VSOutput output;
    output.position = float4(ndc, 0.0, 1.0);
    output.local = local;  // [0, 1]
    output.size = inst.rect.zw;
    output.radius = min(inst.radius, min(inst.rect.z, inst.rect.w) * 0.5);
    output.color = inst.color;
    return output;
}
```

```hlsl
// shaders/rounded_rect.ps.hlsl

struct PSInput {
    float4 position : SV_Position;
    float2 local : TEXCOORD0;
    nointerpolation float2 size : TEXCOORD1;
    nointerpolation float radius : TEXCOORD2;
    float4 color : COLOR0;
};

float sdRoundedRect(float2 p, float2 halfSize, float radius) {
    float2 q = abs(p) - (halfSize - radius);
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - radius;
}

float4 main(PSInput input) : SV_Target0 {
    // 转换为中心坐标
    float2 p = (input.local - 0.5) * input.size;
    float2 halfSize = input.size * 0.5;
    
    float dist = sdRoundedRect(p, halfSize, input.radius);
    
    // 抗锯齿（约 1 像素软边）
    float alpha = saturate(0.5 - dist);
    
    float4 result = input.color;
    result.a *= alpha;
    return result;
}
```

---

#### 4.4.3 MSDF Glyph Shader

```hlsl
// shaders/glyph_msdf.vs.hlsl

struct GlyphInstance {
    float4 rect;     // x, y, width, height
    float4 uv;       // u0, v0, u1, v1
    float4 color;
};

[[vk::binding(0, 0)]] StructuredBuffer<GlyphInstance> instances : register(t0);

VSOutput main(VSInput input) {
    GlyphInstance inst = instances[input.instanceID];
    
    float2 local = float2(
        input.vertexID == 1 || input.vertexID == 3,
        input.vertexID >= 2
    );
    
    float2 pos = inst.rect.xy + local * inst.rect.zw;
    float2 uv = float2(
        lerp(inst.uv.x, inst.uv.z, local.x),
        lerp(inst.uv.y, inst.uv.w, local.y)
    );
    
    VSOutput output;
    output.position = float4(worldToNDC(pos), 0.0, 1.0);
    output.uv = uv;
    output.color = inst.color;
    return output;
}
```

```hlsl
// shaders/glyph_msdf.ps.hlsl

Texture2D msdfAtlas : register(t0);
SamplerState linearSampler : register(s0);

struct PSInput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

float4 main(PSInput input) : SV_Target0 {
    float3 msdf = msdfAtlas.Sample(linearSampler, input.uv).rgb;
    float dist = median(msdf.r, msdf.g, msdf.b);
    
    // 计算屏幕空间导数，用于抗锯齿
    float2 unitRange = float2(ddx(input.uv.x), ddy(input.uv.y));
    float pxRange = length(unitRange) * 4.0;  // MSDF range = 4
    
    float alpha = saturate((dist - 0.5) * pxRange + 0.5);
    
    float4 result = input.color;
    result.a *= alpha;
    return result;
}
```

---

## 5. 代码目录结构

```
dong/src/render/
├── display_list.hpp          # DisplayList IR 定义
├── display_list.cpp
├── layer_tree.hpp            # LayerTree 定义
├── layer_tree.cpp
├── layer_tree_builder.hpp    # 从 DisplayList 构建 LayerTree
├── layer_tree_builder.cpp
├── gpu_command_list.hpp      # GPU IR 定义
├── gpu_command_list.cpp
├── gpu_compiler.hpp          # DisplayList/LayerTree → GPUCommandList
├── gpu_compiler.cpp
├── gpu_driver.hpp            # 抽象 GPU 接口
├── gpu_driver_sdl.hpp        # SDL_gpu 后端实现
├── gpu_driver_sdl.cpp
├── gpu_driver_opengl.hpp     # OpenGL 后端（可选）
├── gpu_driver_opengl.cpp
├── glyph_atlas.hpp           # MSDF 字体 Atlas
├── glyph_atlas.cpp
├── image_atlas.hpp           # 图片 Atlas
├── image_atlas.cpp
├── atlas_packer.hpp          # 纹理装箱算法
├── atlas_packer.cpp
├── msdf_generator.hpp        # MSDF 生成（封装 msdfgen）
├── msdf_generator.cpp
├── painter.hpp               # 高层 Painter（生成 DisplayList）
├── painter.cpp
└── shaders/
    ├── rect.vs.hlsl
    ├── rect.ps.hlsl
    ├── rounded_rect.vs.hlsl
    ├── rounded_rect.ps.hlsl
    ├── glyph_msdf.vs.hlsl
    ├── glyph_msdf.ps.hlsl
    ├── image.vs.hlsl
    └── image.ps.hlsl
```

---

## 6. 实施路线图

### Phase 1: DisplayList 构建（1-2 周）
- [ ] 完善 `DisplayList` 结构（添加 Border、Clip、SaveLayer）
- [ ] 在 `Painter` 中实现 `buildDisplayList(DOM, Layout)` 方法
- [ ] 集成 HarfBuzz 进行文本 shaping，生成 `DrawGlyphRun`
- [ ] 实现 DisplayList 序列化/反序列化（用于调试）

### Phase 2: LayerTree 与缓存（2-3 周）
- [ ] 实现 `Layer` 类和 `LayerTree` 结构
- [ ] 实现 `LayerTreeBuilder::build(DisplayList)`
- [ ] 实现 dirty 标记策略（DOM/Style/Scroll 变化）
- [ ] 为每个 Layer 创建 offscreen 纹理

### Phase 3: Atlas 系统（2-3 周）
- [ ] 集成 `msdfgen` 库
- [ ] 实现 `GlyphAtlas`（MSDF 生成 + 纹理装箱）
- [ ] 实现 `ImageAtlas`
- [ ] 实现纹理装箱算法（MaxRects 或 Skyline）

### Phase 4: GPU Compiler（2-3 周）
- [ ] 实现 `GPUCompiler::compile(LayerTree)`
- [ ] 实现批处理优化（按 pipeline/texture 排序）
- [ ] 实现 instancing 数据准备（RectInstance, GlyphInstance 等）
- [ ] 生成 `GPUCommandList`

### Phase 5: Shader Pipeline（1-2 周）
- [ ] 编写 Rect/RoundedRect/Glyph/Image 的 HLSL shader
- [ ] 使用 `SDL_shadercross` 编译为 SPIR-V
- [ ] 集成到 `GPUDriver` 的 pipeline 创建流程

### Phase 6: 后端集成与测试（2-3 周）
- [ ] 实现 `GPUDriver::execute(GPUCommandList)`
- [ ] 测试 Rect/RoundedRect 渲染
- [ ] 测试 MSDF 文本渲染
- [ ] 测试 Image 渲染
- [ ] 性能优化（batch 合并、atlas 缓存）

### Phase 7: 高级特性（按需）
- [ ] 实现 Layer 合成（CompositeLayer）
- [ ] 实现 Clip/Transform/Opacity 层
- [ ] 实现 partial repaint（dirty rect）
- [ ] 多后端支持（OpenGL/Metal）

---

## 7. 性能优化策略

### 7.1 Batching 优化
- **排序键**: `(pipeline_id << 48) | (texture_id << 16) | blend_mode`
- **合并条件**: 相邻且排序键相同的 item 合并为一个 batch
- **目标**: 将数千个 draw call 降至数十个

### 7.2 Atlas 优化
- **分页**: Glyph atlas 满时自动创建新页（避免单张过大）
- **LRU 淘汰**: 长时间未用的 glyph 从 atlas 中移除
- **预热**: 常用字符（ASCII）预加载到 atlas

### 7.3 Layer Caching
- **触发条件**: 只在以下情况标记 layer dirty：
  - DOM 结构变化
  - 样式变化（颜色、边框、背景）
  - 内容变化（文本、图片）
- **不触发条件**:
  - 滚动（只更新 transform）
  - CSS transform（只更新 Layer.transform）
  - Opacity 变化（只更新 Layer.opacity）

### 7.4 Dirty Rect
- 维护每帧的 `dirty_rect`
- 只重绘脏区域内的 layer
- 减少 GPU 工作量

---

## 8. 调试与工具

### 8.1 DisplayList 可视化
- 导出 DisplayList 为 JSON，用于离线分析
- 实现 `DisplayListDebugger::render(list)` 输出 PNG

### 8.2 GPU 调试
- 使用 RenderDoc 捕获帧
- 检查 batch 数量、draw call 数量
- 检查 atlas 纹理利用率

### 8.3 性能分析
- 添加计时点：
  - DisplayList 构建耗时
  - GPU Compiler 耗时
  - GPU 执行耗时
- 输出性能报告

---

## 9. 依赖库

| 库 | 用途 | 许可证 |
|----|------|--------|
| **msdfgen** | 生成 MSDF 字形 | MIT |
| **HarfBuzz** | 文本 shaping | MIT |
| **stb_image** | 图片解码 | Public Domain |
| **SDL3** | 窗口 + GPU 抽象 | Zlib |
| **SDL_shadercross** | Shader 编译 | Zlib |

---

## 10. 与现有代码的对接

### 10.1 过渡策略

**短期（保持兼容）**:
- `Painter` 同时生成 DisplayList 和调用 Skia（双路径）
- GPU 路径使用 DisplayList → GPUCommandList
- CPU 路径使用 Skia

**中期（逐步替换）**:
- GPU 路径完全不依赖 Skia
- CPU 路径可选：保留 Skia 或实现 CPU 光栅化器

**长期（完全移除）**:
- 移除 SkiaBackend
- 所有渲染走 GPU 管线

### 10.2 入口点修改

```cpp
// dong/src/core/view.cpp

void View::update() {
    // ... layout ...
    
    if (use_gpu_) {
        // GPU 路径
        auto display_list = painter->buildDisplayList(dom_manager->getRoot(), layout_engine.get());
        auto layer_tree = layer_tree_builder_->build(display_list);
        auto gpu_commands = gpu_compiler_->compile(layer_tree);
        gpu_driver_->execute(gpu_commands);
        gpu_driver_->present();
    } else {
        // CPU 路径（保留或移除 Skia）
        painter->renderDOM(dom_manager->getRoot(), layout_engine.get());
    }
}
```

---

## 11. 风险与挑战

| 风险 | 缓解策略 |
|------|----------|
| MSDF 生成性能 | 预生成常用字符，运行时异步生成 |
| Atlas 碎片化 | 定期重打包，LRU 淘汰 |
| Shader 兼容性 | 使用 SDL_shadercross 跨平台编译 |
| 文本 shaping 复杂度 | 使用成熟库（HarfBuzz） |
| 调试困难 | 完善日志、可视化工具 |

---

## 12. 参考资料

- Ultralight GPU Driver 技术分析
- Flutter LayerTree 设计文档
- Chrome Compositor Architecture
- WebKit Accelerated Compositing
- msdfgen GitHub: https://github.com/Chlumsky/msdfgen
- HarfBuzz Documentation

---

**下一步**: 根据此文档开始实施 Phase 1（DisplayList 构建）

