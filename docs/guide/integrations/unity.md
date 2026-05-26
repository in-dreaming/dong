# Dong 集成 Unity 草案

> ⚠️ **优先级：低（Long-term Reference）**
>
> 本文档为方向性草案，**Core 团队短期不会投入工程实现**。Phase 1 仅交付 `DongDrawList` C ABI（见 [路线图](../../roadmap.md) P1-1），具体的 Unity adapter 由有需求的客户 / 社区 / 内部业务团队驱动。
>
> 上游设计来自 [引擎适配设计（内部）](../../../docs/developer/ideal/引擎适配.md)；本文档把其中"Unity 部分"具体化、ABI 草案化。
>
> 状态：v0 草案 2026-04-17。

---

## 1. 集成目标

让 dong 的 UI 能以**原生 Unity UI 元素**的形式嵌入 Unity 项目，**不创建额外 RenderTexture**，并支持双向嵌套：

- Unity UI（uGUI / UI Toolkit）内嵌 HTML（dong）UI。
- HTML UI 内嵌 Unity UI 子树（`<host-view>`）。
- 两者按 sorting 自由穿插。

详细动机与 RT 方案的对比见 `docs/developer/design/引擎适配.md` § 1。

---

## 2. 双路径策略：uGUI 与 UI Toolkit 都要支持

| 路线 | 优势 | 取舍 |
|---|---|---|
| **uGUI**（`MaskableGraphic`） | 老项目兼容、生态成熟、几乎所有 Unity 项目都能用 | clip 能力弱（`RectMask2D` / Stencil），复杂 clip 需要 CPU 裁剪 |
| **UI Toolkit**（`VisualElement` + `MeshGenerationContext`） | 现代、原生支持 transform / clip stack / SDF | Unity 2021+ 才稳定，老项目不友好 |

**建议两条路并行**，在 plugin 内提供两个组件，开发者按项目选用：

```
DongUnity.unitypackage
├── Runtime/
│   ├── Core/                       # P/Invoke + 通用 adapter
│   ├── uGUI/HtmlViewGraphic.cs     # MaskableGraphic 实现
│   └── UIToolkit/HtmlViewElement.cs # VisualElement 实现
├── Plugins/
│   ├── x86_64/dong.dll
│   ├── osx/dong.bundle
│   └── linux/libdong.so
└── Editor/
    └── DongInspector.cs            # 蓝图友好的 inspector
```

---

## 3. P/Invoke 桥与生命周期

### 3.1 P/Invoke 包装

```csharp
public static class DongNative
{
    const string DLL = "dong";

    [DllImport(DLL)] public static extern IntPtr dong_engine_create(ref EngineDesc desc);
    [DllImport(DLL)] public static extern void dong_engine_destroy(IntPtr engine);
    [DllImport(DLL)] public static extern int dong_engine_load_html(IntPtr engine, [MarshalAs(UnmanagedType.LPUTF8Str)] string html);
    [DllImport(DLL)] public static extern int dong_engine_resize(IntPtr engine, uint w, uint h);
    [DllImport(DLL)] public static extern int dong_engine_tick(IntPtr engine);

    [DllImport(DLL)] public static extern void dong_engine_send_mouse_move(IntPtr engine, int x, int y);
    [DllImport(DLL)] public static extern void dong_engine_send_mouse_button(IntPtr engine, int btn, int pressed);
    [DllImport(DLL)] public static extern void dong_engine_send_mouse_wheel(IntPtr engine, float dx, float dy);
    [DllImport(DLL)] public static extern void dong_engine_send_key(IntPtr engine, uint key, int pressed);
    [DllImport(DLL)] public static extern void dong_engine_send_text(IntPtr engine, [MarshalAs(UnmanagedType.LPUTF8Str)] string text);

    // Phase 1 P1-1 之后
    [DllImport(DLL)] public static extern IntPtr dong_engine_get_drawlist(IntPtr engine);
    // ... iter / inspect drawlist commands
}
```

### 3.2 生命周期

- `MonoBehaviour.OnEnable` → `dong_engine_create`
- `OnDisable` / `OnDestroy` → `dong_engine_destroy`
- `Update` 推送输入 + `dong_engine_tick`
- `OnPopulateMesh` (uGUI) / `generateVisualContent` (UI Toolkit) 翻译 DrawList

