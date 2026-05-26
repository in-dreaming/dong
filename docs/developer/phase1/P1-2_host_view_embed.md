# P1-2 — `<host-view>` 嵌入元素 + `DrawHostView` 命令

> 上游：[`docs/roadmap.md`](../roadmap.md) Phase 1 P1-2
> 设计原案：[`docs/developer/design/引擎适配.md`](../ideal/引擎适配.md) § 3.2 / § 5.3
> 前置：[P1-1 DongDrawList C ABI](./P1-1_drawlist_c_abi.md)
>
> 状态：方案 v1 / 2026-04-17

---

## 1. 目标

在 dong 的 HTML 里支持一种"占位元素"，让宿主 UI / 3D 子树作为 DOM 子节点参与布局、clip、transform、opacity、hit-test：

```html
<div class="hud-panel">
  <h1>Inventory</h1>
  <host-view id="3d-preview" style="width: 400px; height: 300px;"></host-view>
  <button>Equip</button>
</div>
```

收益：

- 引擎 UI 与 HTML UI 真正"互嵌"（[`positioning.md`](../positioning.md) 总目标之一）。
- HTML 控制布局（弹性 / sticky / scroll），宿主控制内容（3D viewport / native widget / 视频）。
- 与 [P1-1 DrawList](./P1-1_drawlist_c_abi.md) `DONG_DRAW_HOST_VIEW` 命令配套，宿主 adapter 直接消费。

---

## 2. 现状

| 项 | 状态 |
|---|---|
| `<host-view>` HTML 标签 | ❌ |
| Replaced element 通用框架 | 部分（`<img>` `<video>` 已是 replaced） |
| 嵌入命令 `DRAW_HOST_VIEW` | ❌（依赖 P1-1） |
| 宿主 hit-test 反馈机制 | ❌ |
| Slot intrinsic size 上报 | ❌ |

代码索引：
- HTML 解析：`dong/src/dom/html/html_parser.{hpp,cpp}`
- DOM 节点：`dong/src/dom/dom/dom_node.cpp`
- Layout：`dong/src/layout/layout_engine.cpp`（含 replaced element 路径）
- Hit-test：`dong/src/core/engine_view.cpp` 内部 `hitTestElementAt`
- Painter：`dong/src/render/painter.cpp` / `painter/painter_media.cpp`

---

## 3. 设计

### 3.1 HTML 元素

新增**自定义元素 tag**：`<host-view>`。

| 属性 | 描述 |
|---|---|
| `id` | 元素 id（HTML 标准） |
| `slot` | 给宿主用的稳定字符串名称（独立于 DOM id；可重复，宿主自行命名空间） |
| `intrinsic-width` `intrinsic-height` | 默认 intrinsic size（px）；宿主未上报时使用 |
| `aspect-ratio` (CSS) | 与 `<img>` 同样 valid |
| `data-*` | 透传给宿主 callback 的自由字段 |

行为：

- DOM 树中是普通 `Element`，但 `displayKind = Replaced`（与 `<img>` 同枝）。
- 默认 CSS：`display: inline-block`（与 `<img>` 一致）。
- 宽高：CSS width/height > intrinsic-width/height attribute > 宿主上报 intrinsic > 默认 (300×150)（参考 `<img>` 兜底）。
- 不渲染任何像素；emit `DONG_DRAW_HOST_VIEW(slot_id, rect)` 命令。

### 3.2 SlotId 分配

- 在 dom 节点上分配一个**单调递增 uint32_t** `host_slot_id_`（首次 paint 时分配，跨帧稳定，节点销毁时回收）。
- 宿主通过此 id 与 dong 双向通信。
- 字符串 `slot` attribute 可读，但**id 是宿主对接的主 key**（避免字符串比对开销）。

### 3.3 C ABI 扩展

