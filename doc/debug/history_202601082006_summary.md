## 对话总结（history_202601082006）

> 目的：把本次“GPU/MSDF 字体渲染 + 截图验证 + 中文字体 + JS 脚本加载”的排障过程沉淀成**可复用的检查清单**与**关键结论**，供后续类似问题快速定位。

---

## 背景与症状

- **项目**：`dong`（SDL3 + `SDL_GPU`），文字采用 **MSDF**（`msdfgen` + `FreeType`），排版/字形由 **HarfBuzz** shaping。
- **常见症状**：
  - Offscreen 截图（BMP）里文字消失/全白。
  - 文字渲染出现但“全部挤在一坨”（advance/offset 失效）。
  - 中文字符不显示（只剩英文）。
  - MSDF 边缘锯齿明显。
  - JS 侧 `console` 未定义，或外部脚本加载失败。

---

## 关键结论（值得记住）

- **SDL_GPU HLSL binding 规则（非常关键）**
  - Vertex shader 的 uniform buffer：`(b[n], space1)`
  - Fragment shader 的 uniform buffer：`(b[n], space3)`
  - 纹理/采样器 space 也有区分（vertex 常用 `space0`，fragment 常用 `space2`）。
  - 现象：space 写错时，最直接表现是“shader 看起来运行了，但读到的 uniform 全是 0/垃圾，最终什么都画不出来”。

- **Uniform buffer 4KB 上限（本项目/该管线中被证实）**
  - 一次 text batch 的 `TextBatchUniformData` 总大小必须 **< 4096 bytes**，否则后续字段（例如 `uViewport`）读取失败，顶点 NDC 计算错导致被剔除。
  - 实用策略：把 `viewport/clip` 放前面，glyph 数组放后面；必要时把最大 glyph 数从 64 减到 62（对齐后保持 <4KB）。

- **HarfBuzz 度量单位不要重复缩放**
  - 当 `hb_font_set_scale` 已设置到 `units_per_em`（design units）时：
    - `x_advance/y_advance` **已经是 design units**，不应再除以 64。
    - 只有 `x_offset/y_offset` 可能仍是 26.6 fixed（需要按实际返回值确认）。
  - 现象：advance 被错误缩小会导致“所有字堆在一起”。

- **MSDF 编码与坐标系只翻转一次**
  - MSDF 纹理写入要用标准编码：`distance / range + 0.5`（再 clamp 到 [0,1]）。
  - msdfgen 的 Y 轴（数学坐标）与 GPU 纹理/屏幕坐标（Y 向下）不一致：
    - 如果在生成 MSDF 时做了 Y 翻转，则 UV 传递时不要再额外翻转，否则会出现二次翻转。

- **中文不显示通常不是“引擎不支持中文”，而是“字体不包含字形”**
  - 仅按 `font-family` 找到一个字体文件并不足够；必须做**字符级字体回退**。
  - 可靠判断：用 FreeType `FT_Get_Char_Index()`（glyph_id=0 即不支持）。
  - 工程化策略：每个 glyph 携带自己的 `font_path` + `units_per_em`，渲染阶段按 glyph 实际字体生成/查找 MSDF。

---

## 本次修复/优化点汇总（按主题）

### 1) “看不到字/只有灰色矩形”——渲染管线与 shader 绑定

- **修复点**：`gpu_driver_sdl.cpp` 中 vertex shader uniform `space` 修正为 `space1`。
- **验证方法**：
  - 先把 fragment shader 改成输出固定颜色（例如纯红）验证 drawcall/管线通不通。
  - 再用 BMP 像素统计（红色像素计数）判断是否真的输出。

### 2) “读不到 glyph 数据/布局错乱”——cbuffer layout 与大小限制

- **修复点**：
  - 解决 HLSL cbuffer 与 C++ struct 对齐/偏移不一致问题。
  - 控制 `TextBatchUniformData` 总大小 < 4096 bytes（必要时减少每 batch glyph 数）。
- **验证方法**：
  - 用 debug shader 把 `uViewport/uGlyphData` 某些字段映射到颜色输出，逐步确认哪个 offset 开始读错。

