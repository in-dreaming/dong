# Dong 集成 Unreal Engine 草案

> ⚠️ **优先级：低（Long-term Reference）**
>
> 本文档为方向性草案，**Core 团队短期不会投入工程实现**。Phase 1 仅交付 `DongDrawList` C ABI（见 [路线图](../../roadmap.md) P1-1），具体的 UE adapter 由有需求的客户 / 社区 / 内部业务团队驱动。
>
> 上游设计来自 [引擎适配设计（内部）](../../../docs/developer/ideal/引擎适配.md)；本文档把其中"UE 部分"具体化、ABI 草案化。
>
> 状态：v0 草案 2026-04-17。

---

## 1. 集成目标

让 dong 的 UI 能以**原生 Slate / UMG widget** 的形式嵌入 UE 项目，**不创建额外 RenderTarget**，并支持双向嵌套：

- UE UI（UMG / Slate）内嵌 HTML（dong）UI。
- HTML UI 内嵌 UE UI 子树（`<host-view>`）。
- 两者按 z-order 自由穿插。

详细动机与 RT 方案的对比见 `docs/developer/design/引擎适配.md` § 1。

---

## 2. 集成形态总览

```
┌─────────────────────────────────────────────────────┐
│                  UE Application                      │
│ ┌──────────────────────────────────────────────────┐ │
│ │  UMG / Slate widget tree                         │ │
│ │   ├── ... 普通 UE widgets ...                    │ │
│ │   └── UHtmlViewWidget   (我们要做的)              │ │
│ │         └─ SHtmlViewWidget (SLeafWidget)         │ │
│ │             └─ 内含 dong DongView + DrawList 翻译 │ │
│ │             └─ 子节点 (HostViewSlot 对应的)       │ │
│ │                  └─ ... UE widgets ...            │ │
│ └──────────────────────────────────────────────────┘ │
│                                                      │
│  Dong Core (.dll/.so/.dylib)                         │
│   ├─ HTML / CSS / Layout / Script                    │
│   └─ DongDrawList Emitter                            │
│                                                      │
│  UE-side Adapter (.uplugin)                          │
│   ├─ FDongRenderer:    DrawList → FSlateDrawElement  │
│   ├─ FDongResources:   glyph/image atlas → FRHITexture│
│   ├─ FDongInput:       Slate input → dong_engine_send│
│   └─ FDongIME:         Slate IME → dong composition  │
└─────────────────────────────────────────────────────┘
```

**关键：dong 输出的是 `DongDrawList`（Phase 1 P1-1），不是像素。** UE adapter 把它翻译为 `FSlateDrawElement::MakeBox / MakeText / MakeCustomVerts`，一切走 Slate 自己的 batcher。

---

## 3. UE Plugin 架构

### 3.1 文件布局（建议）

```
DongUE.uplugin/
├── DongUE.uplugin
├── Source/
│   ├── DongCore/                  # dong.dll 包装
│   │   ├── DongCore.Build.cs
│   │   └── Public/DongCore.h
│   ├── DongRuntime/               # 运行时模块
│   │   ├── DongRuntime.Build.cs
│   │   ├── Public/
│   │   │   ├── DongView.h         # FDongView (持有 dong_engine_t*)
│   │   │   ├── DongDrawList.h     # FDongDrawListTranslator
│   │   │   └── HtmlViewWidget.h   # SHtmlViewWidget
│   │   └── Private/
│   │       ├── DongView.cpp
│   │       ├── DongDrawList.cpp
│   │       ├── DongInputAdapter.cpp
│   │       ├── DongResourceAdapter.cpp
│   │       └── HtmlViewWidget.cpp
│   └── DongUMG/                   # UMG 包装
│       ├── DongUMG.Build.cs
│       └── Public/UHtmlViewWidget.h
├── Resources/
└── Content/
```

### 3.2 三个核心 UE 类

| 类 | 父类 | 职责 |
|---|---|---|
| `UHtmlViewWidget` | `UWidget` | 蓝图友好的 UMG 包装，参数：HTML 内容/路径、宽高、是否启用 React、HostView slot 子 widgets 列表 |
| `SHtmlViewWidget` | `SLeafWidget`（或 `SCompoundWidget`，含子 slots 时） | Slate 实现，`OnPaint` 调 dong 出 DrawList 并翻译 |
| `FDongDrawListTranslator` | — | 纯函数 / 状态机：吃 `DongDrawCommand` stream，吐 `FSlateDrawElement` |

