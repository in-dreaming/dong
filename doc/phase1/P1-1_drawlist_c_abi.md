# P1-1 — `DongDrawList` C ABI v1

> 上游：[`doc/roadmap.md`](../roadmap.md) Phase 1 P1-1
> 设计原案：[`doc/ideal/引擎适配.md`](../ideal/引擎适配.md)（路线 A）
>
> 状态：方案 v1 / 2026-04-17

---

## 1. 目标

把 dong 的渲染输出从"内部 `GPUCommandList` IR"升级为**稳定、版本化、宿主无关的 C ABI** —— `DongDrawList`。

收益：

- 宿主引擎（UE / Unity / 自研）能直接消费 dong 的绘制输出，**不需要 RT**，不依赖 SDL，也不需要包含 dong 内部头文件。
- 把"游戏内 web-like UI"和"宿主原生 UI 的 batcher"合并到同一条 paint 序列。
- 给 [P1-2 `<host-view>` 嵌入](./P1-2_host_view_embed.md)、[`integration_ue.md`](../integration_ue.md)、[`integration_unity.md`](../integration_unity.md) 提供前置基础。

---

## 2. 现状

| 项 | 状态 |
|---|---|
| `GPUCommandList`（内部 IR） | ✅，`dong/src/render/gpu_ir.hpp` |
| `DongGPUDriver` plugin vtable | ✅，`include/dong_gpu_driver.h`；execute 接收 `void* command_list` |
| 稳定 C ABI | ❌ |
| 宿主可枚举的命令流 | ❌（command list 是 fat struct，字段可能随版本变） |
| 嵌入元素命令 | ❌（P1-2 引入） |

**根本问题**：`GPUCommandType` 与 `GPUCommand` 是 dong Core 的实现细节，跟随 Core 重构而变（如 P0-1 Uber Quad 直接改了 enum 与 struct）。宿主只能跟着 Core 一起重编译。

---

## 3. 设计

### 3.1 ABI 总览

新增公共头：`dong/include/dong_drawlist.h`

```c
#ifndef DONG_DRAWLIST_H
#define DONG_DRAWLIST_H
#include <stdint.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

#define DONG_DRAWLIST_ABI_VERSION 1

typedef struct DongDrawList DongDrawList;        // 不透明
typedef uint32_t DongTextureId;                  // dong 内 atlas 句柄；0 = invalid
typedef uint32_t DongFontId;                     // dong 内 font 句柄
typedef uint32_t DongSlotId;                     // host-view slot id
```

### 3.2 命令枚举（v1，最小集）

```c
typedef enum {
    DONG_DRAW_NOOP                 = 0,

    // 状态栈
    DONG_DRAW_PUSH_TRANSFORM       = 10,    // 3x3 affine
    DONG_DRAW_POP_TRANSFORM        = 11,
    DONG_DRAW_PUSH_CLIP_RECT       = 12,    // axis-aligned
    DONG_DRAW_PUSH_CLIP_ROUNDED    = 13,    // rect + 4 角 radii
    DONG_DRAW_POP_CLIP             = 14,
    DONG_DRAW_PUSH_OPACITY         = 15,
    DONG_DRAW_POP_OPACITY          = 16,

    // 隔离层（合成提示，宿主可选实现）
    DONG_DRAW_BEGIN_LAYER          = 20,
    DONG_DRAW_END_LAYER            = 21,

    // 基础几何
    DONG_DRAW_SOLID_RECT           = 100,
    DONG_DRAW_ROUNDED_RECT_FILL    = 101,
    DONG_DRAW_ROUNDED_RECT_STROKE  = 102,
    DONG_DRAW_BORDER               = 103,    // 4 边可不同宽度+颜色
    DONG_DRAW_BOX_SHADOW           = 104,    // SDF + blur
    DONG_DRAW_LINEAR_GRADIENT      = 105,
    DONG_DRAW_RADIAL_GRADIENT      = 106,
    DONG_DRAW_CONIC_GRADIENT       = 107,
    DONG_DRAW_NINESLICE            = 108,    // P0-2 产物

    // 图片 / 文本
    DONG_DRAW_IMAGE                = 200,
    DONG_DRAW_TEXT_RUN             = 210,    // glyph run，引用 atlas

    // 嵌入（P1-2）
    DONG_DRAW_HOST_VIEW            = 300,    // emit slot 占位，宿主回调绘制
} DongDrawCmdType;
```

> v1 不包含：mask（P0-3 走通后再决定纳入命令还是只内部走离屏 layer）；YUV video（保留独立路径）。

### 3.3 命令负载（POD struct，紧凑）

