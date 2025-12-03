# rel-box / abs-badge 调试经验

场景：`doc/align_basic_layout.html` 中的 `rel-box` + `.abs-badge` 角标在 GPU 截图里“看不到 / 对不齐”。

目标：
- 对齐文档里的 1.4 要求：`position: relative/absolute` 正确工作，ABS 角标精确落在 `rel-box` 右上角。
- 把这次排查的通用套路沉淀成“以后查类似问题可重复使用”的步骤。

---

## 一、整体方法论

遇到“元素看不到 / 对不齐”时，不猜原因，按管线从后往前查：

1. GPU 输出是否有东西：
   - 通过 demo 生成 BMP 截图：`gpu_screenshot_demo_basic_layout` → `zig-out/tmp/gpu_screenshot_basic_layout.bmp`。
   - 用 Python 脚本做基础检查：
     - `analyze_bmp.py`：整体非白像素比例、颜色分布；
     - `analyze_layout.py`：每一行非白像素范围、文本块大致位置。

2. 布局是否有盒子：
   - 在 `View::renderToGPUTexture` 里打印 DOM 布局：
     - `getElementsByTagName("div")` → `[Layout] div[i]: at (x,y) size WxH`；
     - `getElementsByClassName("abs-badge")` → `[Layout] abs-badge[...]`；
   - 确认：
     - 这个 class 是否存在；
     - 它的 `display` / `position`；
     - 它的布局矩形是否非零。

3. Painter 是否在画：
   - 在 `Painter::buildDisplayListNode` 中打点：
     - 通过 `node->getAttribute("class")` 区分特定元素（如 `.abs-badge`）；
     - 打印 layout rect + 原始文本：`[Painter] ABS badge layout rect...`, `[Painter] ABS badge text raw='...'`。
   - 看日志确认：
     - 有没有走到此节点；
     - 文本内容是否正确；
     - layout rect 是否和布局阶段一致。

4. 布局引擎是否按照预期模型工作：
   - 在 `LayoutEngine::layoutPositionedElements` 等关键路径对特定节点加日志：
     - intrinsic 宽高：content 宽度、padding、border → box 宽高；
     - 包含块坐标、top/right/bottom/left 偏移；
     - 计算前后 `layout->width/height`、`layout->x/y` 的变化。

5. 样式是否真的生效：
   - DOM 层：
     - `getElementsByClassName("abs-badge")` 判断节点是否存在。
   - StyleEngine 层：
     - 检查 `ComputedStyle` 是否拿到正确的 `display/position/top/right/...`；
     - 如果怀疑样式缺失，在 `applyStyleProperty` / `computeStyles` 中打印某个 class 的关键属性。

6. 如果日志被截断：
   - 不要只依赖 `zig build` / demo 的即时输出，可以重定向再用工具看完整内容：
     - `zig build > /tmp/zig_build.log 2>&1 || true`；
     - `./gpu_screenshot_demo_basic_layout > /tmp/gpu_basic_layout.log 2>&1 || true`；
     - 再用 `sed -n` / `grep` 或直接在 IDE 里打开 log 文件。

---

## 二、这次问题的具体拆解过程（简版）

这次 ABS 角标的问题，其实依次踩中了几层：

1. 起点：肉眼“看不到 ABS 角标” → 先怀疑：
   - 可能没画；
   - 可能画了但几何完全不对（比如宽度 200，被拉成整条条）。

2. DOM / class 检查：
   - 一开始通过 `getElementsByClassName("abs-badge")` 打印，发现数量为 1：
     - 说明 DOM 里确实有这个节点，class 匹配没问题。

3. 布局检查：
   - 在 `View::renderToGPUTexture` 里 dump 了所有 `div` 和 `.abs-badge`：

     ```text
     [Layout] div[13]: at (16.0, 220.0) size 200.0x70.0   ← rel-box
     [Layout] div[16]: at (28.0, 230.0) size 34.0x16.0    ← abs-badge
     [Layout] abs-badge[0]: at (28.0, 230.0) size 34.0x16.0 display=block position=absolute
     ```

   - 结论：
     - `.abs-badge` 有 layout 盒；
     - `position` 确实是 `absolute`；
     - 尺寸已经是 34×16，而不是整个 200 宽卡片的大小。

