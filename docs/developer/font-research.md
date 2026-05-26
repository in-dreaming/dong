# 字体渲染方案调研：`3rd/gpu-font-rendering` 与 Slug（`3rd/C4-Engine`）

> 目标：梳理本仓库中两套 GPU 字体/矢量图形渲染方案的核心思路、数据流与特性，为后续在 gweb 项目中的选型与集成提供参考。
https://github.com/GreenLightning/gpu-font-rendering/tree/master

---

## 1. `3rd/gpu-font-rendering` 方案

### 1.1 整体思路概述

- 基于 FreeType 解析字体轮廓，直接在 GPU 上按矢量轮廓做精确填充，而不是预先 CPU 光栅化到位图。
- 算法来源：
  - Will Dobbie《GPU text rendering with vector textures》
  - 结合 Eric Lengyel Slug 相关论文中的一些思路，但刻意没有实现 Slug 的专利算法（readme 中有明确说明）。
- 基本流程：
  1) CPU 端：
     - 使用 FreeType 解析 TrueType / OpenType 字体轮廓。
     - 将每个字形的轮廓拆分成若干二维二次贝塞尔曲线（quadratic Bezier）。
     - 所有曲线线性排布到一个 buffer 中，记录每个 glyph 对应的曲线区间。
  2) GPU 端：
     - 每个字形画一个四边形（quad）。
     - 片元着色器在该 quad 内，对当前像素发射一条（或两条）射线，与所有相关贝塞尔曲线求交，计算缠绕数 / 覆盖率，得到抗锯齿的 alpha。
- 适用场景：
  - 大字号文字、3D 场景中任意变换的文本、需要锐利边缘、对纹理放大有较高质量要求的场景。

### 1.2 CPU 端字形预处理

核心逻辑在 `Font` 类（`source/font.cpp`）中：

- 字体加载：
  - `Font::loadFace(FT_Library, filename)`：包装 `FT_New_Face`，并检查字体是否可缩放（`FT_FACE_FLAG_SCALABLE`）。
- 构造 `Font(FT_Face face, float worldSize, bool hinting)` 时：
  - 根据是否启用 hinting 决定 FreeType 的加载模式：
    - `hinting == true`：
      - `loadFlags = FT_LOAD_NO_BITMAP`，使用像素尺寸 + hinting；
      - `kerningMode = FT_KERNING_DEFAULT`；
      - `emSize = worldSize * 64`，并 `FT_Set_Pixel_Sizes` 设置像素大小。
    - `hinting == false`：
      - `loadFlags = FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP`；
      - `kerningMode = FT_KERNING_UNSCALED`；
      - `emSize = face->units_per_EM`（字体单位）；
  - 为渲染分配 GPU 资源：
    - 顶点数组/缓冲：`vao/vbo/ebo`。
    - 纹理缓冲：`glyphTexture`、`curveTexture`。
    - 对应的缓冲对象：`glyphBuffer`、`curveBuffer`。

- 主要数据结构：
  - 按字形：
    - `Glyph`（逻辑层，存放在 `std::unordered_map<uint32_t, Glyph> glyphs` 中）：
      - `index`：FreeType glyph index。
      - `bufferIndex`：指向 buffer 中这个 glyph 的曲线段范围。
      - `curveCount`：曲线数量。
      - 一组以 em 为单位的 metrics：宽、高、bearingX/bearingY、advance。
  - GPU 侧 buffer 格式：
    - `BufferGlyph { int32_t start, count; }`
      - 当前 glyph 对应的曲线在 `bufferCurves` 中的起始索引与数量。
    - `BufferCurve { float x0,y0,x1,y1,x2,y2; }`
      - 单条二次贝塞尔曲线三个控制点的坐标（已除以 `emSize`，变成 em 相对坐标）。
    - `BufferVertex { float x,y,u,v; int32_t bufferIndex; }`
      - 每个顶点的屏幕空间坐标 `(x,y)`、字形局部坐标 `(u,v)`（同样是 em 相对），以及 `bufferIndex` 用于在着色器中找到该 glyph 对应的曲线范围。

- 轮廓转换 `convertContour(...)`：
  - 输入：FreeType 的 `FT_Outline` 中某个 contour 的点序列 `[firstIndex, lastIndex]`。
  - 主要处理逻辑：
    - 将 FreeType 点及标签 (`FT_CURVE_TAG_ON / CONIC / CUBIC`) 转成一系列二次贝塞尔：
      - TrueType 只使用 CONIC（quadratic）与 ON 点；CFF（PostScript）轮廓可能使用 CUBIC（三次贝塞尔）。
      - 对线段 `ON-ON`：用中点作为虚拟控制点，转成二次贝塞尔。
      - 对二次贝塞尔 `ON-CONIC-ON` / `ON-CONIC-CONIC-ON`（含虚拟点）的情况做展开。
      - 对三次贝塞尔 `ON-CUBIC-CUBIC-ON`：
        - 使用论文《Quadratic Approximation of Cubic Curves》中的方法，将一条 cubic 拆成两条 C¹ 连续的 quadratic，以保证端点与切线匹配，误差可控。
    - 自动处理轮廓方向（根据 `FT_OUTLINE_REVERSE_FILL` 决定遍历方向），确保 winding 约定正确。
  - 最终得到：
    - 每个 contour 对应若干 `BufferCurve`，顺序 push 到 `curves`（即 `bufferCurves`）。