```c
// dong/include/dong_host_view.h（新增）

typedef struct {
    DongSlotId  id;
    const char* name;        // 元素 slot attribute；可能为空
    const char* element_id;  // 元素 id attribute；可能为空
    DongRect    rect;        // 当前在 view 像素空间的位置（已应用 transform / scroll）
    float       opacity;     // 累积 opacity
    int         visible;     // 0/1（visibility / display:none / inert）
} DongHostViewInfo;

// 枚举当前所有 host-view（每帧 tick 后；宿主只读）
size_t dong_engine_get_host_views(dong_engine_t* eng,
                                  DongHostViewInfo* out_array,
                                  size_t capacity);

// 查找单个（按 slot name 或 element id）
int dong_engine_find_host_view(dong_engine_t* eng, const char* name_or_id,
                               DongHostViewInfo* out);
```

### 3.4 宿主回调（intrinsic size + hit-test 反馈）

```c
typedef struct {
    // 上报 slot 的 intrinsic size（宿主决定）；返回 0 = 无 intrinsic（dong 用 css/attr/兜底）
    int (*get_intrinsic_size)(DongSlotId id, void* user, int* out_w, int* out_h);

    // 命中测试反馈：hit-test 命中本 slot 后，宿主决定该点对其内部子树是否有效
    //   返回 1 = 由宿主"消费"（dong 不再 dispatch 给本元素 / 不冒泡）
    //   返回 0 = dong 当作普通 element 处理
    int (*hit_test_consume)(DongSlotId id, void* user, int local_x, int local_y);

    // 通知 slot 出现 / 消失（节点 attach / detach）
    void (*on_slot_added)(DongSlotId id, const char* name, void* user);
    void (*on_slot_removed)(DongSlotId id, void* user);
} DongHostViewCallbacks;

void dong_engine_set_host_view_callbacks(dong_engine_t* eng,
                                          const DongHostViewCallbacks* cb,
                                          void* user_data);
```

### 3.5 Layout 集成

`<host-view>` 走 replaced 路径：

1. `aspect_ratio_resolver`（`src/layout/aspect_ratio_resolver.cpp`）已对 replaced element 处理；按 `aspect-ratio` CSS / intrinsic ratio 求维度。
2. Intrinsic size 来源：
   - CSS width + CSS height：直接用。
   - 缺一边：`aspect-ratio` 算出。
   - 都无：调宿主 callback `get_intrinsic_size`；无回调或返回 0 → fallback 300×150。
3. Layout 完成后，`getRect()` 给出布局矩形；累积 transform / clip / opacity 后形成最终屏幕坐标。

### 3.6 Paint 集成

`Painter::paintElement` 遇 `<host-view>`：

```cpp
if (element->tagIs("host-view")) {
    DisplayItem item;
    item.kind = DisplayItem::HostView;
    item.host_slot_id = element->host_slot_id();
    item.dest_rect    = element->getScreenRect();   // 含 transform / scroll
    display_list.push(item);
    return;  // 不绘制任何子像素
}
```

`DrawListEmitter` 把 `DisplayItem::HostView` → `DONG_DRAW_HOST_VIEW`。

`SDLBackend` 自己的 GPU 路径下：emit 一条空白 layer + 触发"slot 帧通知"，不绘制（宿主在 dong_app demo 中无 3D 内容时完全空）。

### 3.7 Hit-test 集成

`hitTestElementAt(x, y)`：

1. 走通常的命中测试。
2. 命中元素是 `<host-view>` 时：
   - 转换坐标到 slot 局部空间 (`local_x = x - rect.x; local_y = y - rect.y`)
   - 调 `hit_test_consume(slot_id, local_x, local_y)`。
   - 返回 1 → 把命中视为"宿主已消费"，本元素仍是 `event.target`，但 dispatch 时 `event.preventDefault()` 默认设置（约定见 § 3.8）。
   - 返回 0 → 当普通元素处理，事件正常 dispatch / bubble。
3. 子节点：`<host-view>` 在 dong DOM 里**通常无 dom 子节点**；若用户在标签内放了内容，按 fallback 内容渲染（与 `<canvas>` 内 fallback 一致）。

### 3.8 事件穿透模型