---

## 4. 核心数据通路

### 4.1 一帧数据流

```
SHtmlViewWidget::Tick()
  ├── 同步尺寸: dong_engine_resize(view, w, h)
  ├── 推送累积输入: FDongInput::FlushQueuedInputs(view)
  └── dong_engine_tick(view)         // 内部 layout + paint + DrawList emit

SHtmlViewWidget::OnPaint(args, geometry, ..., layerId, ...)
  ├── const DongDrawList* dl = dong_engine_get_drawlist(view)
  ├── FDongDrawListTranslator t(allottedGeometry, layerId, outDrawElements)
  └── t.Translate(dl)                // 遍历命令，逐条 emit FSlateDrawElement
       ├── DRAW_SOLID_RECT       → FSlateDrawElement::MakeBox (FCoreStyle::WhiteBrush, color)
       ├── DRAW_ROUNDED_RECT     → MakeBox with custom material 或 MakeCustomVerts
       ├── DRAW_BOX_SHADOW       → 同上 (shadow shader / 9-slice)
       ├── DRAW_IMAGE            → MakeBox(brush_for_dong_texture)
       ├── DRAW_TEXT_RUN         → MakeText 或 MakeCustomVerts (走 dong glyph atlas)
       ├── PUSH_TRANSFORM        → translator 内部维护 transform stack；下条 element 用累积 transform
       ├── PUSH_CLIP_RECT        → outDrawElements.PushClip(...)
       ├── POP_CLIP              → outDrawElements.PopClip()
       ├── PUSH_OPACITY          → translator 维护 opacity stack；color *= opacity 应用
       └── DRAW_HOST_VIEW(slot)  → 找到对应子 SWidget，OnPaint 它，layerId++ 继续
```

### 4.2 DongDrawList 命令草案

详细 ABI 见 Phase 1 P1-1 产出的 `include/dong_drawlist.h`。下表是与 UE adapter 强相关的命令清单：

| 命令 | 字段 | UE 翻译方式 |
|---|---|---|
| `DRAW_SOLID_RECT` | rect, rgba | `MakeBox` 白色 brush + tint |
| `DRAW_ROUNDED_RECT` | rect, rgba, radii[4] | 自定义 material（uber quad shader 转 Slate material） |
| `DRAW_BOX_SHADOW` | rect, blur, color | 同上 |
| `DRAW_BORDER` | rect, widths[4], colors[4] | 拆 4 个 `MakeBox`（或 9-slice material） |
| `DRAW_IMAGE` | rect, dong_texture_id, uv_rect | `MakeBox` + `FSlateBrush(FRHITexture)` |
| `DRAW_TEXT_RUN` | glyphs[], color, font_id, mode (msdf/slug) | `MakeCustomVerts` 走 dong glyph atlas，**避免** `MakeText`（要保证跨引擎像素一致） |
| `PUSH_TRANSFORM(matrix)` | mat3x3 / mat4x4 | 累积到 `FPaintGeometry`，必要时 `MakeBox` 之前用 `geometry.ToPaintGeometry()` 重投影 |
| `POP_TRANSFORM` | — | 出栈 |
| `PUSH_CLIP_RECT(rect)` | rect | `OutDrawElements.PushClip(FSlateClippingZone(...))` |
| `POP_CLIP` | — | `OutDrawElements.PopClip()` |
| `PUSH_OPACITY(alpha)` | float | adapter 内部维护 opacity stack（Slate 没有原生 push/pop opacity） |
| `POP_OPACITY` | — | 出栈 |
| `BEGIN_LAYER` / `END_LAYER` | (kind) | 通常映射为 layer id 增量 |
| `DRAW_HOST_VIEW(slot_id)` | slot_id, rect | 查 `SHtmlViewWidget::SlotMap[slot_id]` → `Child->OnPaint(...)` |

---

## 5. HostView Slot：HTML 内嵌 UE 子树