- 字形构建 `buildGlyph(charcode, glyphIndex)`：
  - 记录当前 `bufferCurves.size()` 为 `bufferGlyph.start`。
  - 遍历 glyph 的每个 contour，调用 `convertContour` 填充 `bufferCurves`。
  - `bufferGlyph.count = bufferCurves.size() - start`。
  - 将 `bufferGlyph` push 到 `bufferGlyphs`，并记录其索引为 glyph 的 `bufferIndex`。
  - 同时从 `face->glyph->metrics` 读出 metrics，存入 `Glyph` 结构并放入 `glyphs[charcode]`。

- 运行时动态加载字形：
  - `prepareGlyphsForText(const std::string& text)`：
    - 遍历 UTF‑8 字符串（通过 `decodeCharcode`），对未在 `glyphs` 中出现的字符，调用 FreeType 加载并 `buildGlyph`；如果有新增 glyph，就整体重新 `uploadBuffers()`。

- buffer 上传 `uploadBuffers()`：
  - `glyphBuffer`：存 `BufferGlyph` 数组，绑定成 `GL_TEXTURE_BUFFER` 并 `glTexBuffer(GL_RG32I, glyphBuffer)`。
  - `curveBuffer`：存 `BufferCurve` 数组，绑定成 `GL_TEXTURE_BUFFER` 并 `glTexBuffer(GL_RG32F, curveBuffer)`。

### 1.3 GPU 渲染与抗锯齿

- 顶点阶段：
  - `Font::draw(x,y,text)` 中：
    - 对字符串逐字遍历，考虑 kerning（`FT_Get_Kerning`）；
    - 对每个非空 glyph（`curveCount > 0`）构建一个 quad：4 个 `BufferVertex` + 6 个索引；
      - `(u0,v0,u1,v1)` 由 glyph metrics 和 `dilation` 得出；
      - `(x0,y0,x1,y1)` = `(x,y)` + `(u,v) * worldSize`；
      - `bufferIndex` = 该 glyph 在 `bufferGlyphs` 中的索引。
    - 顶点缓冲更新：
      - `glBufferData(GL_ARRAY_BUFFER, vertices)` / `glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices)`，然后 `glDrawElements(GL_TRIANGLES, ...)`。
  - VAO 中顶点属性设置：
    - 位置 `location 0`：`vec2(x,y)`。
    - 字形本地坐标 `location 1`：`vec2(u,v)`。
    - glyph 索引 `location 2`：`int bufferIndex`（`glVertexAttribIPointer`）。

- 片元阶段（来自 `shaders/font.*`，逻辑在 readme 已详细推导）：
  - 数据输入：
    - uniform：
      - `samplerBuffer glyphs` —— 存每个 glyph 的 `(start,count)`。
      - `samplerBuffer curves` —— 存所有曲线控制点。
      - `antiAliasingWindowSize`（窗口宽度，单位像素）。
      - `enableSuperSamplingAntiAliasing` （是否启用第二条射线）。
      - `enableControlPointsVisualization` （调试用）。
    - varying：从 VS 传入的 `(u,v)` 和 `bufferIndex`。
  - 基本算法：
    1) 根据 `bufferIndex` 从 `glyphs` 纹理中取出当前 glyph 对应的曲线区间 `[start,count]`。
    2) 对当前像素中心在 glyph 局部坐标中的位置 `p`：
       - 以 `p` 为原点，沿 +x 方向发一条射线；
       - 遍历 glyph 的所有曲线：
         - 对每条二次曲线，代入 `y=0`，得到一元二次方程并求解 `t`；
         - 过滤出 `0 ≤ t < 1` 且对应的 `x ≥ 0` 的交点；
         - 根据导数符号或几何关系，判断是“进入”还是“退出”填充区域，更新 winding 数。
    3) 根据最终 winding 数（非零则在内部）计算 coverage：
       - 使用一个横向像素宽度的 1D 窗口，对靠近边界的交点按线性权重计算覆盖面积，实现子像素抗锯齿。
       - 如果启用 `enableSuperSamplingAntiAliasing`，再沿 y 方向做一遍类似的射线求交，组合成更接近 2D 的 coverage。
    4) 输出：
       - 使用预乘 alpha（`GL_ONE, GL_ONE_MINUS_SRC_ALPHA`）进行混合。

