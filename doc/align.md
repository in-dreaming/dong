我先简单看一下当前的运行文档，确保后面列的验证项和你的 demo/运行方式对齐，然后给出一个按维度分组的验证清单，方便你后面在拷贝的 gpu_screenshot_demo 里逐一构造场景。
先把“要对齐的东西”拆成几个维度，后面你在拷贝出来的 `gpu_screenshot_demo` 里，可以按章节一个个构造静态场景来验证。

建议验证维度（P0 → P1 优先级）：

1. 布局几何（P0）
   - 块级/内联元素的位置、宽高：
     - `display: block/inline/inline-block`
     - `margin/border/padding` 叠加后的盒子大小
   - 行内排版：
     - 多行文本换行位置（`white-space` 默认值）
     - 行高 `line-height`、baseline 对齐
   - 简单定位：
     - `position: relative/absolute` 基于包含块的偏移
   → 场景：几行 block + inline 文本，标出预期坐标，GPU 截图 vs HTML 截图逐像素比。

2. 文本排版 / 字形（P0）
   - 字体族、字号、字重：
     - `font-family/font-size/font-weight/font-style`
   - 行间距 / 基线：
     - 多行段落的 baseline 间距
   - 字距相关（如果引擎已实现）：
     - `letter-spacing/word-spacing` 的效果
   - MSDF 渲染细节：
     - 不同字号、小字号可读性
     - 开关 `DONG_MSDF_SUBPIXEL` 前后字形边缘是否与 HTML 足够接近
   → 场景：字体对照表（几种 font family、size、weight），同一文案在浏览器和 GPU 截图对比。

3. 颜色 / 背景 / 边框（P0）
   - 纯色背景、前景色：
     - `color` / `background-color`（含 alpha）
   - 边框：
     - `border-width/style/color`，含圆角边框
   - 阴影（如果已支持）：
     - `box-shadow` 基础形态的位置和模糊范围
   → 场景：若干彩色卡片，组合不同边框和背景，叠在浅灰背景上，对比每块的颜色和尺寸。

4. 裁剪 & 滚动容器（P0）
   - `overflow: hidden/scroll/auto` 产生的裁剪区域
   - 内嵌滚动容器 + 圆角：
     - 圆角裁剪边缘是否和 HTML 一致
     - 容器滚动后内容的可见区域
   - 多级嵌套滚动（简单两层即可）
   → 场景：一个卡片里放长列表，卡片有 `border-radius` 和 `overflow: auto`，滚动到几个关键位置后截图对比。

5. transform + opacity + 图层隔离（P0）
   - `transform: translate/scale` 的几何效果：
     - 平移后的最终位置
     - 缩放后的尺寸（尤其是非整数 scale）
   - `opacity` 和混合：
     - 不同透明度下前景色/背景色混合结果
   - `isolation: isolate` / stacking context：
     - overlay 弹窗是否与下面内容正确叠加和混合
   → 场景：半透明 overlay（带 `opacity` 和 `transform`），覆盖在内容列表之上，比较位置、透明度和边缘混合。

6. 层级 / 叠放顺序（P0-P1）
   - `z-index` 与 DOM 顺序：
     - 同一 stacking context 内，`z-index` 不同的叠放顺序
   - 新 stacking context：
     - 带 `opacity/transform/isolation` 的元素与普通元素之间的前后关系
   → 场景：多个彩色块交叉覆盖，部分用 transform/opacity 提升为新 layer，检查前后顺序是否与 HTML 一致。

7. 图片 / 位图缩放（P1）
   - `<img>` 或背景图：
     - 不同比例缩放（0.5×、1.5×、2×）
     - 居中 / 填满（对应引擎目前支持的模式）
   - 像素对齐：
     - 1px 线条在缩放和非整数坐标下是否模糊程度与 HTML 大致一致
   → 场景：几张测试图片和 1px 线条，在不同缩放下排列，对比清晰度和对齐。

8. 混合 / 叠加效果（如果已实现，P1）
   - `background` 叠加（多层背景）
   - 简单 `mix-blend-mode` 或 `backdrop-filter`（如果已经有）
   → 如果当前引擎尚未实现，可先仅验证最基本的 alpha 混合。

9. 误差标准 & 比对方式（横向要求）
   - 对齐标准：
     - 允许的坐标误差（如 ≤ 0.25px）
     - 允许的颜色误差（如每通道差值 ≤ 1–2）
   - 比对方法：
     - 同一 HTML 结构，浏览器截图（参考） vs 引擎 `gpu_screenshot_demo` 截图
     - 后面可以再加一个小工具做差值可视化（现在先不实现，只当规划）

后续建议的构造路线（供你后面拷贝 demo 时参考）：
- 先做一个“基础布局 + 文本 + 颜色”的场景，把 1/2/3 一次性覆盖；
- 再做“滚动容器 + 圆角裁剪”的场景覆盖 4；
- 做“overlay + transform + opacity + stacking”的场景覆盖 5/6；
- 最后补一个“图片缩放/像素对齐”场景覆盖 7，以及视情况加混合效果场景。

这样一来，所有这些场景在一个新的 `gpu_screenshot_demo_*` 文件里可以被单独开启/截图，对齐浏览器结果，一项项把 GPU 管线拉齐 HTML。