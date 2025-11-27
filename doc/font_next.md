
一、在「当前这版」基础上还能做的画质优化（短期、立竿见影）

这些都是在现有代码上加一点点参数/逻辑就能看到变化的：

1) 调粗/调锐：给 shader 加“笔画粗细”参数  
- 现在公式是大路货：  
  - `sd = median(msdf.rgb) - 0.5`  
  - `width = max(fwidth(sd), 1e-4)`  
  - `opacity = saturate(sd / width + 0.5)`  
- 可以加两个很实用的小参数，让你肉眼调风格：
  - `u_msdf_bias`：整体向内/向外推轮廓（加粗/变细），例如：
    - `float sig = sd + u_msdf_bias;`
  - `u_msdf_contrast`：控制边缘硬一点或软一点，例如对 `opacity` 做 `pow(opacity, u_msdf_contrast)`。
- 这不会破坏“随缩放自适应 AA”的特性，但能把现在偏细、偏淡的感觉调回到你喜欢的权重。

2) 针对小字号做宽度下限（防止太细）  
- 对特别小的字号，`fwidth(sd)` 会非常小，AA 区域接近 0.5 像素，看起来就像“发虚”。  
- 可以加一个最小宽度（按 devicePixelRatio 调整）：
  - `float width = max(fwidth(sd), u_minWidthInPx / device_pixel_ratio);`
- 小字号时线条会稍微加粗一点，大字号基本不受影响。

3) 改字体族/字重，让整体观感更稳定  
- 你现在 demo CSS 是：`font-family: Arial, sans-serif;`，在 mac 上落到的多半是 Helvetica，字本身就偏细。  
- 可以在 `gpu_view_demo.cpp` 的 `<style>` 里试试：
  - 换成更粗一点的 sans（例如 `system-ui`, `SF Pro`, 或你自带的 UI 字体）。
  - 或者单独给标题/正文指定不同的 `font-family` 和 `font-weight`，让 Demo 更接近你预期的 UI 风格。
- 字体本身的设计对“模不模糊”的主观感受影响很大，这一步成本最低，但收益很直观。

4) 修正颜色管线，把“发灰感”压下去  
- 现在颜色是走“自己管 gamma + sRGB”的一套，自带一点洗白的风险。  
- 短期可以做的：
  - 明确：CSS 颜色在 CPU 端转成线性再上传（而不是把 sRGB 当线性用）。  
  - Shader 里：
    - 只在需要的地方做 `toLinear`/`toSRGB`，避免对 alpha 再 gamma 一次。  
- 这一块你已经在片元里预留了 gamma 参数（`uParams.w`），只需要把 CPU 侧和 swapchain 的假设统一一下。

5) 基础 UI 调整：字号/行距/对齐  
- 现在 demo 里几行说明文字字号很小 + 行宽很长，本身就不太利于阅读。  
- 可以在 demo 的 CSS 里：
  - 略增大正文字号（比如从 12 提到 13–14）。  
  - 稍微增加 `line-height`。  
- 这不影响 MSDF 本身，但整个界面的“看起来模不模糊”的主观感受会好很多。

二、围绕 MSDF / 字体渲染的「待办清单」（按优先级）

下面这部分基本就是 `doc/next.md` 里的内容，结合你现在的实现状态重新梳理一下：

短期（画质 P0，直接影响文字质量）

1) MSDF 采样公式的“完全版校准”（你已完成一半）  
- 已经做的：
  - 从 AltPS / WebGPU 那堆复杂公式切换到简洁的 `fwidth(sd)` 版本，解决了“整条白砖”的根本 bug。  
- 还可以做的：
  - 把 `msdfgen` 返回的 `scale/translate` 和实际 `glyph_scale` 真正接进来，而不是完全丢掉：
    - 正确记录每个 glyph 的 `msdf_scale/msdf_translate`。  
    - 让 shader 里的 `sd` 真正意义上对应“距离像素”的线性量，而不是模糊单位。  
  - 把 `fwidth()` 里的尺度乘以 `inv_device_pixel_ratio`，在高 DPI 下保持相同视觉锐度。  
- 这会让“大幅缩放／高 DPI／非整数 scale” 情况下的锐度更稳。

2) 多级 glyph atlas（解决小字号/大字号同时好看）  
- 现在应该是“一套 atlas 打天下”，大/小字号都用同一个 bitmap 尺寸。  
- TODO：
  - 至少准备几档：例如 32px / 48px / 72px。  
  - 渲染一个 glyph 时，按 `ceil(font_px_size / 1.5)` 选最接近的 atlas。  
  - 背后要有简单的 glyph 缓存和 `FT_Face` 复用，避免重复初始化。  
- 作用：
  - 大字号不会被“小 atlas 放大”搞得锯齿或模糊。  
  - 小字号不会浪费太多 atlas 空间。

3) 字体度量层（真正的 GlyphRun，而不是魔数排版）  
- TODO：
  - 用 `FT_Size_Metrics` 拿到 ascender/descender/line_gap，替换掉 `line_advance = 1.25x` 这种估算。  
  - 通过 `FT_Load_Glyph` 的 advance/kerning 做出真正的 `GlyphRun`：  
    - 字间距、kerning、baseline 都来自同一份度量。  
  - 让布局引擎和 GPU 渲染共享这套几何数据。  
- 这样可以让字间距、行间距、baseline 都更自然，也便于之后做多语言（CJK/RTL）支持。

4) 完整打通 sRGB→linear→sRGB 管线  
- TODO：
  - 规范：  
    - `CSS 颜色` 在 CPU 上转成 linear 浮点再上传。  
    - `MSDF 纹理` 用线性采样（不要在纹理格式上额外做 sRGB/gamma）。  
    - 输出统一在 swapchain 或最后一步统一转回 sRGB。  
  - 避免在 shader 内对同一份值做两次 gamma，造成“灰蒙蒙”的感觉。  

中长期（架构 vNext，更多是 GPU driver 的事）

1) DisplayListBuilder  
- 把 `render::Painter` 的输出提升为 UI 原语的 DisplayList：rect / rounded-rect / glyphRun / image 等。  
- 这样才能统一跑 GPU pipeline，而不是现在 demo 里半测试半特例。

2) LayerTree + caching  
- 为滚动、opacity、transform 等场景建立 Layer，只有 dirty 的 layer 才重 Raster。  
- 文本渲染在绝大多数滚动场景下只需要平移 layer，而不是重新跑文字栅格。

3) GPUCompiler + 专用 GPUCommandList  
- DisplayList → 按 pipeline/纹理/混合键排序 → 写 instance buffer → SDL_gpu 后端执行。  
- 文本和矩形、图片全部走 instanced quad，真正发挥 MSDF + atlas 的批处理优势。

4) 多后端 + 离线/后台 MSDF 生成  
- 在 `GPUDriver` 抽象下将来接 Metal/OpenGL/Vulkan。  
- 根据 glyph atlas 需求，离线预生成或后台生成 MSDF 页，运行期只查表。