- 很多数学细节在 `readme.md` 中有完整推导（求交、根的分类、导数符号与 entry/exit 关系等），实现与文档是一致的。

### 1.4 特性与优缺点

- 优点：
  - 结果真正分辨率无关，可在任意缩放下保持锐利的边与顶点。
  - 直接从轮廓渲染，不需要 CPU 端生成位图 atlas，也不需要为不同字号重复生成纹理。
  - 实现相对简洁：
    - CPU 端只是把轮廓转为二次贝塞尔并线性打包；
    - GPU 端通过简单纹理缓冲 + 单一 fragment shader 即可；
    - 支持动态加载字形（`prepareGlyphsForText`）。
  - 支持 TT、CFF 混合的轮廓数据，自动把三次贝塞尔用二次近似；对一般字体效果良好。

- 限制：
  - 不包含复杂排版：OpenType GPOS/GSUB、高级连字、变体、脚标/上标、emoji 多图层等都不处理，只依赖 FreeType 的基础 kerning。
  - 性能：
    - 单个像素可能需要与很多曲线做求交；大段长文本或极复杂字体在高分辨率下，片元成本较高。
    - 没有 Slug 那种“banding” 加速结构，所有曲线都线性遍历。
  - 效果：
    - 抗锯齿主要基于 1D coverage + 可选第二条射线，理论上比真正 2D 精确覆盖略弱，但实际观感已经很好。
  - 法律风险较低：
    - 参考了 Slug 文献，但没有实现其专利的核心 banding 算法；readme 中也 explicit 声明 Slug 算法未实现。

---

## 2. Slug 方案（`3rd/C4-Engine` 集成）

### 2.1 概览

- Slug 是 Eric Lengyel / Terathon Software 开发的 GPU 字体渲染库，在本仓库中以源码形式存在（`SlugCode/`）。
- 版本：7.2；头文件中注明：
  - 受美国专利 #10373352 保护：
    - “Method for rendering resolution-independent shapes directly from outline control points”。
  - 文件内容为专有、保密，仅授权用户可使用。
- C4 Engine 通过 `EngineCode/C4Slug.*` 与 Slug 紧密集成，用于：
  - 文本字体（`Font` / `TextLayout`）。
  - 图标与矢量图片（`Album` / `IconLayout` / `PictureLayout`）。
- Slug 提供了完整的“从 Font/ SVG 到 GPU 渲染”的一站式管线：
  - 字体解析（OpenType/OTF/TTF）。
  - OpenType 排版特性（GSUB/GPOS/GDEF 等）。
  - 高级布局（多段、多字体、双向文本、段落属性）。
  - GPU 曲线/带纹理结构 + fragment shader 实时求交。
  - 多色字形、渐变、描边、光学笔画加粗等效果。

### 2.2 字体/图形数据准备管线

Slug 的离线/CPU 管线分为几层：

1) OpenType/字体解析 —— `SLOtfReader.h`

- 大量结构体镜像 OpenType 各种表：
  - `OpenTypeFontDirectory`, `OpenTypeTableHeader`, `OpenTypeCmapHeader`, `OpenTypeNameTable` 等。
  - `OpenTypeGposGsubHeader` + 各种 Coverage / ClassDef / PairPos / Ligature / Sequence / Alternate 等结构。
  - `OpenTypeGlyphHeader`（glyph 轮廓）、`OpenTypeKernHeader`（kerning）、`OpenTypeColorHeader`（COLR/CPAL 彩色字体）等。
- `OpenTypeReader`：
  - 持有字体文件的内存映射（`openTypeFileData`）。
  - 提供一系列构建函数：
    - `ProcessFontHeaders` / `ProcessNameTable` / `ProcessMetrics`。
    - `BuildCharacterMap` / `BuildAdvanceStructures`。
    - `BuildCaretStructures` / `BuildKernStructures`。
    - `BuildAlternateStructures` / `BuildSequenceStructures`。
    - `BuildAnchorStructures` / `BuildEmojiStructures`。
    - `BuildGlyphPathStructures`：把 glyph 轮廓解析成曲线数据。
  - 内部根据 glyph 类型选择 TrueType 或 PostScript 路径：
    - `BuildTrueTypeGlyph` / `BuildPostScriptGlyph`。
    - TrueType 通过 `FT_Outline` 风格的数据结构；CFF 通过内置的 `CFFInterpreter` 解释 CharString 指令（`CFFCharStringInterpreter`）。

2) OpenVex 中间表示 —— `SLOpenVex.h`

