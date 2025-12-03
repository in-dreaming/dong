简单按你 `next2.md` 的结构对一下现状：

一、短期（画质 P0）

1）MSDF 采样公式的精确校准  
- 已做：
  - 用 msdfgen 的 `scale/translate` + HarfBuzz 的 `scale_to_pixels` 做闭环，算出屏幕空间 pxRange。
  - shader 里用 `ddx/ddy` + `inv_device_pixel_ratio` 做 fwidth 补偿。
  - 通过 `uParams.w` 提供可调 gamma，颜色管线是 linear→sRGB。
- 还没做的“额外项”：
  - 真正的 subpixel mask 渲染路径（现在只有单通道 MSDF，没有 RGB 子像素滤波）。
  - 更高级的“对数/自定义曲线”调节只是预留了参数位，还没扩成一套完整 preset（但不算硬需求）。

=> 按文档里的核心目标（参数来源 + DPR 补偿 + gamma）来说，P0 基本对齐；subpixel 可以视为后续增强项。

2）多级 glyph atlas 策略化设计  
- 已做：
  - 档位改成 `(32, 48, 72, 96)`，每档 `distance_range = 4.0`。
  - 用 `target_msdf_px ≈ ceil(font_size / 1.5)`，在这些档位中选误差最小的一档（误差相同偏向更高分辨率）。
  - GlyphAtlas 本身是持久的，FT_Face 也有缓存（metrics 层和 GPUDriverSDL 各自一层）。
- 还缺的：
  - 更系统的“按字体族/字号”的 atlas/FT_Face 生命周期策略（例如：何时回收、跨 view 的共享池），现在还是“需要时 new，一直留着”，还没做真正的容量管理和淘汰策略。

3）字体度量层（line-height / baseline / GlyphRun）  
- 已做：
  - 用 FreeType + OS/2 的 metrics（`sTypoAscender/Descender/LineGap`）统一行高/baseline。
  - TextShaper 在 design units 空间输出 `units_per_em / scale_to_pixels / line_height_units / ascent_units / descent_units`。
  - Painter 完全用这套 metrics 算 baseline 和行高，早期的 `line_advance = 1.25x` 魔数已经去掉。
  - DisplayList / GPUCommandList / GPUDriver 共享同一套 GlyphRun + metrics。
- 基本可以认为 P0 目标已实现，不再有“度量没搭完”的功能缺口。

二、中长期（架构 vNext）

这里还差的东西比较多：

1）DisplayListBuilder  
- 现状：
  - 已有 `DisplayListBuilder`，Painter 也已经输出 `DisplayList` 并在 GPU 路径使用。
- 欠缺的主要是“工程化”包装：
  - 更系统的 API / 文档化使用方式（但功能本身已经在用，不是硬缺口）。

2）LayerTree + 缓存  
- 现状：
  - 有基于命令栈的 PushLayer/PopLayer，对应 GPU 侧的 Begin/EndIsolatedLayer。
  - GPUDriverSDL 会为 isolate 图层创建 offscreen 纹理并合成。
- 还没做的：
  - 真正的 `LayerTree` 结构（节点上挂滚动/变换/透明度等属性）。
  - 基于 Layer 的 dirty 标记和跨帧缓存：非 dirty Layer 只做 compositing，不重栅格。
  - 系统化的 offscreen render target 管理（当前更接近“即时建一个 layer 纹理就用一次”的风格）。

3）GPUCompiler + 专用 GPUCommandList  
- 已做：
  - 正式的 `GPUCommandType / GPUCommand` IR，去掉了未实现的命令类型。
  - `GPUCompiler` 负责从 DisplayList 产生完整的 `GPUCommandList`。
  - 每条绘制命令带有 `sort_key = pipeline_id + resource_hash`，为“按 pipeline/纹理 key 分组”打好基础。
- 还没做的：
  - 真正基于 `sort_key` 的排序和批次合并：
    - 比如：把同一 pipeline + 同一纹理的一串 DrawRect/DrawImage 合成一个批次。
  - instance buffer + instancing：
    - 现在文字仍然是“每个 glyph 一次 PushUniform + DrawPrimitives”，只是 IR 已经 ready，但还没上真正的 instance buffer 管线。

4）多后端抽象  
- 已做：
  - 抽象接口 `GPUDriver`，并有一个 SDL_gpu 的具体实现 `GPUDriverSDL`。
  - 上层（View）已经通过 `GPUDriver` 接口驱动 GPU 渲染。
- 还没做的：
  - 额外的后端实现（Metal/OpenGL/Vulkan），以及“通过统一入口挂不同后端”的配置/工厂。
  - 部分 API（比如 offscreen 路径）还偏 SDL_gpu 语义，没有完全抽象化。

5）离线/后台 MSDF 生成管线  
- 现状：
  - 所有 MSDF 仍在运行期、在热点路径内通过 msdfgen 即时生成。
- 完全还没做的：
  - 构建期 / 后台线程批量生成 atlas 页。
  - 运行时以查表 + 上传为主，减少 msdfgen 在渲染关键路径里的出现。
  - 构建期对 distance range / scale 做预验证的工具链。

如果一句话总结：

