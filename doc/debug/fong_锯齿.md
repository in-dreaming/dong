# Font / MSDF 小字号锯齿问题调研（不改代码版）

> 背景：当前项目文字使用 MSDF（`msdfgen`）+ FreeType/HarfBuzz shaping，在 SDL backend（`dong/backends/sdl`）中用 `text_fs.hlsl` 解码并做抗锯齿。
>
> 现象：字体渲染结果存在锯齿，小字号尤为明显；感觉“MSDF 的能力没有用到极致”。
>
> 目标：不改代码，仅梳理**当前链路的关键点**、**小字锯齿的高概率根因**、以及**可以验证/调参的方向**。

---

## 0. 当前实现速览（按数据流）

### 0.1 字形生成（CPU，msdfgen）

代码：`dong/src/render/glyph_atlas.cpp`

- 使用 `msdfgen::generateMSDF(msdf, shape, range, scale, translate)` 生成固定尺寸 MSDF（`glyph_bitmap_size_` 对应 tier 的 `bitmap_px`）。
- 将距离按标准方式编码到 [0,1]：`encoded = distance / range + 0.5`。
- 保存 `UnifiedGlyphMetrics` 的 MSDF 元信息：
  - `msdf_scale`：design units → msdf 像素的 scale
  - `msdf_translate_x/y`：projection translate

注意：这里的 `scale` 不是“渲染缩放”，而是 msdfgen 投影参数。后续渲染必须正确把它折算进屏幕 space 的 AA 参数，否则会产生异常粗细/锯齿/模糊。

### 0.2 Tier 配置（决定 MSDF 分辨率与 range）

代码：`dong/src/core/global_shared.cpp`（以及 SDL driver 的同配置）

当前 tiers：
- `128px, range=7`（9-14px）
- `192px, range=9`（14-22px）
- `256px, range=11`（22-36px）
- `384px, range=12`（36px+）

结论：小字号确实用更高分辨率的 MSDF（例如 font=12px 却用 128px 的 MSDF tile），理论上应该“更不容易锯齿”，但现实依然锯齿，说明问题更可能在**采样/AA**而不是 MSDF 纹理本身分辨率。

### 0.3 纹理采样器（关键）

代码：`dong/backends/sdl/sdl_gpu_driver_init.cpp`

- 文字 sampler 被强制设置为 **NEAREST**：
  - `text_sampler_info.min_filter = SDL_GPU_FILTER_NEAREST`
  - `text_sampler_info.mag_filter = SDL_GPU_FILTER_NEAREST`
  - `mipmap_mode = NEAREST`

注释写法：
> “MSDF 必须使用 NEAREST，Linear 会插值 MSDF 的距离值，导致边缘模糊”

这句话只对了一半：
- **对 MSDF 的解码确实不应该先做线性过滤到距离值**（否则会改变距离分布，导致边缘变软/发糊）。
- 但 **minification（缩小）时的 NEAREST 采样会产生非常明显的 alias**，尤其是：
  - 小字号、细笔画、对比强
  - 纹理在屏幕上的 footprint < 1px（或接近 1px）

这通常表现为“小字边缘锯齿、闪烁、笔画断续”。

### 0.4 GPU 端 AA（text_fs.hlsl）

代码：`dong/backends/sdl/shaders/text_fs.hlsl`

- 使用 `median(r,g,b)` 恢复 signed distance。
- `screenPxRange` 的计算采用：
  - CPU 预计算 `px_range_screen = atlas_range * (glyph_pixel_scale / msdf_scale)`
  - 再和 `fwidth(uv)` 推导的动态项混合（70%/30%）
  - clamp 到 `[1.5, 8.0]`
- 用 `smoothstep` 做边缘过渡，并做了：
  - stem darkening（小范围加粗）
  - gamma 调整（`gamma=1.75`）

这是一套“标准 MSDF AA”思路，但它的有效前提是：

> 采样得到的 `msdf` 值需要足够稳定，并且距离场在纹理空间上的采样要满足 Nyquist（至少别被 NEAREST minify 直接破坏）。

---

## 1. 小字号锯齿的高概率根因（按优先级）

### 1.1 关键矛盾：NEAREST min_filter + 无 mip => 小字必然 alias

当前文字 sampler 的 min_filter=NEAREST。

在小字号场景中，虽然每个 glyph 使用 128/192 的 tile，但屏幕上 glyph 可能只占十几像素甚至更少；等价于对 MSDF 纹理做强烈 minify。

- **NEAREST 在 minify 的时候不是“更锐利”，而是“随机挑一个 texel”**。
- 对 MSDF 来说，一个 texel 的距离变化对最终 alpha（`smoothstep` 输出）非常敏感。

因此：
- 你看到的“锯齿”很多时候不是 MSDF 分辨率不足，而是采样策略导致的 alias。

验证思路（不改代码也能推断）：
- 若锯齿主要发生在**缩放、动画、摄像机移动**时，且边缘有“跳动/闪烁”，基本就是 minify alias。

### 1.2 `screenPxRange` 下限 clamp 可能过低/逻辑偏保守

shader 里 clamp 到 `min=1.5`。

对极小字（特别是浅色字在深底）来说，`screenPxRange` 太小会导致 smoothstep 过渡区太窄，最终看起来像硬边（alias）。

这属于“AA 参数偏锋利”。

### 1.3 AA 在 Gamma/颜色空间中的位置可能不一致

`text_fs.hlsl` 里有 `toLinear/toSRGB` 函数，但当前输出路径是：
- `result.rgb = input.color.rgb`
- `result.a = input.color.a * opacity`