- OpenVex（Open Vector Exchange）是 Slug 定义的一种高层矢量描述格式，基于 `TSOpenDDL` 和 `TSGraph`：
  - 结构类型枚举：`kStructureCanvas`, `kStructureLayer`, `kStructureTypography`, `kStructureFont`, `kStructureGlyph`, `kStructureLine/Rect/Circle/Path` 等，表示画布、图层、几何体、字体、字形、排版规则等。
  - `OpenVexStructure` 及其子类：
    - 几何层：`GeometryStructure` + `LineStructure` / `RectStructure` / `CircleStructure` / `PolygonStructure` / `PathStructure`，内部统一用二次贝塞尔曲线 `QuadraticBezier2D` + `SubpathData` 描述轮廓，并维护 `boundingBox`。
    - 样式层：`FillStructure` / `StrokeStructure` / `ColorStructure` / `GradientStructure` / `OpacityStructure` / `DashStructure` 等，用于填充规则、描边、渐变、虚线等。
    - 排版层：`FontStructure` / `GlyphStructure` / `CharMapStructure` / `KernStructure` / `AlternateStructure` / `SequenceStructure` / `DecomposeStructure` / `AnchorStructure` / `EmojiStructure` 等，包含了字体的各种 OpenType 特性和关系图。
  - `OpenVexDataDescription`：
    - 管理一个 OpenVex 文档：`canvasBox`, `geometryScale`, `FontPolygonData`, `FontOutlineData` 等。
    - 负责把所有 `FontStructure`、`GlyphStructure` 聚合在一起，后续交给 `FontBuilder` 构建 SlugFile。

3) SVG 输入 —— `SLSvgReader.h`

- `SvgReader`：
  - 解析 SVG 文本，构造 OpenVex 几何结构：
    - 直线、矩形、圆、多边形、路径等元素转换为 `GeometryStructure` + 二次贝塞尔曲线数组；
    - 填充/描边/渐变等属性转换为对应的 `FillStructure`、`StrokeStructure`、`GradientStructure` 等。
  - 主要用于构建图标/矢量图片（Slug 的 album/picture 模块）。

4) 字体构建 —— `SLFontBuilder.h`

- `FontBuilder` 的职责：
  - 入口：`BuildFont(SlugFile *slugFile, const void *fileHeader, int32 fontIndex)`。
  - 将 `OpenVexDataDescription` 中的 `FontStructure`/`GlyphStructure` 转换为最终的 `SlugFile` 数据：
    - 为所有 glyph 建立曲线表与 band 数据：
      - `BuildExpandedGlyphCurveTable`：从 glyph 的基础曲线转为内部的、可能展开的曲线列表。
      - `CalculateGlyphBoundingBox` / `CalculateBandCounts` / `BuildGlyphBandTable` / `BuildBandRemapTable`：
        - 计算每个 glyph 的包围盒，按 x/y 方向划分 band，记录每个 band 与哪些曲线相交。
      - `GenerateGlyphCurveTexels`：在一张 curve texture 上写入所有曲线控制点坐标（`Texel16` 或 float 格式）。
      - `GenerateGlyphBandTexels`：在 band texture 上写入 band 数据（每个 band 包含曲线数量 + 指向曲线索引列表的偏移），支持重映射与压缩；
      - `GenerateHorizontalStrokeTexels`：特殊处理水平描边等情况。
    - 构建 glyph 级别的各种数据：
      - `GenerateCurveData`、`GenerateCaretData`、`AddGlyphGroup` 等。
      - kerning、分解、序列、替代、装饰（下划线、删除线）等：`BuildFontKernData` / `BuildFontDecomposeData` / `BuildFontAnchorData` / `BuildFontSequenceData` / `BuildFontAlternateData`。
    - 构建字体全局元数据：
      - `CalculateBoundingBoxData` / `BuildGlyphIndexTable` / `BuildFontKeyData`。

- 小结：Slug 的字体构建链路非常重，基本把 OpenType 的绝大多数高级特性完整映射到自己的 OpenVex + SlugFile 数据结构中，然后预烘焙出适合 GPU 求交的曲线纹理和 band 纹理。

### 2.3 GPU 数据表示与渲染算法

GPU 端主要通过 `SLVertexShader.h` / `SLFragmentShader.h` 提供的一组通用着色器函数实现：

1) 纹理布局

- `curveData`（CurveSampler）：
  - 存储所有曲线控制点和附加数据，使用 float16/float32 的 RGBA 纹理格式：
    - 每条曲线通常占用两个 texel：
      - 第一个 texel：`p1.x, p1.y, p2.x, p2.y`；
      - 第二个 texel：`p3.x, p3.y, ...`，并可能存储其它信息（例如渐变参数、图层色等）。
  - 坐标通常已经转换到“每 Em”空间，并且相对像素采样点偏移，方便在 shader 中直接做多项式求解。

