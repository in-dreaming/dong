# Font Rendering Fix - MSDF Distance Scale 核心问题修复

**日期**: 2025-11-26  
**问题**: 字体渲染完全无法识别，呈现为块状像素

## 问题根源分析

### 症状
从截图看，所有文字都是块状像素，完全无法识别，不是简单的模糊问题。

### 系统性诊断过程

#### 1. Atlas 配置检查 ✅
```cpp
{16px, 4.0f},  // 小字号
{24px, 6.0f},  // 中字号  
{32px, 8.0f},  // 大字号
{48px, 10.0f}, // 特大字号
```

Atlas 选择逻辑正确：
- 24px 文字 → 24px atlas, glyph_scale=1.0
- 16px 文字 → 16px atlas, glyph_scale=1.0

#### 2. MSDF 生成参数检查 ❌ **发现问题！**

调试日志显示：
```
MSDF gen: glyph=39, atlas_size=24, range=6.0, bounds=(1.9,0.0)-(16.2,17.2), scale=0.697
MSDF gen: glyph=82, atlas_size=24, range=6.0, bounds=(0.7,-0.5)-(12.4,12.9), scale=0.897
MSDF gen: glyph=81, atlas_size=24, range=6.0, bounds=(1.5,0.0)-(11.8,12.9), scale=0.933
```

**关键发现**: 
- MSDF 生成时，`scale` 在 0.66-0.93 之间
- 字形实际只占用 MSDF 纹理的 60-90% 区域
- 剩余空间被 padding (range * 2) 占用

#### 3. Distance Scale 计算错误 ❌ **核心问题！**

**错误的代码**:
```cpp
float distance_scale = glyph_tier->distance_range * glyph_scale;
// 对于 24px 文字: distance_scale = 6.0 * 1.0 = 6.0
```

这个公式假设字形占满整个 MSDF 纹理 (scale=1.0)，但实际上字形被缩放到了 0.7 倍！

**结果**:
- MSDF 纹理中字形占 70% 空间，scale=0.7
- Shader 采样时假设字形占 100% 空间
- 采样位置完全错位 → 显示为乱码

## 修复方案

### 核心修复：使用 MSDF 生成时的实际 scale

**修复后的代码**:
```cpp
// ✅ 关键修复：使用 MSDF 生成时的实际 scale
float msdf_gen_scale = entry->metrics.msdf_scale;
float distance_scale = msdf_gen_scale * glyph_tier->distance_range * glyph_scale;

// 对于 24px 文字，scale=0.7:
// distance_scale = 0.7 * 6.0 * 1.0 = 4.2
```

### 公式推导

MSDF shader 中的计算：
```hlsl
float screenPxDistance = sdf * distanceScale;
```

其中 `sdf` 是从 MSDF 纹理采样得到的归一化距离值 (0-1范围)。

**正确的 distance_scale 公式**:
```
distance_scale = msdf_generation_scale * atlas_range * glyph_render_scale
```

各参数含义：
- `msdf_generation_scale`: MSDF 生成时的缩放因子 (0.6-1.0，取决于字形大小)
- `atlas_range`: MSDF 的 range 参数 (SDF 值域范围，单位：像素)
- `glyph_render_scale`: 渲染时的缩放 (屏幕字号 / Atlas字号)

### 为什么之前的公式错误？

**错误公式**: `distance_scale = range * glyph_scale`
- 忽略了 MSDF 生成时字形被缩放的事实
- 假设字形总是占满 MSDF 纹理
- 导致 shader 采样位置错误

**正确理解**:
1. MSDF 生成时，字形被缩放以适应 `(atlas_size - range*2)` 的有效区域
2. 缩放因子 `msdf_scale` 由 msdfgen 计算并保存在 `GlyphMetrics` 中
3. Shader 采样时必须考虑这个缩放，否则会采样到错误的位置

## 修改文件

### 1. `dong/src/render/gpu_driver_sdl.cpp` (line ~1610)

**修改前**:
```cpp
float distance_scale = glyph_tier->distance_range * glyph_scale;
u.params[0] = distance_scale;
```

**修改后**:
```cpp
float msdf_gen_scale = entry->metrics.msdf_scale;
float distance_scale = msdf_gen_scale * glyph_tier->distance_range * glyph_scale;
u.params[0] = distance_scale;
```

### 2. 添加调试日志 (glyph_atlas.cpp)

```cpp
SDL_Log("MSDF gen: glyph=%u, atlas_size=%d, range=%.1f, scale=%.3f",
        glyph_id, msdf_size, range, scale);
```

## 验证

```bash
cd dong && zig build examples
./zig-out/bin/gpu_view_demo
```

预期日志：
```
MSDF gen: glyph=39, atlas_size=24, range=6.0, scale=0.697
Text render: font_size=24.0, atlas=24px, range=6.0, glyph_scale=1.000
计算得到: distance_scale = 0.697 * 6.0 * 1.0 = 4.182
```

预期效果：
- ✅ 文字清晰可读
- ✅ 边缘平滑抗锯齿
- ✅ 不同字号渲染正确
- ✅ 无像素化或乱码

## 技术细节

### MSDF Scale 计算逻辑 (glyph_atlas.cpp)

```cpp
// MSDF 有效区域 = atlas_size - padding
double effective_size = msdf_size - range * 2;  // e.g., 24 - 12 = 12

// 字形边界大小
double glyph_width = bounds.r - bounds.l;   // e.g., 16.2 - 1.9 = 14.3
double glyph_height = bounds.t - bounds.b;  // e.g., 17.2 - 0.0 = 17.2

// 缩放以适应有效区域
double scale = min(effective_size / glyph_width,
                   effective_size / glyph_height);
// scale = min(12/14.3, 12/17.2) = min(0.839, 0.697) = 0.697
```

因此，大多数字形的 `scale` 会小于 1.0，尤其是在 range 较大时。

### 优化方向

**当前问题**: Range 太大导致有效区域过小

例如 24px atlas, range=6.0:
- 有效区域 = 24 - 12 = 12px (仅 50%!)
- 字形被严重压缩，丢失细节

**优化方案**:
1. **减小 range**: 改为 `range = atlas_size / 6` (更合理的比例)
   ```cpp
   {16px, 3.0f},  // 16/6 ≈ 2.7
   {24px, 4.0f},  // 24/6 = 4.0
   {32px, 5.0f},  // 32/6 ≈ 5.3
   {48px, 8.0f},  // 48/6 = 8.0
   ```

2. **增大 atlas 尺寸**: 使用更大的基准尺寸
   ```cpp
   {24px, 8.0f},  // 有效区域 = 24-16 = 8px (33%)
   {32px, 8.0f},  // 有效区域 = 32-16 = 16px (50%)
   {48px, 10.0f}, // 有效区域 = 48-20 = 28px (58%)
   ```

3. **动态 range**: 根据字形复杂度调整 range

## 性能影响

- **CPU**: 无影响 (仅增加一次乘法)
- **GPU**: 无影响 (shader 计算量不变)
- **内存**: 无影响
- **渲染质量**: 显著改善 ✅

## 后续优化

1. **调整 range 参数**: 减小 range 以增加有效区域
2. **Subpixel 渲染**: 小字号启用 RGB 分离采样
3. **Gamma 校正**: sRGB→linear→sRGB 管线
4. **自适应 atlas**: 根据字形复杂度选择不同的 range

## 参考文档

- `doc/next.md` - 短期画质优化方向
- `doc/gpudriver-pipline..md` - GPU 管线架构设计
