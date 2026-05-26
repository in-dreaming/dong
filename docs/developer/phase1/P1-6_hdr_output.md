# P1-6 — HDR 输出（基础）

> 上游：[`docs/roadmap.md`](../roadmap.md) Phase 1 P1-6
> 既有占位：[`docs/developer/design/hdr.md`](../ideal/hdr.md)
>
> 状态：方案 v1 / 2026-04-17

---

## 1. 目标

让 dong 在支持 HDR 的显示器 / 主机上**输出 HDR 信号**，UI 高亮元素亮度可超出 SDR 白点（300+ nits → 800–4000 nits）：

- swapchain 支持 scRGB（FP16 线性）/ Rec.2100 PQ（HDR10）。
- CSS 颜色与 gradient 在线性空间正确插值。
- 提供 CSS 扩展能让业务方"指定 HDR 强度"（例如：`color: oklch(0.95 0.4 30 / 1)` 与高亮区域 `--hdr-multiplier: 4` 配合）。
- SDR 显示器上无回归（自动 fallback 到 sRGB）。

不在 v1：HDR 字体（亮发光字）、HDR 图片解码（OpenEXR / HEIF + HDR / AVIF HDR）、HDR 视频。

---

## 2. 现状

| 项 | 状态 |
|---|---|
| swapchain 格式 | sRGB BGRA8（`backends/sdl/sdl_window.cpp` / `sdl_gpu_driver_init.cpp`） |
| Shader 输出 | 8-bit unorm，sRGB encoding 由 swapchain 完成 |
| CSS 颜色空间 | sRGB（[`html_css_dom_草案.md`](../specific/html_css_dom_草案.md) § 3.16） |
| Color space metadata | 无 |
| HDR 占位文档 | [`docs/developer/design/hdr.md`](../ideal/hdr.md)（仅链接） |

---

## 3. 设计

### 3.1 输出格式与色域

| 平台 | HDR swapchain 格式 | 色域 | EOTF |
|---|---|---|---|
| Windows D3D12 / Vulkan | `R16G16B16A16_FLOAT` (scRGB) | Rec.709 → 显示器 | Linear（驱动 PQ 编码） |
| macOS Metal | `R16G16B16A16_FLOAT` (extended sRGB) | Display P3 | Linear → EDR scale |
| iOS / iPadOS | 同上 | Display P3 | EDR |
| 主机（Xbox/PS5）| 平台特有，下文标注 | Rec.2020 | PQ |
| Linux Wayland HDR | Vulkan `R10G10B10A2_UNORM` (HDR10) | Rec.2020 | PQ（业务侧编码） |

**v1 优先 PC（Windows + macOS）+ 主机 stub**。Linux HDR 标准刚定（2024–2025），观察。

实施上：

```cpp
// sdl_gpu_driver_init.cpp 增加
DongOutputColorSpace cs = detect_or_user_request();
SDL_GPUTextureFormat fmt = (cs == DongOutputColorSpace::HDR10
                             || cs == DongOutputColorSpace::scRGB)
                            ? SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT
                            : SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM_SRGB;
SDL_SetGPUSwapchainParameters(device, window, cs, fmt);
```

### 3.2 公共 ABI

```c
// dong/include/dong.h（扩展）

typedef enum {
    DONG_COLOR_SPACE_SRGB         = 0,    // 默认；SDR
    DONG_COLOR_SPACE_SRGB_LINEAR  = 1,    // 线性 sRGB
    DONG_COLOR_SPACE_DISPLAY_P3   = 2,    // P3 SDR
    DONG_COLOR_SPACE_SCRGB        = 10,   // FP16 linear, Rec.709 primaries (Windows HDR)
    DONG_COLOR_SPACE_HDR10        = 11,   // PQ EOTF + Rec.2020
    DONG_COLOR_SPACE_DISPLAY_P3_EDR = 12, // macOS EDR
} DongColorSpace;

int  dong_engine_set_output_color_space(dong_engine_t* eng, DongColorSpace cs);
DongColorSpace dong_engine_get_output_color_space(dong_engine_t* eng);

// HDR 显示能力查询（返回 0 = SDR only）
typedef struct {
    int   hdr_supported;       // 0/1
    float max_luminance_nits;  // 显示器报告
    float min_luminance_nits;
    float white_point_nits;    // SDR 参考白
    float max_avg_luminance_nits;
} DongDisplayHDRCapabilities;

int dong_engine_query_hdr_capabilities(dong_engine_t* eng,
                                        DongDisplayHDRCapabilities* out);
```

### 3.3 CSS 颜色与 HDR

#### 3.3.1 颜色空间

CSS Color Module Level 4 已经规定了 `color()`、`oklch()`、`oklab()`、`color-mix()` 等线性 / 感知均匀颜色函数（[`gap_analysis.md`](../specific/html_css_dom_gap_analysis.md) § 3.5）。

dong 内部把所有颜色统一存为 **sRGB Linear FP32**：

```cpp
struct Color { float r, g, b, a; };  // linear sRGB, premultiplied 在 shader 末端
```

解析时：