- `bandData`（BandSampler）：
  - 存储 band 带信息和对 curveData 的索引，类型是 `usampler2D` 或整数纹理：
    - 对于某个 glyph：
      - 一行或多行 texel 描述这个 glyph 的所有横向 band（hband）和纵向 band（vband）。
      - hband/vband texel 中包含：
        - `count`：该 band 中的曲线数量；
        - `offset`：在 bandTexture 中进一步索引曲线位置的偏移。
    - 函数 `CalcBandLoc`：根据 glyph 的 band 基址 + offset + 全局纹理宽度（`kLogBandTextureWidth`）计算 band 在纹理中的 `(x,y)` 坐标。

- `glyphData`：
  - 作为 vertex attribute 被打包进 `int4`，经 `SlugUnpack` 解码：
    - `glyphData.xy`：glyph 在 bandData 中 band 行的基址。
    - `glyphData.zw`：band 计数、flag 等；部分位用于 encode 渐变类型、填充规则、stroke 相关信息。

2) 顶点着色器辅助函数 —— `SLVertexShader.h`

- `SlugUnpack(float4 tex, float4 bnd, out float4 vbnd, out int4 vgly)`：
  - 从顶点属性中拆出 band 变换参数与 glyphData（通过 `floatBitsToUint`/`asuint` 还原 `uint2`）。

- `SlugDilate(...)`：
  - 用于描边 / 光学加粗等情况，将字形 quad 进行“膨胀”：
    - 接收原始位置与法线、Jacobian 等；
    - 计算合适的偏移 `d`，输出新的顶点位置 `vpos` 和在纹理中的采样坐标；
    - 支持非等距缩放与复杂投影下的等宽描边。

3) 片元着色器核心 —— `SLFragmentShader.h`

主要函数：`RenderGlyph` 和 `SlugRender`。

- 根分类逻辑：
  - `CalcRootCode` / `TestCurve` / `TestRoot1` / `TestRoot2`：
    - 通过对 `y1, y2, y3`（或者 `x1, x2, x3`）做位运算/查表判断曲线与射线是否相交以及交点数量；
    - 在 `SLUG_LOP3` 环境可使用更高效的 bit trick，否则使用普通 `asuint` + 移位组合。

- 解多项式：
  - `SolveHorizPoly(float4 p12, float2 p3)`：
    - 针对 `y=0` 的情形，计算射线与二次曲线的两个交点在 x 方向的坐标（已经除以像素尺寸，转换到“每像素的 Em 单位”）。
    - 内部显式写出 a/b/d 判别式与退化处理（`|a|` 很小时退化为线性）。
  - `SolveVertPoly` 对 y 方向射线进行同样操作。

- band 遍历与 coverage 计算（简化描述）：

  1) 横向 band：
     - 从 bandData 中取出当前像素所属的横向 band：
       - `bandIndex.y = clamp(int(renderCoord.y * bandTransform.y + bandTransform.w), 0, bandMax.y)`；
       - `hbandData = TexelLoad2D(bandData, glyphLoc + bandIndex.y)`。
     - `hbandData.x` 是曲线数量，`hbandData.y` 是 offset；通过 `CalcBandLoc` 得到曲线索引所在的区域 `hbandLoc`；
     - 对每个曲线：
       - 从 bandData 再取出曲线在 curveData 中的位置 `curveLoc`；
       - 从 curveData 取出 `p12`、`p3`，并减去 `renderCoord` 以把采样点移到原点；
       - 早退剪枝：如果所有控制点在 x 方向“足够远”则 break；
       - 利用 `CalcRootCode` 判定是否可能有交点；
       - 调 `SolveHorizPoly` 得到两个可能的根 r.x、r.y，乘以 `pixelsPerEm.x` 转换为像素单位；
       - 对每个有效根：
         - 根据 root code 决定是 entry 还是 exit；
         - 使用 `saturate(r + 0.5)` 和 `1 - |r|*2` 更新 `xcov` 与 `xwgt`（类似于在一个 1 像素宽的窗口内算权重）。

  2) 纵向 band：
     - 过程与横向类似，只是 bandIndex.x、vband 数据和 `SolveVertPoly`；
     - 最终得到 `ycov`, `ywgt`。

  3) super sampling（`SLUG_SUPER`）：
     - 沿 x/y 分别对多个样本（最多 4×4）重复上述 band 遍历，最后对 `xcov` / `ycov` 做平均，提高复杂边界处的质量。

  4) coverage 合成：
     - `CalcCoverage(xcov, ycov, xwgt, ywgt, flags)`：
       - 将水平方向与垂直方向的 coverage 按权重合成：
         - 先计算 `abs(xcov*xwgt + ycov*ywgt) / max(xwgt + ywgt, eps)` 与 `min(abs(xcov), abs(ycov))` 的组合；
       - 根据填充规则：
         - 非零规则：Clamp 到 `[0,1]`；
         - 奇偶规则（`SLUG_EVENODD`）：通过对 coverage 做 `frac`/折返，得到 0..1 间周期变化，实现 even-odd 填充；
       - 光学加粗（`SLUG_WEIGHT`）：对 coverage 做一次 `sqrt`，提高窄笔画的权重。

  5) 多图层/多色与渐变：
     - 多色字形（`SLUG_MULTICOLOR`）：
       - `bandData` 中存有 color layer 数据；
       - 对每个 layer：
         - 取 band/glyph 信息和 layer 色值；
         - 复用上面的 coverage 计算；
         - 用 `ApplyLayerColor` 将 coverage 叠加到最终颜色和 alpha。
     - 渐变（`SLUG_GRADIENT`）：
       - 从 curveData 读出渐变几何（直线或圆）和两端颜色；
       - 对像素位置求 `t`，进行线性或径向插值，再与 coverage 相乘。