4. Painter 检查：
   - 增加调试日志：

     ```cpp
     std::string debug_class = node->getAttribute("class");
     if (debug_class.find("abs-badge") != std::string::npos) {
         SDL_Log("[Painter] ABS badge layout rect: x=%.1f y=%.1f w=%.1f h=%.1f has_layout_rect=%d",
                 node_rect.x, node_rect.y, node_rect.width, node_rect.height, has_layout_rect ? 1 : 0);
         SDL_Log("[Painter] ABS badge text raw='%s'", raw_text.c_str());
     }
     ```

   - 日志显示：

     ```text
     [Painter] ABS badge layout rect: x=28.0 y=230.0 w=34.0 h=16.0 has_layout_rect=1
     [Painter] ABS badge text raw='ABS'
     ```

   - 说明：
     - Painter 确实在这个矩形里画了 "ABS"；
     - 看不到不是因为“没画”，而是几何不对。

5. 真正的根因（这次踩到的两个）：

   1）absolute 元素被 Yoga 当成 100% 宽 block 处理

   - `LayoutEngine::mapComputedStylesToYoga` 里对所有 block 元素（包括 `position:absolute`）有：

     ```cpp
     if (!has_explicit_width && style.layout_mode == dom::LayoutMode::Block) {
         YGNodeStyleSetWidthPercent(yoga_node, 100.0f);
     }
     ```

   - 这会把 `.abs-badge` 之类的 absolute 盒子默认为 100% 宽，严重偏离 CSS 语义。
   - 修复：对 `position:absolute` 禁用这个 fallback，仅让普通 block 用。

   2）StyleEngine 没把 top/right 写进 ComputedStyle

   - `applyStyleProperty` 里支持 `position/top/right/bottom/left`：

     ```cpp
     else if (prop == "position") {
         style.position = val;
     }
     else if (prop == "top") {
         style.top = parseLength(val);
     }
     else if (prop == "right") {
         style.right = parseLength(val);
     }
     ...
     ```

   - 但在 `computeStyles` 的 cascade 中，之前只把 `position` 抄给了 `computed_style`：

     ```cpp
     if (!rule.style.position.empty())
         computed.position = rule.style.position;
     ```

   - `top/right/bottom/left` 没有被同步，所以最终 `ComputedStyle.top/right` 是 AUTO，absolute 布局里的 `has_top/has_right` 一直是 false，只能沿用 Yoga 的原始位置 `(28,230)`。
   - 修复：在 cascade 里同步这几个 offset：

     ```cpp
     if (rule.style.top.unit != CSSValue::Unit::AUTO)
         computed.top = rule.style.top;
     if (rule.style.right.unit != CSSValue::Unit::AUTO)
         computed.right = rule.style.right;
     if (rule.style.bottom.unit != CSSValue::Unit::AUTO)
         computed.bottom = rule.style.bottom;
     if (rule.style.left.unit != CSSValue::Unit::AUTO)
         computed.left = rule.style.left;
     ```

---

## 三、这次调试过程里值得复用的“手段”

可以当成一个 checklist，用于以后遇到类似问题：

1. 尽量用 **现有 demo + 截图管线** 重现问题
   - 不要写最小 demo 之前就重写逻辑。
   - 先保证：`zig build` + `gpu_screenshot_demo_xxx` 能稳定产出 BMP 和日志。

2. **从图片出发，用脚本粗定位区域**
   - `analyze_bmp.py`：确认整张图是不是只有背景色，非白像素有多少；
   - `analyze_layout.py`：看哪些 y 行有大量非白像素，大致知道文本/卡片落在哪些行。