---

## 4. uGUI 路径

### 4.1 类设计

```csharp
[RequireComponent(typeof(CanvasRenderer))]
public class HtmlViewGraphic : MaskableGraphic, IPointerClickHandler, ...
{
    [SerializeField] string htmlContent;
    [SerializeField] string htmlPath;
    [SerializeField] int viewWidth = 1024;
    [SerializeField] int viewHeight = 768;

    IntPtr engine;
    DongDrawListTranslator translator;

    protected override void OnEnable()
    {
        var desc = new EngineDesc { ... };
        engine = DongNative.dong_engine_create(ref desc);
        DongNative.dong_engine_load_html(engine, htmlContent);
        translator = new DongDrawListTranslator();
    }

    void Update()
    {
        DongNative.dong_engine_resize(engine, (uint)viewWidth, (uint)viewHeight);
        DongInputAdapter.FlushInputs(engine);
        DongNative.dong_engine_tick(engine);
        SetVerticesDirty();
    }

    protected override void OnPopulateMesh(VertexHelper vh)
    {
        vh.Clear();
        IntPtr dl = DongNative.dong_engine_get_drawlist(engine);
        translator.Translate(dl, vh, rectTransform.rect);
    }
}
```

### 4.2 OnPopulateMesh 翻译规则

| dong 命令 | uGUI 实现 |
|---|---|
| `DRAW_SOLID_RECT` | 4 个 `UIVertex` → `vh.AddUIVertexQuad`，颜色取 rgba |
| `DRAW_ROUNDED_RECT` | 同上，但 UV 编码 radii，材质走自定义 shader（uber quad shader 移植到 Unity ShaderLab） |
| `DRAW_BOX_SHADOW` | 同上，shader 内做 SDF + blur |
| `DRAW_IMAGE` | 切 sub-mesh：`material.mainTexture = dongTexture` 后 emit quad；多个 image 不同 texture 会分多个 sub-mesh |
| `DRAW_TEXT_RUN` | emit glyph quads，UV 指向 dong glyph atlas；走 `text` shader（msdf / slug） |
| `PUSH_TRANSFORM(matrix)` | translator 内部维护 matrix stack，每个顶点位置应用累积 matrix |
| `PUSH_CLIP_RECT` | 在 translator 内做 CPU 裁剪（顶点级别），或转 `RectMask2D` |
| `PUSH_OPACITY` | translator 内部 stack，乘到顶点 color |
| `DRAW_HOST_VIEW(slot_id)` | flush 当前 mesh，子 RectTransform 由 Unity 自然渲染 |

### 4.3 多材质问题

uGUI 的 `MaskableGraphic` 一个组件只能用一个材质。dong 同时有多种命令（rect / image / text）的混合是常态，解法：

1. **方案 A：每个 dong view 拆多个 Graphic** — 一个 root + 子 GameObject 含 image graphic / text graphic / shape graphic。复杂。
2. **方案 B：自定义 `BaseMeshEffect` + `IMeshModifier`** — 在 `OnPopulateMesh` 内 emit 全部，但不同材质走 sub-mesh。Unity 不直接支持 sub-mesh，要绕。
3. **方案 C（推荐）：自定义 CanvasRenderer 用法** — 一个 GameObject 多个 `CanvasRenderer.SetMesh(mesh, material, texture)`。

这部分 adapter 实现复杂度较高；UI Toolkit 路径不存在该问题。

---

## 5. UI Toolkit 路径

### 5.1 类设计

```csharp
public class HtmlViewElement : VisualElement
{
    IntPtr engine;
    DongDrawListTranslator translator;

    public HtmlViewElement()
    {
        var desc = new EngineDesc { ... };
        engine = DongNative.dong_engine_create(ref desc);
        translator = new DongDrawListTranslator();
        generateVisualContent += OnGenerateVisualContent;
        RegisterCallback<GeometryChangedEvent>(OnGeometryChanged);
    }

    public void LoadHtml(string html)
    {
        DongNative.dong_engine_load_html(engine, html);
        MarkDirtyRepaint();
    }

    void OnGenerateVisualContent(MeshGenerationContext mgc)
    {
        DongNative.dong_engine_tick(engine);
        IntPtr dl = DongNative.dong_engine_get_drawlist(engine);
        translator.Translate(dl, mgc, contentRect);
    }
}
```