- `SlugRender`：
  - 封装 `RenderGlyph` 的结果，并根据不同宏（`SLUG_COVERAGE` / `SLUG_INVERSE_COVERAGE` 等）决定输出：
    - 输出覆盖率图、反转覆盖率、或直接乘上 vertexColor alpha。

4) 引擎侧 Shader 集成 —— `C4Slug.cpp` 中 `GlyphProcess`

- `GlyphProcess::GenerateProcessData`：
  - 判断当前 `Drawable` 是否有 SlugContainer：
    - 如果有：
      - 注入自定义 interpolant（`bandingParam` / `glyphParam`）用于在 VS/FS 间传递 band 与 glyph 参数。
      - 设置输出/输入、纹理数为 2：curve + band。
      - 绑定实际的 `TextureObject` 和 sampler。
    - 如果没有：
      - 直接把输入颜色 passthrough。

- `GlyphProcess::GenerateShaderFunction`：
  - 调用 `Slug::GetFragmentShaderSourceCode(*shaderIndex, code, 0)`，将 Slug 的 fragment 代码片段插入引擎 shader；
  - 实际插入的是上面描述的 `SlugRender` 及其依赖函数。

- `GlyphProcess::GenerateShaderCode`：
  - 若启用 Slug，则生成一行 shader 代码：
    - `@ = SlugRender(%tex0, %tex1, %0, %1, FRAGMENT(bandingParam), FRAGMENT(glyphParam));`
  - 告诉 C4 的 shader graph：用两个纹理 + 位置/颜色 + band/glyph 参数调用 Slug 的光栅化函数。

### 2.4 引擎层字体/图标布局与资源管理

1) 纹理与资源封装 —— `C4Slug.h` / `C4Slug.cpp`

- `SlugContainer`：
  - 持有：
    - `Texture *curveTexture, *bandTexture`；
    - 原始 texel 缓冲 `curveTexelData, bandTexelData`；
    - 两个 `TextureHeader`，分别配置格式：
      - curve：`kTextureFormatHalfRGBA` 或 `kTextureFormatFloatRGBA`；
      - band：`kTextureFormatUint16RG`；
  - 方法：
    - `AllocateTexelData(Slug::TextureBuffer*, Slug::TextureBuffer*)`：
      - 根据 Slug 构建阶段写入的 `textureSize` / `writeLocation` 分配 texel 数组，并重置写指针。
    - `CreateTextures(size, texels...)`：
      - 用 C4 的 `Texture::GetTexture(header, data)` 创建 `TextureObject`；
      - 若 band 缺失，则使用全局静态 `nullBandTexture`。
    - `DestroyTextures`：释放纹理与 texel buffer。
    - 静态 `Initialize/Terminate`：
      - 创建 `nullBandTexture`；
      - 构建静态/动态两种 `MaterialObject`，把 `GlyphProcess` 注入到 shader graph 中（静态一个直接用 vertexColor，动态一个可以叠加外部颜色）。

- `FontResource` / `AlbumResource`：
  - 资源层，封装 `.fnt` 与 `.alb` 文件，文件数据即 SlugFile；
  - 提供 `GetSlugFileHeader()`、`GetFontHeader()/GetAlbumHeader()` 等。

- `Font` / `Album`：
  - 继承 `SlugContainer` + `Shared`（引用计数）：
    - 构造时：
      - 从资源取得 `SlugFileHeader`，计算 curve/band texel 存储大小；
      - 分配连续内存，调用 `Slug::ExtractCurveTexture` / `Slug::ExtractBandTexture` 解包；
      - 调 `CreateTextures` 创建 GPU 纹理。

2) 文本布局 —— `TextLayout` / `FontLayout`