| 事件 | 行为 |
|---|---|
| `mousemove / mousedown / mouseup / wheel` | 命中 host-view 时调 hit_test_consume 决定是否消费；消费后 dong 不冒泡 |
| `pointerover / pointerleave` | 同上 |
| `keydown / keyup` | 焦点在 host-view 内时，宿主负责处理；dong 不接收（除非宿主主动 forward） |
| `focus / blur` | host-view 可被 `tabindex` 标记为可聚焦；spatial nav 命中时 dispatch focus 给本元素，宿主自行管理内部焦点 |
| 其他 DOM 事件（custom）| 走标准 dispatch；与 host-view 无特殊关系 |

宿主可主动 forward 事件给 dong：

```c
// 反向：宿主把内部事件 emit 到 host-view 元素上
void dong_engine_dispatch_host_event(dong_engine_t* eng, DongSlotId id,
                                     const char* event_type, /* event payload */);
```

v1 仅支持自定义 event；keyboard / mouse 反向 forward 留 v2。

### 3.9 与三轨的协作

| 模式 | 是否支持 |
|---|---|
| **DOM 模式** | ✅ 完整支持 |
| **Scene Graph 模式** | ✅；`SceneNode` 加 `host_slot_id`；位置由 scene 节点几何决定 |
| **Direct Draw** | N/A（无 DOM 树） |

---

## 4. 实施步骤

| Step | 范围 |
|---|---|
| **S1 — HTML 解析 + DOM 节点 + 默认 CSS** | `<host-view>` 可被识别；`display: inline-block`；CSS w/h 生效 |
| **S2 — Layout（intrinsic size + aspect-ratio + 宿主 callback）** | demo：CSS 给 400×300，HTML 内布局正确占位 |
| **S3 — DisplayItem::HostView + Painter emit** | 不画像素；DOM 兄弟元素仍正常绘 |
| **S4 — DrawListEmitter 输出 `DONG_DRAW_HOST_VIEW`** | 与 P1-1 配合；sample C 程序能看到该命令 |
| **S5 — Hit-test + 事件穿透** | 鼠标在 slot 区域：调 callback；其余区域正常 |
| **S6 — Scene Graph 模式集成** | scene 编译时识别 `<host-view>` |
| **S7 — Demo + 文档** | `dong_app` demo：用 SDL 在 host-view 区域画一个 SDL primitive 验证 clip/opacity 传递 |

---

## 5. 通过验证规则

### 5.1 Hard

| 项 | 阈值 |
|---|---|
| `<host-view>` 在 DOM 树中正常存在，可被 `getElementById` 取到 | 必须 |
| CSS width/height 可控制 slot 大小 | 像素 baseline 验证 |
| `aspect-ratio` 与 intrinsic size 优先级与 `<img>` 一致 | spec 一致 |
| `DONG_DRAW_HOST_VIEW` 命令 rect 正确反映累积 transform / scroll / clip | sample C 程序 + 多用例 |
| Hit-test 命中 host-view 时调 `hit_test_consume` callback；消费后 dong 不 dispatch | 单元 + 手动 |
| 包裹 host-view 的 `overflow: hidden` 能正确裁剪：DrawList 中先 PUSH_CLIP_RECT 再 HOST_VIEW | 验证 |
| `opacity / visibility / display:none / inert` 正确反映在 `DongHostViewInfo` | 单元 |
| 多个 host-view 并存（同 slot name 不同 id）行为正确 | 单元 |
| 切到 SDL backend 自己的渲染路径下，host-view 区域仅留空（无残影） | 像素 |

### 5.2 Soft

| 项 | 阈值 |
|---|---|
| 单帧最多 host-view 数量 | ≥ 16（典型 editor 多 viewport 场景） |
| `dong_engine_get_host_views` 调用开销 | < 0.05 ms（N=16） |
| Slot intrinsic callback 触发频率 | 仅在 layout 重算时；不每帧 |
| 命中测试 callback 触发频率 | 仅在 mouse 事件时 |

