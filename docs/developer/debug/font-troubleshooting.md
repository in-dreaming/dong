# 字体渲染问题排查

汇总常见字体/坐标/锯齿问题的排查方向。

## 小字号锯齿（MSDF）

**现象**：小字号文字边缘锯齿明显。

**方向**：

1. 切换 Slug：`DONG_TEXT_RENDERER=slug`
2. 检查 MSDF `range` / atlas tier 是否匹配字号
3. 见 [font_msdf_aliasing.md](./font_msdf_aliasing.md) 详细分析

关键代码：`src/render/glyph_atlas.cpp`、`backends/sdl/shaders/text_fs.hlsl`

## 坐标 / baseline 偏移

**现象**：文字与 CSS box 不对齐、relbox 计算异常。

**方向**：

1. 对比浏览器 baseline：用 `html_render_test` 截图
2. 检查 `painter.cpp` baseline 与 Yoga 布局结果
3. 中文/emoji 混排时确认 HarfBuzz cluster 边界

## 字体加载失败

**现象**：fallback 字体、 tofu 方块。

**方向**：

1. 确认字体文件路径与 `build.env` 配置
2. `FontResolver` 日志：`DONG_LOG_LEVEL=DEBUG`
3. 见 [font_msdf_aliasing.md](./font_msdf_aliasing.md) 历史案例分析

## 相关文档

- [文本渲染架构](../../architecture/text-rendering.md)
- [text-rendering-spec.md](../arch/text-rendering-spec.md)
