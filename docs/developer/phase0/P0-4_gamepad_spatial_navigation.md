# P0-4 — Gamepad Spatial Navigation

> 上游：[`docs/roadmap.md`](../roadmap.md) Phase 0 P0-4
> 性能门槛：[`docs/developer/perf-budget.md`](../perf_budget.md) § 3.1
>
> 状态：方案 v1 / 2026-04-17

---

## 1. 目标

让 dong 内的 UI 能用游戏手柄完整操作：

- 方向键 / 左摇杆 → 焦点在可聚焦元素之间按"空间最近"跳转。
- A 键 → activate（等价 click + Enter）。
- B 键 → cancel / back（关闭 dialog / 返回）。
- 默认行为可被 CSS / JS 显式覆盖（导航图、自定义路由）。
- 与既有 `:focus` `:focus-visible` 完全兼容。

这是游戏 UI 与 web UI 最大的交互差异，必须做。

---

## 2. 现状

| 项 | 状态 |
|---|---|
| `:focus` / `:focus-visible` / `:focus-within` | ✅ |
| `tabindex` 属性 | ⚠️ 可存储，但仅 Tab 顺序使用 |
| `focus()` / `blur()` | ✅ |
| 手柄事件 | ❌（SDL gamepad 已有，未接入 dong） |
| Spatial nav 算法 | ❌ |
| `nav-up/down/left/right` CSS 属性 | ❌ |

代码索引：
- 焦点：`dong/src/dom/focus_manager.{hpp,cpp}`
- SDL 输入：`dong/backends/sdl/sdl_input_adapter.cpp`
- 输入抽象：`dong/src/input/input_adapter.hpp`

---

## 3. 设计

### 3.1 三层 API

```
应用 / 游戏代码
    ↓
JS API（dong.focusNav 命名空间） + HTML 标记 + CSS（nav-* / tabindex）
    ↓
C ABI（dong_engine_send_gamepad_*, dong_engine_focus_nav）
    ↓
Core 算法（SpatialNav engine）
```

### 3.2 C ABI 扩展（`include/dong.h`）

```c
typedef enum {
    DONG_GAMEPAD_DPAD_UP, DONG_GAMEPAD_DPAD_DOWN,
    DONG_GAMEPAD_DPAD_LEFT, DONG_GAMEPAD_DPAD_RIGHT,
    DONG_GAMEPAD_BUTTON_A,            // confirm
    DONG_GAMEPAD_BUTTON_B,            // cancel
    DONG_GAMEPAD_BUTTON_X,            // alt action（应用决定）
    DONG_GAMEPAD_BUTTON_Y,            // alt action
    DONG_GAMEPAD_LB, DONG_GAMEPAD_RB, // tab prev / next
    DONG_GAMEPAD_LT, DONG_GAMEPAD_RT,
    DONG_GAMEPAD_BUTTON_START,        // menu
    DONG_GAMEPAD_BUTTON_BACK,         // alt back
    // ... 标准 SDL gamepad 全集
} dong_gamepad_button_t;

void dong_engine_send_gamepad_button(dong_engine_t* eng, int gamepad_id,
                                     dong_gamepad_button_t btn, int pressed);

void dong_engine_send_gamepad_axis(dong_engine_t* eng, int gamepad_id,
                                   int axis, float value); // -1..1

// 编程触发：dong.focusNav('up' | 'down' | 'left' | 'right' | 'next' | 'prev')
typedef enum {
    DONG_NAV_UP, DONG_NAV_DOWN, DONG_NAV_LEFT, DONG_NAV_RIGHT,
    DONG_NAV_NEXT, DONG_NAV_PREV,
} dong_nav_dir_t;

int dong_engine_focus_nav(dong_engine_t* eng, dong_nav_dir_t dir);
```

### 3.3 JS API

```js
// 默认行为：dong 内置 SDL adapter 自动把 gamepad 输入转 nav；
// 应用想自定义只用拦截 gamepadbutton 事件即可

dong.focusNav('up');                    // 编程触发空间跳转
dong.focusNav('next');                  // tab 顺序

window.addEventListener('gamepadbutton', (e) => {
    // e.button === 'A' | 'B' | 'DpadUp' | ...
    // e.pressed === true | false
    // e.gamepadId === 0
    // 默认行为可 e.preventDefault() 拦截
});

document.activeElement;                 // ✅ 已支持
element.focus({ preventScroll: true }); // 兼容 web 标准
```

### 3.4 HTML / CSS 控制

```html
<!-- 默认 spatial nav 算法适用 -->
<button>Item 1</button>
<button>Item 2</button>

<!-- 显式覆盖：CSS Basic UI L4 草案 -->
<style>
  .item-a { nav-right: var(--item-b); }
  .item-b { nav-left: var(--item-a); nav-down: #confirm; }
  #confirm { nav-up: var(--item-b); }
</style>

<!-- 退出导航 -->
<div tabindex="-1">                <!-- 不可空间到达 -->
<div tabindex="0">                 <!-- 可，按 DOM 顺序 -->
<div data-nav-trap="true">         <!-- 自定义：nav 不出此容器 -->
<div data-nav-default="#submit">   <!-- 进入容器时默认聚焦此子元素 -->
```