```c
typedef struct { float x, y, w, h; } DongRect;
typedef struct { float r, g, b, a; } DongColor;        // 非预乘
typedef struct { float m[9]; } DongMat3;                // 行优先 affine
typedef struct { float tl, tr, br, bl; } DongCornerRadii;

typedef struct {
    DongRect      rect;
    DongColor     color;
} DongCmd_SolidRect;

typedef struct {
    DongRect          rect;
    DongCornerRadii   radii;
    DongColor         color;
    float             stroke_width;          // 0 = fill (用 RoundedRectFill 时忽略)
} DongCmd_RoundedRect;

typedef struct {
    DongRect      rect;
    DongCornerRadii radii;
    DongColor     color;
    float         blur;                       // px
    float         spread;                     // px (CSS box-shadow spread)
    float         offset_x, offset_y;         // px
    int           inset;                      // 0/1
} DongCmd_BoxShadow;

typedef struct {
    DongRect      rect;
    DongColor     color_top, color_right, color_bottom, color_left;
    float         width_top, width_right, width_bottom, width_left;
    DongCornerRadii radii;
} DongCmd_Border;

typedef struct {
    DongRect      rect;
    float         start_x, start_y, end_x, end_y;
    uint32_t      stop_count;                 // ≤ 8
    float         stop_pos[8];                // 0..1
    DongColor     stop_color[8];
} DongCmd_LinearGradient;

typedef struct {
    DongRect      rect;
    float         center_x, center_y;
    float         radius_x, radius_y;
    uint32_t      stop_count;
    float         stop_pos[8];
    DongColor     stop_color[8];
} DongCmd_RadialGradient;

typedef struct {
    DongRect      rect;
    float         center_x, center_y;
    float         from_angle_rad;
    uint32_t      stop_count;
    float         stop_pos[8];
    DongColor     stop_color[8];
    int           repeating;                  // 0/1
} DongCmd_ConicGradient;

typedef struct {
    DongRect      dest_rect;
    DongTextureId texture;
    float         uv_x, uv_y, uv_w, uv_h;     // 源图归一化 UV
    float         slice_top, slice_right, slice_bottom, slice_left;
    float         width_top, width_right, width_bottom, width_left;
    uint8_t       repeat_h;                   // 0=stretch 1=repeat 2=round
    uint8_t       repeat_v;
    uint8_t       fill_center;
    uint8_t       _pad;
    DongColor     tint;                       // 通常 (1,1,1,opacity)
} DongCmd_Nineslice;

typedef struct {
    DongRect      dest_rect;
    DongTextureId texture;
    float         uv_x, uv_y, uv_w, uv_h;
    DongColor     tint;
    uint8_t       sampling;                   // 0=nearest 1=linear 2=mip
    uint8_t       _pad[3];
} DongCmd_Image;

typedef struct {
    float         x, y;                       // 字形左上像素位置
    DongTextureId atlas;                      // 字形 atlas（MSDF）；slug 路径下复用为 curve_tex
    float         u0, v0, u1, v1;             // atlas UV
    float         w, h;                       // 屏幕像素 size
    DongColor     color;
} DongGlyph;

typedef struct {
    DongFontId    font;
    uint8_t       renderer;                   // 0=msdf 1=slug
    uint8_t       _pad[3];
    uint32_t      glyph_count;
    const DongGlyph* glyphs;                  // 由 DrawList 拥有，宿主只读
} DongCmd_TextRun;

typedef struct {
    DongRect      rect;
    DongMat3      transform;
} DongCmd_PushTransform;

typedef struct {
    DongRect      rect;                       // clip 区域（push 时的累积坐标）
} DongCmd_PushClipRect;

typedef struct {
    DongRect          rect;
    DongCornerRadii   radii;
} DongCmd_PushClipRounded;

typedef struct {
    float opacity;                            // 0..1
} DongCmd_PushOpacity;

typedef struct {
    DongRect      rect;
    uint32_t      layer_id;                   // 稳定 ID（跨帧；宿主可缓存）
    float         opacity;
    int           dirty;                      // 1 = 需要重新栅格化（宿主可观测）
} DongCmd_BeginLayer;

typedef struct {
    DongSlotId    slot_id;
    DongRect      rect;                       // slot 在当前坐标空间内的矩形
} DongCmd_HostView;
```

### 3.4 命令流的访问方式

不暴露 raw struct array（避免 ABI 锁死）。提供**只读迭代器** + **tagged 访问**：

```c
typedef struct DongDrawIter DongDrawIter;

DongDrawIter*  dong_drawlist_iter(const DongDrawList* dl);
void           dong_drawlist_iter_destroy(DongDrawIter* it);

// 推进；返回当前 cmd type，或 0 = 结束
DongDrawCmdType dong_drawlist_iter_next(DongDrawIter* it);

// 取当前命令的 payload（与 type 配套；type 不匹配返回 NULL）
const void* dong_drawlist_iter_get(DongDrawIter* it, DongDrawCmdType expected);

// 便捷宏
#define DONG_DRAW_AS(it, type, ctype) \
    ((const ctype*)dong_drawlist_iter_get((it), (type)))
```

