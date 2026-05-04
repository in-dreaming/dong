# P2-3 — `dong_world_overlay_t`：世界空间任意 Quad UI

> 上游：[`doc/roadmap.md`](../roadmap.md) Phase 2 P2-3
> 与 P2-1 / P2-2 互补：[`P2-1`](./P2-1_world_text.md) 是 billboard 文字，[`P2-2`](./P2-2_decal.md) 是表面贴花，本任务是**任意朝向的 world quad**。
> 性能门槛：[`doc/perf_budget.md`](../perf_budget.md) § 3.5 S5
>
> 状态：方案 v1 / 2026-04-17

---

## 1. 目标

让宿主能把 dong 渲染的 HTML view 当作"贴在世界中的一块屏幕"，可任意朝向、可被深度遮挡，典型场景：

- 游戏内可交互终端 / 显示器（坐在椅子前的电脑屏）
- 广告牌（不一定 billboard，可固定朝向）
- 可走近读的便条 / 标牌
- 3D 商店货架上的标价牌
- VR/AR 浮窗（v2）

与 `dong_scene3d_*`（demo 用）的区别：本 API 假定**宿主 3D 引擎主导**，dong 只提供 quad → texture + 输入路由 + 深度协同；不带相机 / 输入循环。

---

## 2. 现状

| 项 | 现状 |
|---|---|
| HTML view → GPU texture | ✅，`dong_engine_render_to_gpu_texture` |
| 多 view + GlobalShared | ✅ |
| `dong_scene3d_*` 自带 quad / 相机 / hit-test | demo 用，不做生产 |
| 宿主驱动的 world quad API | ❌ |
| Hit-test 反馈给宿主 | ❌（`dong_scene3d` 内部封了，没有公共 ABI） |

---

## 3. 设计

### 3.1 与 P2-1 / P2-2 的边界

| API | 几何 | 朝向 | 深度 | 输入 |
|---|---|---|---|---|
| **P2-1 WorldText** | billboard quad（永远面向相机） | 屏幕空间正向 | 可选写深度 | 不响应 |
| **P2-2 Decal** | unit cube volume + 表面投影 | 沿投影轴 | 不写深度，仅采样 | 不响应 |
| **P2-3 WorldOverlay**（本任务） | 任意四点 quad（含 unit quad transform） | 任意 | 必须写深度 + 测试深度 | **响应**（输入路由） |

### 3.2 C ABI

```c
// dong/include/dong_world_overlay.h（新增）
typedef struct DongWorldOverlay DongWorldOverlay;
typedef uint32_t DongWorldOverlayId;

typedef struct {
    // 几何：4 个世界坐标顶点（顺序：BL, BR, TR, TL）
    float corners[4][3];

    // 内容来源
    DongTextureId texture;            // 0 = 用 view_id
    const char* view_id;              // dong offscreen view（含 HTML / Direct Draw）
    int   view_pixel_width;           // view 的渲染分辨率（建议 与 quad 屏幕投影面积匹配）
    int   view_pixel_height;
    float uv_x, uv_y, uv_w, uv_h;

    // 视觉
    float color_r, color_g, color_b, color_a;
    float fade_distance;
    int   layer;
    int   visible;
    int   double_sided;               // 1 = 背面也渲染
    int   billboard_axis;             // 0=none 1=Y axis 2=full（v1 仅 0；其他留 v2）

    // 输入
    int   interactive;                // 1 = 接收宿主转发的输入
    int   focusable;                  // 1 = 可被 spatial nav 命中
} DongWorldOverlayDesc;

DongWorldOverlay* dong_world_overlay_create(dong_engine_t* eng);
void              dong_world_overlay_destroy(DongWorldOverlay* wo);

DongWorldOverlayId dong_world_overlay_add(DongWorldOverlay* wo, const DongWorldOverlayDesc* desc);
int                dong_world_overlay_update(DongWorldOverlay* wo, DongWorldOverlayId id,
                                              const DongWorldOverlayDesc* desc);
void               dong_world_overlay_remove(DongWorldOverlay* wo, DongWorldOverlayId id);

// 渲染
int dong_world_overlay_render(DongWorldOverlay* wo, const DongWorldRenderContext* ctx);

// 输入路由（宿主驱动）：宿主 ray-cast 命中本 overlay 后转发
//   返回 1 = 命中并消费；0 = 未命中
typedef struct {
    int    type;        // 0=move 1=down 2=up 3=wheel 4=key
    float  ray_origin[3];
    float  ray_dir[3];
    int    button;      // for down/up
    float  wheel_dx, wheel_dy;
    uint32_t key_code;  // for key
    int    pressed;     // 0/1
} DongWorldInput;

int dong_world_overlay_send_input(DongWorldOverlay* wo, DongWorldOverlayId id,
                                  const DongWorldInput* input);

// Ray-cast helper：dong 内置 quad-ray 求交
//   命中返回非空 id；out_local_uv 填命中的 quad 局部 UV（0..1）
int dong_world_overlay_pick(DongWorldOverlay* wo,
                            const float ray_origin[3], const float ray_dir[3],
                            DongWorldOverlayId* out_id,
                            float* out_local_uv_x, float* out_local_uv_y);
```

