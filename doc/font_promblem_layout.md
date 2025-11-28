**Examining vertical alignment**

I'm analyzing vertical alignment, which mainly uses baseline_y from Painter. This comes from FT metrics (ascent) but is independent of hb metrics. Jitter in vertical direction for Latin seems unlikely. Spacing issues might arise from neglecting hb's glyph advance, although we've accounted for pen_x. It's essential to determine if glyph_bitmap_size_ and font_size align proportionally due to scaling. I'll check the scaling ratio while ensuring everything corresponds correctly. It looks like I'm on the right track with this approach!**Analyzing GlyphAtlas settings**

In GlyphAtlas, I set the FT pixel size to glyph_bitmap_size_, possibly 24 or 32. I used metrics at that pixel size, then multiplied by glyph_scale, calculated as font_size/base_bitmap. So, if metrics.width is at 24 px, I translate this to width20 by scaling it down. The same goes for bearing_x to ensure spacing matches. I'm also considering how cropping might affect this. When interpreting `glyph.pen_x` in GPUDriverSDL::DrawText, I ensure vertical alignment is properly handled by subtracting bearing_y from glyph.pen_y. This seems sound so far!**Reviewing spacing calculations**

I'm examining how spacing works, particularly with `glyph_w` as width. I don’t use bearing_x again, so the rightmost point of the glyph bounding box relates to its calculations. FreeType's metrics have `horiAdvance` as `advance_x`, which might differ from the sum of `bearing_x` and `width` due to the right side bearing difference. Letters usually have an internal side bearing, making bounding boxes narrower than the advance. I need to ensure I'm using pen_x from HB shaped correctly, considering the gap between bounding boxes is based on the right side bearing. Everything seems to add up!**Investigating letter spacing issues**

I’m noticing that the screenshot shows letter spacing is unusually wide and inconsistent, particularly with letters like D, o, n, g. This might stem from a mismatch between our font_size and the layout width. Perhaps the text width was measured using `pen_x` differences, while the alignment logic used a smaller `inner_width`, which shouldn't create extra spaces inside but may affect line alignment. Alternatively, there could be a mismatch between HarfBuzz scaling and the layout's font_size due to devicePixelRatio, leading to manual scaling differences, causing the text to look condensed or more spaced out. But, the proportions seem stable overall.**Troubleshooting spacing and scaling issues**

User's complaint about "字外来歪去" may suggest glyph-specific rounding issues. I think we might be using `fwidth(input.uv)` in the shader, which can cause thickness variations for smaller glyph sizes. We need to examine `TextShaper` and the recent changes, particularly around `FT_LOAD_NO_HINTING`. Although we applied the same load flags, the spacing remains off, indicating deeper architectural problems. The MSDF scaling seems inconsistent as well, potentially affecting how narrow and wide glyphs appear. I suspect issues with how we're measuring whitespace and applying kerning, which doesn't seem to align with expectations. A thorough review of how we convert and handle these metrics is necessary to resolve the spacing anomalies.**Analyzing font metrics and spacing issues**

I’m looking into why there’s wide spacing and potential misalignment for "字外来歪去." It seems we’re possibly misusing `GlyphInstance.pen_x` as the baseline origin while also feeding `origin_x=0`, which could lead to miscalculations. The dual shaping paths in HarfBuzz and GlyphAtlas create a misalignment issue. Although those glyph indices should match, we face irregular spacing potentially from zoom levels or subtle metric inconsistencies. To address this, we'd need to unify our font metrics, consolidate the use of FreeType, and ensure consistent coordinate systems for generating MSDF, avoiding manual scaling problems. The user wants a professional-level fix, not temporary hacks, so I’ll propose a thorough design-level revision to address the root causes around architecture mismatches and improve consistency in glyph rendering.**Proposing a professional solution for font metrics**

I need to address multiple mismatch issues related to glyph spacing and layout. First, I’ll introduce a Font Metrics Layer to query FT face metrics during font loading, computing pixel-per-unit values accurately. For shaping, I’ll configure HarfBuzz to utilize design units effectively, enhancing MSDF generation with precise boundaries and measurements.

Next, I’ll remove the redundant FreeType instance in GlyphAtlas to streamline measurements. Adapting a unified approach for coordinate systems will mitigate tilting and inconsistencies. Detailing a step-by-step plan includes defining a canonical coordinate system and restructuring components for consistent measurements. Ultimately, adopting this method will enhance precision without rounding errors.
好，这次我们不再讨论“调参/补偿”，而是从架构和度量体系上，把问题拆开讲清楚，然后给一套专业、长期可维护的方案。

先说结论：  
你现在这张图里“字东倒西歪 + 间距不自然”，根本原因不是单个公式写错，而是“字体几何有三套来源、四个坐标系，却没有一个统一的‘字体度量层’来约束它们”。  
在这种前提下，就算 MSDF 本身已经清晰，字形位置也必然是“差不多对、但肉眼看着总怪”的。

我按“问题拆解 → 根因总结 → 专业方案”三段说。

一、现状拆解：同一个字形被算了三遍几何（互相没对齐）

1）HarfBuzz + FreeType：负责“字符串 → glyph run + pen_x/pen_y”