### 5.2 翻译规则

| dong 命令 | UI Toolkit 实现 |
|---|---|
| 几何 | `mgc.Allocate(vertCount, indexCount, atlasTexture).SetAllVertices(...)` |
| `PUSH_TRANSFORM` | translator 内部 stack；UI Toolkit 已支持 element 级 transform，但绘制内部 transform 还是 translator 处理 |
| `PUSH_CLIP_RECT` | UI Toolkit 自带 clip stack（VisualElement.style.overflow），可考虑包嵌套 child VisualElement；或在 translator 内做顶点裁剪 |
| `DRAW_HOST_VIEW(slot_id)` | 该 slot 对应一个子 VisualElement，由 UI Toolkit 自动渲染 |

UI Toolkit 路径**总体更干净**，推荐为 Unity 2021+ 项目的默认路径。

---

## 6. HostView Slot：HTML 内嵌 Unity 子树

### 6.1 HTML 写法（与 UE adapter 完全相同）

```html
<host-view id="3d-preview" style="width: 400px; height: 300px;"></host-view>
```

### 6.2 Unity 侧绑定（uGUI）

```csharp
htmlView.SetSlotChild("3d-preview", myChildRectTransform);
// 内部：myChildRectTransform 作为 htmlView 的 RectTransform 子物体，
// 每帧根据 dong 上报的 slot rect 更新 anchoredPosition / sizeDelta
```

### 6.3 Unity 侧绑定（UI Toolkit）

```csharp
htmlView.SetSlotChild("3d-preview", myChildVisualElement);
// 子 VisualElement 的 style.left / top / width / height 由 dong 驱动
```

---

## 7. 资源契约（字体 / 图片）

### 7.1 GlyphAtlas

dong 的 GlyphAtlas 通过 `DongGPUDriver` plugin 上传到 Unity 的 `Texture2D`。

Unity adapter 实现 `DongGPUDriver` vtable（用 native plugin C++ 写）：
- `create_texture` → `Texture2D` 包装为 `IntPtr`（用 GraphicsBuffer / Texture2D.GetNativeTexturePtr）
- `upload_texture_subrect` → `Texture2D.SetPixels32` 后 `Apply` 或更高效的 `Graphics.CopyTexture`
- `get_native_texture_handle` → 返回 native texture pointer

由于 P/Invoke + GPU 资源跨界比较麻烦，**建议先做 CPU 路径**（dong 出 CPU bitmap，C# 上传 Texture2D），待 perf 不达标再做 native plugin 走 GPU 直传。

### 7.2 Image

支持两种路径：

1. **dong 自己解码**：`<img src="...">` → dong 内部 atlas → 上传 Unity Texture2D。
2. **Unity Asset**：`<img src="unity:Assets/UI/MyIcon">`，adapter 拦截 `unity:` 协议，直接拿 `Texture2D`，dong 拿 native handle 绘制（零拷贝）。

---

## 8. 输入与 IME

### 8.1 输入映射

| Unity 事件 | dong API |
|---|---|
| `IPointerMoveHandler.OnPointerMove` | `dong_engine_send_mouse_move` |
| `IPointerDownHandler / IPointerUpHandler` | `dong_engine_send_mouse_button` |
| `IScrollHandler.OnScroll` | `dong_engine_send_mouse_wheel`（注意 sign） |
| `Input.inputString` / new Input System `Keyboard.current.onTextInput` | `dong_engine_send_text` |
| Key event | `dong_engine_send_key` |
| Touch (Mobile) | dong touch API（Phase 0 之后补） |
| Gamepad | dong spatial nav (Phase 0 P0-4) |

uGUI 需手动实现 EventSystem handler；UI Toolkit 用 `RegisterCallback<MouseMoveEvent>` 等。

### 8.2 IME

Unity 的 `TouchScreenKeyboard` / `Input.imeCompositionMode` 与 dong composition 三件套（Phase 0 P0-5）需要桥接。

简单实现：监听 `Event.current.imeCompositionMode == Compositing` 期间的 `Input.compositionString`，转 forward 到 dong。

---

## 9. 渲染顺序与混合模型

