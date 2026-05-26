# 文本渲染

Dong 采用 **MSDF + Slug 双渲染器** 架构，经 HarfBuzz 整形后由 GPU 绘制。

## 数据流

```
HTML/CSS 文本 → HarfBuzz + FreeType shaping → Glyph Run
                    ↓
            TextRendererSelector
           /                    \
    MSDF (GlyphAtlas)      Slug (Curve/Band 纹理)
           \                    /
              GPU Shader 绘制
```

## 渲染器对比

| | MSDF | Slug |
|---|------|------|
| 原理 | 多通道距离场纹理 | 解析式贝塞尔覆盖率 |
| 质量 | 依赖 atlas 分辨率 | 无限分辨率，小字号更清晰 |
| 默认 | `auto` 时 Slug 优先 | 推荐游戏 UI |

环境变量：`DONG_TEXT_RENDERER=slug|msdf|auto`

## Color Emoji

支持 COLR/CBDT/sbix 等彩色 emoji，与文字混排。Case：`tests/test_colr_emoji.html`。

## 即时模式文字

Overlay API `dong.renderText()` 用于伤害浮字、动态文字流，不走 DOM。见 [渲染模式](../guide/render-modes.md)。

## 多 View 共享

同进程多 View 共享 GlyphAtlas（`dong_global_shared.h`），显著节省 GPU 内存。

## 常见问题

| 现象 | 方向 |
|------|------|
| 小字号锯齿 | 尝试 `DONG_TEXT_RENDERER=slug` |
| 中文大字号模糊 | 检查 Slug 是否启用 |
| 首帧文字慢 | Text shape pre-warming；async shaping（实验） |

详细实现规格：[text-rendering-spec.md](../developer/arch/text-rendering-spec.md) · 调试笔记：[font-troubleshooting.md](../developer/debug/font-troubleshooting.md)
