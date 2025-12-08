先帮你把整个问题“系统化”拆开，说清楚是哪一层没做到位，以及一个**正式的修复方案**应该长什么样。

---

### 一、当前框架为什么会出问题？——三套几何被我们“手工拼”错了

你现在的文字管线，实际上有 **三套几何坐标** 在同时存在：

1. **排版几何（HarfBuzz + UnifiedFontMetrics，设计单位）**

   - `text_shaper.cpp` 里：
     - `hb_font_set_scale(units_per_em, units_per_em)`  
     - HarfBuzz 输出 `pen_x_units / pen_y_units / x_advance / y_advance`。
   - `painter.cpp` 里：
     - 用 `ascent_units / descent_units / line_height_units` 算 `baseline_y`。
     - `GlyphInstance.pen_x_units / pen_y_units` 都是在 **baseline = y=0** 的设计空间里。
   - 从你发的 `TextDebug` 日志看：

     ```text
     overlay-title raw='LayerTree & Cache Preview' ...
     glyph[2]: glyph_id=92 char='y' cluster=2 pen=(2390.00,0.00)
     ```

     所有 glyph 的 `pen_y=0`，说明 **这一行的基线在 HarfBuzz 世界里是完美对齐的**，问题不是排版侧。

2. **字体几何（FreeType metrics + outline，设计单位）**

   在 `glyph_atlas.cpp::generateMSDF`：

   ```cpp
   out_metrics.bearing_y_units = face->glyph->metrics.horiBearingY;
   msdfgen::Shape::Bounds bounds = shape.getBounds();
   out_metrics.bounds_bottom = bounds.b;
   out_metrics.bounds_top = bounds.t;
   ```

   对于 overlay 里的 `'y'`（Arial Bold, glyph=92），日志是：

   ```text
   [MSDF] glyph=92 ... metrics: ... by=1062.0 ...
   [MSDF] glyph=92 ... bounds: l=14.0 b=-431.0 r=1106.0 t=1062.0
   ```

   这说明在 **FreeType 设计空间** 中：

   - baseline 在 y=0；
   - 顶部在 1062（ascent 区）；
   - 底部在 -431（descender）。

3. **MSDF / GPU 几何（纹理像素 + 屏幕像素）**

   - MSDF 生成：
     ```cpp
     scale    = (msdf_size - 2*range) / max(width,height)
     translate = (range - bounds.l*scale, range - bounds.b*scale)
     ```
   - atlas 中存储 `u0/v0/u1/v1`，以及 `msdf_scale` 等。
   - `gpu_driver_sdl.cpp` 里，我们自己推了一套公式，用 HarfBuzz 的 `pen_x/pen_y` + FreeType 的 `bearing_y` + msdfgen 的 `bounds_top/bottom` + 自己定义的 `padding_units` 去算：

     ```cpp
     glyph_w = (bounds_top - bounds_bottom + 2*padding) * pixel_scale;
     glyph_y = pen_y_px - bearing_y_px ± padding_px;   // 这一段我们来回改过几次
     ```

   问题就出在这里：**这一层是我们“手动拼装”的，没有任何一个库给你现成公式**，所以一旦在符号或 padding 上搞错号，descender 就会多沉几像素。

---

### 二、从你给的日志精确看：现在的公式仍然“有偏”

拿 overlay-title 里的几个字做一个对比（都在同一行，`pen_y=72.04`）：

- `'/'`（glyph=47，非 descender）：

  ```text
  rect=(475.38,59.62,11.11,14.20)
  ```

  - 顶部距基线：`72.04 - 59.62 ≈ 12.42 px`
  - 底部距基线：`59.62 + 14.20 - 72.04 ≈ 1.78 px`

- `'y'`（glyph=92，descender）：

  ```text
  rect=(491.78,62.44,11.64,14.58)
  bounds=(14.0,-431.0,1106.0,1062.0) bearing_y=1062.0 padding=248.83 scale=0.0073
  ```

  - 顶部距基线：`72.04 - 62.44 ≈ 9.60 px`
  - 底部距基线：`62.44 + 14.58 - 72.04 ≈ 4.98 px`

结合度量：

- ascent_px ≈ `1062 * 0.0073 ≈ 7.8`
- descender_px ≈ `431 * 0.0073 ≈ 3.1`
- padding_px ≈ `248.83 * 0.0073 ≈ 1.8`

