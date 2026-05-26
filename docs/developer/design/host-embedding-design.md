# 引擎适配：HTML/CSS/DOM UI 嵌入 Unity/UE（不额外RT，可与原生UI互相嵌套叠加）

> 目标：把基于 HTML+CSS+DOM 的 UI（以 Dong 为代表）以“原生 UI 节点”的形态嵌入到任意游戏引擎（Unity/Unreal/自研）中，并且**不创建额外 RenderTexture / RenderTarget**，同时支持：
> - 引擎 UI 内嵌 HTML UI
> - HTML UI 内嵌 引擎 UI
> - 两者可互相叠加、穿插（例如三层：引擎UI / HTML UI / 引擎UI）

---

## 1. 背景：为什么“渲染到 RT 再贴图”不够

“HTML 渲染到离屏 RT，再把 RT 当成一张图片贴到引擎 UI/3D 物体上”是最简单路径，但与你的约束冲突：

- **开销大**：每个 HTML 视图一套 RT（尺寸变化/多实例/多层叠加会指数增长），且很难复用引擎现有 RT 生命周期与 RenderGraph。
- **无法真正互相嵌套/穿插**：RT 方案把 HTML UI 变成一个“扁平贴图层”，
  - 引擎 UI 只能在它上面或下面，不能在它内部按 DOM 层级插入。
  - HTML UI 也无法把引擎 UI 当作 DOM 的一个子节点进行布局/裁剪/滚动。

因此要达成“任意层次互相嵌套”，核心是：

> **让 HTML UI 和 引擎 UI 共享同一个合成/绘制序列（同一套 clip/transform/z-order），而不是把其中一方先光栅化成贴图。**

---

## 2. 总体思路（引擎无关）：把 HTML 渲染输出变成“UI 绘制指令”，交给宿主 UI 合成

把 HTML UI 的输出从“像素结果（RT）”改为“可合成的绘制指令（Draw Commands）”。

- HTML/CSS/DOM → layout → **RenderIR / DrawList**（类似：矩形/圆角/阴影/图片/文字/裁剪/透明度/层）
- 引擎适配层把 DrawList 转换成引擎 UI 的原生绘制元素（Slate/UMG DrawElement、uGUI Mesh、UI Toolkit Mesh 等）
- 最终由引擎 UI 的渲染管线在同一个 pass（或同一条 UI pass 链路）完成合成。

这样：
- **不需要额外 RT**：HTML UI 只是“多了一批 UI 三角形/文字”进入引擎 UI 渲染。
- **天然支持叠加/穿插**：只要双方都能在同一 draw list 里插入“子树绘制”，就能实现 3 层甚至 N 层混排。

---

## 3. 双向嵌入（必须同时支持）

为了做到「引擎 UI 内嵌 HTML」和「HTML 内嵌引擎 UI」并且可穿插，推荐把嵌入抽象成两类节点：

### 3.1 HTMLView：引擎 UI 的一个 Widget

- 引擎侧提供一个 `HtmlViewWidget`（UE: `SWidget/UWidget`，Unity: `Graphic/VisualElement`）
- 引擎 UI 在 layout/paint 阶段回调该 widget：
  - `Measure(constraints)`：返回期望尺寸
  - `Paint(paintContext)`：把 HTML 渲染产生的 DrawList 转成引擎 draw elements

### 3.2 HostViewSlot：HTML DOM 中的一个“宿主占位元素”（承载引擎 UI 子树）

- HTML DOM 提供一个特殊元素（概念上类似 replaced element）：
  - 例如 `<host-view id="mini-map" />` 或自定义 tag
- layout 阶段：宿主给该 slot 提供测量回调（intrinsic size / min/max / stretch）
- paint 阶段：当渲染走到该 slot，触发宿主绘制该引擎 UI 子树，并继承 HTML 的 clip/transform/opacity

> 这个 slot 是实现“引擎 UI 在 HTML 里当子节点”的关键。

---

## 4. 与当前 Dong 工程的对齐点（渲染链路）