3. **在 View 层统一打印布局（强烈推荐）**
   - 在 `View::renderToGPUTexture` 或 `View::update` 里统一 dump：
     - `html/body/div` 的总数和每个 div 的 `(x,y,W,H)`；
     - `getElementsByClassName("xxx")` 的数目和每个的 `display/position`；
   - 这是连接“抽象 CSS”和“具体数值坐标”的非常关键一环。

4. **在 LayoutEngine 针对特定节点打“结构化日志”**
   - 不要只打印一个 w/h，要打印：
     - content 宽、高；
     - padding/border；
     - 是否 auto；
     - 覆盖前后的 layout 值；
   - 例如这次的 `.abs-badge`：

     ```text
     [LayoutEngine] ABS badge intrinsic: content_w=... line_h=... pad_l=... -> box_w=... box_h=... (width_auto=1 height_auto=1 before: w=... h=...)
     [LayoutEngine] ABS badge final size: w=... h=...
     ```

   - 可以快速判断：
     - intrinsic 分支是否命中；
     - 算出来的尺寸是否合理；
     - 是否成功覆盖了 Yoga 的结果。

5. **在 Painter 侧校验“布局 rect + 文本内容”**
   - 通过 class 名找到目标节点，打印：
     - 最终 layout rect；
     - 原始 text / collapsed text；
   - 可快速确认：
     - Painter 是否遍历到了该节点；
     - 是否用的是布局引擎计算的最新 rect。

6. **样式引擎的问题，尽量用“ComputedStyle → LayoutEngine”这条链路查**
   - 避免只看 CSS 文本，要看最后送入 layout 的 `ComputedStyle`：
     - display/layout_mode；
     - position/top/right/bottom/left；
     - width/height 是否 auto；
   - 一旦 layout 里判断 `has_top/has_right` 失败，很大概率是 StyleEngine 没把 value 填进 `ComputedStyle`。

7. **日志截断时，优先重定向到文件再分析**
   - `zig build` / demo 输出太长时，管道很容易只看到尾部：
     - 用 `> /tmp/xxx.log 2>&1 || true` 把完整输出存盘；
     - 再用 `read_file`/IDE 打开或 `grep`/`sed` 查。

8. **Offscreen 路径要确认是否真的跑了“最新布局”**
   - 不要默认 `root->isLayoutDirty()` 会是 true；
   - 对于截图对齐场景，宁可在 offscreen 路径强制跑一次完整布局，也不要复用旧布局，避免“屏幕上改好了，截图还是旧的”这种坑。

---

## 四、对后续调试的启发

1. 把“从 GPU 像素往上追溯到 DOM/CSS”的链路固定下来：
   - GPU 像素 → Painter rect + 文本 → LayoutEngine layout rect → ComputedStyle → CSS/HTML。
   - 每一层都留有可以开关的日志入口（最好通过 env 开关或 class 过滤）。

2. 针对特定 class 或节点 ID，尽量用“**局部高信息日志**”而不是全局海量日志：
   - 例如这次只针对 `.abs-badge` 打了详细日志，日志量可控但信息密度很高。

3. 把这次的日志模式沉淀成可复用的调试宏 / helper：
   - 比如：
     - `DEBUG_LAYOUT_FOR_CLASS("abs-badge")`；
     - `DEBUG_TEXT_FOR_CLASS("inline-chip")`；
   - 避免以后每次手写一大堆重复的 `SDL_Log`。

4. 对 align 系列场景，可以考虑后续加一个自动 diff 工具：
   - 对比 `align_basic_layout.html` 的浏览器截图和 `gpu_screenshot_basic_layout.bmp`；
   - 自动标出“误差最大区域”，优先看这些区域的布局日志。

总之，这次 rel-box/abs-badge 的调试，核心经验不是某一个 if，而是一整套“从 GPU 像素往 DOM/CSS 回溯，再从 CSS 往下验证 layout 和 painter”的路径；以后遇到类似“某个元素对不齐 / 看不到”的问题，可以直接照这个套路走一遍，很快就能把问题收缩到具体的一两个环节上。