理论上，**“符号化”对称的矩形** 应该是：

- 顶部：`ascent_px + padding_px ≈ 7.8 + 1.8 ≈ 9.6`（与你日志 9.6 一致）
- 底部：`descender_px + padding_px ≈ 3.1 + 1.8 ≈ 4.9`（与你日志 4.98 一致）

也就是说：**在我们当前这套公式下，从纯数学角度，y 的 bbox 是符合字体度量 + padding 的**。  
但你截图里的“ABCayvi”对比明显告诉我们：即便 bbox 是对称的，**字型的“视觉重心”和浏览器对比仍差一截**——这不是两三个像素这么简单，而是整个 descender 区相对其他字母“略抬不够”。

根本原因是：

> 我们把 **FreeType 的度量（horiBearingY / bounds）** 当成了“浏览器的视觉 baseline 规范”，  
> 但浏览器实际用的是 **更复杂的一套：OS/2 Typo metrics + glyph extents + 字体设计习惯**，并且往往在渲染管线里做了“视觉上更自然”的偏移。

---

### 三、为什么 HarfBuzz / FreeType / msdfgen 自己没有帮我们处理 y/p/q？

这些库各自负责的是不同层面的事情：

- **FreeType** 提供的是「字体自身声明的几何」：
  - `horiBearingY`、`bbox`、OS/2 表；  
  - 它不会帮你决定“CSS line-height: normal 时 baseline 精确在哪条光栅行上”。
- **HarfBuzz** 负责 **shaping**：
  - 字序、kerning、ligature、glyph ids、`pen_x/pen_y`；
  - 它也只告诉你：“在字体设计空间里，baseline = 0，该 glyph 的 extents 是多少”，不会替你选择哪套度量当 CSS baseline。
- **msdfgen** 纯粹是几何→距离场：
  - 给你 `scale + translate`，保证轮廓带上一/两圈 range-pixel 进一个正方形纹理；
  - 它根本不了解 baseline，更不懂 CSS。

**浏览器之所以“看起来正确”，是因为它们在这几层之上又加了一层「统一字体度量 → 布局几何 → 光栅化几何」的抽象。**  
我们当前框架已经尝试做了这件事（`UnifiedFontMetrics` / `line_height_units`），但在「glyph→屏幕像素矩形」这最后一段上，仍然是在 FreeType + msdfgen 度量上直接“瞎拼”。

---

### 四、正式解决方案应该怎么做？

#### 1. 统一一个“排版几何层”，禁止再在 GPU 里玩度量推导

**目标**：GPU 层只做两件事：

- 按“排版几何”给的矩形（顶、底、左、右）画一个字形；
- 按“MSDF 投影参数”给的 UV 矩形去采样距离场。

不要在 GPU 里再推导 `bearing_y / bounds_top / bounds_bottom`。

**具体设计：**

- 在 `TextShaper` / `font_metrics` 层：
  - 用 OS/2 Typo metrics / HarfBuzz 的 `hb_font_get_glyph_extents` 计算出每个 glyph 相对于 baseline 的 **“逻辑 bbox”**：
    - `logical_top`, `logical_bottom`, `logical_left`, `logical_right`（单位：design units）。
  - 这一步等价于浏览器内部的字体度量规范（可以对照 HarfBuzz / FreeType 的推荐实现）。
- 在 `painter.cpp`：
  - 对每个 glyph 直接生成 **世界空间矩形**：
    - `world_top    = baseline_y - logical_top * scale`;
    - `world_bottom = baseline_y - logical_bottom * scale`;  
      （注意 descender 是负值，所以这里会向下）
  - **这一层完全不关心 MSDF 的 padding / bounds / scale**。

这样 `A/B/C/a/y/p/q` 的所有竖直关系就完全回到了一个统一的“逻辑字体几何”里，**所有特殊字母都自动跟随字体设计，而不是手推公式**。

#### 2. MSDF 层只负责“逻辑 bbox ↔ 纹理 bbox”的投影

在 `glyph_atlas.cpp` / `gpu_driver_sdl.cpp`，我们的任务只剩下：

- 保证生成 MSDF 时，“包住 logical bbox + range padding”的那块区域，被缩放进 `glyph_bitmap_size`；
- 把这个映射保存成一套简洁的参数，供 GPU 采样用。

一个比较干净的方案是：