### 3) “纯白截图但日志说有红色像素”——截图/写文件链路要单独验证

- **要点**：渲染成功不等于“BMP 写对了”。建议把验证拆成两段：
  - **内存像素**：运行时统计 `pixels` 缓冲中的颜色分布。
  - **文件像素**：从 BMP 文件读取像素字节再做一次统计。
- **BMP 重要常识**：
  - BMP 默认 bottom-to-top（行倒序）。
  - 每行需要 4 字节对齐 padding（24-bit BMP 特别要注意）。

### 4) “中文不渲染”——字体回退机制

- **修复点（工程改动）**：
  - `display_list.hpp`：`GlyphInstance` 增加 glyph 级的 `font_path`、`units_per_em`。
  - `font_resolver.hpp/cpp`：加入 CJK fallback 列表，并提供“按码点找字体”。
  - `text_shaper.hpp/cpp`：逐字符检查主字体是否支持，不支持则从 CJK fallback 里找；shape 输出携带实际字体信息。
  - `painter.cpp` / `gpu_driver_sdl.cpp`：渲染阶段按 glyph 实际字体加载/生成 MSDF，并据 `units_per_em` 计算缩放。
- **经验**：Windows 上优先尝试 `Microsoft YaHei (msyh.ttc)`、`SimSun`、`SimHei` 等。

### 5) “MSDF 锯齿明显”——AA 参数与采样策略

- **优化点**：
  - 用 `fwidth()` 动态估算 screen-space 的变化率（动态 `screenPxRange`）。
  - 设定 `screenPxRange` 最小值（例如 >= 2.0，msdfgen 常见建议值）。
  - 在边缘区间做 4x 超采样并平均，减少 stair-step。
  - 用更平滑的 `smoothstep` 过渡。

### 6) “JS console 未定义 / 外部 script 加载失败”——初始化时序与路径

- **修复点**：
  - 执行 `<script>` 前确保 JS bindings 已初始化（否则 `console` 为空）。
  - 外部脚本路径建议以 `SDL_GetBasePath()` 或 HTML 所在目录为基准做解析，避免多 view 实例工作目录不一致导致随机失败。

---

## 可复用的排障套路（建议以后照这个顺序走）

- **1. 先判定“到底有没有画出来”**
  - Fragment shader 输出常量色 → 看截图/像素统计是否出现该颜色。

- **2. 再判定“数据有没有喂对”**
  - 逐字段验证 uniform：先 `viewport`，再 `clip`，最后 `glyph data`。
  - 一旦某个 offset 开始读错，优先怀疑：cbuffer packing、对齐、大小上限、space/slot 绑定。

- **3. 处理“文字挤成一团/间距不对”**
  - 先把问题限定到 shaping：打印 `x_advance/x_offset` 的原始值与单位。
  - 检查 `hb_font_set_scale` 的设置是否与缩放代码一致，避免重复 /64。

- **4. 处理“中文缺字”**
  - 不要只看 `font-family`；对每个码点用 `FT_Get_Char_Index()` 做真实支持性检查。
  - 缺字就 fallback：最终让“每个 glyph 都知道自己来自哪个字体”。

- **5. 处理“截图看起来不对”**
  - 内存像素统计与文件像素统计必须一致；否则先修 BMP 写入/路径/工作目录问题。

---

## 相关文件（便于回溯）

- **渲染/着色器**：`dong/src/render/gpu_driver_sdl.cpp`
- **MSDF atlas**：`dong/src/render/glyph_atlas.cpp`
- **Shaping**：`dong/src/render/text_shaper.cpp`
- **字体解析/查找**：`dong/src/render/font_resolver.cpp`、`dong/src/render/font_finder.cpp`
- **绘制/显示列表**：`dong/src/render/painter.cpp`、`dong/src/render/display_list.hpp`
- **Demo**：`dong/examples/gpu_screenshot_demo*.cpp`
- **JS/HTML 执行**：`dong/src/core/view.cpp`