### 3.3 渲染管线

```
dong_world_overlay_render(ctx):
  1. 收集所有 visible WorldOverlay
  2. 对每个 overlay 检查内部 view 是否需 tick + render_to_gpu_texture（offscreen pass）
  3. CPU 侧：corners 投影到 clip space → frustum cull
  4. CPU 侧：sort 按 (front-to-back if opaque, back-to-front if transparent)
  5. 上传 instance buffer：matrix + texture handle + tint
  6. 分组 batch：按 texture / blend mode
  7. instanced draw → color_target，开 depth test + 写深度（不透明）/ 仅测试（透明）
```

**关键**：内部 view 走与主 view 一样的 dong tick 循环，但渲染目标是 offscreen RT 而非 swapchain。可与主 view 共享 GlobalShared。

### 3.4 输入路由模型

宿主负责 3D 世界 ray-cast；命中 overlay 后调 `dong_world_overlay_send_input`：

```c
// 宿主每帧
DongWorldOverlayId hit_id;
float uv_x, uv_y;
if (dong_world_overlay_pick(wo, mouse_ray.origin, mouse_ray.dir, &hit_id, &uv_x, &uv_y)) {
    DongWorldInput in = {
        .type = mouse_event_type, .button = btn, ...
    };
    // dong 内部把 ray 转 quad 局部 UV → view 像素坐标
    // → 调对应 view 的 dong_engine_send_mouse_*
    dong_world_overlay_send_input(wo, hit_id, &in);
}
```

**dong 内部转换**：

```c
view_pixel_x = uv_x * view_pixel_width;
view_pixel_y = uv_y * view_pixel_height;
// 然后走 view 内常规事件路径
dong_engine_send_mouse_move(view, view_pixel_x, view_pixel_y);
```

### 3.5 与 P0-4 Spatial Nav 协作

`focusable=1` 的 overlay 进入 dong 主 view 的 spatial nav 候选集（视为 view 的"嵌入元素"），但只在主 view 焦点切到该 overlay 后，dong 把后续 dpad 事件 forward 到 overlay 内部。

复杂场景（多 view 之间 nav）建议宿主管理；dong v1 不强求支持跨 view nav。

### 3.6 与 `<host-view>`（P1-2）的关系

| API | 用途 | 关系 |
|---|---|---|
| **`<host-view>`**（P1-2） | HTML 内**嵌入宿主 widget** | HTML 主，宿主补 |
| **`dong_world_overlay_*`**（本任务） | 宿主 3D 世界**嵌入 dong view** | 宿主主，dong 补 |

完全互补。两者都用 dong DOM/CSS/Layout。

### 3.7 与 P2-1 / P2-2 的渲染顺序

宿主 3D pass 内推荐顺序：

```
clear color/depth
render world geometry      ← 写深度
render WorldOverlay (P2-3) ← 写深度（让其他后续物体能遮挡 overlay）
render Decal (P2-2)        ← 仅采样深度
render WorldText (P2-1)    ← 视 depth_test 决定
present
```

### 3.8 性能与内存

| 项 | 限制 |
|---|---|
| 同帧 overlay 数 | 推荐 ≤ 16；上限 64 |
| 每 overlay 内部 view 分辨率 | 默认 512×512；超过 1024×1024 console.warn |
| Offscreen RT 池 | dong 维护 RT pool；同尺寸可复用 |
| 内部 view tick 频率 | 默认每帧；可按 dirty / FPS cap 节流（业务侧通过 view API 控制） |

---

## 4. 实施步骤

| Step | 范围 |
|---|---|
| **S1 — C ABI + Pool + 输入 ray-cast** | quad-ray 求交单元；不渲染 |
| **S2 — 单 overlay 渲染（含 offscreen view 联动）** | demo：3D 场景中浮一个网页 |
| **S3 — Instanced 多 overlay + sort** | 16 overlay perf |
| **S4 — 输入路由：ray → uv → view pixel → DOM 事件** | 鼠标点击 overlay 内按钮工作 |
| **S5 — 双面 + alpha 排序 + frustum cull** | 边界正确 |
| **S6 — 与 spatial nav 集成** | dpad 切到 overlay 后内部 nav |