| 输入 | 转换 |
|---|---|
| `#rrggbb` / `rgb()` | sRGB → linear via gamma |
| `hsl()` | hsl → sRGB → linear |
| `oklch()` / `oklab()` | OKLab → linear sRGB |
| `color(display-p3 r g b)` | display-p3 → linear sRGB（适当 chromatic adaptation） |
| `color(rec2020 r g b)` | rec2020 → linear sRGB（v1 截断或 gamut map） |

#### 3.3.2 HDR 颜色 — 私有扩展 `--hdr-boost`

**问题**：CSS 没有定义"超出 1.0"的颜色。HDR UI 需要"亮 4 倍"的高亮区域。

**v1 方案**：私有 CSS 自定义属性，shader 末端读取：

```css
.hud-flash {
    background: white;
    --hdr-boost: 4;       /* dong 私有；shader 末端 color *= boost；仅 HDR 输出生效 */
}
```

实施：
- 解析时 `--hdr-boost` 数值存入 ComputedStyle（与 CSS variable 同机制）。
- Painter emit `box.hdr_boost_top/bottom/...`（按属性继承）。
- shader 末端：`out_rgb = linear_rgb * hdr_boost;`（SDR 模式 boost 强制为 1.0）。

#### 3.3.3 标准 CSS HDR：观望

CSS WG 在讨论 `dynamic-range-limit` 等属性。等草案稳定后再纳入；v1 用私有 `--hdr-boost`。

### 3.4 Shader 改造

| 阶段 | sRGB 模式 | HDR 模式 |
|---|---|---|
| 输入颜色（CSS）| linear sRGB FP32 | linear sRGB FP32 |
| 内部计算（gradient / blend）| linear FP32 | linear FP32 |
| Image atlas | sRGB BGRA8（或 BC7_SRGB） | 同 + 标记 `srgb_decode` |
| **swapchain 写入** | linear → swapchain（驱动做 sRGB encode） | linear * hdr_boost → swapchain（FP16 直接写线性） |
| 显示器 EOTF | sRGB | scRGB / PQ（驱动） |

shader 关键改动（`final_*.hlsl`）：

```hlsl
float4 final_compose(float3 linear_rgb, float a, float hdr_boost) {
#if HDR_OUTPUT
    return float4(linear_rgb * hdr_boost * a, a);
#else
    return float4(linear_rgb * a, a);   // 驱动 sRGB encode
#endif
}
```

通过 shader specialization / 编译宏控制；同一管线可两个变体。

### 3.5 Tonemapping（SDR 元素在 HDR 输出下）

HDR 模式下 SDR 颜色（boost = 1.0）显示器上**亮度等于 SDR 参考白**（通常 100–200 nits），不会自动变亮。

仅 `--hdr-boost > 1` 的元素超出参考白。这是符合 UI 设计预期的（业务方控制哪些元素 HDR）。

不做 inverse tonemap，避免误把所有 UI 拉亮。

### 3.6 自动检测与 fallback

启动时：

```cpp
1. dong_engine_query_hdr_capabilities()
2. if (hdr_supported && user_pref == HDR):
     set_output_color_space(SCRGB);
     compile shader HDR variant;
   else:
     set_output_color_space(SRGB);
     compile shader SDR variant;
```

显示器中途切 SDR/HDR：监听 `SDL_EVENT_DISPLAY_HDR_STATE_CHANGED`（SDL3）→ rebuild swapchain + reload shader variant。

### 3.7 与三轨的关系

| 模式 | HDR 支持 |
|---|---|
| **DOM** | ✅，CSS `--hdr-boost` 可用 |
| **Scene Graph** | ✅，`SceneNode.hdr_boost` 字段 |
| **Direct Draw** | ✅，`dong.drawRect(..., {hdrBoost: 4})` |

### 3.8 与 P1-1 DrawList ABI 的协调

`DongColor` 在 v1 是 4×float 非预乘 sRGB。HDR 引入后：

- 沿用 4×float；**约定改为 linear sRGB**（不是非线性 sRGB）。
- 新增 `DongCmd_PushHDRBoost(float)` / `DongCmd_PopHDRBoost`，与 opacity stack 同语义。
- 宿主消费 DrawList 时，如果自身在 HDR 输出，应用 boost；SDR 输出忽略 boost。

> 这是 P1-1 v1 时已经预留的"reserved 字段"扩展点。如果 P1-1 已发布则走 v1.1 minor。

---

## 4. 实施步骤

| Step | 范围 |
|---|---|
| **S1 — Color 内部统一为 linear sRGB** | parser 转换；既有像素 baseline 不变（驱动 sRGB encode 抵消） |
| **S2 — `dong_engine_query_hdr_capabilities` ABI** | 仅返回，不切；用于业务侧检测 |
| **S3 — swapchain 格式切换 + shader 变体** | 显式 `set_output_color_space(SCRGB)` 后 swapchain 改 FP16；像素无回归（boost = 1） |
| **S4 — `--hdr-boost` 解析与传递** | 高亮元素亮度可超 SDR |
| **S5 — Display 切换事件 + 自动 fallback** | 拔插 HDR 显示器时不崩 |
| **S6 — `oklch` / `oklab` / `color(display-p3 ...)` 解析** | 与 P0-8 / [`gap_analysis.md`](../specific/html_css_dom_gap_analysis.md) § 3.5 协同 |
| **S7 — DrawList ABI v1.1 hdr boost 命令** | 与 P1-1 联动 |

