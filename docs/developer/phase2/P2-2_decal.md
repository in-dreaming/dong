# P2-2 — `dong_decal_t`：UI 贴花投影到世界几何

> 上游：[`docs/roadmap.md`](../roadmap.md) Phase 2 P2-2
> 性能门槛：[`docs/developer/perf-budget.md`](../perf_budget.md) § 3.5 S5
>
> 状态：方案 v1 / 2026-04-17

---

## 1. 目标

dong 提供"贴花" API，把 UI 内容（HTML 渲染的 RGBA 纹理或简单图标）按表面法线投影到宿主 3D 几何上，典型用例：

- 地面提示（"按 E 拾取"标记、技能落点圆圈、警戒区域）
- 弹孔 / 涂鸦
- AOE 范围指示
- 地面菜单（RTS 单位选中圈）

不实现复杂材质 / 物理；纯**几何投影 + alpha blend**。

---

## 2. 现状

| 项 | 现状 |
|---|---|
| HTML → texture 渲染 | ✅，`dong_engine_render_to_gpu_texture` |
| World space rendering | 仅 P2-1 WorldText（billboard） |
| 贴花路径 | ❌ |
| Depth buffer 协同 | ❌（同 P2-1） |
| 投影几何重建 | ❌ |

---

## 3. 设计

### 3.1 实现策略：Screen-space deferred decal

业界常见两条路：

| 方案 | 思路 | 取舍 |
|---|---|---|
| **Mesh decal** | 用真实贴花 mesh 顺着表面摆 | 需要碰撞检测 / mesh 切割；复杂 |
| **Screen-space deferred** | 渲染时根据 depth buffer 反推世界坐标，落在 decal volume 内的像素被着色 | 只需 depth；不切 mesh；适配性广 |

**v1 走 screen-space deferred**：

```
宿主 3D pass 完成 → 拿到 depth buffer
        │
        ▼
dong_decal_render(ctx) ：
    bind dong's box volume mesh (unit cube)
    instanced draw 每个 decal 一个 box
    fragment shader：
        - 用 depth 反推该像素的 world pos
        - 检查是否在 decal local box 内
        - 在 → 采样 decal 纹理（UV = local pos）
        - 不在 → discard
        - alpha blend 到 color target
```

### 3.2 C ABI

新增 `dong/include/dong_decal.h`：

```c
typedef struct DongDecal DongDecal;
typedef uint32_t DongDecalId;

typedef enum {
    DONG_DECAL_PROJECTION_TOP_DOWN = 0,    // 沿世界 -Y 方向投影（典型地面装饰）
    DONG_DECAL_PROJECTION_BOX = 1,         // 任意 OBB，沿 -Z 方向投影
} DongDecalProjection;

typedef struct {
    // 位置 / 朝向
    float pos_x, pos_y, pos_z;             // 中心点
    float rotation_deg;                    // 沿投影轴旋转
    float size_x, size_y, size_z;          // OBB size（z = 投影深度）
    DongDecalProjection projection;

    // 纹理来源
    DongTextureId texture;                 // 0 = 用 HTML view 路径
    const char* html_view_id;              // 若 texture==0：用此 view 渲染出 RGBA
    float uv_x, uv_y, uv_w, uv_h;          // 纹理子矩形

    // 视觉
    float color_r, color_g, color_b, color_a;  // tint
    float fade_distance;                   // 距视点 > fade_distance 后渐隐
    float angle_fade_deg;                  // 表面法线与投影轴夹角 > X 度后渐隐
    int   layer;                           // 排序
    int   visible;
    int   receive_lighting;                // 0 = unlit；1 = 接受场景光（v2，先 0）
} DongDecalDesc;

DongDecal* dong_decal_create(dong_engine_t* eng);
void       dong_decal_destroy(DongDecal* d);

DongDecalId dong_decal_add(DongDecal* d, const DongDecalDesc* desc);
int         dong_decal_update(DongDecal* d, DongDecalId id, const DongDecalDesc* desc);
void        dong_decal_remove(DongDecal* d, DongDecalId id);
void        dong_decal_clear(DongDecal* d);

// 高频补丁
void dong_decal_set_position(DongDecal* d, DongDecalId id, float x, float y, float z);
void dong_decal_set_rotation(DongDecal* d, DongDecalId id, float deg);
void dong_decal_set_color(DongDecal* d, DongDecalId id, float r, float g, float b, float a);

// 渲染
typedef struct {
    float view[16], proj[16];
    float viewport_w, viewport_h;
    float near_plane, far_plane;
    void* color_target;
    void* depth_target;          // 必须；用于反投影
    int   depth_format;          // DONG_GPU_TEXTURE_FORMAT_DEPTH24_STENCIL8 ...
    int   depth_is_reversed_z;   // 0 = 标准；1 = reverse-Z（部分引擎用）
} DongWorldRenderContext;        // 与 P2-1 共用同名 struct

int dong_decal_render(DongDecal* d, const DongWorldRenderContext* ctx);
```

