# 字体 / 文本渲染问题排查经验（GPU + MSDF）

> 目标：给未来自己一个可以反复套用的“模板”，遇到文字挤在一团、行距/行宽异常、抗锯齿怪异等问题时，能按相同策略快速定位。

---

## 一、问题特征与优先判断

典型异常现象：
- 文本整体“挤成一坨”：
  - 多行文本垂直位置大致正确，但每一行的行内宽度严重偏小，字符都堆在左侧/某个区域。
- DOM 日志看起来“没问题”：
  - 布局引擎（Yoga 等）输出的 `x / y / width / height` 都符合预期，但真正 GPU 输出的图像明显不对。
- 只在某条渲染路径异常：
  - 例如 GPU offscreen / 截图路径异常，而窗口实时显示路径正常，或者反过来。

第一步判断重点：
1. 分清问题是在“布局”还是“绘制”：
   - 布局错：DOM 节点的 `x/y/width/height` 本身就错误。
   - 绘制错：布局正确，但 glyph quad、MSDF 解码、坐标变换出错。
2. 分清是“整张画布”缩放/变换错，还是“文字自己的 advance/metrics”错：
   - 整体缩放/变换错：所有元素（矩形、图片、文本）一起错。
   - 只文字错：背景矩形 OK，文字行宽/间距异常 → 优先怀疑 TextShaper / glyph metrics / shader。

本次案例属于：
- DOM 布局正确；
- 背景矩形位置也大致正确；
- 文本行宽明显偏小 → 最终根因在 TextShaper 对 HarfBuzz 位置结果的缩放错误（多除了一次 64）。

---

## 二、统一测试场景：同一份 HTML 多路径对比

排查前先统一“输入”：
- 准备一份足够简单但敏感的 HTML：
  - 多行文本，分别使用不同字号、颜色，行间有背景块区分。
  - 示例（精简版）：
    - 行 1：`font-size: 72px; color: black;`
    - 行 2：`font-size: 48px; color: red;`
    - 行 3：`font-size: 36px; color: blue;`
- 在多个 demo 中使用同一份 HTML：
  - 窗口 demo（例如 `gpu_view_demo`）。
  - GPU offscreen demo（例如 `gpu_texture_demo`）。
  - 截图 demo（例如 `gpu_screenshot_demo`）。

这样做可以快速回答两个问题：
- 不同渲染路径是否行为一致？（窗口 vs offscreen vs 截图）
- 问题是否只出现在某个特定路径？

---

## 三、DOM / 布局侧验证：日志先保证“盒子”没问题

1. 在 View 层（如 `view.cpp`）打印关键节点的布局：
   - `html / body / 关键 div / 文本容器` 的 `x/y/width/height`。
   - 对比：
     - 窗口 demo 的布局日志。
     - offscreen / 截图 demo 的布局日志。
2. 如果布局日志在各路径下基本一致：
   - 可以初步排除 Yoga / 布局引擎本身的问题。
   - 把重点转向 GPU 渲染和文本 shaping。

经验要点：
- 每次改动布局相关代码后，先对比布局日志，而不是盯着眼睛看截图猜。
- 对比时不要只看某一行，要看 DOM 树中几层关键节点是否整体一致。

---

## 四、GPU 管线验证：先检查 viewport，再用“纯色矩形”替换文本

### 1. viewport 与 NDC 映射

在 GPU driver 的执行入口（例如 `GPUDriverSDL::execute`）中：
- 对 offscreen 模式和窗口模式分别打印 viewport：
  - 宽高、缩放因子等。
- 确保 shader 中用于从屏幕坐标转 NDC 的参数与实际 viewport 一致。

如果 viewport 异常：
- 所有元素的位置都会按相同比例/偏移出现问题，而不仅仅是文字。
- 本次案例中 viewport 是正常的，因此进一步排除。

### 2. 用“纯色矩形”替换文字 shader

在 TEXT 命令路径下：
1. 暂时改写文字 fragment shader：
   - 忽略 MSDF，直接输出 `input.color`，把所有 glyph 画成实心矩形。
2. 然后观察：
   - 背景矩形 + glyph 矩形的排布是否合理：
     - 行高、行间距是否正常？
     - 行内宽度是否合理？

好处：
- 可以把“MSDF 解码 / 距离场边界”相关问题完全排除，只看几何和坐标。
- 如果“纯色矩形模式”下行高正常、行宽不正常 → 高度 OK，宽度有问题，指向 advance / pen_x / glyph width。 

本次案例：
- 纯色矩形模式下：
  - 行与行的垂直位置 OK。
  - 每行中的矩形极度瘦，集中在一侧 → 暗示 `pen_x` 或 advance 算错。

---