---

## 5. 通过验证规则

### 5.1 Hard

| 项 | 阈值 |
|---|---|
| SDR 模式下既有 baseline 全集 | 100% 像素一致（不引入 HDR 回归） |
| HDR 模式下 swapchain 创建成功（验证 SDL3 返回值） | 必须 |
| `dong_engine_query_hdr_capabilities` 在 SDR 显示器上返回 `hdr_supported = 0` | 必须 |
| `--hdr-boost: 4` 在 HDR 显示器上实测亮度 > 400 nits（业务报告或 colorimeter） | 手动 |
| 显示器切换 HDR/SDR 时 dong 不崩，画面在 1 s 内恢复正确 | 手动 |
| Linear 颜色插值正确：`linear-gradient(red, blue)` 中点为 dark purple（与浏览器 LIN 模式一致） | 像素 baseline |
| `oklch()` / `color(display-p3 ...)` 颜色解析正确 | 像素 baseline |

### 5.2 Soft

| 项 | 阈值 |
|---|---|
| HDR 模式 GPU 时间 vs SDR | 增量 ≤ 10%（FP16 vs UNORM8） |
| 颜色解析 `oklch` 速度 | < 5 µs / 颜色（缓存命中后） |
| Shader variant 编译时间 | 与 SDR 同等量级 |
| 主机平台（Xbox / PS5）支持 | 留测试 backlog；ABI 已具备 |

### 5.3 必须新增的测试

| 文件 | 验证 |
|---|---|
| `test_color_linear_blend.html` | linear gradient 中间值（与 Chrome `color-interpolation: oklab` 对照） |
| `test_oklch_basic.html` | oklch 颜色 |
| `test_color_display_p3.html` | display-p3 颜色（SDR 显示器降级到 sRGB） |
| `test_hdr_boost_basic.html` | `--hdr-boost: 4` 元素 |
| `test_hdr_boost_gradient.html` | gradient 中部分 stop boost |
| `test_hdr_capabilities_query.html` | JS 调 `dong.queryHDRCapabilities()` |
| `tests/render/test_hdr_swapchain_init.cpp` | C++ 单元：swapchain HDR 创建 |

### 5.4 验证命令

```bash
# SDR 回归
python dong/scripts/tools/run_baseline_compare.py --suite examples/data/tests

# HDR 模式 dry run（无 HDR 显示器也能跑，输出 fp16 图）
DONG_OUTPUT_COLOR_SPACE=scrgb dong_app.exe --html data/tests/test_hdr_boost_basic.html
# 截图 fp16 → 工具确认 boost 区域 > 1.0

# 真实 HDR 显示器（手动）
# Windows: 系统设置开 HDR；启动 dong_app；用屏摄或 colorimeter 确认
```

---

## 6. 风险与回退

| 风险 | 缓解 |
|---|---|
| 既有 baseline 在"内部统一 linear" 后像素变（gradient blend 修正后中点变深紫）| 接受；spec 正确性优先；用 baseline 更新 + 文档说明 |
| Shader 双 variant 维护负担 | 用 specialization constant；同一文件两个变体 |
| 主机平台 SDL3 HDR 支持不全 | 主机分支后做；v1 PC + macOS 优先 |
| 业务方滥用 `--hdr-boost: 100` 烧屏 | shader clamp boost ≤ 16；超出 console.warn |
| 显示器实际能力远低于报告值 | 提供 `dong_engine_set_max_hdr_boost(eng, n)` 让业务方限制 |
| sRGB / linear 双计算误差，文字描边走样 | text shader 与 quad shader 共享 final_compose；统一空间 |
| Plugin (DongGPUDriver) backend 不支持 FP16 | get_capabilities 增 `hdr_supported`；查询失败 fallback SDR |

---

## 7. 不在本方案范围

- ❌ HDR 字体效果（亮发光字、辉光描边；要看 Slug shader 兼容性）
- ❌ HDR 图片解码（OpenEXR / HEIF HDR / AVIF HDR）
- ❌ HDR 视频
- ❌ Tone mapping（SDR 内容拉亮 HDR；不做，避免误亮）
- ❌ Scene-referred 工作流（dong 是 display-referred）
- ❌ Color management profile (ICC profile 应用到 image)
- ❌ Wide gamut 色彩选择器 UI

---

## 8. 完成后更新

- [ ] `docs/developer/design/hdr.md` 替换为正式实现说明
- [ ] `docs/reference/css-subset.md` § 3.6 / § 3.16 oklch / display-p3 状态
- [ ] `docs/reference/features-index.md` 新增 § "HDR 输出"
- [ ] `dong/include/dong.h` HDR ABI 文档化
- [ ] `docs/developer/perf-budget.md` § 3 加 HDR 模式 GPU 时间对比
