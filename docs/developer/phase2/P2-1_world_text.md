# P2-1 — `dong_world_text_t`：世界空间 Billboard 文字

> 上游：[`docs/roadmap.md`](../roadmap.md) Phase 2 P2-1
> 定位依据：[`docs/overview/positioning.md`](../positioning.md) § 5.2 World Space UI primitives
> 性能门槛：[`docs/developer/perf-budget.md`](../perf_budget.md) § 3.5 S5
>
> 状态：方案 v1 / 2026-04-17

---

## 1. 目标

dong 提供"世界空间文字"一等公民 API，**复用 Slug 字体 + GlobalShared atlas + 主线程渲染管线**，覆盖：

- **伤害浮字**（高频；500+ 同帧；带运动 + 透明度衰减）
- **名字版**（中频；30–100 同帧；含图标 / 头像）
- **3D HUD 文字**（低频；几个一直存在；可包含 emoji / 中英混排）

不假设 dong 拥有 3D 摄像机：宿主提供 view / proj 矩阵；dong 出 instanced glyph quads。

---

## 2. 现状

| 项 | 现状 |
|---|---|
| 2D 文字（Slug / MSDF） | ✅，全 view 已支持 |
| `dong.renderText()` Direct Draw | ✅，单 view 屏幕空间（[`docs/developer/arch/text-rendering-spec.md`](../arch/arch_font.md) § 9） |
| GlobalShared GlyphAtlas | ✅，多 view 共享 |
| 世界空间 / 3D 接口 | ❌（`dong_scene3d_*` 自带 demo 相机但定位为 demo / 工具） |
| Billboard 顶点 / instanced 路径 | ❌ |
| Depth test 协同 | ❌（dong 渲染 swapchain 不带深度） |

---

## 3. 设计

### 3.1 总思路：宿主主导 3D，dong 出 GPU 命令

```
宿主 (UE / Unity / 自研)：
  ├── 拥有 3D 场景、深度 buffer、render pass
  ├── 每帧把 view / proj 矩阵传给 dong
  └── 在自己的 render pass 内调一条 dong API → dong 直接 draw 到当前 render target

dong：
  ├── 持有 WorldText 对象池（id 索引）
  ├── 每帧根据 view/proj 计算 billboard 顶点
  ├── 使用宿主提供的 RT + depth buffer
  └── 单条 instanced draw call 输出全部 WorldText
```

**核心约束**：dong 不切 swapchain；它是宿主 3D pass 的"一个 draw call"。

### 3.2 C ABI

新增 `dong/include/dong_world_text.h`：