## 五、从 GPU 实际输出采样：离屏 + BMP + Python 分析

核心策略：
1. 不再“凭肉眼看屏幕”。
2. 把 GPU 输出的图像保存到文件，再用脚本做像素级统计。

### 1. 推荐抓图方式：dong_view_render_offscreen + BMP

统一用引擎已有的 offscreen 渲染 API：
- 使用 `dong_view_render_offscreen(view, device, width, height, rgba_buffer)`：
  - 由引擎内部负责：
    - 组织 command buffer。
    - 绑定 offscreen target。
    - 执行 TEXT/RECT 等命令。
- 渲染完成后，将 RGBA buffer 写成 BMP：
  - 注意 BMP 默认自下而上存储行：第 0 行在文件底部。

避免的做法：
- 尽量不要直接抓 swapchain / backbuffer（例如从 SDL GPU swapchain texture 读）：
  - 各平台/后端差异大；
  - 容易踩到还没呈现/格式不一致等坑。
- 统一走引擎封装好的“离屏渲染 + 内存 buffer”路径更稳定。

### 2. Python 分析脚本的注意事项

常用分析方式：
- 按行统计“非背景”像素：
  - 每一行中哪些列是“黑/灰”等文字像素。
  - 统计每一行文字像素的最小/最大 x，得到行宽。
  - 统计有文字的行的 y 范围，得到行高/行间距。

关键注意点：
1. BMP 行序：
   - 文件中的第 0 行是图像底部。
   - 分析时如果希望使用“屏幕坐标系”（原点在左上），需要倒序或转换 y。
2. 背景阈值：
   - 不要把浅灰背景块当成文字像素。
   - 可以按 luminance 或 RGB 直接阈值：
     - 例如 brightness < 50 → 黑；brightness > 200 → 白；中间视为“灰/可能有文字”。
   - 针对具体 demo，先跑一份，把某些明显背景区域的 brightness 打印出来再调参数。
3. 只统计“像文本”的像素：
   - 如果有背景矩形，最好要么排除背景区域，要么使用更严格阈值。

本次踩坑：
- 一开始把浅灰背景当成文字，导致结论是“所有行都有大量文字”或“文字集中在奇怪的 y 范围”。
- 修正行序 + 阈值之后，才能真实看到三行文字的纵向/横向分布。

---

## 六、TEXT 命令与 glyph 几何：从日志验证 pen / glyph quad

在 GPU driver 的 TEXT 分支中（例如 `GPUDriverSDL::execute`）：
1. 打印每个 glyph 的关键数值：
   - baseline（行基线位置）。
   - pen_x / pen_y（笔尖位置，通常是 design units * scale_to_pixels）。
   - glyph 的 width/height、bearing_x、advance_x 等。
   - 最终 quad 的屏幕坐标：`glyph_x / glyph_y / glyph_w / glyph_h`。
2. 对比以下几组关系：
   - 行内 pen_x 是否单调递增，增量大小是否接近字号。
   - 相邻 glyph quad 的 x 是否按预期间距排列。

结论类型：
- 如果 `baseline`、`pen_y` 合理，但 `pen_x` 的增量极小 → advance/units 的缩放有问题。
- 如果 glyph quad 的宽度明显太小或太大 → glyph metrics 或 `scale_to_pixels` 异常。

本次案例的特征：
- baseline / 垂直方向相关值基本正确。
- 行内 pen_x 增量极小（类似除以了 64），导致所有 glyph 挤在一起。

---

## 七、TextShaper / HarfBuzz / units_per_em：只允许有一处“设计单位 → 像素”缩放

通用原则：
- HarfBuzz 输出的是“design units”，单位与 font face 的 `units_per_em` 对齐。
- 整条链路上，应该只有一个明确的“design units → 像素”的缩放因子（通常称为 `scale_to_pixels`）。

### 1. HarfBuzz 常见误区：重复除 64

有些教程里提到：
- HarfBuzz 的 `hb_glyph_position_t` 等字段是 26.6 fixed，需要除以 64 才能得到整数单位。但这和 `hb_font_set_scale` 的使用方式强相关。

经验规则：
- 如果已经调用 `hb_font_set_scale(font, units_per_em, units_per_em)`：
  - 则 HarfBuzz 的 `pos.x_advance / x_offset` 等已经是“以 design units 为单位”的整数/浮点，不需要再除以 64。
- 如果不清楚，打印原始值与 `units_per_em`、glyph metrics 做对比：
  - “一字的 advance”应该大约是`font_size_px / scale_to_pixels` 对应的 design units 数量。

本次真实 bug：
- TextShaper 在取 HarfBuzz 结果时多做了一次 `/64`：
  - 导致行内 advance 被缩小 64 倍。
  - 屏幕上表现为：每个字几乎都画在同一个 x 位置附近，挤在一团。