### 3.3 Shader 关键

```hlsl
// decal_fs.hlsl
float4 decal_fs(VS_Out i) : SV_Target {
    // 1. 用 screen UV + depth 反推 world pos
    float2 uv = i.pos.xy / viewport;
    float d = SAMPLE(depth_target, uv).x;
    if (depth_is_reversed_z) d = 1.0 - d;
    float4 ndc = float4(uv * 2 - 1, d, 1);
    ndc.y *= -1;  // 平台差异
    float4 world = mul(g_inv_view_proj, ndc);
    world /= world.w;

    // 2. 转 decal 局部空间
    float3 local = mul(g_inv_decal_model, float4(world.xyz, 1)).xyz;

    // 3. 在 decal box 内？
    if (any(abs(local) > 0.5)) discard;

    // 4. 法线角度 fade（用 ddx/ddy 估算 surface normal）
    float3 surface_normal = normalize(cross(ddx(world.xyz), ddy(world.xyz)));
    float3 decal_normal = g_decal_axis_world;   // 例 (0, -1, 0)
    float angle_cos = dot(surface_normal, decal_normal);
    float angle_alpha = saturate((angle_cos - cos(radians(angle_fade_deg))) /
                                 (1 - cos(radians(angle_fade_deg))));
    if (angle_alpha < 0.001) discard;

    // 5. 距视点 fade
    float dist = length(world.xyz - g_camera_pos);
    float dist_alpha = saturate(1 - dist / fade_distance);

    // 6. UV → decal texture
    float2 decal_uv = local.xy + 0.5;
    float4 tex = SAMPLE(decal_texture, decal_uv * uv_size + uv_origin);

    return float4(tex.rgb * tint.rgb,
                  tex.a * tint.a * angle_alpha * dist_alpha);
}
```

### 3.4 HTML view 作为 decal 内容

```c
// 用法 A：固定纹理
DongDecalDesc d = { .texture = my_atlas_id, .uv_x = 0.5, ... };

// 用法 B：HTML view 实时渲染（典型：动态文字、多语言、复杂图标）
dong_engine_create_offscreen_view(eng, "decal_pickup", 256, 256);
dong_engine_load_html_for_view(eng, "decal_pickup", "<div>按 E 拾取</div>");
DongDecalDesc d = { .texture = 0, .html_view_id = "decal_pickup", ... };
```

dong 自动每帧把 view 渲染成 RGBA texture（已有 `render_to_gpu_texture`），喂给 decal shader。

### 3.5 性能优化

1. **Instanced**：一个 decal volume mesh + 每个 decal 一个 instance（matrix + texture handle）。
2. **Frustum culling**：CPU 侧丢 box-frustum 不相交的 decal。
3. **Occlusion**：远距离贴花（dist > fade_distance + margin）跳过。
4. **Texture atlas**：所有静态 decal 进同一 atlas，instance 内只存 UV rect。
5. **Sort**：按 (depth_test, layer, texture_id) 排，最大化 batch。

### 3.6 与 P2-1 World Text 共存

| 项 | World Text | Decal |
|---|---|---|
| Rendering 阶段 | 宿主 3D pass 末尾（叠在 3D 上）| 宿主 3D pass 末尾，在 World Text 之前（贴在 3D 表面） |
| 深度处理 | 写不写深度由 depth_test 决定 | 永不写深度（仅采样） |
| 排序 | 按 z + layer | 按 layer + texture |

宿主推荐顺序：
```
clear color/depth
render world geometry (写深度)
render decals          ← 这里
render world text      ← 然后这里
present
```

### 3.7 LOD 与边界处理

| 边界 | 行为 |
|---|---|
| Decal 投影到无穷远（无几何） | depth 反推得超大 world，box 外，自然 discard |
| Decal 跨多个表面（地面 + 墙壁） | shader 内 angle_fade 自动衰减侧面 |
| 远距离 decal 闪烁（depth 精度） | shader 内对 depth 加 epsilon；超 fade_distance 跳过 |
| Decal 内贴上薄物体（如门）| 默认透过（screen-space 算法）；可加 thickness 限制（v2） |