仓库现状（见 `docs/developer/arch/libraries-and-legacy.md`、`docs/developer/arch/libraries-and-legacy.md`）：

- core 侧：DOM/CSS/Layout → `Painter` 构建 DisplayList → `GPUCompiler` 产出 `GPUCommandList`
- backend 侧：`DongGPUDriver->execute(command_list)` 负责真正提交 GPU

要做到“引擎 UI 合成（非 RT）”，有两条架构路线：

### 路线 A（推荐，长期稳定）：新增**稳定的 RenderIR/DrawList C ABI**

- core 输出一个**引擎无关、可版本化**的 `DongDrawList`（而不是把内部 `GPUCommandList` 暴露给引擎）
- 引擎适配层只需要实现：
  - 把 `DongDrawList` 翻译成引擎的 UI draw elements
  - 资源上传接口（glyph/image）走 `DongGPUDriver` 或单独的 `ResourceOps`

优点：
- 适配任意引擎（不依赖 SDL/图形 API）
- DrawList 可以做版本管理、功能子集清晰
- 引擎侧无需包含内部头文件

### 路线 B（短期快速）：把引擎 UI 当作一种 `DongGPUDriver` 后端

- 在引擎内实现一个 `DongGPUDriver`：
  - `execute(command_list)` 解析 `GPUCommandList` 并在 UI pass 里生成 draw elements

缺点：
- `GPUCommandList` 是内部 IR，不利于长期兼容
- 引擎适配层要跟随内部结构变化

> 文档后续讨论以路线 A 为主（可扩展、可适配任意引擎）。

---

## 5. RenderIR / DrawList 设计要点（为了“可穿插嵌套”）

要支持“HTML 与引擎 UI 任意层次互嵌”，RenderIR 必须具备：

### 5.1 状态栈：Transform / Clip / Opacity / Z

- `PushTransform(matrix)` / `PopTransform()`
- `PushClipRect(rect)` / `PopClip()`（至少支持矩形；圆角 clip 可选）
- `PushOpacity(alpha)` / `PopOpacity()`
- `PushLayer(kind)` / `PopLayer()`（用于隔离层、混合、缓存提示）

引擎适配层要把这些映射到：
- UE Slate 的 clip state + render transform + layer id
- Unity 的 mesh + stencil/clip（或 UI Toolkit 的 clip stack）

### 5.2 基础绘制原语（RT-free 子集）

建议 RenderIR v1 聚焦“无需离屏”的 CSS 子集：

- `DrawSolidRect` / `DrawRoundedRect`
- `DrawBorder`（可拆成 4 条 rect 或 9-slice）
- `DrawImage`（UV + 9-slice 可选）
- `DrawTextRun`（字形 run + glyph atlas 引用）
- `DrawBoxShadow`（可用 SDF/9-slice 近似；或拆多层）

明确标注需要离屏的特性（v1 不保证）：
- `filter: blur()`、backdrop-filter、复杂 mask、mix-blend-mode 的部分模式等

> 结论：要“零 RT”，就必须接受“CSS 支持分级”。不影响架构，影响的是 feature matrix。

### 5.3 嵌入点命令（关键）

为了双向嵌入，需要两类“回调命令”：

- `DrawHostView(slot_id)`：绘制宿主 UI 子树（HTML 内嵌引擎 UI）
- `DrawHtmlView(view_id)`：宿主 UI 里也可以有 HTML 子 view（引擎 UI 内嵌 HTML）

实现方式：
- RenderIR 里记录一个 `EmbedNode { type, id, rect, clip, transform }`
- 引擎适配层在遍历到它时：
  - 调用宿主回调绘制对应子树
  - 并把当前 clip/transform 传递给子树

---

## 6. 输入与命中：同一棵“可穿插 UI 树”

如果渲染能穿插，输入也必须穿插。

### 6.1 统一命中/事件分发