若渲染链/最终 swapchain 在 sRGB/线性空间之间的处理不一致（例如 UI 在 gamma space 混合），就会出现：
- 同样的 opacity 过渡在视觉上更硬、更像锯齿

这不是“MSDF 本身”，是“混合的色彩空间”。

### 1.4 glyph quad 的像素对齐 / subpixel positioning

小字时，glyph quad 的位置如果出现大量非整数像素（例如 baseline/pen 在 0.3px、0.6px），那么边缘采样会更敏感。

MSDF 通常能抗 subpixel positioning，但在 NEAREST 采样下反而更差。

代码线索：
- `sdl_gpu_driver_execute.cpp` 里 glyph 的 `tile_x/tile_y` 是浮点计算，没有强制对齐。

### 1.5 msdfgen range/scale 的组合可能导致“有效可用区域”偏小

历史文档 `doc/font-rendering-fix.md` 里提到：
- range 太大 => 可用区域小 => scale 变小

虽然当前 tiers 使用的是大 tile（128/192）+ range（7/9），看似可用区域仍然充足，但：
- 不同字体/字重/字符（尤其 CJK）可能让 `max_glyph_dim` 很大，导致 `scale` 变小
- 进而 `px_range_screen = atlas_range * (glyph_pixel_scale / msdf_scale)` 可能发生极端值

极端值会让 shader clamp 生效，从而变成“统一 AA 形态”，导致某些字体/字重看起来更锯齿。

---

## 2. “MSDF 没用到极致”具体指什么

MSDF 的上限能力一般来自几个方向：

1) **正确的采样策略**：
   - 常见做法不是单纯 NEAREST，而是：
     - 使用 linear 采样 + 正确的解码/补偿（或使用 MIP 生成策略确保距离场在 mip 上仍然成立）。
     - 或者使用多点采样（supersampling）来降低 alias。

2) **多通道 SDF 的边缘一致性**：
   - `edgeColoring` 的策略、误差修正等会影响尖角处 artifact。
   - 当前使用 `edgeColoringSimple(shape, 3.0)`，属于“可用但不最强”的默认策略。

3) **小字号专用处理**：
   - stem darkening、contrast compensation、hint-like 的对齐策略。
   - 当前 shader 有 stem darkening，但配合 NEAREST minify 可能适得其反（更像硬边）。

---

## 3. 建议的排查/验证清单（不改代码前提下）

### 3.1 先确认是不是“minify alias”

观察/截图对比：
- 固定 UI 缩放，逐步改变 DPI / window scale（比如 100%→125%→150%）：
  - 若锯齿随缩放变化显著，且在某些缩放倍率更糟，强指向采样/像素对齐问题。
- 缓慢平移 UI（或滚动一段文本）：
  - 若边缘出现“跳动/闪烁”，几乎可判定 NEAREST minify。

### 3.2 记录关键数值（已有日志能力）

`sdl_gpu_driver_execute.cpp` 已有 `DONG_LOG_DEBUG("[DrawText] ... msdf_scale ... render_scale ...")`。
建议在出现锯齿的 case 中抓取：
- `font_size`
- `msdf_scale`
- `glyph_pixel_scale`
- `px_range_screen`（= atlas_range * glyph_pixel_scale / msdf_scale）

判断：
- `px_range_screen` 是否经常被 clamp 到 1.5？（意味着 AA 过硬）
- `msdf_scale` 是否极小/极大？（意味着 msdfgen 投影参数异常）

### 3.3 针对 CJK 与拉丁分别观察

CJK 复杂轮廓更容易在：
- edge coloring
- range 不足
- 采样 alias
上出现问题。

同一字号下对比：
- “IIIIllll” vs “繁體中文測試”

如果 CJK 更差，可能需要：
- 更大的 range 或更稳健的 edge coloring（后续改代码时）

---

## 4. 后续可能的改进方向（这里只列方向，不在本文落地）

> 以下是“如果允许改代码”时的方向，用于你评估投入产出。

### 4.1 让文字 sampler 在 minify 时不 alias

典型策略（任选其一或组合）：

- **多采样 supersampling**：在 fragment 里对 MSDF 做 4x/8x 采样再 average（成本较高）。
- **改用线性采样，但在解码中补偿**：
  - 需要验证 linear 是否会引入过度模糊，以及是否能用更好的 AA 函数抵消。
- **生成 mipmaps + 专用 MSDF mip 生成**：
  - 普通 box-filter 生成 mip 会破坏距离场；需要特殊 mip 策略（例如对距离进行 min/max 或重计算近似）。

### 4.2 小字号专用 AA 参数

- 提高 `screenPxRange` 的最小值
- 调整 `stemDarkening` 曲线，避免在 alias 采样条件下加剧锯齿

### 4.3 像素对齐策略（hint-like）

- 对小字号的 baseline / x 位置做半像素对齐（看平台的 raster 规则）
- 或在 shaping 输出阶段做 snap

---

## 5. 与已有文档的关系

- `doc/font-rendering-fix.md`：历史上修过 distance_scale（msdf_scale 参与计算）与 range/可用区域问题。
- `doc/debug/history_202601082006_summary.md`：提过 “MSDF 锯齿明显” 的通用方向（fwidth、最小 screenPxRange、超采样）。

本文补充的核心结论是：

> **当前最可疑的根因是文字 sampler 的 min_filter=NEAREST 导致 minify alias，而 shader 的 AA 再怎么调也很难完全救回来。**