```c
typedef struct DongWorldText DongWorldText;
typedef uint32_t DongWorldTextId;

typedef struct {
    float pos_x, pos_y, pos_z;     // 世界坐标
    float offset_x, offset_y;      // 屏幕空间偏移（px，billboard 后应用）
    float scale;                   // 字号缩放（1 = 默认 16px）
    float rotation_deg;            // 屏幕空间旋转
    float color_r, color_g, color_b, color_a;
    int   layer;                   // 排序用；同 z 时按 layer 升序
    int   depth_test;              // 1 = 受深度遮挡；0 = always on top
    int   visible;
    DongFontId font;               // 0 = 默认
    const char* text;              // UTF-8
} DongWorldTextDesc;

// 创建 / 销毁世界空间文字管理器（每个 view 一个）
DongWorldText* dong_world_text_create(dong_engine_t* eng);
void           dong_world_text_destroy(DongWorldText* wt);

// 增 / 改 / 删（O(1) 平均）
DongWorldTextId dong_world_text_add(DongWorldText* wt, const DongWorldTextDesc* desc);
int             dong_world_text_update(DongWorldText* wt, DongWorldTextId id,
                                       const DongWorldTextDesc* desc);
void            dong_world_text_remove(DongWorldText* wt, DongWorldTextId id);
void            dong_world_text_clear(DongWorldText* wt);

// 高频补丁更新（不传整 Desc）
void dong_world_text_set_position(DongWorldText* wt, DongWorldTextId id, float x, float y, float z);
void dong_world_text_set_color(DongWorldText* wt, DongWorldTextId id, float r, float g, float b, float a);
void dong_world_text_set_text(DongWorldText* wt, DongWorldTextId id, const char* text);

// 渲染：宿主在自己的 render pass 内调
typedef struct {
    float view[16];           // column-major 4x4
    float proj[16];
    float viewport_w, viewport_h;     // 像素
    float near_plane, far_plane;
    void* color_target;       // 宿主 RT；DongGPU 句柄（与 dong_gpu_driver 一致）
    void* depth_target;       // 可选；NULL = 不写深度
} DongWorldRenderContext;

int dong_world_text_render(DongWorldText* wt, const DongWorldRenderContext* ctx);

// 简易批量增（伤害浮字最常用）
typedef struct {
    DongWorldTextDesc desc;
    float lifetime_s;          // 自动衰减/移除
    float velocity_x, velocity_y, velocity_z;  // 世界空间
    float screen_drift_x, screen_drift_y;       // 屏幕空间漂移
    float fade_start_s;        // 开始 alpha 衰减时间
} DongWorldFloatingTextDesc;

DongWorldTextId dong_world_text_spawn_floating(DongWorldText* wt,
                                               const DongWorldFloatingTextDesc* desc);
void dong_world_text_tick(DongWorldText* wt, float dt);   // 处理 lifetime/velocity/fade
```

### 3.3 渲染管线

```
dong_world_text_render(ctx):
  1. 收集所有 visible WorldText
  2. CPU 侧：每条 → text shape (HarfBuzz / Slug)，命中 GlobalShared cache
  3. CPU 侧：world_pos → MVP → screen_pos；丢弃 frustum 外
  4. 排序：按 (depth_test, screen_z) 排序
       depth_test=1：远→近 alpha blend；shader 内开 depth test (LESS_EQUAL)
       depth_test=0：仅按 layer 排，always on top
  5. Build instance buffer：
       struct InstanceVertex { x, y, scale, rotation, color, glyph_uv, ... }
  6. Bind atlas (Slug curve+band, MSDF atlas)
  7. instanced draw 一次（数千 glyph）
```

**关键点**：
- 文字 shaping 走 main thread（与 P0-3 / P0-5 相同）；shape 结果按 `(text, font, scale)` 哈希缓存。
- Glyph atlas 走 GlobalShared，不为 WorldText 单独建。
- 同字号同字体的 N 条 WorldText 共享 instance buffer，**1 draw call**。

### 3.4 Billboard 顶点构造

```hlsl
// world_text_vs.hlsl
VS_Out world_text_vs(uint vid, uint iid, ...) {
    InstanceData inst = instances[iid];
    GlyphData glyph = glyphs[inst.glyph_index];

    // 1. 世界坐标 → 屏幕中心
    float4 world_pos = float4(inst.world_pos, 1);
    float4 clip_pos = mul(g_proj, mul(g_view, world_pos));
    float2 screen_center = clip_pos.xy / clip_pos.w;       // NDC

    // 2. 屏幕空间偏移 + glyph quad
    float2 offset_px = inst.offset + glyph.local_offset[vid];   // glyph 在文本内的位置
    float2 quad_corner = quad_corners[vid] * glyph.size_px * inst.scale;

    // 3. 旋转
    float2 rotated = rotate2d(quad_corner + offset_px, inst.rotation);

    // 4. NDC → clip → 加回 z
    out.pos = float4(screen_center + rotated / viewport * 2.0,
                     clip_pos.z, clip_pos.w);
    out.uv  = glyph.atlas_uv[vid];
    out.color = inst.color;
    return out;
}
```