- `TextShaper::shape` 大致流程：

  ```cpp
  uint32_t pixel_size = clampPixelSize(request.font_size); // ceil(font_size)
  FT_Set_Pixel_Sizes(face, 0, pixel_size);

  hb_font = hb_ft_font_create_referenced(face);
  hb_font_set_scale(hb_font, round(request.font_size * 64), ...);
  hb_ft_font_set_load_flags(hb_font, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP);

  // hb_shape → 每个 glyph 的 x_advance / x_offset / y_offset
  glyph.pen_x = pen_x + x_offset;
  glyph.pen_y = pen_y - y_offset;
  pen_x += x_advance;
  ```

- 这一条链路在“字号 = font_size 的逻辑像素空间”里给出：
  - 每个 glyph 的笔尖位置 `(pen_x, pen_y)`；
  - 整行宽度 `ShapedText.width`；
  - 行高、ascent/descender 由 `face->size->metrics` 得出。

→ 这套几何是 layout / 排版真正依赖的来源。

2）GlyphAtlas + 第二套 FreeType + msdfgen：负责“生成 MSDF 纹理 + 自己的度量”

- `GlyphAtlas::generateMSDF` 里又重新开了一个 FreeType：

  ```cpp
  FT_Init_FreeType(&ft);
  FT_New_Face(ft, font_path, 0, &face);
  FT_Set_Pixel_Sizes(face, 0, glyph_bitmap_size_); // 16/24/32/48 之类

  FT_Load_Glyph(face, glyph_id, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP);

  // 读一套自己的好，这次我们不讲“临时补偿/调参”，而是站在“字体度量层 + 多坐标系统一”的角度，把问题拆开分析，给一套能长期站得住的方案。

先说结论：  
你现在看到的“字东倒西歪 + 间距不自然”，根本原因不是单个公式，而是：

- 同一个字形的几何信息，被“排版（HarfBuzz）”、“MSDF 生成”、“GPU 绘制”三套系统分别算了一遍；
- 每一层用的 FreeType 实例、像素尺寸、scale/translate 不一致；
- 最后 GPU 在组合这些信息时，没有一个“统一的字体度量坐标系”来约束，导致：
  - 有的量在“font_size 像素空间”里；
  - 有的在“glyph_bitmap_size 像素空间”里；
  - 有的在“msdfgen 内部 scale/translate 之后的设计空间”里；
- 这些误差叠加，就表现为：  
  - glyph 在 baseline 附近略微上下漂、左右轻微错位 → 视觉上“外来歪去”；  
  - glyph 间 advance 和实际可见轮廓不匹配 → 间距看起来“过宽/不自然”。

下面分三部分说清楚：

一、问题拆解：现在一条文字，要经过三套不统一的几何

1）排版几何：HarfBuzz + FreeType（TextShaper）

- 在 `TextShaper::shape` 里：

  - 使用 FreeType 的 `FT_Face(face, pixel_size=ceil(font_size))`；
  - 建立 `hb_font`，`hb_font_set_scale(font_size * 64)`；
  - 由 HarfBuzz 输出每个 glyph 的：
    - `x_advance / x_offset / y_offset`（26.6 定点，转换为 float 像素）；
    - 按顺序累加成一条 glyph run 的 `pen_x` 序列；
  - 行高/ascent/descender 再由 `face->size->metrics` 读出。

- `Painter` 再做了一层包装：
  - 用 `ascent`、`line_height` 算出行的 baseline；
  - 把 `ShapedGlyph.pen_x/pen_y` 平移到最终的 `GlyphInstance.pen_x/pen_y`（加上 padding、text-align 等）。

→ 到这一步为止，所有“字与字之间的相对位置”是在“字号像素 + HarfBuzz 几何”这一套坐标里定义的，这套坐标是内容语义唯一靠谱的来源。

2）MSDF 几何：GlyphAtlas + 第二套 FreeType + msdfgen

- 在 `GlyphAtlas::generateMSDF` 中，你又开了一套 FreeType：

  - 重新 `FT_Init_FreeType`，`FT_New_Face(font_path)`；
  - `FT_Set_Pixel_Sizes(face, 0, glyph_bitmap_size_)`，这里的 `glyph_bitmap_size_` 是 tier 的 16/24/32/48 像素；
  - 用 `FT_Load_Glyph(face, glyph_id, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP)` 读出：
    - `advance_x`、`horiBearingX/Y`、`width/height` → 存到 `GlyphMetrics`；
  - 然后把 FreeType 的 outline 丢给 msdfgen：
    - `shape.getBounds()` 得到轮廓设计空间的 `bounds`；
    - 自己算一个 `scale`、`translate`，让轮廓塞进 `msdf_size × msdf_size` 的方形：
      - `scale = min( (msdf_size - 2*range)/safe_width, (msdf_size - 2*range)/safe_height )`
      - `translate` 让 glyph 居中并留 range 边距；
    - `generateMSDF(msdf, shape, range, scale, translate)` 生成 3 通道距离场；
  - 最后：
    - 把位图写进 atlas（`out_width = out_height = msdf_size`）；
    - 把 `msdf_scale / msdf_translate` 存进 `GlyphMetrics`，但渲染时基本没用。

→ 这里实际上构造了第二套与排版完全独立的几何：  
- 一套来源于 `GlyphMetrics`（像素单位 = glyph_bitmap_size_）；  
- 一套是 msdfgen 内部的“设计空间 → MSDF 像素”的仿射变换（`scale + translate`）。

3）GPU 几何：把两套度量“硬拼”在一起

在 `GPUDriverSDL::execute` 的 `DrawText` 分支里：

- 你拿到的是：

  - 来自 HarfBuzz 的 `GlyphInstance.pen_x / pen_y`（排版世界的“笔尖位置”）；
  - 来自 GlyphAtlas 的 `AtlasEntry.metrics.*`：
    - `bearing_x / bearing_y / width / height`（glyph_bitmap_size_ 分辨率下的度量）；
    - `msdf_scale / msdf_translate`（MSDF 内部坐标，但目前基本没参与 GPU 逻辑）。

- 然后用以下公式生成每个 glyph 的 quad：

  ```cpp
  float glyph_scale = font_size / base_bitmap; // base_bitmap = tier.bitmap_px，如 16/24/32/48

  float glyph_x = glyph.pen_x + entry->metrics.bearing_x * glyph_scale;
  float glyph_y = glyph.pen_y - entry->metrics.bearing_y * glyph_scale;
  float glyph_w = entry->metrics.width  * glyph_scale