- `GlyphMetrics` 增加字段：
  - `logical_left/right/top/bottom`（设计单位，来自上一步统一度量）；
  - `msdf_scale`、`msdf_translate_x/y`（已经有）。
- 计算 UV 时，**不要再用 entry 的 `u0/v0/u1/v1` + 手写 padding 推导**，而是在 CPU 里直接解这个仿射：

  \[
  \text{tex\_coord} = \text{project}(\text{logical\_coord})
                    = \text{logical\_coord} * \text{msdf\_scale} + \text{translate}
  \]

  再除以 atlas 宽高得到 [0,1] 区间的 `uv`。

- 这样：
  - 世界空间矩形边界 = `logical_bbox * scale_to_pixels + baseline`；
  - 纹理空间矩形边界 = `project(logical_bbox)`；
  - GPU 只做线性插值，**不会再自己“想象”任何 descender 或 padding**。

#### 3. 过渡期的实现路径（不用一次性大改）

如果一次性引入“完整逻辑 bbox + 仿射投影”太重，可以分两步走：

- **短期（保证正确）：**
  - 在 `glyph_atlas.cpp` 里，用 FreeType 的 `FT_Outline_Get_CBox` 或 `FT_Glyph_Get_CBox(FT_GLYPH_BBOX_UNSCALED)` 计算 **baseline 坐标系下的 bbox**，写入 `GlyphMetrics.logical_*`；
  - 在 GPU 侧，**只用 `logical_*` + HarfBuzz baseline 去算 `glyph_y`**，把现在所有用 `bearing_y` 和 `bounds_*` 拼接的地方替换掉；
  - 继续用现有 `u0/v0/u1/v1` 填充 UV（只要 bbox 对了，UV 现在就是视觉误差的次要来源）。
- **中期（更优雅）：**
  - 清理掉 `glyph_driver_sdl.cpp` 里那一大坨注释 + 手推公式；
  - 把“设计空间 → MSDF 纹理”的仿射映射变成一个小工具函数，集中在 `glyph_atlas.cpp` 里维护。

---

### 五、为什么说“更可能是我们使用错误”？

你问这句非常关键：“为什么已有的几套基础设施没有处理 ypq 这些情况？我觉得不太可能，更可能是我们使用错误。”

从上面的拆解可以看出来：

- HarfBuzz / FreeType / msdfgen **都在各自职责内给出了正确、精确的信息**：
  - HarfBuzz：pen_x/pen_y 全 0，说明 shaping 基线没错；
  - FreeType：ascent=1062, desc=-431 正常；
  - msdfgen：bounds + scale + translate 一致。
- 出问题的是我们在 **GPU 层用“半拍的公式”去拼这些度量**：
  - 一会儿用 `bearing_y`，一会儿用 `bounds_top/bottom`，中间夹了一个自己定义的 `padding`；
  - 没有一个「统一的逻辑 bbox 抽象」来兜住它们。

所以你的判断是对的：**不是这些基础库不支持 descender，而是我们把它们的输出“缝合”得太随意了**。  
正式方案就是：把“字体逻辑几何”抽象出来，GPU 只吃 bbox+UV，不再在最底层临时推导 y/p/q 的垂直位置。

--
# 方案
### 最终正式方案：统一字体几何 & 简化 GPU 责任

#### 目标

- **统一一套“逻辑字体几何”**（基于 HarfBuzz + FreeType metrics），所有排版和渲染都只认这一套。
- **GPU 层不再推导基线/descender**，只根据上层给的矩形画字，再根据 MSDF 的仿射变换采样。
- 在 `Arial Bold` 等西文字体上，`ABCaypq` / `LayerTree & Cache Preview` 的对齐效果与现代浏览器肉眼一致。

#### 核心思路

1. **逻辑字体度量层（排版空间）**

- 在 [`dong/src/render/font_metrics.cpp`](dong/src/render/font_metrics.cpp) 和 [`dong/src/render/text_shaper.cpp`](dong/src/render/text_shaper.cpp) 中：
- 引入统一的 `LogicalGlyphBox` / `LogicalFontMetrics` 概念：
- 每个 glyph 相对于 baseline 的 `logical_top`, `logical_bottom`, `logical_left`, `logical_right`（design units）。
- 整体字体的 `ascent`, `descent`, `line_height` 继续沿用现有 OS/2 Typo 逻辑。
- `logical_*` 的来源：
- 基础版：使用 FreeType 的 `FT_Outline_Get_CBox` 或 `FT_Glyph_Get_CBox(FT_GLYPH_BBOX_UNSCALED)` 计算 glyph bbox，然后通过 baseline 偏移（`horiBearingY`/`metrics.ascender`）换算到 baseline 坐标系；
- 进阶（可选）：对西文字体可以参考 HarfBuzz / FreeType 推荐做法，用 OS/2 / winAscent / winDescent 做校正，使逻辑 bbox 更贴近浏览器实现。