宿主用法：

```c
DongDrawIter* it = dong_drawlist_iter(dl);
DongDrawCmdType t;
while ((t = dong_drawlist_iter_next(it))) {
    switch (t) {
    case DONG_DRAW_SOLID_RECT: {
        const DongCmd_SolidRect* c = DONG_DRAW_AS(it, t, DongCmd_SolidRect);
        host_emit_quad(c->rect, c->color);
        break;
    }
    case DONG_DRAW_TEXT_RUN: {
        const DongCmd_TextRun* c = DONG_DRAW_AS(it, t, DongCmd_TextRun);
        for (uint32_t i = 0; i < c->glyph_count; ++i) host_emit_glyph(c->glyphs[i]);
        break;
    }
    /* ... */
    }
}
dong_drawlist_iter_destroy(it);
```

### 3.5 Engine 与 DrawList 的入口

扩展 `dong/include/dong.h`：

```c
// 拉取本次 tick 产生的 drawlist；若未脏 → 与上次相同指针
const DongDrawList* dong_engine_get_drawlist(dong_engine_t* eng);

// 获取版本号（运行时检查）
uint32_t dong_drawlist_abi_version(void);   // 等于 DONG_DRAWLIST_ABI_VERSION

// 帧序号（宿主可用作缓存 key）
uint64_t dong_drawlist_frame_id(const DongDrawList* dl);

// 是否本帧重生（false = 与上次相同，宿主可跳翻译）
int      dong_drawlist_is_dirty(const DongDrawList* dl);

// 资源句柄解析（宿主拿 native 资源；走 plugin api）
typedef struct {
    void*   native_handle;     // 由当前 GPU plugin 提供（如 SDL_GPUTexture* 或 ID3D12Resource*）
    int     width, height;
    int     format;            // DongGPUTextureFormat
} DongTextureInfo;
int dong_engine_resolve_texture(dong_engine_t* eng, DongTextureId id, DongTextureInfo* out);

typedef struct {
    const char* family;
    float       size_px;
    uint8_t     renderer;      // 0=msdf 1=slug
} DongFontInfo;
int dong_engine_resolve_font(dong_engine_t* eng, DongFontId id, DongFontInfo* out);
```

### 3.6 DrawList 与 GPUCommandList 的关系

```
内部:    DOM/Layout → Painter → DisplayList → GPUCompiler → GPUCommandList
                                              ↘
                                        DrawListEmitter → DongDrawList (公共 ABI)

GPUCommandList 仅供 dong 自己的 SDL backend 用（通过 DongGPUDriver.execute）；
DongDrawList 给宿主用。两者由同一个 DisplayList 派生，保证像素一致。
```

新增模块：`dong/src/render/drawlist_emitter.{hpp,cpp}`，逻辑非常薄（POD 拷贝）。

> 不删除 `GPUCommandList`：dong 自己的 SDL backend 仍按原路径走，避免 SDL backend 跟着 ABI 改动；只是另起一条 emitter。

### 3.7 版本与兼容承诺

- **`DONG_DRAWLIST_ABI_VERSION = 1`**：本 v1 冻结的命令枚举值与 payload struct 布局不再改。
- 后续新增命令 → 用新枚举值（≥ 400）；老宿主见到未知 type 应**跳过**（iterator 保证可推进）。
- 后续给现有 payload 加字段 → **不允许**；改用新命令类型代替（保持 struct size）。
- **大端 / 32 位平台 / NEON 对齐**：v1 仅承诺 little-endian + 64bit；其他需求另行评估。

---

## 4. 实施步骤

| Step | 范围 | 依赖 |
|---|---|---|
| **S1 — 头文件 + 命令枚举 + payload struct** | `include/dong_drawlist.h`；不实现；CI 编译通过 | — |
| **S2 — DrawListEmitter 实现：基础几何 + 状态栈** | solid_rect / rounded / border / clip / transform / opacity | DisplayList 现状 |
| **S3 — 图片 + 文本** | 与 GlyphAtlas / GPU plugin 协作；texture id 解析 | dong glyph atlas |
| **S4 — 渐变 + nineslice + 阴影** | 余下命令 | P0-2 / P0-3 输出 |
| **S5 — host_view 命令** | 与 P1-2 联动 | P1-2 |
| **S6 — Iterator + 资源句柄解析** | 完整 C ABI 暴露；写一个最小 C 程序消费 DrawList 并打印 | S2-S5 |
| **S7 — 字段冻结 + 文档发布** | 头文件 ABI 锁；写 spec doc | — |

---

