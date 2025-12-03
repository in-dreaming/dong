

整体目标（验收标准）：
- 对当前 `align_basic_layout.html` 场景：
  - 布局盒子坐标误差 ≤ 0.25px；
  - 文本 baseline/行高误差在肉眼不可察觉范围（≈0.25px 量级）；
  - 颜色每通道误差 ≤ 1–2；
- 上述标准通过“浏览器截图 vs gpu_screenshot_basic_layout.bmp 图像 diff”来回归。

我按模块列出“应该做什么”，这些都是正式长期方案。

1. 布局：实现真正可用的 block + inline 模型

1.1 显式区分四种 display 模式
- 在 `ComputedStyle` / layout 里增加明确的 layout mode：
  - `Block`（对应 `display: block`、`inline-block` 的外层盒）
  - `Inline`（内联格式化上下文）
  - `Flex`（`display: flex`，保留 Yoga）
  - `None`
- `StyleEngine`：
  - 准确解析 `display: block/inline/inline-block/flex`。
- `LayoutEngine`：
  - 不再把所有非 flex 都当成 `YGDisplayFlex + column`；
  - 对于 block/flex 元素仍可使用 Yoga，但 inline/inline-block 走自定义逻辑。

1.2 block / inline-block 的盒模型与尺寸
- Block 级元素：
  - 宽度默认 `auto` 时，等于包含块宽度减去 margin/border/padding（标准块格式化）。
  - 已有 Yoga 的“无显式宽度就 100%”可以继续使用，但要严格用 CSS margin/padding/border 映射，确保：
    - `margin/border/padding` 叠加后，`block-box` 和 `block-box primary` 的几何与浏览器一致。
- Inline-block：
  - 宽度高度由内容决定（文本/子元素尺寸 + padding + border），再按 CSS 规则进行基线对齐；
  - 在 `inline-block-row` 中，多个 inline-block 按内联盒子横向排列，遇到行宽不够时换行；
  - 这部分不能依赖 Yoga，要在“行内格式化上下文”里自己排布。

1.3 内联格式化上下文（Inline Formatting Context）
- 为 `span` / inline 元素实现一套简单但正确的行内布局：
  - 输入：一系列 inline 盒子（文本片段、inline/inline-block 元素），容器宽度、字体/行高信息；
  - 输出：一组 line box，每个 line box 内每个 inline box 的 x、baseline y。
- 行内排版规则：
  - `white-space: normal`，按单词 + CJK 字符做换行；
  - 使用 HarfBuzz shaping 的 glyph 宽度决定换行点（不再用 `estimateTextWidth`）。
- Baseline 对齐：
  - 每个 inline box 记录自己的 ascent/descent；
  - 行 box 的 baseline 取该行所有盒子的最大 ascent；
  - `inline-chip` 和普通文本共享同一 baseline，保证和浏览器对齐。

1.4 position: relative / absolute 支持
- `ComputedStyle` 补齐 `top/right/bottom/left`、`z-index`；
- 布局阶段：
  - 对 `position: relative`：
    - 先按普通流排布，得到正常盒子位置；
    - 再应用相对位移（不影响兄弟元素盒位），仅改变绘制位置。
  - 对 `position: absolute`：
    - 从包含块（最近的 `position != static` 祖先）取坐标系；
    - 使用 `top/right/bottom/left` 计算绝对位置；
    - 不参与兄弟的正常流布局。
- 这样 `rel-box` 里的 `ABS` 标记可以精确落在右上角。

1.5 margin 折叠与间距
- 继续使用已实现的“兄弟间 vertical margin 折叠”；
- 确保 block 段落之间的真实间距与浏览器的 `margin-bottom/margin-top` 行为一致；
- 这对 Typography 部分两段正文之间的“段落间距”非常关键。

2. 文本排版：基于设计单位的行高与换行

2.1 字体解析与选择
- `font_resolver`：
  - 正式支持系统 UI 字体：
    - 在 macOS 上，将 `-apple-system`、`BlinkMacSystemFont` 精确映射到 SF 系列（通过具体路径）。
  - 对 `font-family` 列表按顺序匹配：
    - 优先按 family name 匹配（`SF UI Text` / `Helvetica` / `Arial`）；
    - 再回退 generic family（`sans-serif` 等）。
- `font-weight`：
  - 将 CSS `font-weight` 映射到具体的文件或 face index（Regular / Medium / Bold）；
  - 至少支持 normal(400)/bold(700)，保证标题 “The quick brown fox…” 的粗细一致。