### 5.1 HTML 写法

```html
<div class="hud">
  <h1>Inventory</h1>
  <host-view id="3d-preview" style="width: 400px; height: 300px;"></host-view>
  <button>Equip</button>
</div>
```

`<host-view>` 是 dong 内置的 replaced element：

- 解析时识别为特殊 tag。
- Layout：当作普通 inline-block，使用 CSS 给定的 width/height（或宿主上报的 intrinsic size）。
- Paint：emit `DRAW_HOST_VIEW(slot_id)` 命令；不绘制任何像素。
- Hit-test：命中后返回该 slot；事件路由由宿主决定。

### 5.2 UE 侧绑定

```cpp
UHtmlViewWidget* HtmlView = ...;
HtmlView->SetSlotWidget(TEXT("3d-preview"), MyViewportWidget);
```

`SHtmlViewWidget::OnArrangeChildren` 根据 dong 上报的 slot rect 摆放子 widget。

### 5.3 输入路由

```
SHtmlViewWidget::OnMouseMove(geometry, event)
  ├── localPos = geometry.LocalToScreen 反算
  ├── dong_engine_hit_test(view, localPos) → returns hit info
  │     ├── 若 hit 是 normal element → dong_engine_send_mouse_move(view, ...)
  │     └── 若 hit 是 host-view slot → 路由给该 slot 的 SWidget
```

---

## 6. 资源契约（字体 / 图片）

### 6.1 GlyphAtlas

dong 的 GlyphAtlas 是 GPU 纹理，UE 需要把它包装为 `FRHITexture` 给 Slate batcher 使用。

**契约**：

```cpp
// dong 已有的 plugin API
DongGPUDriver* drv = ...;  // UE adapter 实现
drv->vtable->upload_texture_subrect(...);  // dong 把 glyph 数据上传到 atlas
void* native_tex = drv->vtable->get_native_texture_handle(drv, atlas_tex);
// native_tex 在 UE adapter 中实际是 FRHITexture*

// Slate batcher 用法
FSlateBrush brush;
brush.SetResourceObject(WrapAsUTexture(native_tex));
FSlateDrawElement::MakeCustomVerts(..., brush, glyphVerts, ...);
```

UE adapter 实现 `DongGPUDriver` vtable：
- `create_texture` → `RHICreateTexture2D`
- `upload_texture_subrect` → `RHIUpdateTexture2D`
- `get_native_texture_handle` → 返回 `FRHITexture*` 强转 `void*`

### 6.2 Image

支持两种路径：

1. **dong 自己解码**：HTML 里 `<img src="...">` 走 dong 的 `DongImageDecoder`，最终通过 `DongGPUDriver` 上传到 UE RHI 纹理。
2. **UE Asset**：HTML 里 `<img src="ue:/Game/Textures/UI/MyIcon">`，adapter 拦截 `ue:` 协议，直接用 UE 已有 `UTexture2D`，dong 拿到 native handle 直接绘制（零拷贝）。

---

## 7. 输入与 IME

### 7.1 输入映射

| Slate 事件 | dong API |
|---|---|
| `OnMouseMove` | `dong_engine_send_mouse_move` |
| `OnMouseButtonDown/Up` | `dong_engine_send_mouse_button` |
| `OnMouseWheel` | `dong_engine_send_mouse_wheel`（注意 sign：`positive = scroll down`，见 AGENTS.md） |
| `OnKeyDown/Up` | `dong_engine_send_key` |
| `OnKeyChar` | `dong_engine_send_text` |
| `OnTouchStarted/Moved/Ended` | （Phase 0+ 之后补）`dong_engine_send_touch` |
| Gamepad | 走 dong 自己的 spatial nav（Phase 0 P0-4），UE adapter 把 gamepad 事件转 nav 命令 |

### 7.2 IME

UE 有 `ITextInputMethodSystem`，dong 有 IME composition 三件套（Phase 0 P0-5）。

UE adapter 要做：
- `ITextInputMethodContext` 的实现：把 dong 的 caret rect 上报给系统 IME；把 IME 候选字符 forward 给 `dong_engine_send_composition_*`。

---

## 8. 渲染顺序与混合模型