## 5. 通过验证规则

### 5.1 Hard

| 项 | 阈值 |
|---|---|
| `dong_drawlist.h` 在 macOS/Linux/Windows + clang/gcc/msvc 编译通过 | 必须 |
| `DongCmd_*` struct sizeof 在三平台一致（little-endian, 64bit） | sizeof regression test 通过 |
| Sample C 程序能消费 DrawList 并打印结构化输出（不依赖 SDL / 不依赖 dong 内部头文件） | 必须 |
| 同一 HTML 在 SDL backend（`GPUCommandList` 路径）与 DrawList 翻译路径下，像素一致 | `examples/data/tests/` 全集 ≥ 99% 通过 |
| ABI version 检查：`dong_drawlist_abi_version()` 与编译时常量一致 | 单元 |
| 未知命令枚举（模拟从未来版本读取）iterator 不崩 | 单元 |
| `DongDrawList` 内部内存不向宿主暴露指针生命周期歧义 | 文档 + iterator 释放约定明确 |

### 5.2 Soft

| 项 | 阈值 |
|---|---|
| 单帧 emit DrawList 开销 | < 0.5 ms（典型场景 < 200 命令） |
| iterator 无堆分配 | per next 调用 0 alloc |
| 头文件无 STL / C++ 依赖 | 必须（纯 C） |
| host_view 嵌入 demo（与 P1-2）跑通 | 完成时验证 |

### 5.3 必须新增的测试

| 测试 | 内容 |
|---|---|
| `tests/abi/test_drawlist_struct_sizes.cpp` | 静态 assert 各 payload sizeof |
| `tests/abi/test_drawlist_endianness.cpp` | 在小端机器上写入字节模式验证 |
| `examples/abi_consumer.c` | 纯 C 程序：load HTML → tick → iter drawlist → print 命令名 + 关键字段；不链接 dong 内部头 |
| `tests/render/test_drawlist_pixel_parity.html` | 同一 HTML 走两条路径像素 diff < 1% |

### 5.4 验证脚本

```bash
# ABI struct size 回归
cd dong && zig build run-feature-tests --filter "DrawListAbi"

# 像素回归
python dong/scripts/tools/run_baseline_compare.py --suite examples/data/tests

# Sample consumer
gcc -std=c11 -Iinclude examples/abi_consumer.c -ldong -o abi_consumer
./abi_consumer data/tests/test_simple_panel.html
# 期望输出: PUSH_TRANSFORM, SOLID_RECT, BORDER, TEXT_RUN, ...
```

---

## 6. 风险与回退

| 风险 | 缓解 |
|---|---|
| ABI v1 设计漏字段，v2 必须破坏兼容 | 字段宽松（color 留 4 通道；rect 用 float 不用 int；预留 reserved bits）；新命令走新 type，不改老 struct |
| 宿主滥用 `glyphs` 指针跨帧使用 | 文档强约束：仅在 `dong_engine_get_drawlist` 返回值有效；宿主必须复制；提供 sample debugging mode 主动打 poison |
| 内部 GPUCommandList 与 DrawList 信息密度不匹配（如 image_fit / object-position） | 在 emitter 阶段把这些"复杂语义"展开为基础命令组合（如 transform + image），宿主只看简单图元 |
| 性能：emit 阶段两次 POD 拷贝 | 性能 budget 已留 0.5 ms；profile 后若超可改为 in-place 编码 |
| `host-view` 嵌入需要宿主实现 hit-test 反馈 | 与 P1-2 配套设计；本 ABI 仅 emit，回流走另一条 callback API |

---

## 7. 不在本方案范围

- ❌ 完整 RHI 命令暴露（如 pipeline / shader / vertex buffer）—— 那是 `DongGPUDriver` 的职责，不与 DrawList 混
- ❌ 字幕 / 视频 YUV 命令（保留独立路径，宿主不需要看到）
- ❌ Mask 命令（v1 由 dong 内部用 BeginLayer + 二次 pass 处理；宿主只看 BeginLayer/EndLayer）
- ❌ 跨帧 diff / patch 协议（宿主需要时自行 hash）
- ❌ JS 层访问 DrawList（不在 web 标准范畴；想要的话单独提案）

---

## 8. 完成后更新

- [ ] `doc/ideal/引擎适配.md` 第 5 章追加"已实施"段
- [ ] `doc/integration_ue.md` § 4.2 / 4.3 状态从"草案"改为"可对接"
- [ ] `doc/integration_unity.md` 同上
- [ ] `doc/重要特性.md` 新增 § "DongDrawList 公共 ABI"
- [ ] `doc/positioning.md` § 5.2 表格 `DongDrawList` 行从"待做"改"已交付"
- [ ] `doc/perf_budget.md` 新增"DrawList emit 时间"指标条目