建议定义 `IHitTest`/`IEventRouter`：
- 引擎先做一次 UI 级 hit-test，定位到最上层 widget
- 若命中的是 `HtmlViewWidget`：
  - 把屏幕坐标映射到 HTML 坐标（包含 transform/scroll）
  - 调用 `dong_engine_send_*` 或等价接口
- 若命中的是 `HostViewSlot`（HTML 内嵌引擎 UI）：
  - HTML hit-test 先返回 slot
  - 宿主再把事件路由给该 slot 对应的引擎 UI 子树

### 6.2 文本输入与 IME

- HTML 输入框需要宿主提供：
  - composition string
  - caret rect（用于 IME 候选框定位）

建议在 RenderIR 或独立 API 提供：
- `GetCaretRect()` / `GetSelectionRects()`

---

## 7. 资源与字体：不依赖 RT 的关键是“纹理复用与上传契约”

无 RT 不代表没有纹理：文字和图片仍然需要纹理/atlas。

### 7.1 GlyphAtlas / ImageAtlas

对齐当前工程方向（见 `docs/developer/arch/libraries-and-legacy.md`）：
- `GlyphAtlas` 逻辑应在 core
- GPU 上传通过注入能力完成（`DongGPUDriver.upload_texture_subrect` / fence）

引擎适配层需要提供：
- 创建纹理（atlas page）
- 子矩形上传（stride + fence）
- 返回 native texture handle（供引擎 UI 材质绑定）

### 7.2 “引擎字体系统 vs 自研字体系统”

两种策略都要支持：

- **自研字体（推荐一致性）**：HarfBuzz/FreeType/MSDF → glyph atlas → RenderIR `DrawTextRun` 直接引用 atlas
- **复用引擎字体（推荐集成体验）**：适配层把 `DrawTextRun` 转为引擎的文字绘制 API

可行的折中：
- v1 先走自研字体（可控、跨引擎一致）
- v2 增加 `HostTextOps`：允许宿主提供 glyph rasterization，core 仍保留 shaping 与布局

---

## 8. 引擎适配层分层（保证“可适配任意引擎”）

建议把适配拆成三层，避免 UE/Unity 逻辑侵入 core：

1) **Core（引擎无关）**
   - DOM/CSS/Layout
   - RenderIR/DrawList 生成
   - 资源策略（glyph/image）

2) **Host Abstraction（引擎抽象）**
   - `HostRenderer`：接收 DrawList，发出 draw primitives
   - `HostResources`：纹理创建/上传/句柄
   - `HostViewEmbedding`：slot 的 measure/paint 回调
   - `HostInput`：输入、IME、剪贴板、光标

3) **Engine Adapter（引擎实现）**
   - UE 实现一套 Host*（映射到 Slate/UMG + RHI）
   - Unity 实现一套 Host*（映射到 uGUI/UI Toolkit + SRP）

---

## 9. Unreal Engine 落地方式（概念到抓手）

### 9.1 HTML 作为 UE Widget

- UMG：`UHtmlViewWidget : UWidget`
- Slate：内部持有 `SHtmlViewWidget : SLeafWidget` 或 `SCompoundWidget`

在 `OnPaint`：
- 拉取/缓存 `DongDrawList`
- 转为 `FSlateDrawElement`：
  - 图片/圆角矩形：`MakeBox` / `MakeCustomVerts`
  - 文字：优先 `MakeText`（走 Slate 字体）或 `MakeCustomVerts`（走自研 glyph atlas）
  - clip/transform：使用 Slate 的 `FPaintGeometry` / clipping state

### 9.2 引擎 UI 嵌入 HTML（slot）

- 每个 `<host-view id>` 对应一个 `SWidget` 子树
- HTML layout 输出 slot 的矩形（相对 HtmlView）
- UE 侧把 slot 子 widget 挂到 HtmlViewWidget 的 children 里，并在 `OnArrangeChildren` 中按矩形摆放
- 渲染顺序：
  - HtmlViewWidget 在遍历 DrawList 时遇到 `DrawHostView(id)`，将该 child widget 的 layer 插入到当前 layer 之间