2.2 HarfBuzz + FreeType 的一体化布局
- 保持当前“design units + OS/2 度量”的设计，但在布局中全面使用：
  - 行高：
    - `TextShaper` 返回 `ascent_units`, `descent_units`, `height_units`（OS/2 Typo）；
    - `line-height: normal` 使用 `height_units * scale_to_pixels`；
  - baseline：
    - baseline = 行顶部 y + top-leading + ascent，按照 CSS 模型计算；
  - 这样第二段正文、caption、`Small text/BIG TEXT/SPACED LETTERS` 的 baseline 对齐可以接近浏览器。

2.3 使用真实宽度做换行
- 删除 `estimateTextWidth` 在换行决策中的作用，只保留它作为极限 fallback；
- 换行算法：
  - 对于一段连续文本：
    - 先对整段文本（或 chunk）跑 HarfBuzz，得到 glyph advance；
    - 采用 greedy line breaking：逐字/逐词累积宽度，超过 `inner_width` 就回退到上一个 break point；
    - break point 的选择与 CSS `white-space: normal` 对齐（空格 / CJK / 标点规则）。
- 行宽/对齐：
  - line box 宽度由 glyph 实际宽度决定；
  - `text-align: center/right` 按剩余空间加偏移。

2.4 letter-spacing / word-spacing
- 在 `StyleEngine` 解析 `letter-spacing`、`word-spacing`；
- 在 shaping 或布局阶段，将额外间距叠加到 glyph pen_x 上；
- 确保 “SPACED LETTERS” 的视觉字距与浏览器基本一致。

3. 颜色与视觉效果：完整 CSS 颜色模型子集 + box-shadow

3.1 正式 CSS 颜色解析器
- 替换当前 `parseCssColor` 的“临时版”，实现完整子集：
  - 支持：
    - `#rgb/#rgba/#rrggbb/#rrggbbaa`
    - `rgb()/rgba()`（整数/百分比形式）
    - 必要的命名色（至少 gray 系 + brand 用到的几种）
  - alpha 与 `opacity`：
    - 最终绘制颜色的 alpha = 颜色 alpha × 元素 `opacity` × 父层合成结果（已有 LayerTree 可以配合）。
- 这可以保证：
  - “Soft alpha” 用的 `rgba(16,185,129,0.08)` 在 GPU 输出中颜色和 HTML 一致；
  - Primary 卡片 `#1D4ED8 on #EFF6FF` 精确匹配。

3.2 border / border-radius
- 已经有 `border-radius`/`border-width` 映射，需要进一步精确化：
  - 把 CSS 边框宽度映射到 Painter 的绘制逻辑（实线/虚线暂时只支持 solid/dashed）；
  - 颜色从 `border-color` 解析，若未指定则继承 `color` 或浏览器默认行为；
  - 圆角边框与背景的裁剪路径一致，避免锯齿/错位。

3.3 box-shadow 子集实现
- 实现 `box-shadow` 基础子集：
  - 格式：`offset-x offset-y blur-radius spread-radius color`；
  - 支持多阴影叠加；
  - 在 GPU 侧可先实现简单的 nine-patch 模糊或基于 pre-baked kernel 的近似；
- 在当前 align 场景中：
  - “Shadow card” 阴影的形状/范围要基本一致（颜色和模糊半径允许轻微差异）。

4. 组合 / z-index / stacking context

- 对本场景（没有 transform/opacity overlay）只需要：
  - 正确处理 painting order = DOM tree order + z-index；
  - 支持 `position: relative/absolute` 产生 stacking context；
- 方案：
  - 在布局完成后，对同一父元素的子盒子按 CSS painting order 排序；
  - 当存在 `z-index` 或新 stacking context 时，构造嵌套的 painting list；
  - Painter 按 painting list 顺序构建 DisplayList → GPUCompiler → GPUDriver。

5. 验证与回归机制（正式对齐流程的一部分）

5.1 对齐场景的 reference pipeline
- `doc/align_basic_layout.html` 为浏览器参考；
- `gpu_screenshot_demo_basic_layout` 作为 GPU 输出；
- 每次改动：
  - 在固定浏览器（如 Safari/Chrome）和固定窗口尺寸下截图；
  - 和 `gpu_screenshot_basic_layout.bmp` 做像素 diff。

5.2 自动化差异检测（后续可做）
- 写一个小工具（Python 或 C++）：
  - 计算两张图的 per-pixel 误差统计（坐标和颜色）；
  - 标记超过阈值的区域并导出可视化图（红色高亮）；
- 在你引擎迭代布局/字体/颜色时持续用这个工具验证回归。

—

这就是“正式版本”的对齐方案：  
- 布局上引入真正的 block/inline/relative/absolute 模型（保留 Yoga 只用于 block/flex）；  
- 文本上全面以 HarfBuzz/FreeType 的设计单位为真，做 line box + baseline 对齐和换行；  
- 颜色上实现完整的 CSS 颜色子集和基本 box-shadow；  
- 然后用 `align_basic_layout` 场景 + 截图 diff 做长期回归。

