# 第三方许可证

Dong 本体采用 [MIT License](../LICENSE)。以下为主要 bundled 依赖及其许可证。

| 依赖 | 用途 | 许可证 |
|------|------|--------|
| [SDL3](https://libsdl.org/) | 窗口、GPU、输入 | [Zlib](https://github.com/libsdl-org/SDL/blob/main/LICENSE.txt) |
| [Porffor](https://github.com/CanadaHonk/porffor) | JavaScript 引擎（AOT: JS→Wasm→C） | MIT |
| [Lexbor](https://github.com/lexbor/lexbor) | HTML 解析 | Apache-2.0 |
| [Yoga](https://github.com/facebook/yoga) | Flexbox 布局 | MIT |
| [FreeType](https://freetype.org/) | 字体光栅化 | [FTL / GPLv2 双许可](https://freetype.org/license.html) |
| [HarfBuzz](https://harfbuzz.github.io/) | 文本整形 | Old MIT |
| [msdfgen](https://github.com/Chlumsky/msdfgen) | MSDF 字体 | MIT |
| [FFmpeg](https://ffmpeg.org/) | 视频解码（可选 plugin） | LGPL 2.1+ / GPL 2+（按组件） |

完整许可证文本位于 `dong/third_party/<name>/` 各目录。

## 注意事项

- **FFmpeg**：仅在使用视频 plugin 时链接；分发时需遵守 LGPL/GPL 要求
- **FreeType**：默认 FTL 许可；若修改 FreeType 源码需关注 GPL 选项
- **着色器**：HLSL 源码为 Dong 项目一部分（MIT）

如有许可证疑问，请在 Issue 中讨论。