> 关键点：Slate 本身支持 children 的 paint 递归，适配层要把“slot 的 draw 顺序”与“HTML 的 z-order”对齐。

---

## 10. Unity 落地方式（uGUI 与 UI Toolkit 都要）

### 10.1 uGUI 路线

- `HtmlViewGraphic : MaskableGraphic`
  - `OnPopulateMesh(VertexHelper vh)`：把 DrawList 中的 rect/image/text 转为三角形
  - clip：使用 `RectMask2D`/stencil（需要把 clip 栈映射为嵌套 mask，或做 CPU 侧裁剪）

slot（引擎 UI 嵌入 HTML）：
- `<host-view id>` 对应一个子 `RectTransform`（挂载任意 uGUI 子树）
- HTML layout 输出 slot rect → 更新该 RectTransform 的 anchored position/size
- Z 顺序：通过 sibling index/Canvas sorting 结合 HTML z-index 规则进行映射

### 10.2 UI Toolkit 路线

- `HtmlViewElement : VisualElement`
  - `generateVisualContent`：向 `MeshGenerationContext` 提交 mesh
  - clip/transform：利用 UI Toolkit 自带的 clip stack 与 transform

slot：
- `<host-view id>` 对应一个子 `VisualElement`
- 同样由 HTML layout 驱动其 `style.left/top/width/height`

---

## 11. 性能与工程化（避免“每帧全量重建”）

为了不比 RT 方案更慢，关键在于：

- **脏矩形/增量**：DOM/CSS/Layout 的 invalidation 要尽量细。
- **DrawList 可缓存**：对 stable 子树输出可缓存，遇到输入/动画局部失效。
- **资源准备与执行分离**：对齐当前 `prepare_resources` + `execute` 的方向，避免在 paint 中临时创建纹理。
- **引擎批次友好**：DrawList 合并同材质/同纹理的段，减少 draw call。

---

## 12. 风险与边界（必须提前写清）

- **CSS 特性分级**：不额外 RT 的前提下，部分 CSS（filter/backdrop、复杂 mask）需要降级或引擎特化实现。
- **文本一致性**：走引擎字体 vs 自研字体，会带来指标差异；需要统一度量（baseline、line-height）。
- **剪裁栈映射**：Unity uGUI 的 clip/mask 能力有限，复杂 clip 可能需要 CPU 裁剪或 stencil 方案。
- **渲染顺序与透明度**：不同引擎的 UI 合成规则不同（预乘 alpha、批次重排），适配层必须对齐混合模型。

---

## 13. 推荐实施路径（从可跑到可扩展）

1) 定义 `DongDrawList`（RenderIR v1）与版本号（仅 RT-free 子集）
2) 实现一个最小 HostRenderer：
   - UE：SWidget 输出矩形/图片/文字
   - Unity：uGUI Graphic 输出矩形/图片（先不做复杂 clip）
3) 加入 `HostViewSlot`：先支持 layout + hit-test + 简单 clip
4) 完善资源上传契约（glyph/image atlas + fence）
5) 扩展 CSS 支持等级与性能回归门槛（对齐 repo 的 baseline/diff 工具链）

---

## 14. 与“只用 HTML 方案”的关系

上述架构的关键不是“必须嵌引擎 UI”，而是：

- HTML UI 作为主 UI：宿主 UI 只负责承载一个 HtmlViewWidget
- 需要时再把引擎 UI 作为 HostViewSlot 插入

因此可以做到：
- **默认全 HTML**（跨引擎一致）
- **按需嵌入引擎 UI**（例如复用引擎的输入法候选框、平台控件、或特定 HUD）

---

## 15. 下一步需要你确认的细节（会影响接口设计）

1) HTML UI 需要支持哪些“必须无 RT”的效果清单？（圆角/阴影/裁剪/半透明/混合模式）
2) 文字渲染优先：一致性（自研）还是原生体验（引擎字体）？是否允许双路径？
3) slot 的交互：是否需要 slot 内部滚动、拖拽穿透、焦点切换？