- 修复措施：
  - 去掉所有对 `hb_glyph_position_t` 的 `/64` 缩放，直接保留 HarfBuzz 已经按 `units_per_em` 处理过的 design units。

### 2. 确认缩放链路的一致性

推荐只保留一处从 design units → 像素的缩放：
1. TextShaper 输出：
   - 所有值保持在 design units 空间。
2. GPU IR / DrawText 命令：
   - 携带一个 `scale_to_pixels`（或同等意义的因子）。
3. GPU driver 在组装 glyph quad 时：
   - 对 design units 统一乘以 `scale_to_pixels`，得到像素坐标。

检查点：
- 若发现有两处类似 `*scale` 或 `./units_per_em` 的逻辑，需要认真梳理：
  - 是否重复缩放？
  - 是否有一处错误地用 px 当成 design units？

---

## 八、MSDF shader 问题的排除与恢复

当已经确认 glyph quad 几何和 pen_x/pen_y 都正确后，若仍有“文字边界模糊/断裂/反相”等问题：
1. 恢复 MSDF fragment shader：
   - 使用 median(r,g,b) 取 signed distance。
   - 计算屏幕空间的 distance range（结合 ddx/ddy）。
   - 使用正确的 `uParams`（scale / bias）进行线性变换后再做 smoothstep。
2. 再次截图并用同样的 Python 分析工具确认：
   - 文本粗略行宽/行高仍与“纯色矩形模式”一致。
   - 若行宽变了，说明 shader 中还有几何相关 bug。

本次案例：
- 在纯色矩形模式下定位完 TextShaper 缩放错误后，恢复 MSDF shader。
- 再次截图并分析，行宽/行高与布局一致，说明问题已经根治。

---

## 九、gpu_screenshot_demo 的推荐使用方式

为了统一调试体验，准备一个专门的截图 demo（例如 `gpu_screenshot_demo`）：
- 典型流程：
  1. 创建 SDL 窗口 + GPU device（复用 `SDL3Window`）。
  2. 创建 dong context + view。
  3. 调用 `dong_view_set_external_gpu_device()` 绑定 GPU 后端。
  4. 加载固定的测试 HTML（如高亮多行文本）。
  5. 调用 `dong_view_update()`，给一次布局/渲染机会。
  6. 调用 `dong_view_render_offscreen()` 渲染到 RGBA buffer。
  7. 调用 `writeBMP("gpu_screenshot.bmp", width, height, pixels)` 写 BMP。
  8. 可选：在 C++ 里做一轮简单像素统计（黑/灰/白像素数量），初步判断图像是否为空白。

注意事项：
- 不要在 demo 中重复实现复杂的 GPU 抓图逻辑（比如从 swapchain 读回），容易引入 demo 级别 bug 干扰判断。
- 让 demo 尽可能薄，只负责：
  - 初始化引擎。
  - 调用引擎正式的 offscreen 渲染 API。
  - 把结果写磁盘 + 简单统计。

---

## 十、未来调试的推荐 Checklist

遇到新的文本渲染问题时，可以按以下顺序执行：

1. 统一输入：
   - 选定一份包含多行、多字号、多背景块的 HTML，在所有相关 demo 中复用。

2. 布局验证：
   - 在 View/DOM 层打印 `x/y/width/height`，确保布局在所有渲染路径下表现一致。

3. GPU 几何验证：
   - 打印 TEXT 命令中的 baseline、pen_x/pen_y、glyph width/height。
   - 检查行内 pen_x 是否单调递增且增量合理。

4. 纯色矩形模式：
   - 暂时改写文字 shader 为纯色填充。
   - 确认行高/行间距/行宽是否合理。

5. 截图 + 分析：
   - 用 `dong_view_render_offscreen + BMP` 捕获实际输出。
   - 用 Python 脚本按行/列统计：
     - 注意 BMP 行序；
     - 调整亮度阈值，排除背景影响。

6. Shaper / metrics 检查：
   - 检查 TextShaper 是否错误地对 HarfBuzz 结果做 `/64` 或其他多余缩放。
   - 确认整个链路上只有一处“design units → 像素”的缩放。

7. MSDF shader：
   - 在几何确认正确之后，再恢复 MSDF shader，检查视觉细节问题。

8. 回归验证：
   - 同时在窗口 demo + offscreen + screenshot demo 上观察行为是否一致。

只要严格按上述步骤执行，一般能在“布局 vs 几何 vs shader vs shaping”这几个层级中快速定位问题所在，并复用同一套截图 + 采样 + 脚本分析的策略，避免再次陷入纯肉眼调试的误区。