- 画质 P0（MSDF 公式 + 多级 atlas + 字体度量层）已经基本按文档完成，只剩下 subpixel MSDF 和更 aggressive 的 atlas/FT 缓存策略可以作为后续增强；
- 架构 vNext 这块，目前你已经有 DisplayList + 正式版 GPUCommandList + 可拓展的 GPUDriver 抽象，但：
  - LayerTree + 缓存机制、
  - 按 sort_key 真正做排序/合批/instancing 的 GPUCompiler + Driver、
  - 多后端实现、
  - 离线/后台 MSDF 管线  
 这几块还基本是空白或只实现了最薄的一层。

 写一个专门的 demo 或调试开关，打印每帧哪些 layer 走了“重栅格”、哪些走了“缓存复用”，方便肉眼确认缓存命中情况。
或者继续往上推 LayerTree 结构，把滚动 / transform / opacity 也挂到 Layer 上，让这个缓存机制覆盖更多场景。


# 

我按“属性变化类型”帮你列一组最常见、值得用 LayerTree+缓存覆盖的场景，方便你挑优先级：

一类：Scroll（滚动相关）

- 整页滚动（`body/html` 主滚动）
  - 典型：长文档、文章页。
  - 期望：大块静态背景/装饰层只做滚动变换，不重栅格。

- 内嵌滚动容器（`overflow: scroll/auto` 的列表/面板）
  - 典型：聊天窗口、长列表、右侧属性面板。
  - 期望：列表内容 raster 一次，滚动时只移动视口（调整 layer 的 scroll/transform），只在新增/删除/更新 item 时重栅格对应子树。

- 复杂滚动区域上的裁剪
  - 典型：带圆角的滚动卡片、滚动视图内嵌图片/文本。
  - 期望：Clip + scroll 全在 Layer 做合成，不把裁剪后的每一帧当“新几何”画一遍。

二类：Transform（变换相关）

- 卡片 hover / press 动画
  - 典型：卡片、按钮轻微 scale/translate。
  - 期望：内容 raster 成纹理，后续 scale/translate 都用 TransformLayer 合成。

- 弹窗 / Drawer / BottomSheet 的出现/消失动画
  - 典型：从底部滑出、从侧边推入、缩放+淡入。
  - 期望：弹层内容复杂但结构不变时，动画期间只改 transform，不重栅格。

- 图片浏览/预览的平移缩放
  - 典型：大图查看、画布 pan/zoom。
  - 期望：图片所在 Layer 一次 raster，多次 zoom/pan 用合成矩阵搞定。

- Parallax/视差滚动
  - 典型：背景图和前景内容不同滚动速度。
  - 期望：前后景各自是 Layer，通过 transform 叠加，而不是每帧重画整个背景。

三类：Opacity（透明度相关）

- 淡入淡出（fade in/out）
  - 典型：卡片出现/消失、tooltip、toast。
  - 期望：Layer 内容不变时，opacity 动画只改合成 alpha。

- Cross-fade 两个视图/页面
  - 典型：页面 A → 页面 B 的淡入淡出切换。
  - 期望：A/B 各自一层，交叉调 opacity，实现“只重绘变更部分”的过渡。

- 背景蒙层 + 前景弹层
  - 典型：`backdrop + modal`，蒙层渐显/渐隐。
  - 期望：蒙层/弹层各自单独 layer，反复打开关闭只做 opacity 动画。

四类：Scroll + Clip + Transform 组合

- 固定/吸附头部（sticky header）
  - 典型：列表顶部 sticky tab、表头。
  - 期望：header 自成一层，内容滚动只动下面那一层；header 的阴影/渐隐也只在合成层里调。

- 带圆角裁剪的滚动卡片 + 内部动画
  - 典型：卡片内有图片和文本，卡片本身滚动、内部还有小动画。
  - 期望：外层 Clip/scroll/transform 在一个 SurfaceLayer 上处理，内部子节点独立 dirty。

五类：特效/昂贵渲染

- 模糊 / 阴影 / 滤镜
  - 典型：模糊背景、重投影阴影。
  - 期望：效果应用在 layer 纹理上（一次 offscreen pass），后续只搬运缓存结果，不重复跑昂贵 shader。

- 复杂文本/矢量内容
  - 典型：大段富文本、SVG-like 图形。
  - 期望：文本/图形所在 layer raster 一次，后续 scroll/transform/opacity 都在合成阶段处理。

六类：页面级结构

- Tab / Route 切换
  - 典型：多个 tab 页，切换时某些 tab 暂时不可见。
  - 期望：每个 tab/page 是一个 layer，当前活跃 tab 常驻缓存；切换只改可见性/transform/opacity。

- 多面板布局（侧边栏 + 主内容）
  - 典型：IDE、后台管理系统。
  - 期望：侧边栏、主内容、浮动工具栏各自是独立 layer，折叠/展开/resize 时主要调 transform/clip，不重 raster 未变的那部分。

如果只看“下一步最值得做”的优先级，我建议你优先考虑：

1）内嵌滚动容器 + 圆角裁剪（列表/聊天/面板）  
2）弹窗/Drawer 等 overlay 的 transform + opacity 动画  
3）大图平移缩放（Image Viewer/Canvas）  

这三个一搞，LayerTree + 缓存的价值在视觉和性能上会非常直观。