---

## 4. 实施步骤

| Step | 范围 |
|---|---|
| **S1 — C ABI + Pool** | add/update/remove；不渲染 |
| **S2 — Shader + Pipeline + 单 decal 渲染** | 反投影正确性 |
| **S3 — Instanced 多 decal + sort** | 100 decal perf |
| **S4 — angle/dist fade + 边界处理** | 远近过渡平滑 |
| **S5 — `html_view_id` 集成** | HTML view → decal 纹理 |
| **S6 — Frustum cull + LOD** | perf budget 达成 |

---

## 5. 通过验证规则

### 5.1 Hard

| 项 | 阈值 |
|---|---|
| 100 个 decal 同帧 | ≥ 60 FPS（[`perf_budget.md`](../perf_budget.md) § 3.5） |
| 反投影像素与"基准 mesh decal 实现"一致（demo 对照） | 像素 diff < 2% |
| Angle fade 在 90° 平面相邻处自然过渡（无锯齿） | 像素 |
| 距离 fade 端到端无闪烁 | 必须 |
| Decal 不写深度（不影响后续 World Text 排序） | 验证 |
| `html_view_id` 实时更新内容自动反映在 decal | demo |
| Reverse-Z 模式下行为正确 | 必须 |

### 5.2 Soft

| 项 | 阈值 |
|---|---|
| 100 decal CPU cull + sort 时间 | < 0.3 ms |
| 同 atlas 同 layer 的 decal 合 batch | ≤ 3 draw call（100 decal） |
| HTML view → decal texture 更新延迟 | ≤ 1 frame |
| Decal 内 alpha 边缘抗锯齿 | 视觉 OK |

### 5.3 必须新增的测试

| 文件 | 验证 |
|---|---|
| `examples/decal_pickup_marker.c` | 5 个地面拾取标记，绕地形铺放 |
| `examples/decal_aoe_circle.c` | 技能 AOE 圆圈（含动画 scale/rotation） |
| `examples/decal_html_text.c` | HTML view 渲染的多语言文字贴在地面 |
| `examples/decal_100_perf.c` | 100 decal perf 验证 |
| `examples/decal_reverse_z.c` | reverse-Z 模式 |
| `tests/world/test_decal_inverse_proj.cpp` | 反投影数学单元 |
| `tests/world/test_decal_culling.cpp` | frustum cull |
| `dong/scripts/test_decal_perf.py` | perf 验收 |

### 5.4 验证命令

```bash
./decal_aoe_circle
# 期望：地面有圆圈贴花，玩家走过时贴花跟随，墙壁侧面 fade

python dong/scripts/test_decal_perf.py
# 期望：100 decal ≥ 60 FPS, draw ≤ 3
```

---

## 6. 风险与回退

| 风险 | 缓解 |
|---|---|
| 不同宿主 depth buffer 编码差异（linear vs perspective）| `depth_format` 字段 + shader 自动适配 |
| reverse-Z 漏处理 | flag 显式；测试用例覆盖 |
| Mesh decal 想要 vs screen-space 想要 | 文档明示走 screen-space；mesh decal 业务侧自己用 mesh + dong texture |
| 透明物体（玻璃、植被）后面的 decal 可见性错乱 | 文档警告：透明物体不写深度 → decal 落在不透明背景上；游戏 UI 通常 acceptable |
| decal 太多（数千）渲染过载 | 业务侧分组 + LOD；超过阈值 console.warn |
| HTML view 渲染开销叠加在主线程 | offscreen view 与主 view 共享 dong tick；按需 update（IDLE 时不重渲） |

---

## 7. 不在本方案范围

- ❌ Mesh decal（业务侧自己处理）
- ❌ Receive lighting（unlit only；v2 评估）
- ❌ Normal map / PBR decal（仅 RGBA）
- ❌ Animated decal sequence（用 css animation 在 HTML view 内做）
- ❌ Volumetric decal（雾、3D 文字残影）
- ❌ Decal cluster culling（GPU 侧 culling；性能不够时再做）

---

## 8. 完成后更新

- [ ] `docs/reference/features-index.md` 新增 § "World Decal"
- [ ] `docs/overview/positioning.md` § 5.2 P2-2 行从"待做"改"已交付"
- [ ] `docs/developer/perf-budget.md` § 3.5 S5 实测填入
- [ ] `dong/include/dong_decal.h` 文档化
- [ ] `docs/guide/integrations/unreal-engine.md` / `integration_unity.md` 加 "Decal" 集成段