| 议题 | 处理 |
|---|---|
| **预乘 alpha** | dong 默认非预乘；Slate 也是非预乘。直接对接 OK。 |
| **z-order** | 一个 `SHtmlViewWidget::OnPaint` 内 layerId 单调递增；遇到 `DRAW_HOST_VIEW` 调子 widget 后 layerId++ 继续。 |
| **clip stack** | 用 `OutDrawElements.PushClip / PopClip`；dong 的 transform 通过 `FPaintGeometry` 传递。 |
| **opacity stack** | adapter 内部 stack；最终乘到每个 element 的 color。 |
| **blend mode** | Slate 默认 `SE_BLEND_TranslucentAlphaOnly`；如需特殊（如 `mix-blend-mode: multiply`）走自定义 material。Phase 0 不支持。 |

---

## 9. 性能考量

| 项 | 注意 |
|---|---|
| **OnPaint 不要每帧重做完整 layout** | dong 内部已有 `commands_dirty_` 机制；没变化时 DrawList 应可复用（Phase 1 P1-1 设计中加 `is_dirty` 标志） |
| **避免 MakeText（走 UE 字体系统）** | 跨引擎像素一致性优先；用 dong glyph atlas + MakeCustomVerts |
| **HostView slot 太多** | Slate 子 widget 树深度增加，OnPaint 递归成本上升；建议单个 HtmlView 内 ≤ 16 个 slot |
| **DongDrawList 内存** | per-frame 重置；adapter 不应缓存指针跨帧使用 |

性能预算与 `docs/developer/perf-budget.md` 的 S2（DOM 业务面板）保持一致；额外加 ~10% 给 Slate batcher 翻译开销。

---

## 10. 不在本草案范围

- ❌ Editor extension（让美术在 UE Editor 里直接预览 / 编辑 HTML）—— 留作社区扩展。
- ❌ Live reload UE 内—— 通过 `DONG_HTML_HOT_RELOAD=1` 环境变量启用 dong 自己的 watch（Phase 1 P1-4），UE 侧无需特殊处理。
- ❌ 把 HTML 当作 World Space material 来用 —— 走 Phase 2 的 `dong_world_overlay_t` API，跟本 adapter 无关。
- ❌ Cross-engine HLSL 翻译—— UE 自己的 RHI 已经处理；adapter 不需要带 shader。

---

## 11. 实施路径（如果有人接手）

| Step | 内容 | 依赖 |
|---|---|---|
| Step 1 | UPlugin 骨架 + UHtmlViewWidget 空实现 + 集成 dong.dll | dong Phase 0 完成 |
| Step 2 | DongDrawList 翻译器 v1：solid_rect / image / text 三个命令走通 | dong Phase 1 P1-1 完成 |
| Step 3 | clip / transform / opacity stack | Step 2 |
| Step 4 | DongGPUDriver vtable for UE RHI（glyph atlas 走 RHI） | Step 2 |
| Step 5 | 输入路由 + IME | Step 1 |
| Step 6 | HostView slot：`<host-view>` ↔ Slate child | dong Phase 1 P1-2 完成 |
| Step 7 | UMG 蓝图节点 + 示例 demo | Step 6 |

合理工时：**4–8 人周**（不含 dong Core 侧的 P1-1 / P1-2）。

---

## 12. 与已有文档的关系

| 文档 | 关系 |
|---|---|
| `docs/overview/positioning.md` | 本草案在"长期低优先级"清单里 |
| `docs/roadmap.md` | Phase 1 P1-1 / P1-2 是本草案的前置；本身不进路线图 |
| `docs/developer/design/引擎适配.md` | 上游设计；本文档是其 UE 部分的具体化 |
| `docs/guide/integrations/unity.md` | 平行草案（Unity 路线） |

---

## 13. 决策记录

| 日期 | 决策 | 说明 |
|---|---|---|
| 2026-04-17 | 标为低优先级长期参考 | dong Core 团队短期不投入 UE adapter 工程实现 |
| 2026-04-17 | 字体走 dong glyph atlas 而非 Slate FSlateFontMeasure | 保持跨引擎像素一致性（同步决定 Unity adapter 也走相同路） |