**深度处理**：
- `depth_test=1` 时 shader 输出 `gl_FragDepth = clip_pos.z / clip_pos.w`（即文字"挂在 world_pos 上"被遮挡）。
- `depth_test=0` 时 disable depth test（始终在最前）。

### 3.5 Frustum culling 与 LOD

| 场景 | 策略 |
|---|---|
| Frustum 外 | CPU 跳过 + 不进 instance buffer |
| 远距离（屏幕投影 < 4 px）| 缩成单像素或跳过；可选 `min_screen_px` 阈值 |
| 极近距离 | 限制最大 scale，避免吞屏 |
| 同位置堆叠（粒子文字爆炸）| 自动散开（`screen_drift_*` 字段） |

CPU 处理 ≤ 0.5 ms / 1000 entries（O(N) 简单遍历）。

### 3.6 与 P1-1 DrawList ABI 协调

WorldText **不**走 DrawList（它不是 view 内 UI 命令）：

- 直接调 `dong_world_text_render(ctx)`，宿主在自己 pass 中调用。
- 宿主可以选择把 ctx 切成多次（先渲 depth=1 再渲 depth=0），dong 不强制顺序。
- `dong_engine_get_drawlist` 不包含 WorldText 命令。

设计理由：DrawList 假定屏幕空间；WorldText 强依赖 view/proj，不放进通用 drawlist 更干净。

### 3.7 与 GlobalShared 协作

WorldText 创建时：

```c
// 默认共享主 view 的 GlyphAtlas
DongWorldText* wt = dong_world_text_create(main_engine);

// 多 view 场景（如 multi-window editor）：用 GlobalShared
DongGlobalShared* gs = dong_global_shared_get();
DongWorldText* wt = dong_world_text_create_shared(gs);
```

GlobalShared GlyphAtlas 已经支持多 view；WorldText 直接挂 GlobalShared 即可。

### 3.8 与 Direct Draw / DOM 文字共存

| 路径 | 用途 | 不重叠 |
|---|---|---|
| DOM 文字 | 屏幕空间 UI | view 内 |
| Direct Draw `dong.renderText` | 屏幕空间 overlay | view 内 |
| **`dong_world_text_*`** | **世界空间** | 宿主 3D pass |

互不影响，但共享 GlyphAtlas + Slug 曲线纹理。

---

## 4. 实施步骤

| Step | 范围 |
|---|---|
| **S1 — C ABI + Pool 数据结构** | add/update/remove/clear；存 vector + free list；不渲染 |
| **S2 — render(ctx)：CPU 计算 + CPU 顶点** | 单 instance 单 draw（先验证正确性） |
| **S3 — Instanced glyph buffer + 1 draw call** | 1000 字符 perf 验证 |
| **S4 — Frustum culling + 排序 + depth test** | depth=1 / 0 两条路径都对 |
| **S5 — `spawn_floating` 高级接口 + tick** | 伤害浮字 demo 跑通 |
| **S6 — GlobalShared 接入** | 多 view 共享 atlas |
| **S7 — Slug 曲线纹理 instanced + perf 调优** | 达到 perf budget |

---

## 5. 通过验证规则

### 5.1 Hard

| 项 | 阈值 |
|---|---|
| 500 同帧 WorldText（伤害浮字 demo） | ≥ 144 FPS（[`perf_budget.md`](../perf_budget.md) § 3.5） |
| 30 名字版 + 5 个 3D 终端 | ≥ 60 FPS |
| `depth_test=1` 时被 3D 几何正确遮挡（demo 中放一个大 cube） | 像素验证 |
| `depth_test=0` 时始终最前 | 必须 |
| 1000 个 entry frustum cull 后只渲染可见的 | counter 验证 |
| Glyph atlas 与 main view DOM 文字共享（无重复 cache） | counter 验证 |
| `dong_world_text_destroy` 不泄漏 | ASAN |
| 切换字体 / 字号正常工作 | demo |

### 5.2 Soft