[]


**Analyzing User Requirements**

The user wants a professional analysis and robust solution rather than temporary fixes. They’ve chosen Plan A, focusing on certain metrics but are still seeking a fundamental fix. Since I'm a coding assistant, I can modify the code myself. The user specifically requests a thorough analysis and the continuation of unfinished tasks. Their goal is to implement structural fixes for glyph geometry, despite the potential complexity. The proposed plan includes creating a "FontMetrics/GlyphGeometry" layer to unify measurements for consistency.**Proposing a Glyph Pipeline Improvement**

I’m thinking we can create a “minimum coherent MSDF glyph pipeline” based on the msdfgen documentation. The idea is to utilize a single FT face for both shaping and MSDF generation, or at least share FT size metrics to improve efficiency. Currently, the GlyphAtlas has its own FT lifetime and reloads fonts for each glyph, which seems wasteful. A more professional approach would be to modify the GlyphAtlas to accept FT_Face and metrics from the caller, which could enhance consistency, although it would be a more invasive change.**Defining Canonical Glyph Positions**

I’m considering that we could define canonical glyph positions for rendering by finding the design-space bounding box relative to the origin. By using msdf_scale and translate, I could calculate where the MSDF quad should be in relation to pen_x and pen_y while ignoring the FT metrics from the GlyphAtlas. This would let us ensure that “pen_x is the anchor, with MSDF correct around it.” I’ll need to double-check the GlyphMetrics struct and possibly re-examine the glyph_atlas.hpp header to clarify its fields.
我刚才的分析被截断了，接下来会把“字体几何三套度量未统一”的问题完整讲清楚，然后给出一套长期可用的、非临时性的改造方案（包括如何统一坐标系、如何使用 msdf_scale/translate 作为唯一几何来源），并指出需要修改哪些模块。暂时先不动代码，只把设计方案讲透。
**Calculating glyph positioning**

I'm computing the adjustments for each glyph's pen_y by subtracting y_offset. In the Painter, as I build the glyph run, I set baseline_y using the line height. The TextShaper assumes an origin_y of 0, which aligns with default settings when calling TextShaper::shape. My notes mention that the y_offset from HarfBuzz is typically around 26.6, but tends to be 0 for Latin. Consequently, this means all glyphs can maintain the same vertical position, avoiding any jitter.**Analyzing glyph jitter**

I've noticed that the screenshot shows jitter, possibly linked to the glyph's bounding boxes. The phrase "外来歪去" suggests that narrow, tall glyph boxes make letters look individually center-aligned due to cropping. Additionally, the resolution of the screenshot might distort the viewer’s perspective. The spacing also feels off. I initially wondered about rotation or shear, but it doesn't seem to apply here. It looks like the user wants a more theoretical explanation and a professional solution, rather than a quick fix right now.**Planning font metrics implementation**

I’d share that a critical missing element is "字体度量层," as noted in the document. Our current pipeline duplicates font metrics across several modules, lacking a unified representation for consistency. My plan includes designing a `FontMetricsCache` and `GlyphGeometryProvider` to compute and cache values like `units_per_em`, `ascender`, `descender`, and `line_gap` based on font path and size. 

Additionally, I’d implement conversions for layout and atlas pixel sizes. It’s also essential that MSDF generation uses these consistent font-to-atlas metrics, ensuring our text shaping with HarfBuzz follows this efficient structure.**Decoupling scaling and pixel size**

I'm focusing on separating HarfBuzz's internal scaling from pixel size to ensure consistent metrics throughout our pipeline, as recommended in the HarfBuzz documentation. Right now, we set hb_font scale to `font_size*64`, which is tied to pixel size. A more robust solution involves using `hb_font_set_scale(hb_font, units_per_em, units_per_em)` and possibly also `hb_font_set_ppem`. This professional approach unifies everything in font units and converts to pixels only once. 

However, fully implementing this will be challenging, especially since the user prefers a more professional plan and avoids temporary solutions.**Presenting a professional plan**

Since they’ve requested to continue, let’s outline a comprehensive "专业方案". First, we introduce canonical font units into the TextShaper, ensuring it outputs glyph positions in these units. Then, we modify GlyphAtlas to use the same units, recording the transformations to atlas UVs.

Next, in GPUDriverSDL, we will treat pen_x and pen_y as pixels, so we can compute the glyph quad accordingly. 

It's clear I need to present this as a three-phase plan while explaining root causes such as coordinate mismatches and missing elements like "字体度量层." The professional fix involves creating a unified `FontMetricsCache` module for efficient font handling, laying the groundwork for better glyph management.**Implementing structural changes**

To ensure consistent scaling for layout and atlas, I’m planning to adjust the GlyphAtlas. Instead of creating its own FT library, it will reuse the `FontMetricsCache` and load outlines in font units. I propose adding a method, `generateMSDFInFontUnits`, that keeps the outlines consistent with the layout.