| 议题 | uGUI 处理 | UI Toolkit 处理 |
|---|---|---|
| **预乘 alpha** | uGUI 默认非预乘，OK | UI Toolkit 默认非预乘，OK |
| **z-order** | Canvas 内按 hierarchy 顺序；HostView slot 是子 GameObject 自动正确 | VisualElement 树内自然顺序 |
| **clip stack** | `RectMask2D` / Stencil；复杂 clip 走 translator 内 CPU 裁剪 | UI Toolkit 自带 clip stack |
| **opacity** | translator 维护 stack，乘到顶点 color | 同上，或用 VisualElement.style.opacity |
| **blend mode** | 自定义 ShaderLab pass | 自定义 shader |

---

## 10. 性能考量

| 项 | 注意 |
|---|---|
| **P/Invoke 跨界开销** | 每次 `dong_engine_*` 调用都有 P/Invoke 成本。批量化输入：每帧 flush 一次而非一次一调。 |
| **OnPopulateMesh 频率** | 只在 dong DrawList dirty 时调用 SetVerticesDirty；否则 uGUI 缓存 mesh |
| **GC 压力** | translator 内部用 NativeArray / 池化对象，避免每帧分配 |
| **mobile** | iOS IL2CPP 下 P/Invoke 有限制；建议 dong 提供 `dong_engine_tick_batch(...)` 等批量接口减少跨界次数 |

性能预算与 `docs/developer/perf-budget.md` 的 S2（DOM 业务面板）保持一致；额外加 ~15% 给 P/Invoke + uGUI batcher 翻译开销（uGUI 比 Slate / UI Toolkit 都重）。

---

## 11. 不在本草案范围

- ❌ Editor extension（让美术在 Unity Inspector 里直接预览 / 编辑 HTML）。
- ❌ Unity 主线程外的 dong 调用（dong 与 QuickJS 都不是线程安全的；只能在主线程）。
- ❌ Asset Bundle 内的 HTML 资源管理 —— 业务层用 `TextAsset` 即可。
- ❌ HDR 输出 —— Unity HDRP / URP 自带，dong 这边只要走线性空间颜色就行（Phase 1 P1-6）。

---

## 12. 实施路径（如果有人接手）

| Step | 内容 | 依赖 |
|---|---|---|
| Step 1 | UPM package 骨架 + dong.dll P/Invoke 桥 | dong Phase 0 完成 |
| Step 2 | UI Toolkit `HtmlViewElement`：solid_rect / image / text 三个命令走通 | dong Phase 1 P1-1 完成 |
| Step 3 | uGUI `HtmlViewGraphic`（subset：能跑就行，不追求完整 clip / 多材质） | Step 2 |
| Step 4 | clip / transform / opacity stack（CPU 裁剪 fallback） | Step 2 |
| Step 5 | `DongGPUDriver` native plugin for Unity（GPU 直传字体 atlas） | Step 2 |
| Step 6 | 输入路由 + IME | Step 1 |
| Step 7 | HostView slot：`<host-view>` ↔ Unity 子 GameObject / VisualElement | dong Phase 1 P1-2 完成 |
| Step 8 | UPM package 发布 + 示例 demo | Step 7 |

合理工时：**6–10 人周**（不含 dong Core 侧的 P1-1 / P1-2）。比 UE adapter 略重，主要是 uGUI 多材质处理 + P/Invoke mobile 兼容。

---

## 13. 与已有文档的关系

| 文档 | 关系 |
|---|---|
| `docs/overview/positioning.md` | 本草案在"长期低优先级"清单里 |
| `docs/roadmap.md` | Phase 1 P1-1 / P1-2 是本草案的前置；本身不进路线图 |
| `docs/developer/design/引擎适配.md` | 上游设计；本文档是其 Unity 部分的具体化 |
| `docs/guide/integrations/unreal-engine.md` | 平行草案（UE 路线） |

---

## 14. 决策记录

| 日期 | 决策 | 说明 |
|---|---|---|
| 2026-04-17 | 标为低优先级长期参考 | dong Core 团队短期不投入 Unity adapter 工程实现 |
| 2026-04-17 | uGUI 与 UI Toolkit 都做 | uGUI 兼容老项目，UI Toolkit 是未来；不强制业务方升级 |
| 2026-04-17 | 字体走 dong glyph atlas 而非 Unity TMP | 跨引擎像素一致性优先（同 UE adapter 决策） |