- `TextLayout`：单字体或主字体 + `FontLayout` 的总控：
  - 持有：
    - `Font *textFont; ResourceName fontName`：主字体。
    - `FontLayout *fontLayout`：可选，用于配置多字体 fallback/混排。
    - `Slug::LayoutData layoutData`：布局+渲染配置，包括：字号、拉伸、缩放、偏移、颜色、对齐、行距、段落属性、装饰线、OpenType 功能掩码、效果类型、阴影/描边参数等。
    - `Slug::FontDesc fontDescArray[]`：数组，描述每个使用的字体（header、缩放、偏移）。
  - 关键方法：
    - `PreprocessTextLayout(Renderable *renderable)`：
      - 若有 `fontLayout`，调用其 `PreprocessFontLayout` 为每个字体创建 `FontDrawable`（负责把 SlugContainer 传给渲染管线）；
      - 填充 `fontDescArray[0]` 为主字体，后续为 layout 中额外字体；
      - 设置 `fontCount`。
    - `CompileString(const char *text)`：
      - 调 `Slug::CompileStringEx(fontCount, fontDescArray, fontMap, &layoutData, text, -1)`：
        - 使用 Slug 内部的排版引擎，对字符串执行所有 GPOS/GSUB/替代/连字/emoji/多字体 fallback 等操作，返回编译好的 `CompiledText` 对象。
    - `BreakText(const CompiledText*, float maxWidth, Array<LineData>* lineDataArray)`：
      - 使用 `Slug::BreakMultiLineTextEx` 按最大宽度 + 软换行/硬换行/trim 规则分行。
- `FontLayout`：多字体映射配置：
  - 通过 `FontMap` 描述字体类型（粗体、斜体、脚本等）到具体字体资源的映射，以及优先级（source 列表）。
  - `PreprocessFontLayout(Renderable*)`：
    - 根据 `fontTypeArray` / `fontIndexTable` 决定需要多少字体；
    - 构造对应数量的 `FontDrawable` 放入 `drawableStorage`；
    - 运行时 TextLayout 编译文本时即可在一段文字中同时使用多种字体。

3) 图标与矢量图片布局 —— `IconLayout` / `PictureLayout`

- `IconLayout`：
  - 持有 `Album* iconAlbum` 与 `iconIndex`，以及 `iconFlags` / `renderFlags`。 
  - 调用 Slug 的 `GeometryType`：
    - `kIconPolygonGeometry` / `kIconRectangleGeometry` 结合 GPU 能力（如是否支持矩形 primitive）选择多边形、矩形或四边形几何类型。

- `PictureLayout`：
  - 类似 `IconLayout`，但默认 `pictureFlags` 为 `kPicturePolygonGeometry`，`renderFlags` 包含 `kRenderLinearCurves`（可选线性近似以提升性能）。

### 2.5 特性与优缺点

- 特性：
  - 完整的字体与矢量图形管线：
    - 支持字体 + 图标 + 矢量图片（SVG），统一使用 Slug 的 banding + 曲线纹理结构。
  - OpenType 特性支持非常完整：
    - kerning、mark、ligature、alternate、sequence、decompose、emoji、多色字体、layered glyph 等；
    - 排版方面支持双向文本、段落属性、软连字符、 justification、tab 等。
  - GPU 端的 banding + 求交算法高度优化：
    - band 提前剪枝，大幅减少每像素需要考虑的曲线数量；
    - 横向 + 纵向射线 + 可选 super sampling，覆盖计算质量高；
    - 支持矩形 primitive（在 GPU 支持时）作为进一步优化。
  - 可插拔的渲染效果：
    - 光学加粗（`kRenderOpticalWeight`）、线性曲线近似（`kRenderLinearCurves`）。
    - 多色 glyph 和渐变 effect type（阴影、描边、描边+阴影）。

- 限制/风险：
  - 专利与授权：
    - 源码头部明确声明 Slug 受美国专利 #10373352 保护，且为专有/保密；
    - 在开源或商业 Web 项目中直接复用 Slug 算法需要确认授权/许可证，存在法律风险。
  - 实现复杂度高：
    - 字体构建链路非常长，涉及 OpenType 的几乎全部子系统与大量自定义数据结构；
    - 学习成本和维护成本都要远高于 `gpu-font-rendering`。
  - 灵活性：
    - 由于是完整商用引擎集成，对“只要一小部分功能”的轻量集成比较困难；更适合直接使用整个引擎或官方 SDK。

---

## 3. 两套方案对比与对 gweb 的启示

### 3.1 算法与数据结构层面对比

- 相同点：
  - 都是“从轮廓直接渲染”的 GPU 字体方案，核心是用二次贝塞尔曲线表示 glyph，并在 fragment shader 中求射线与曲线的交点计算覆盖率；
  - 都能实现任意缩放的锐利文字，不依赖位图纹理。

- 主要差异：

1) 曲线存储与访问方式：

- `gpu-font-rendering`：
  - 所有曲线线性存储在一个 buffer 中；
  - 每个 glyph 只拥有 `[start,count]` 两个整数；
  - 每个像素基本需要遍历该 glyph 的所有曲线，靠简单的几何剪枝减少计算；
  - 无 band 层级结构，数据结构简单、实现较直观。