CSS 属性（v1）：

| 属性 | 值 | 行为 |
|---|---|---|
| `nav-up` `nav-down` `nav-left` `nav-right` | `auto` \| `<id>` \| `none` | `auto` = 用算法；`#id` = 跳到该元素；`none` = 不允许此方向 |
| `--*` 引用 | 用 CSS 变量传 selector 也支持 | |

非标准但极有用的 dong 私有属性：

| 属性 | 值 | 行为 |
|---|---|---|
| `data-nav-trap` | `true` \| `modal` | 焦点不出容器（modal 含强制阻断） |
| `data-nav-default` | `<selector>` | 容器获得焦点时默认聚焦内部哪个元素 |
| `data-nav-priority` | 数值 | 同距离时按 priority 选 |

### 3.5 算法：Spatial Navigation

参考 [W3C CSS Spatial Navigation Level 1](https://drafts.csswg.org/css-nav-1/) 草案，简化实现。

**输入**：当前 focused element + dir。

**输出**：下一个 focused element（或 `null` = 无变化）。

**核心步骤**：

```
1. 收集候选集 C = 所有可聚焦元素（tabindex >= 0 且非 disabled / hidden / inert）
   - 排除当前 focused
   - 应用 tabindex=-1 / data-nav-trap / nav-* 的过滤

2. 显式覆盖优先：
   if (current 有 nav-<dir> 显式 selector) → 直接返回该元素

3. 几何过滤：只保留位于 dir 半平面内的元素
   - dir=right: candidate.center.x > current.right
   - dir=down:  candidate.center.y > current.bottom
   - 其他类似

4. 评分：score = α * orthogonal_displacement + β * directional_distance
   - orthogonal_displacement：与 dir 正交方向的偏移
   - directional_distance：沿 dir 方向的距离
   - 经验权重 α=2, β=1（侧向偏移惩罚更大，符合人眼期望）

5. 取最低 score 元素返回

6. 若 C 空（无候选）：
   - 若处于 nav-trap 容器内 → 返回 null
   - 否则可选 wrap-around（CSS Basic UI L4 的 `nav-loop`，v1 不做）
```

复杂度：O(N)，N = 可聚焦元素数。游戏 UI 通常 N < 100，单次跳转 < 0.1 ms。

### 3.6 与 Scene Graph 模式的协作

Scene Graph 模式不走 DOM 路径：

- `SceneNode` 增加 `focusable: bool` + `nav_id: string`。
- SpatialNav 算法对 SceneGraph 走相同几何评分，但候选集来自 `SceneGraph::collectFocusable()`。
- 复用同一套 SpatialNav engine，分支只在"候选集来源"。

### 3.7 摇杆 → 方向键虚拟事件

```cpp
// SDL adapter 内部
void sdl_input_adapter_pump_axis(float lx, float ly) {
    constexpr float DEAD_ZONE = 0.5f;
    constexpr int REPEAT_FIRST_MS = 350, REPEAT_INTERVAL_MS = 120;

    auto fire = [&](dong_nav_dir_t d) {
        // 内部维护 last_fire_time + 是否 first
        if (now - last >= (first ? FIRST : INTERVAL)) {
            dong_engine_focus_nav(eng, d);
            first = false; last = now;
        }
    };
    if (lx > DEAD_ZONE)  fire(DONG_NAV_RIGHT);
    else if (lx < -DEAD_ZONE) fire(DONG_NAV_LEFT);
    else { reset_h_state(); }
    if (ly > DEAD_ZONE)  fire(DONG_NAV_DOWN);
    else if (ly < -DEAD_ZONE) fire(DONG_NAV_UP);
    else { reset_v_state(); }
}
```

### 3.8 默认按键映射

| 输入 | 默认行为 | 可被 JS 拦截 |
|---|---|---|
| DpadUp/Down/Left/Right | `dong.focusNav('up'/'down'/'left'/'right')` | ✅ |
| 左摇杆 | 同上（含死区 + 重复速率） | ✅ |
| Button A | 等价 `currentTarget.click()` + `keydown Enter` | ✅ |
| Button B | dispatch `gamepadcancel` 事件；默认关闭最上层 dialog | ✅ |
| LB / RB | `focusNav('prev')` / `focusNav('next')`（tab 顺序） | ✅ |
| Start | dispatch `gamepadmenu` 事件 | ✅ |

可通过 `dong_engine_set_gamepad_default_mapping(eng, mapping)` 改默认。

---

## 4. 实施步骤

| Step | 范围 |
|---|---|
| **S1 — C ABI + SDL adapter 接入** | gamepad event 流通；JS 收到 `gamepadbutton` 事件；不做导航算法 |
| **S2 — SpatialNav 算法 + DOM 路径** | `dong_engine_focus_nav` + 默认 dpad 触发；`game_ui2` 跑通 |
| **S3 — CSS `nav-*` + `data-nav-*` 解析** | 显式覆盖优先生效；测试用例 |
| **S4 — Scene Graph 模式接入** | SceneGraph 候选集 + 同算法 |
| **S5 — 摇杆 + 重复速率 + IME 兼容** | 输入聚焦 input 时 dpad 不抢焦点（除非显式 nav-trap=modal） |

---

## 5. 通过验证规则

### 5.1 Hard

| 项 | 阈值 |
|---|---|
| 在 `game_ui2.html` 内用 dpad 完成 "开始按钮 → 选择项 → 确认 → 返回" 完整路径 | 必须成功，无死循环 |
| 单次 `dong_engine_focus_nav` 调用耗时 | < 0.5 ms（N=200 时） |
| 焦点跳转不触发整树 layout / paint | 只 invalidate `:focus` 涉及的元素（与 P0-6 协作） |
| `tabindex=-1` 元素不能空间到达 | 必须 |
| `<input>` 聚焦时 dpad 不抢焦点（除非 modal trap） | 必须 |
| Scene Graph 模式下 spatial nav 同样可用 | 必须 |

### 5.2 Soft

| 项 | 阈值 |
|---|---|
| 摇杆首次响应延迟 | < 50 ms |
| 重复速率（持续按住） | 8–10 跳/秒（INTERVAL ≈ 120ms） |
| `nav-up/down/left/right` 显式覆盖响应 | 100% 命中 |

### 5.3 必须新增的测试用例

| 文件 | 验证 |
|---|---|
| `test_spatial_nav_grid.html` | 5×5 button 网格，dpad 全方向 |
| `test_spatial_nav_explicit.html` | nav-* CSS 显式 |
| `test_spatial_nav_trap.html` | data-nav-trap 容器 |
| `test_spatial_nav_modal.html` | modal trap + B 键关闭 |
| `test_spatial_nav_input.html` | input 聚焦时 dpad 不抢 |
| `test_spatial_nav_scene_graph.html` | Scene Graph 模式 |
| `test_gamepad_a_b_buttons.html` | A/B 默认行为 + JS 拦截 |

### 5.4 自动化验证

由于手柄事件不能像鼠标一样在 headless 中合成，**必须新增编程驱动**：

```cpp
// html_render_test.cpp 扩展
--gamepad-script "dpad_down*3; button_a; dpad_up; button_b; render_frame"
```

每条命令 → 调 `dong_engine_send_gamepad_*` → 跑 tick → 输出当前 `document.activeElement` 的 selector 到 log。

测试脚本验证 log 里 activeElement 序列符合预期：

```python
# scripts/tools/run_gamepad_test.py
expected = ["#start", "#item-1", "#item-2", "#item-3", "#confirm"]
assert active_sequence == expected
```

集成到 CI。

---

## 6. 风险与回退

| 风险 | 缓解 |
|---|---|
| 复杂布局（嵌套 + 不规则）spatial 算法选错元素 | nav-* 显式覆盖 + 测试用例覆盖；提供 dev mode visualize（按 F12 显示候选+评分，与 P1 DevTools 协同） |
| 摇杆死区不合适导致漂移 | DEAD_ZONE 可通过 env var / API 配置 |
| 与浏览器/IME 焦点冲突 | input 聚焦时 dpad 默认不抢；用户可 modal trap 强制 |
| Scene Graph 模式下 hit-test 与 focus 解耦，行为不一致 | 共用 SpatialNav engine；测试用例两轨都要覆盖 |
| 多手柄场景无法区分 | gamepad_id 全 ABI 透传；v1 仅响应 0 号手柄，多手柄留 Phase 1+ |

---

## 7. 不在本方案范围

- ❌ 触屏 spatial nav（Switch 触屏极少；按需做）
- ❌ 鼠标右键菜单 → spatial nav 集成
- ❌ Wraparound (`nav-loop`)（暂无强需求）
- ❌ 多手柄独立焦点（多人本地游戏，留 Phase 2）
- ❌ Haptic feedback（手柄震动；与 nav 解耦，可走业务自建 API）

---

## 8. 完成后更新

- [ ] `docs/reference/css-subset.md` 新增 `nav-*` `data-nav-*` 表格
- [ ] `docs/reference/features-index.md` 新增 § "Gamepad Spatial Navigation"
- [ ] `docs/overview/positioning.md` § 4 三轨表格补充"两轨都支持 spatial nav"
- [ ] `dong/include/dong.h` 文档化 gamepad ABI