### 5.3 必须新增的测试

| 文件 | 验证 |
|---|---|
| `examples/data/tests/test_host_view_basic.html` | 单 slot；位置 / 大小正确 |
| `test_host_view_aspect_ratio.html` | aspect-ratio + 一边 auto |
| `test_host_view_clip.html` | 父级 `overflow: hidden` 裁剪 slot |
| `test_host_view_transform.html` | 父级 transform → DrawList 内 PUSH_TRANSFORM 累积 |
| `test_host_view_opacity.html` | 父级 `opacity: 0.5` → opacity 反映在 HostViewInfo |
| `test_host_view_scroll.html` | 滚动容器内 slot 跟随滚动 |
| `test_host_view_hidden.html` | display:none / visibility:hidden / inert 行为 |
| `test_host_view_multi.html` | 同帧多 slot |
| `test_host_view_scene_graph.html` | Scene Graph 模式 |
| `examples/host_view_demo.c` | dong_app + 注册 callback；slot 区域用 SDL 直接画一个 colored quad |

### 5.4 验证命令

```bash
# 像素回归 (host-view 区域应全透明，由宿主补)
python dong/scripts/tools/run_baseline_compare.py --filter test_host_view_

# DrawList 命令检查
./examples/abi_consumer data/tests/test_host_view_clip.html
# 期望: PUSH_CLIP_RECT, HOST_VIEW(slot_id=1, rect={...}), POP_CLIP

# Demo
./host_view_demo --html data/host_view_demo.html
# SDL 在 slot 区域画橙色矩形；HTML 滚动时矩形跟随；overflow 时被裁
```

---

## 6. 风险与回退

| 风险 | 缓解 |
|---|---|
| 宿主忘记实现 callback → 视觉空洞 | 提供 default callback：在 slot 区域画 magenta 棋盘格 + slot id 文字（debug 模式） |
| 宿主 hit-test consume 行为不一致导致事件诡异 | 文档强约束：消费后 dong 不再冒泡；提供 `event.dong_consumed_by_slot` 字段方便业务排查 |
| 多 view 多 slot 命名冲突 | slot id 在 view 内唯一；宿主跨 view 自行加前缀 |
| Slot 内的子 widget 被 HTML 父级 transform 旋转后，宿主自己的 hit-test 不知道这个 transform | 在 HostViewInfo 中加 `transform[6]`（v1.1）；v1 先把 rect 做 axis-aligned bounding box |
| Layout invalidation 漏触发：宿主 intrinsic size 变了 dong 不知道 | 提供 `dong_engine_invalidate_host_view(eng, slot_id)` 让宿主显式触发重布局 |
| Scene Graph 模式下原本 O(1) 更新被 host-view 拖慢 | 仅 host-view 节点参与 scene；slot 几何变化触发该节点重 emit，O(1) 不变 |

---

## 7. 不在本方案范围

- ❌ HTML 内反向嵌入文本流（host-view 内文字与 HTML 文字混合 inline）—— 太复杂，留 v3
- ❌ 宿主 widget 接收 dong 焦点的完整生命周期（Phase 1 仅基础）
- ❌ `<host-view>` 内含 dong DOM 子节点（fallback content）—— 暂不支持
- ❌ 跨 view 嵌入（一个 dong view 嵌入另一个 dong view）—— 用 multiview 已有路径
- ❌ Drag & drop：drag 跨越 host-view 边界—— 留 Phase 2

---

## 8. 完成后更新

- [ ] `docs/developer/design/引擎适配.md` § 3.2 / § 5.3 状态从"草案"改"已实施"
- [ ] `docs/guide/integrations/unreal-engine.md` § 5 + `integration_unity.md` § 6 状态可对接
- [ ] `docs/reference/features-index.md` 新增 § "宿主嵌入 (`<host-view>`)"
- [ ] `docs/overview/positioning.md` § 5.2 表格 EmbedSlot 行从"待做"改"已交付"
- [ ] `dong/include/dong_host_view.h` 文档化