- Slug：
  - 曲线存储在 curve texture 中，但访问是通过 band texture 间接寻址：
    - 先根据 y/x 找到 band，再通过 band 的 offset/index 找到具体曲线；
  - band 结构使得曲线查询复杂度与 glyph 总曲线数弱相关，而更多与该像素附近局部 band 内的曲线数相关；
  - 同时在 band 中还能编码 stroke、layer、gradient 等额外信息。

2) 抗锯齿与数值稳定性：

- `gpu-font-rendering`：
  - 基于 1D coverage + 可选第二条射线的“2D 近似”；
  - 支持可配置的窗口宽度（`antiAliasingWindowSize`），以及 super-sampling；
  - readme 中也指出：存在一定的浮点误差，但整体已经比较稳定；若要彻底消除，建议使用 Slug 算法。

- Slug：
  - 全面考虑横向和纵向 coverage，并在 band 结构下控制要参与求交的曲线集合，组合 coverage 时考虑 x/y 两个方向的权重；
  - 通过 bit trick 分类根、专门的退化处理等，数值健壮性更强；
  - 提供 optical weight 等额外的感知质量优化。

3) 功能范围：

- `gpu-font-rendering`：
  - 只负责“单 glyph 精确渲染”，排版与高级 OpenType 功能几乎都没有；
  - 代码适合作为“高质量 GPU 字形光栅化 backend”，上层排版可以自己实现。

- Slug：
  - 从 OpenType 解析、特性展开到布局、段落再到渲染都一揽子包揽；
  - 支持多字体 fallback、完整连字/替代、脚标上标、 emoji layer、多色字体、渐变/描边效果等。

4) 法律与工程成本：

- `gpu-font-rendering`：
  - MIT 开源（原仓库如此），没有专利依赖；
  - 适合直接嵌入 gweb 这样的项目并做二次定制。

- Slug：
  - 明确专利保护 + 专有授权；
  - 适合在已经购买/授权 C4 Engine/Slug 的环境中使用；
  - 若 gweb 是独立 Web 项目，直接移植 Slug 的算法/实现需要额外法律确认，不建议“照抄”。

### 3.2 对 gweb 项目的实践建议（方向性）

结合当前仓库情况，可以考虑的路线大致有：

1) 以 `gpu-font-rendering` 为基础的轻量方案

- 在 gweb 中复用/移植其核心思想：
  - 用 FreeType/Harfbuzz（或浏览器自带排版）拿到 glyph 序列 + 位置信息；
  - 预处理字形轮廓，统一转成二次贝塞尔（可借鉴 `convertContour` 的逻辑）；
  - 在 WebGL / WebGPU 中使用类似的 curve buffer + glyph buffer，按 quad + fragment 求交方式渲染；
  - 根据 Web 端性能需求，视情况增加简单的 band 分桶（比如按 y 分段存一个索引表），在不侵权的前提下引入有限的空间加速结构。

- 优点：
  - 法律上更安全、实现可控；
  - 与现有 demo 代码完全一致，便于调试。

2) 参考 Slug 的设计，而非算法抄袭

- 可以“抽象层面借鉴” Slug 的做法，而避免直接拷贝具体实现：
  - 在资源格式上：
    - 学习其将字体与图标统一为“曲线 + band +元数据”的 SlugFile/Album 思路，为 gweb 设计一套简化版资源格式；
  - 在 API 设计上：
    - 类似 `TextLayout` / `FontLayout` 的分层设计（一个管布局，一个管多字体映射），方便以后加入 OpenType 特性；
  - 在渲染配置上：
    - 考虑 geometryType（quad/polygon/rect）与 renderFlags（opticalWeight/linearCurves）的组合接口，为未来优化留钩子。

3) 如果强需求是“完整的 Slug 功能”

- 那么更现实的路线是：
  - 在 C++/原生端继续使用 C4+Slug 作为后端，通过 IPC/纹理共享/截图等方式把结果送到 gweb，而不是在 Web 侧重写 Slug；
  - 这条路涉及产品架构与授权问题，此处不展开。

---

## 4. 小结

- `3rd/gpu-font-rendering` 提供了一套相对简洁、易理解、无专利负担的 GPU 字体渲染 demo：
  - 核心是“曲线纹理 + 每 glyph quad + fragment 射线求交 + 1D/2D 抗锯齿”。
- `3rd/C4-Engine` 中的 Slug 则是工业级的完整方案：
  - 从 OpenType 解析、OpenVex 中间层、banded 曲线纹理，到 shader 中高度优化的求交与绘制，再到引擎级排版控制，功能非常完备，但有专利和授权限制。
- 对 gweb 而言，更推荐以 `gpu-font-rendering` 为基础，结合自身排版需求做适度扩展，并在设计上抽象性借鉴 Slug 的资源布局与 API 结构，而避免直接复制其专利核心实现。