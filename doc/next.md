 
**后续开发方向**

***短期（画质 P0）***

1. **重新校准 MSDF 采样公式**：  
   - 记录 `msdfgen` 返回的 `scale/translate` 与实际 `glyph_scale`，把 `(atlas_range / atlas_px_size) * font_px_size` 作为 `uParams.x`，并把 `fwidth()` 乘以 `inv_device_pixel_ratio`，解决放大后变糊的问题。  
   - 同时在 shader 中加入伽马/对数调节和可选的 subpixel mask，以贴近文档要求的“着色器优先 + 高质量 MSDF”。  
2. **多级 glyph atlas**：至少准备 (32px, 48px, 72px) 三档页面，依据 `ceil(font_px_size / 1.5)` 选择最接近的 atlas，或者在运行时生成更大的 `glyph_bitmap_size_` 再下采样；同时缓存 `FT_Face`，避免重复初始化。  
3. **字体度量层**：读取 `FT_Size_Metrics`、脚本方向和 baseline，替换掉 `line_advance = 1.25x` 的魔数，通过 `FT_Load_Glyph` 的 advance/kerning 构建 `GlyphRun`，让 UI 布局和 GPU 渲染共享同一份文字几何。  
4. **sRGB→linear→sRGB 管线**：当前片段颜色直接使用 `cmd.color`，可以在上传 uniform 前转换到 linear，再由 swapchain 做 gamma，避免视觉上“发灰”的条纹。

***中长期（架构 vNext）***

1. **实现 DisplayListBuilder**：按照 `doc/gpudriver-pipline..md` 的步骤，把 `render::Painter` 输出提升为 UI 原语，确保文本、圆角矩形、图片统一走 GPU 路径。  
2. **引入 LayerTree 与缓存**：为滚动容器、透明度、变换等节点建立 Layer，只有 dirty Layer 才重栅格，其余直接 Composite，满足文档中“层级合成”与“强缓存”的指导。  
3. **GPUCompiler + 专用 GPUCommandList**：把 DisplayList 按 pipeline/纹理/混合键排序，批量写入 instance buffer，再由 SDL_gpu 后端翻译成最少的 Draw call，彻底摆脱“逐 glyph PushUniform”的模式。  
4. **多后端抽象**：沿着规范里的 `YE_GPUDriver` 接口继续拆分，把 SDL_gpu 视为目前的一个实现，为后续 Metal/OpenGL/Vulkan 保留统一入口，避免再次返工。  
5. **离线或后台 MSDF 生成管线**：结合 glyph atlas 需求，按字库/字号提前生成页，并在构建时校验 range/scale，以保证运行期只做查表和上传。

这些工作会把当前 demo 从“验证功能”推进到文档所描述的“专用化 UI GPU 引擎”：先解决 MSDF 解码与度量误差，紧接着落地 DisplayList→LayerTree→GPUCommandList，最后再扩展多后端和离线 atlas。若你确认优先顺序，我可以从 MSDF 校准和 glyph atlas 分级入手继续实现。

完成要点：诊断了画质问题、对照三份规范梳理了短中期路线。