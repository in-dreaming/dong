# CSS Isolation 属性实现总结

## 概述

本次实现为 Dong 引擎添加了 CSS `isolation` 属性支持，允许元素创建独立的混合上下文（stacking context），从而实现离屏图层合成。

## 实现要点

### 1. DOM 样式层

**文件：** `dong/src/dom/dom_node.hpp`

添加了 `isolation_isolate` 字段到 `ComputedStyle` 结构：

```cpp
struct ComputedStyle {
    // ...
    float opacity = 1.0f;
    bool isolation_isolate = false;  // isolation: auto | isolate
    // ...
};
```

### 2. 样式引擎解析

**文件：** `dong/src/dom/style_engine.cpp`

- 在样式应用函数中添加了对 `isolation` 属性的解析支持
- 当 CSS 中设置 `isolation: isolate` 时，会将 `isolation_isolate` 字段设为 `true`

```cpp
else if (prop == "isolation") {
    std::string lowered = val;
    std::transform(lowered.begin(), lowered.end(), lowered.begin(), 
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    style.isolation_isolate = (lowered == "isolate");
}
```

### 3. DisplayList 层

**文件：** `dong/src/render/display_list.hpp`

扩展了 `LayerData` 结构以支持离屏图层边界和隔离标志：

```cpp
struct LayerData {
    Rect bounds;          // 图层边界（用于创建离屏纹理）
    float opacity = 1.0f;
    bool isolate = false; // 是否需要离屏隔离
};
```

修改了 `pushLayer` 方法：

- 当 `isolate = true` 时，即使 `opacity >= 0.999`，也会创建图层
- 接收 `bounds` 参数以定义离屏纹理的渲染区域

### 4. Painter 渲染逻辑

**文件：** `dong/src/render/painter.cpp`

在节点绘制时判断是否需要创建隔离图层：

```cpp
bool has_isolation = style.isolation_isolate && 
                     (layer_bounds.width > 0.0f && layer_bounds.height > 0.0f);
bool needs_layer = has_isolation || clamped_opacity < 0.999f;

if (needs_layer) {
    opacity_scope = builder.pushLayer(clamped_opacity, has_isolation, layer_bounds);
}
```

### 5. GPU IR 命令扩展

**文件：** `dong/src/render/gpu_ir.hpp`

新增了两个 GPU 命令类型：

```cpp
enum class GPUCommandType {
    // ...
    BeginIsolatedLayer,  // 开始离屏图层渲染
    EndIsolatedLayer,    // 结束离屏图层并合成到父级
};
```

在 `GPUCommand` 中添加了 `layer_opacity` 字段用于图层合成时的透明度控制。

**编译逻辑：**

- 当遇到 `PushLayer` 且 `isolate = true` 时，生成 `BeginIsolatedLayer` 命令
- 维护独立的透明度栈：隔离图层内部使用父级透明度，不累乘当前图层透明度
- 在 `PopLayer` 时检查是否为隔离图层，若是则生成 `EndIsolatedLayer` 命令

### 6. GPU Driver 实现

**文件：** `dong/src/render/gpu_driver_sdl.cpp`

实现了离屏渲染目标的创建、渲染和合成：

#### 6.1 渲染目标栈管理

```cpp
struct RenderTargetState {
    SDL_GPUTexture* texture = nullptr;
    Uint32 width = 0;
    Uint32 height = 0;
    bool is_swapchain = false;
};

std::vector<RenderTargetState> render_target_stack;
std::vector<IsolatedLayerState> isolated_layer_stack;
```

#### 6.2 BeginIsolatedLayer 处理

1. 结束当前 render pass
2. 根据父级渲染目标尺寸创建离屏纹理（`SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER`）
3. 将新纹理压入 `render_target_stack`
4. 开始新的 render pass，clear color 为透明黑 `{0, 0, 0, 0}`
5. 保存图层信息（纹理、边界、透明度）到 `isolated_layer_stack`

#### 6.3 EndIsolatedLayer 处理

1. 结束当前离屏 render pass
2. 从 `render_target_stack` 弹出离屏纹理
3. 恢复父级渲染目标的 render pass（使用 `LOAD_OP_LOAD` 保留已有内容）
4. 使用 image pipeline 将离屏纹理作为普通图片绘制到父级：
   - 设置正确的 UV 矩形（基于 `layer_bounds`）
   - 应用图层的 `layer_opacity` 作为 tint alpha
   - 通过标准的 quad 渲染完成合成
5. 释放离屏纹理

#### 6.4 视口与坐标管理

实现了 `current_target_dimensions` 和 `current_viewport` 辅助函数，确保所有绘制命令使用正确的渲染目标尺寸，而不是硬编码的 swapchain 尺寸。

### 7. 示例验证

**文件：** `dong/examples/isolation_demo.cpp`

创建了对比示例展示 `isolation: auto` vs `isolation: isolate` 的视觉差异：

- **左列**：`isolation: auto` - 半透明元素直接与背景混合
- **右列**：`isolation: isolate` - 先在离屏图层内合成，再整体混合到父级

示例包含详细的 CSS 代码和说明文档。

## 技术细节

### 离屏纹理格式

- **格式**：`SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM`
- **用途**：`SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER`
- **采样**：1x（无 MSAA）
- **尺寸**：继承父级渲染目标尺寸

### 混合模式

离屏图层合成时使用标准的 alpha blending，通过将图层作为纹理采样并应用 `layer_opacity` 实现整体透明度控制。

### 性能考虑

- 每个 `isolation: isolate` 元素都会创建离屏纹理，因此应谨慎使用
- 纹理在使用后立即释放（`SDL_ReleaseGPUTexture`）
- 未来可考虑纹理池复用以减少分配开销

## 与规范对照

此实现符合 `doc/gpudriver-pipline..md` 中的 LayerTree 设计理念：

- ✅ 支持 OpacityLayer 和 IsolationLayer 的概念
- ✅ DisplayList → GPU IR 的编译流程清晰
- ✅ 离屏渲染目标栈管理（为后续 TransformLayer、ClipLayer 预留扩展点）
- ⚠️ 尚未实现完整的 dirty 标记和图层缓存（P1 阶段）

## 后续优化方向

1. **图层缓存**：当 `isolation: isolate` 的内容未发生变化时，复用上一帧的离屏纹理
2. **纹理池**：减少频繁的纹理分配/释放开销
3. **裁剪优化**：根据实际内容边界（而非整个父级尺寸）创建更小的离屏纹理
4. **MSAA 支持**：为高质量渲染添加多重采样
5. **混合模式扩展**：支持 `mix-blend-mode` 与 `isolation` 的组合

## 编译与测试

```bash
# 编译所有示例
cd dong
zig build examples -Doptimize=ReleaseFast

# 运行 isolation 演示
./zig-out/bin/isolation_demo
```

## 已知限制

- 目前仅支持 `isolation: auto` 和 `isolation: isolate`，不支持其他混合模式
- 离屏纹理尺寸固定为父级渲染目标尺寸，未优化为实际内容边界
- 嵌套多个 `isolation: isolate` 时会产生多层离屏渲染，性能开销较大

---

**完成时间：** 2025-11-26  
**相关文件数量：** 8 个核心文件 + 1 个示例  
**新增代码行数：** 约 350 行（含示例和注释）