For GPUDriverSDL's DrawText, I’ll ignore metrics from GlyphAtlas and instead utilize the canonical metrics from FontMetricsCache, scaling them to pixels with the font size. I'll ensure that each step is structural, aligning with established design in major engines, avoiding any temporary solutions.
好，我这次不再给“调参/补偿”，而是把问题从几何和坐标系的层面拆开，给一套可以长期站得住的专业方案。

一、问题的本质：同一个字形被三套系统各算一遍，没有“统一字体度量层”

现在一条文字从 DOM 到 GPU，会经过三套几何来源：

1）排版几何：HarfBuzz + FreeType（`TextShaper`）

- `TextShaper::shape` 里做的事大致是：

  - 用 FreeType 把 `FT_Face` 设成 `pixel_size = ceil(font_size)`；
  - 建立 `hb_font`，`hb_font_set_scale(font_size * 64)`；
  - 由 HarfBuzz 输出每个 glyph 的：
    - `x_advance / x_offset / y_offset`（26.6 定点 → 像素）；
    - 累加得到一条 glyph run 的 `pen_x` 序列；
  - 再从 `face->size->metrics` 读出 ascent/descender/line_height。

- `Painter` 里再加一层：
  - 用 ascent / line_height 算行 baseline；
  - 把 `ShapedGlyph.pen_x/pen_y` 平移成最终的 `GlyphInstance.pen_x/pen_y`（考虑 padding、text-align）。

→ 这一条链路定义了“语义上的排版几何”：  
哪两个字符之间该隔多远、基线在哪、行高是多少，全靠这里。

2）MSDF 几何：`GlyphAtlas` + 第二套 FreeType + msdfgen