---

## 5. 通过验证规则

### 5.1 Hard

| 项 | 阈值 |
|---|---|
| 5 个 1024×768 overlay 同帧 | ≥ 60 FPS（[`perf_budget.md`](../perf_budget.md) § 3.5） |
| Offscreen view 内的 HTML 像素与"主 view 同 HTML"比对一致 | < 1% diff |
| Ray-cast 命中后输入正确路由到 view 的 DOM 事件 | demo + 单元 |
| Overlay 之间深度遮挡正确（前后两个 overlay） | 像素 |
| Overlay 与 3D 物体深度遮挡正确（cube 在前 / 后）| 像素 |
| `double_sided=0` 时背面剔除正常 | 像素 |
| 输入到 disabled / hidden overlay 时不响应 | 必须 |
| 销毁 overlay 不泄漏 offscreen RT | ASAN |

### 5.2 Soft

| 项 | 阈值 |
|---|---|
| 16 overlay 同帧 | ≥ 60 FPS |
| Ray-cast pick 时间（16 overlay） | < 0.1 ms |
| Offscreen RT pool 命中率 | ≥ 80% |
| 内部 view 仅 dirty 时重渲染 | counter 验证 |
| Spatial nav 切到 overlay 时延迟 | < 100 ms |

### 5.3 必须新增的测试

| 文件 | 验证 |
|---|---|
| `examples/world_overlay_terminal.c` | 3D 场景中一个交互终端，鼠标点 overlay 上按钮 → JS log |
| `examples/world_overlay_billboard.c` | 5 个 overlay 朝向相机（业务侧自己每帧 update corners） |
| `examples/world_overlay_depth_occlusion.c` | cube 在 overlay 前 / 后 |
| `examples/world_overlay_perf_16.c` | 16 overlay perf |
| `tests/world/test_overlay_ray_pick.cpp` | quad-ray 求交单元 |
| `tests/world/test_overlay_input_routing.cpp` | uv → view pixel 转换 |
| `dong/scripts/test_world_overlay_perf.py` | perf 验收 |

### 5.4 验证命令

```bash
./world_overlay_terminal
# 期望：场景里有一台终端浮窗；鼠标移到上面 → cursor 变；点击按钮 → 控制台 log

python dong/scripts/test_world_overlay_perf.py
# 期望：5 overlay 1024x768 ≥ 60 FPS
```

---

## 6. 风险与回退

| 风险 | 缓解 |
|---|---|
| Offscreen view 数量爆炸 → 内存占用高 | RT pool 复用 + 限制并发 view 数；超出阈值 console.warn |
| 内部 view 即使无变化也每帧重渲 | dong 内部 dirty 标志（已有 `commands_regenerated_this_tick_`）；overlay 自动跳过 |
| Ray-cast 在密集 overlay 场景 O(N) 慢 | v1 接受 O(N)（典型 N < 16）；v2 加 BVH |
| 与 P2-2 Decal 同时写深度 → 顺序错 | 文档明示渲染顺序；overlay 在 decal 之前 |
| 输入坐标在双面渲染时 UV 翻转 | 根据 ray 与法线 dot 判断；shader / pick 一致处理 |
| 内部 view 含 React，每帧 reconciler diff 卡 | 业务侧自行优化；同主 view |

---

## 7. 不在本方案范围

- ❌ Curved overlay（弯曲 quad；VR 用，留 v2）
- ❌ 自动 billboard（业务侧自己 update corners）
- ❌ Stereo VR overlay
- ❌ 多视点 overlay（split-screen）
- ❌ 自带 ray-cast 体系（宿主 3D 自己 cast）
- ❌ Overlay 内嵌套 overlay（架构上允许，但不推荐）

---

## 8. 完成后更新

- [ ] `doc/重要特性.md` 新增 § "World Space Overlay"
- [ ] `doc/positioning.md` § 5.2 P2-3 行从"待做"改"已交付"
- [ ] `doc/perf_budget.md` § 3.5 S5 实测填入
- [ ] `dong/include/dong_world_overlay.h` 文档化
- [ ] `doc/integration_ue.md` / `integration_unity.md` 加 "World Overlay" 集成段
- [ ] `dong/appcore/include/dong_scene3d.h` 注释里加："生产请用 dong_world_overlay_*"