| 项 | 阈值 |
|---|---|
| WorldText render 单次 GPU draw call | = 2（depth=1 + depth=0 两 batch） |
| CPU 侧 culling + sort 时间 | < 0.5 ms（1000 entries） |
| Floating text spawn 速率 | 1000 个 / 秒（不 GC 抖动） |
| 与 GlobalShared 多 view 共享时 GPU mem 节省 | ≥ 80% |
| 字体切换 cost | < 0.1 ms |

### 5.3 必须新增的测试

| 文件 | 验证 |
|---|---|
| `examples/world_text_basic.c` | 5 个 WorldText 显示在固定世界坐标 |
| `examples/world_text_damage_500.c` | 500 个浮字 + lifetime + 上飘 |
| `examples/world_text_nameplate.c` | 30 名字版（含图标，复用 image quad） |
| `examples/world_text_depth_test.c` | 大 cube 遮挡测试 |
| `examples/world_text_global_shared.c` | 多 view 共享 |
| `tests/world/test_world_text_culling.cpp` | frustum cull 单元 |
| `tests/world/test_world_text_lifetime.cpp` | floating text tick |
| `dong/scripts/test_world_text_perf.py` | perf budget 自动验收 |

### 5.4 验证命令

```bash
# Demo
./world_text_damage_500
# 期望：屏幕中央立方体周围 500 个红色浮字向上飘 + 渐隐；右上角 FPS 显示 ≥ 144

# Perf
python dong/scripts/test_world_text_perf.py
# 期望：500 字符 1080p ≥ 144 FPS，draw call ≤ 2

# 像素验证
DONG_BENCH_AUTOSTOP=1 ./world_text_depth_test --frames 5 --output frame.bmp
python dong/scripts/tools/compare_screenshots.py frame.bmp tests_data/baselines/world_text_depth_test.bmp
```

---

## 6. 风险与回退

| 风险 | 缓解 |
|---|---|
| 宿主深度 buffer 格式与 dong shader 不匹配 | 文档明示需 `D24S8` 或 `D32F`；不匹配则 `depth_test=0` 强制 |
| 不同宿主坐标系（Y 上 / Y 下；左手 / 右手）| `DongWorldRenderContext` 加 `coord_system` 标志；shader 内适配 |
| 宿主在 dong draw 后改 pipeline state | 文档明示 dong 不假设 state；每次 render 自己设全 |
| 文字 shaping 在主线程，500 字符可能阻塞 | spawn_floating 时立即 shape + cache；tick 只更新位置不重 shape |
| `ctx->color_target` 与 dong 主 view 同一个 RT 时绘制顺序 | 文档：宿主必须在 dong main view 渲染之后调 world_text_render |
| Slug 曲线纹理 与 GlobalShared 协作时多 view 同时改 atlas | 已有 GlobalShared lock 机制；本任务复用 |

---

## 7. 不在本方案范围

- ❌ Bones / 跟随骨骼的 attached text（业务侧自己每帧调 set_position）
- ❌ 富文本 inline image / emoji 表情（v1 仅纯文字 + 单色；v2 评估）
- ❌ Outline / shadow（先做 alpha blend；effect 留 v2）
- ❌ 自动 occlusion query（深度 test 已够；不做精确 occlusion culling）
- ❌ 跨进程 / IPC 多 view（GlobalShared 是同进程）
- ❌ 替代宿主的字幕系统（字幕走 video 路径）

---

## 8. 完成后更新

- [ ] `docs/reference/features-index.md` 新增 § "World Space Text"
- [ ] `docs/overview/positioning.md` § 5.2 World Space primitives 表格 P2-1 行从"待做"改"已交付"
- [ ] `docs/developer/perf-budget.md` § 3.5 S5 实测填入
- [ ] `dong/include/dong_world_text.h` 文档化
- [ ] `docs/guide/integrations/unreal-engine.md` / `integration_unity.md` 加 "World Text" 集成段