- `GlyphAtlas::generateMSDF` 又开了一套 FreeType：

  - 重新 `FT_Init_FreeType`、`FT_New_Face(font_path)`；
  - `FT_Set_Pixel_Sizes(face, glyph_bitmap_size_)`（16/24/32/48 这类 tier 尺寸）；
  - 用 `FT_Load_Glyph(face, glyph_id, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP)` 读出：
    - `advance_x / bearing_x / bearing_y / width / height` → 存进 `GlyphMetrics`；
  - 把 outline 丢给 msdfgen：
    - `shape.getBounds()` 得到轮廓边界 `bounds.l/r/t/b`；
    - 自己算 `scale`、`translate`，把轮廓塞进 `msdf_size × msdf_size` 的方块并留出 `range`：
      - `scale = ... (ms好，我按你说的，只做“专业原因分析 + 专业长期方案”，不再给任何临时补丁或调参方案。

先总结一句：  
现在这行字之所以“外来歪去 + 间距不自然”，根本原因是：同一套字体几何被 HarfBuzz、FreeType+msdfgen、GPU 绘制 三套系统分别算了一遍，但它们之间缺少一个“统一的字体度量层”和统一坐标系，导致：

- glyph 的基线对齐点在排版和 MSDF 中不是同一个参考点 → 看起来有上下轻微漂移、重心不稳；
- glyph 的 advance / bearing / 实际轮廓宽度来自不同像素尺寸、不同 scale/translate → 左右间距在视觉上“不像浏览器那样自然”。

下面分两部分讲：  
1）更专业的根因拆解。  
2）一套可落地、长期可维护的“字体度量层 + 单一坐标系”方案（不含任何“补偿因子”）。

一、根因拆解：三套度量、四个坐标系没统一

1）排版侧：HarfBuzz + TextShaper（Layout / Shaping 空间）

- 现状（`text_shaper.cpp` + `painter.cpp`）：

  - `TextShaper`：
    - `FT_Set_Pixel_Sizes(face, ceil(font_size))`；
    - `hb_font_set_scale(font_size * 64)`；
    - `hb_shape` 输出：
      - 每个 glyph 的 `x_advance / x_offset / y_offset`（在“font_size 像素空间”里）；
    - `ShapedGlyph.pen_x/pen_y` 用这些 advance/offset 叠加得到。
  - `Painter`：
    - 用 `face->size->metrics` 算 `ascent/line_height`；
    - 组合 padding、text-align 得到 baseline；
    - 把 `ShapedGlyph.pen_x/pen_y` 平移成最终的 `GlyphInstance.pen_x/pen_y`。

- 这套几何定义了“逻辑布局空间”：
  - pen_x 之间的距离 = HarfBuzz 认为的 advance（含 kerning 和 OpenType 特性）；
  - baseline、高度完全由 `FT_Size_Metrics` 决定。

2）MSDF 生成侧：GlyphAtlas + 自己的 FreeType + msdfgen（Atlas 像素空间 + Shape 空间）

- `GlyphAtlas::generateMSDF` 当前的做法：

  - 再单独 `FT_Init_FreeType + FT_New_Face(font_path)`；
  - `FT_Set_Pixel_Sizes(face, glyph_bitmap_size_)`（对应 tier：16/24/32/48 px）；
  - `FT_Load_Glyph(..., FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP)`：
    - 得到 `advance_x / bearing_x / bearing_y / width / height` → 记在 `GlyphMetrics`，单位是“glyph_bitmap_size_ 像素空间”；
  - 把 FT outline 转到 msdfgen：
    - `shape.getBounds()` 得到“设计空间”的包围盒；
    - 计算：
      - `scale`：把轮廓缩放到 `msdf_size - 2*range` 尺寸内；
      - `translate`：让轮廓在方形里居中/留边；
    - `generateMSDF(msdf, shape, range, scale, translate)`；
    - 把 `msdf_scale / msdf_translate_x/y` 存在 `GlyphMetrics`，但渲染时基本没用上。

- 这里额外引入了两个坐标层：

  - “设计空间”：FT/outlines 原始坐标（单位通常是 font units 的线性缩放）；
  - “MSDF 像素空间”：msdfgen 通过 `scale + translate` 把设计空间的轮廓塞进 `msdf_size × msdf_size`；

- 而 `GlyphMetrics.width/height/bearing` 是在“glyph_bitmap_size_ 像素空间”里算的，和 msdfgen 内部的 `scale/translate` 之间没有被显式关联起来。

3）GPU 绘制侧：把两套度量硬拼（Screen 空间）

- 在 `GPUDriverSDL::execute` 的 `DrawText` 分支里，当前逻辑是：

  - 从 `GPUCommand` 拿到：
    - `GlyphInstance.pen_x/pen_y`（排版空间的基线点）；
  - 从 `GlyphAtlas` 拿到：
    - `AtlasEntry.metrics.*`（在 glyph_bitmap_size_ 像素空间的 `bearing/width/height`，还有 `msdf_scale/translate`）；
    - atlas 的 `u0/v0/u1/v1`（MSDF 像素 → 纹理 UV）；
  - 算 quad：

    ```cpp
    float glyph_scale = font_size / base_bitmap; // base_bitmap = tier.bitmap_px

    float glyph_x = glyph.pen_x + entry->metrics.bearing_x * glyph_scale;
    float glyph_y = glyph.pen_y - entry->metrics.bearing_y * glyph_scale;
    float glyph_w = entry->metrics.width  * glyph_scale;
    float glyph_h = entry->metrics.height * glyph_scale;
    ```

  - UV 用的是整块 `u0/v0/u1/v1`，和 `msdf_scale/translate` 没有任何联系。

- 问题出在这一步的“混搭”：

  - pen_x / pen_y：

[]

**Evaluating glyph construction issues**

I’m looking at how misuse of msdfgen's `scale` and `translate` affects glyph quad construction. It seems the MSDF texture's interior isn’t aligning properly with the metrics width, which leads to horizontal misalignment. Additionally, I wonder about atlas packing and flipping—like how copying rows might invert baseline orientation. The screenshot indicates some vertical jitters possibly related to the bearing_y. Finally, I realize we lack a "font metrics layer" to unify layout and GPU coordinate systems, which we definitely need.**Proposing a font metrics solution**

I’m considering a professional solution that involves creating a dedicated `FontMetrics` or `FontFaceMetrics` service. This would serve as a single source of truth for several attributes, like units per EM, scaling from font design units to layout pixels, ascender/descender/line gap, glyph advances, bearings, and mapping to the MSDF atlas. 

Next, I want to unify FreeType (FT) usage by utilizing a single FT_Library and FT_Face for each font path within a `FontCache`. This would be reused for both shaping and MSDF generation, ensuring consistency throughout the process.**Choosing design units**

I think it's best to use design units as the canonical setting because both HarfBuzz's advances and the outlines of shapes essentially rely on these units. I can use FreeType numbers in font units and load the glyphs without scaling. This way, I can compute glyph positions in design units and apply a single scale to convert them into layout pixels and MSDF pixels. This approach should maintain consistent advances and bearings throughout shaping and MSDF processes. However, it's crucial to ensure everything aligns because many modern MSDF pipelines operate in font units, so unifying these practices is key.**Proposing a professional approach**

I’m considering a potential approach that involves using "font design space (em units)" as the canonical units. For each glyph, I plan to use one FreeType font without scaling or hinting to obtain outlines and metrics in font units. Then, I could set the scale in HarfBuzz using `hb_font_set_scale` or by utilizing `hb_ft_font_set_funcs` and relevant load flags so that HarfBuzz operates in design units too. Finally, when shaping text for a specific font size, I’ll multiply all positions by the `font_size / upem` to convert them into layout pixels.**Developing a comprehensive MSDF plan**

I’m outlining a method that involves shaping in design units, multiplying by a specific scale for various processes. When generating MSDFs, I'll utilize msdfgen's functions directly in design units, calculating the appropriate scale to ensure everything aligns properly. For rendering, I’ll compute the required quads using layout positions in pixels and MSDF metrics, ensuring consistent design-space origins across glyphs.

However, implementing this will require a significant refactor, so I need to propose a structured plan rather than a quick fix. The user expects a thorough analysis and a solid plan, so I'll continue working on that.
我先把“字体度量层统一”的分析补完，然后给出一个分步骤、可落地的长期方案，不再做任何“补偿式”小修小补。为满足系统要求，我会顺带读取一下 `doc/font_next.md` 方便之后和你的文档路线对齐。
**Considering a refactor plan**

I’m thinking about how to maintain consistent anchors while following the widely-used pattern from msdfgen docs. It's great they compute a conversion factor for 'em square radius to pixel'. Since the user wants a professional plan, I can describe the process conceptually without exact formulas. Continuing with unfinished tasks likely means completing the analysis rather than just editing code. I’ll suggest a clearly-labeled refactor plan with aligned tasks, considering what’s already in the documentation.
先给一句总的判断：  
现在这行字“外来歪去 + 间距不自然”，已经不是 shader 的问题，而是“字体几何有三套来源、四个坐标系，却没有统一的字体度量层”，导致 glyph 的 baseline、advance、实际轮廓之间在数值上对不上——所以你无论怎么调 range / fwidth，视觉上都会怪。

我按“更专业的原因分析 → 更专业的长期方案”来讲，不给任何补偿式的小修小补。

一、专业原因分析：哪里在算同一件事、却用的是不同几何

1）排版侧（TextShaper + HarfBuzz）：定义“语义几何”

核心代码在 `text_shaper.cpp` + `painter.cpp`：

- TextShaper：
  - `FT_Set_Pixel_Sizes(face, ceil(font_size))`；
  - `hb_font_set_scale(font_size*64, font_size*64)`；
  - `hb_shape` 输出：
    - 每个 glyph 的 `x_advance / x_offset / y_offset`；  
    - 用它们累加得到 `ShapedGlyph.pen_x / pen_y`。
  - 行高、ascent/descender 从 `face->size->metrics` 读出。

- Painter：
  - 用这些 metrics 算出 baseline（`baseline_y`）和行宽；
  - 把 `ShapedGlyph` 平移成 `GlyphInstance.pen_x/pen_y`（加 padding / text-align 等）。

这套数据定义了“语义上的排版几何”：  
哪两个字之间应该隔多远、baseline 在哪、行距多大，全靠这里。

2）MSDF 生成侧（GlyphAtlas + FreeType + msdfgen）：定义“纹理几何”

在 `glyph_atlas.cpp` 的 `generateMSDF`：

- 再单独初始化一套 FreeType：
  - `FT_Init_FreeType`；
  - `FT_New_Face(font_path)`；
  - `FT_Set_Pixel_Sizes(face, glyph_bitmap_size_)`（由 tier 决定，16/24/32/48 等）。
- 用 `FT_Load_Glyph(face, glyph_id, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP)` 读出：
  - `advance_x, bearing_x, bearing_y, width, height` → 写到 `GlyphMetrics`（单位 = glyph_bitmap_size_ 像素）。
- 把 FT outline 丢给 msdfgen：
  - `shape.getBounds()` 拿到轮廓设计空间的 `bounds.l/r/t/b`；
  - 自己算 `scale`、`translate`：
    - 让轮廓塞进 `msdf_size × msdf_size` 的方形并留 `range` 边；
  - `generateMSDF(msdf, shape, range, scale, translate)`；
  - 把 `msdf_scale / msdf_translate_x/y` 填回 `GlyphMetrics`。

此时，同一个 glyph 已经有两套互相独立的度量：

- 一套是 HarfBuzz 那边在“font_size 像素空间”里算的 advance/offset；
- 一套是 GlyphAtlas 在“glyph_bitmap_size_ 像素空间 + msdfgen 内部设计空间”里算的 width/bearing/scale/translate。

这两套之间没有任何“数学上严格的对应关系”被保存下来。

3）GPU 绘制侧（GPUDriverSDL）：把两套度量直接硬拼

在 `gpu_driver_sdl.cpp` 的 `GPUCommandType::DrawText` 分支：

- 从 `GPUCommand` 拿到：
  - `GlyphInstance.pen_x/pen_y`（来自 HarfBuzz 的排版空间基线点）。
- 从 `GlyphAtlas` 拿到：
  - `AtlasEntry.metrics`（glyph_bitmap_size_ 像素空间里的 `bearing/width/height` 和 `msdf_scale/translate`）；
  - `u0/v0/u1/v1`（atlas UV，只反映“这块 MSDF 图贴在纹理哪个矩形”，并不知道它在设计空间实际宽多少）。
- 再算 glyph 矩形：

  ```cpp
  float glyph_scale = font_size / base_bitmap; // base_bitmap = tier.bitmap_px

  float glyph_x = glyph.pen_x + entry->metrics.bearing_x * glyph_scale;
  float glyph_y = glyph.pen_y - entry->metrics.bearing_y * glyph_scale;
  float glyph_w = entry->metrics.width  * glyph_scale;
  float glyph_h = entry->metrics.height * glyph_scale;
  ```

- 然而：
  - `glyph.pen_x` 的 advance 是 HarfBuzz 在一套 FreeType 状态下算出来的；
  - `entry->metrics.width/bearing` 是另一套 FreeType 状态（不同 pixel size）的结果；
  - `msdf_scale/translate` 更是被完全忽略了，只在生成时用了、渲染时没有参与任何几何计算。

专业上想清楚，这就意味着：

- 水平方向：
  - “pen_x 到 pen_x+advance 的距离”来自 HarfBuzz；
  - 但“可见轮廓占多宽”来自另一个像素尺寸的 `width/bearing`，再乘一个经验式的 `glyph_scale = font_size/base_bitmap`。
  - 再加上 msdfgen 的 scale/translate 可能在 X 方向上“挤压/扩展”轮廓，导致贴图区域和 metrics 不再严格对齐。
  → 结果就是：  
    - 有的字实际轮廓比 advance 窄一点 → 间距肉眼偏大；  
    - 有的字轮廓又被 scale 拉宽了一些 → 局部有挤压感；  
    - 组合起来就是你看到的“间距总觉得不对”。

- 垂直方向：
  - baseline / ascent 来自 `TextShaper` 一套 metrics；
  - `bearing_y / height` 来自另一套 pixel size；
  - atlas 上传时还倒了一次行（`src_row = glyph_height - 1 - row`），坐标上下翻转；
  - shader 并不知道“设计空间的 0, baseline 在哪”，只是把整个 tile 当成某个矩形画出来。
  → 结果就是：
    - 有的字在 baseline 上略往上浮，有的略往下掉；  
    - 轮廓重心相对矩形不一致 → 视觉上的“外来歪去”。

总结：  
你现在的管线，本质上是“用 HarfBuzz 决定 glyph 顺序 + 粗略位置，再用一套相对独立的 MSDF metrics 去填每个格子，中间靠经验 scale 拼起来”，缺少一个“统一的字体度量层”。  
在这种前提下，想靠调一两个参数把字排得像浏览器一样自然，是做不到的。

二、专业方案：建立统一的“字体度量层”，只认一个坐标系

下面是一套长期方案，不含任何“临时补偿系数”，思路是：  
选定一个唯一的“字体几何坐标系”，所有 HarfBuzz / FreeType / msdfgen / GPU 都围绕它来工作。

1）确定统一坐标系：字体设计单位（font design units）

- 选择“字体 design units（FT face 的 units_per_EM 空间）”作为唯一的几何源：
  - HarfBuzz 的 advance/kerning 用 design units 表示；
  - FreeType outline、metrics 用 design units 表示；
  - msdfgen 输入的 shape 直接用 design units；
  - 最后通过一个 `font_scale = font_size_px / unitsPerEM` 把 design units 映射到屏幕像素。

好处：  
- 所有几何关系（advance、bearing、轮廓包围盒）在一个线性空间内定义；  
- 不再依赖“这个 glyph_bitmap_size_ 时测出来的 metrics”。

2）统一 FreeType 使用方式：一个 FT_Face，多处共用

- 做一个 `FontFaceCache` / `FontMetricsManager`：

  - 全局一个 `FT_Library`，不要在 `GlyphAtlas::generateMSDF` 里重复 `FT_Init_FreeType`；
  - 每个 `font_path` 对应一个复用的 `FT_Face`，由这个 manager 负责生命周期（你已有类似逻辑，但分散在 `TextShaper` 和 `GPUDriverSDL` 内）。

- 约定一个“design face 状态”：
  - 使用 `FT_Load_Glyph(face, glyph_index, FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP)`；
  - 读 metrics/outlines 时，得到的全部是 design units（与 pixel size 无关）。

3）让 HarfBuzz 也工作在 design units，而不是当前的“像素空间”

- 创建 `hb_font` 时：

  - 使用共享的 `FT_Face`；
  - `hb_ft_font_set_funcs(hb_font)`；
  - 设置：
    - `hb_font_set_scale(hb_font, unitsPerEM, unitsPerEM)`；
    - `hb_ft_font_set_load_flags(hb_font, FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP)`。

- 这样，`hb_shape` 输出的 `x_advance / x_offset / y_offset`，就是 design units；  
- 你的 `ShapedGlyph.pen_x/pen_y` 也就天然在设计空间里。

- 对于 layout / Painter 层：
  - 把 glyph run 的几何从 design units 转到“布局像素”：
    - `float font_scale = font_size_px / unitsPerEM;`
    - `pen_x_px = pen_x_design * font_scale;`
    - `ascent_px / descent_px / line_height_px` 也由 `FT_Size_Metrics` 的 design units 乘以 `font_scale` 得到。

→ 这一步完成后，“排版几何”和“字体基本度量”已经统一到 design units 上，再线性缩放到像素。

4）MSDF 生成：以 design units 为输入，显式记录“设计空间 → MSDF 像素”的仿射变换

在 `GlyphAtlas::generateMSDF` 中，改为：

- 不再单独建 FT_Library / Face，而是从 FontMetricsManager 取得同一个 `FT_Face`；
- 对这个 face 使用 `FT_Load_Glyph(..., FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP)`；
- 构造 msdfgen::Shape 时直接用 design units（当前你已经基本这样做，只是 metrics 在另一套 pixel size 下读的）；
- 关键是这两点：

  1) 所有 `GlyphMetrics` 字段改成 design units：
     - `advance_x_design`、`bearing_x_design`、`bearing_y_design`、`width_design`、`height_design`；
  2) 认真保存 msdfgen 的仿射变换：
     - `msdf_scale_design_to_px`、`msdf_translate_design_to_px`，它定义了：
       - `msdf_pixel_xy = design_xy * msdf_scale + msdf_translate`。

- atlas tile 在纹理里的位置用像素记录（tile_x_px / tile_y_px / tile_size_px）：
  - 然后可以算出“设计空间 → 纹理 UV”的全局变换（而不是只存 u0/u1）。

这样，每个 glyph 具备两条严格的变换链：

- design_units → layout_pixels（乘以 font_scale）；
- design_units → msdf_pixels → UV（乘以 msdf_scale 再加 translate，再除以 atlas 尺寸）。

5）GPU 绘制：以“design origin + 仿射变换”来算 quad，不再拼凑 bearing/width

在 `GPUDriverSDL::DrawText` 中重构几何：

- GPUCommand 里不再传“已经是像素单位的 `GlyphInstance.pen_x`”，而是：

  - 传 design units：
    - `glyph_pen_x_design` / `glyph_pen_y_design`；
  - 或者传“排版层已算好的像素 + 一个统一的 font_scale”，两种都行，我推荐前者更干净。

- 对每个 glyph：

  - 确定设计空间的原点：
    - 通常是 glyph baseline 左边（Harfbuzz/FT 的 glyph origin）。
  - 从 `GlyphMetrics` 拿到该 glyph 轮廓在设计空间的包围盒 `bbox_design`（你可以在 msdfgen `shape.getBounds()` 后保存）。
  - 用“设计空间 → 布局像素”的 `font_scale` 算出屏幕上的矩形：

    - `quad_x_px = glyph_pen_x_design * font_scale + bbox_design_min_x * font_scale;`
    - `quad_y_px = glyph_pen_y_design * font_scale - bbox_design_max_y * font_scale;`
    - `quad_w_px = (bbox_design_max_x - bbox_design_min_x) * font_scale;`
    - `quad_h_px = (bbox_design_max_y - bbox_design_min_y) * font_scale;`

  - 同时用“设计空间 → MSDF 像素 → UV”的仿射变换算 UV：
    - 对 bbox 的四个顶点套 `msdf_pixel = design * msdf_scale + msdf_translate`，再除以 atlas 尺寸得到 `u/v`；
    - 这样 quad 的几何和采样区域在设计空间是一致的。

- 这样一来：

  - HarfBuzz 的 advance/offset 决定的是 design origin 的位置；
  - 轮廓包围盒决定的是 quad 相对于 origin 的偏移；
续上：

- 这样一来：

  - HarfBuzz 的 advance/offset 决定的是“glyph 设计原点”的位置（baseline 上的 pen）；
  - msdfgen 的 bbox + 仿射变换决定的是“轮廓相对这个原点”的精确范围；
  - 两者都在同一个 design-units 空间里，只是最后分别走：
    - design → layout_px（font_scale）；
    - design → atlas_uv（msdf_scale + translate）。

→ 结果：  
- 水平方向：advance 与实际轮廓宽度是同一套度量下算出来的，间距会自然地跟 browser / Skia 对齐；  
- 垂直方向：baseline / bearing_y / bbox 都在同一空间里，glyph 不会再一上一下“外来歪去”。

6）字体度量层：把这些逻辑集中成“唯一真相”

为了不把逻辑散在 `TextShaper`、`GlyphAtlas`、`GPUDriverSDL` 三个类里，建议引入一个专门的“字体度量层”，例如：

- `FontFaceMetrics`（按 font_path）：
  - 保存：
    - `unitsPerEM`；
    - `ascender_design / descender_design / lineGap_design`；
    - 统一的 `FT_Face` 句柄；
- `GlyphDesignMetrics`（按 font_path + glyph_id）：
  - `advance_x_design`；
  - `bearing_x_design / bearing_y_design`；
  - `bbox_design`；
  - `msdf_scale / msdf_translate`；
  - `atlas_tile_rect_px`（x, y, w, h）。

- 然后：
  - TextShaper 只依赖 `FontFaceMetrics` 和 design-units 的 HarfBuzz；
  - GlyphAtlas 只负责：
    - 把 `GlyphDesignMetrics` 对应的 glyph 用 msdfgen 转成位图；
    - 把 `msdf_scale/translate + atlas_tile_rect` 写进去；
  - GPUCompiler / GPUDriver：
    - 按“设计空间 → layout_px / atlas_uv”两个变换来画 quad。

这样，你的“字体度量层”就成为整个 pipeline 的唯一真相源（single source of truth），和 `doc/next.md` 里提到的“字体度量层 + GlyphRun”是一致的。

三、这套方案的特点（为什么它是“专业而不是临时”）

- 不再有任何“经验系数”：
  - 不靠 `glyph_scale = font_size / tier_bitmap_px` 这种粗算；
  - 不靠额外的 tracking/hack 来弥补间距；
  - 所有位移、宽度、行高都来源于同一套 design-units 度量。

- 与业界成熟实现同构：
  - HarfBuzz 官方推荐的做法就是“在 design units 中 shape，然后乘一个 scale 到像素”；
  - msdfgen 本身就是在 design space 上工作的，`scale/translate` 就是“设计 → 像素”的仿射；
  - 你把这些信息串起来，就能和 Ultralight / Skia 的内部逻辑对齐，而不是做一个“像”的版本。

- 方便以后做更多专业特性：
  - 变更字号，只改 `font_scale`；
  - 做变体（bold/italic）、脚本切换（CJK/RTL）、行内 math，都可以在 design space 里统一处理；
  - MSDF atlas 分级（16/24/32/48）也只影响“设计 → MSDF 像素”的那一段，不会破坏排版几何。

四、后续落地建议（不是补丁，是重构顺序）

如果你准备按这个方向改，建议顺序是：

1）抽象出 `FontFaceMetrics` + 统一 `FT_Face` 管理；
2）把 HarfBuzz 切换到 design-units 模式（NO_SCALE + unitsPerEM scale）；
3）把 `GlyphAtlas::generateMSDF` 改为也在 design-units 上读 metrics，并保存 `bbox_design + msdf_scale/translate`；
4）重写 `DrawText` 的几何部分，让 quad 完全依赖：
   - `pen_x_design / pen_y_design`；
   - `bbox_design`；
   - `font_scale`；
   - `msdf_scale/translate + atlas_tile_rect`；
5）删掉现有的 `glyph_scale = font_size / base_bitmap` 和“第二套 FreeType metrics”。

做到这一步后，即使你暂时不做任何“调粗/调锐”之类的观感微调，文字的对齐感、稳定感、字间距都会大幅接近浏览器/Skia 的效果——因为本质上你也在用同一类几何。

如果你愿意下一步就开始实现，我可以按这个方案，把“先切 HarfBuzz 到 design units + 抽出 FontFaceMetrics”这一小步拆成具体代码改动清单，再逐步改 GlyphAtlas 和 GPUDriver。