2. **GlyphMetrics 中显式存储逻辑 bbox**

- 在 [`dong/src/render/glyph_atlas.hpp`](dong/src/render/glyph_atlas.hpp) 的 `GlyphMetrics` 中新增字段：
- `logical_left_units`, `logical_right_units`, `logical_top_units`, `logical_bottom_units`（基线坐标系下的设计单位）；
- 在 `[dong/src/render/glyph_atlas.cpp](dong/src/render/glyph_atlas.cpp)::generateMSDF` 中：
- 对每个 glyph：
- 通过上一步的统一逻辑（或直接在这里用 FreeType + baseline 偏移）填充 `logical_*`；
- 保留现有的 `bounds_*` 仅用于 MSDF 投影，不再在 GPU 里参与屏幕坐标计算。

3. **Painter 负责世界空间矩形，GPU 不再算基线**

- 在 [`dong/src/render/painter.cpp`](dong/src/render/painter.cpp) 中，构建 `DrawGlyphRunData` 时：
- 目前只传 `baseline_x / baseline_y + pen_x_units / pen_y_units`，让 GPU 再算 `glyph_x/glyph_y/glyph_w/glyph_h`；
- 改为：
- 直接在 Painter 里，为每个 glyph 计算**屏幕空间的逻辑矩形**：
- `top_px    = baseline_y - logical_top_units * scale_to_pixels`；
- `bottom_px = baseline_y - logical_bottom_units * scale_to_pixels`；
- `left_px   = baseline_x + pen_x_units * scale_to_pixels + logical_left_units * scale_to_pixels`；
- `right_px  = ... + logical_right_units * scale_to_pixels`；
- 将这个矩形写入 `GlyphInstance`（例如新增 `rect_x, rect_y, rect_w, rect_h` 字段），连同 `glyph_id` 一并下发到 GPUCommand。
- 这样：
- HarfBuzz 的基线 + FreeType 的逻辑 bbox，已经在 CPU 上完全确定了 top/bottom/left/right；
- GPU 再也不接触 `bearing_y` / `bounds_*` / 手推 padding。

4. **GPUCommand / GPU IR 微调以传递 glyph 矩形**

- 在 [`dong/src/render/gpu_ir.hpp`](dong/src/render/gpu_ir.hpp) 中：
- 扩展 `GlyphInstance` 结构，使其包含：
- `glyph_id`（已有）、
- `rect_x, rect_y, rect_w, rect_h`（屏幕像素）、
- 保留 `pen_x_units/pen_y_units` 作为调试或未来用途（可选）。
- 确保 `GPUCommandType::DrawText` 携带的 `glyphs` 数组里，已经是**世界空间矩形**。

5. **GPUDriverSDL 只做 MSDF 采样与批次构建**

- 在 [`dong/src/render/gpu_driver_sdl.cpp`](dong/src/render/gpu_driver_sdl.cpp) 的 `DrawText` 分支中：
- 删除 / 简化当前那段复杂的 `glyph_w/glyph_h/glyph_x/glyph_y` 推导逻辑：
- 不再使用 `entry->metrics.bounds_*` / `bearing_y_units` / `padding_units` 去推矩形；
- 改为：
- 直接读取 `cmd.glyphs[i].rect_x/y/w/h`，填入 `GlyphInstanceUniform.rect`；
- 仍然用 `atlas_range / msdf_scale` 计算出 SDF 的 pxRange（这是 MSDF 解码所需）；
- 根据 atlas 里的 `u0/v0/u1/v1` 填写 `uv_rect`（仅反映 pad+glyph 在纹理中的边界）。
- 这样 GPU 的职责被限定为：
- 选择正确的 atlas page；
- 为每个 glyph 填好 UV 和 `pxRange`；
- 把已经算好的世界矩形发给 shader。
