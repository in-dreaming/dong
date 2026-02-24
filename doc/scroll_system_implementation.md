# Scroll System Feature Implementation

## Overview

实现了 P1 滚动体系的两个关键 CSS 属性：
- `overscroll-behavior`: 防止滚动链传播
- `scroll-behavior`: 平滑滚动支持

## 1. overscroll-behavior 属性

### 功能描述

`overscroll-behavior` 属性控制当滚动容器达到滚动边界时，滚动事件是否传播到父容器（滚动链）。

### 支持的值

- `auto` (默认): 允许滚动链，滚动可以传播到父容器
- `contain`: 阻止滚动链，滚动在当前容器内停止
- `none`: 同 `contain`，阻止滚动链

### 轴向属性

- `overscroll-behavior-x`: 仅控制水平滚动链
- `overscroll-behavior-y`: 仅控制垂直滚动链
- `overscroll-behavior`: 简写属性，同时设置 x 和 y

### 实现细节

#### 文件修改

1. **computed_style.hpp**: 添加属性字段
   ```cpp
   std::string overscroll_behavior = "auto";
   std::string overscroll_behavior_x = "auto";
   std::string overscroll_behavior_y = "auto";
   ```

2. **css_parser.cpp**: 添加属性解析器
   ```cpp
   {"overscroll-behavior", [](const std::string& val, ComputedStyle& style) {
       style.overscroll_behavior = val;
       style.overscroll_behavior_x = val;
       style.overscroll_behavior_y = val;
   }},
   {"overscroll-behavior-x", ...},
   {"overscroll-behavior-y", ...},
   ```

3. **style_engine.cpp**: 添加属性应用逻辑
   - 在 `applyRuleOverflowProperties()` 中处理继承

4. **dom_node.hpp/cpp**: 修改 `scrollBy()` 返回消耗状态
   ```cpp
   struct ScrollResult {
       bool consumed_x = false;
       bool consumed_y = false;
   };
   ScrollResult scrollBy(float dx, float dy);
   ```

5. **engine_view.cpp**: `sendMouseWheel()` 中实现滚动链控制
   - 检查滚动是否被消耗
   - 根据 `overscroll-behavior` 决定是否传播到父容器
   - 支持分轴控制（x/y 独立）

### 测试用例

- `test_overscroll_behavior_contain.html`: 测试 `contain` 值，嵌套滚动容器
- `test_overscroll_behavior_none.html`: 测试 `none` 值，单个容器
- `test_overscroll_behavior_axis.html`: 测试轴向属性 (`-x` 和 `-y`)

### 使用示例

```html
<style>
  .outer {
    overflow: auto;
    height: 300px;
  }
  .inner {
    overflow: auto;
    height: 150px;
    /* 防止滚动传播到 .outer */
    overscroll-behavior: contain;
  }
</style>

<div class="outer">
  <div class="inner">
    <!-- 在此滚动到底部不会影响 outer -->
  </div>
</div>
```

## 2. scroll-behavior 属性

### 功能描述

`scroll-behavior` 属性控制通过编程方式（如 `scrollTo()`）触发的滚动是即时的还是平滑动画的。

### 支持的值

- `auto` (默认): 即时滚动，无动画
- `smooth`: 平滑滚动动画（TODO: 动画未实现，当前回退到即时滚动）

### 实现细节

#### 文件修改

1. **computed_style.hpp**: 添加属性字段
   ```cpp
   std::string scroll_behavior = "auto";
   ```

2. **css_parser.cpp**: 添加属性解析器
   ```cpp
   {"scroll-behavior", [](const std::string& val, ComputedStyle& style) {
       style.scroll_behavior = val;
   }},
   ```

3. **style_engine.cpp**: 添加属性应用逻辑

4. **dom_node.cpp**: `scrollTo()` 中检查属性并预留动画钩子
   ```cpp
   void DOMNode::scrollTo(float x, float y) {
       // ...
       const std::string& behavior = computed_style_.scroll_behavior;

       if (behavior == "smooth") {
           // TODO: 实现平滑滚动动画
           // 未来实现应该：
           // 1. 存储目标位置和当前时间
           // 2. 使用缓动函数（如 ease-in-out）
           // 3. 在 tick() 中增量更新位置
           // 4. 典型持续时间：300-500ms
       }

       // 即时滚动（也作为 smooth 的回退）
       scroll_x_ = target_x;
       scroll_y_ = target_y;
   }
   ```

### 测试用例

- `test_scroll_behavior_smooth.html`: 对比 `auto` 和 `smooth` 两个容器
  - 包含 JavaScript 按钮触发 `scrollTop` 赋值
  - 当前两者行为相同（都是即时滚动）
  - 未来实现动画后会有区别

### 使用示例

```html
<style>
  .container {
    overflow: auto;
    height: 300px;
    scroll-behavior: smooth;
  }
</style>

<div class="container" id="myContainer">
  <!-- 内容 -->
</div>

<script>
  // 触发平滑滚动
  document.getElementById('myContainer').scrollTop = 500;
</script>
```

## 实现状态

### 已完成

✅ `overscroll-behavior` 属性完整实现
  - CSS 解析和计算
  - 滚动链控制逻辑
  - 分轴支持（x/y）
  - 测试用例

✅ `scroll-behavior` 属性基础实现
  - CSS 解析和计算
  - 属性检查和回退逻辑
  - 测试用例

### 未来工作

🔲 `scroll-behavior: smooth` 动画实现
  - 需要在 `EngineView` 或 `DOMNode` 中添加动画状态机
  - 实现缓动函数（ease-in-out）
  - 在每帧更新中增量更新滚动位置
  - 参考实现时长：300-500ms

## 架构设计

### 滚动链控制架构

```
用户滚轮事件
    ↓
sendMouseWheel(delta_x, delta_y)
    ↓
findScrollContainerAt(x, y) → scroll_container
    ↓
scroll_container->scrollBy(dx, dy) → ScrollResult {consumed_x, consumed_y}
    ↓
检查 overscroll-behavior-{x,y}
    ↓
如果 !consumed && behavior == "auto"
    ↓
向上查找父滚动容器并传播
```

### 关键决策

1. **ScrollResult 结构**: 返回滚动消耗状态而不是布尔值，支持分轴控制
2. **属性继承**: 在 `applyRuleOverflowProperties()` 中处理，与 overflow 属性一起
3. **滚动链传播**: 只传播未被消耗的轴向
4. **smooth 回退**: 当前回退到即时滚动，保证功能可用

## 测试验证

### 手动测试

```bash
# 渲染测试页面
./zig-out/bin/html_render_test \
  /path/to/test_overscroll_behavior_contain.html \
  /tmp/output.bmp 800 600

# 交互式测试（需要手动滚动）
./zig-out/bin/minimal_dong_demo \
  examples/data/tests/test_overscroll_behavior_contain.html
```

### 自动化测试

```bash
# 基线对比（使用 Playwright 渲染参考）
python scripts/tools/run_baseline_compare.py \
  test_overscroll_behavior_contain.html
```

## 规范参考

- [CSS Overscroll Behavior Module Level 1](https://drafts.csswg.org/css-overscroll-behavior/)
- [CSSOM View Module - scroll-behavior](https://drafts.csswg.org/cssom-view/#propdef-scroll-behavior)
- MDN: [overscroll-behavior](https://developer.mozilla.org/en-US/docs/Web/CSS/overscroll-behavior)
- MDN: [scroll-behavior](https://developer.mozilla.org/en-US/docs/Web/CSS/scroll-behavior)
