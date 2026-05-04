# P0-5 — IME Composition 三件套

> 上游：[`doc/roadmap.md`](../roadmap.md) Phase 0 P0-5
>
> 状态：方案 v1 / 2026-04-17

---

## 1. 目标

让 dong 在所有可输入控件（`<input>` / `<textarea>` / `contenteditable`）下正确支持中文 / 日文 / 韩文 / Emoji IME 输入：

- DOM 事件 `compositionstart` / `compositionupdate` / `compositionend` 完整可监听。
- 输入过程中 IME 候选窗位置正确（贴在 caret 位置，不再固定窗口左上）。
- 候选未确定时显示 preedit underline（带下划线的临时字符串）。
- 与 Slug / MSDF 字体 / 文本选区 / 焦点切换 / Disable 状态完全兼容。

---

## 2. 现状

| 项 | 状态 |
|---|---|
| `compositionstart/update/end` 事件 | ❌（[`gap_analysis.md`](../specific/html_css_dom_gap_analysis.md) § 4.5 P1） |
| SDL `SDL_TextEditingEvent` | 已收到（见 `dong/backends/sdl/sdl_input_adapter.cpp`），但仅简单 forward 为 `dong_engine_send_text_editing` |
| `dong_engine_send_text_editing` | 已存在；但仅 overlay 用，未触发 DOM 事件 |
| Caret rect 上报给系统 IME | ❌（IME 候选窗固定在窗口左上） |
| InputElementState 处理 preedit | ❌ |

代码索引：
- SDL 输入：`dong/backends/sdl/sdl_input_adapter.cpp`
- C ABI：`dong/include/dong.h` 已有 `dong_engine_send_text` 但无 `_send_composition_*`
- Input 状态：`dong/src/dom/input_element.{hpp,cpp}`
- ContentEditable：`dong/src/dom/contenteditable.{hpp,cpp}`
- 焦点：`dong/src/dom/focus_manager.{hpp,cpp}`

---

## 3. 设计

### 3.1 三件套语义（与 W3C 对齐）

| 事件 | 触发时机 | data 字段 |
|---|---|---|
| `compositionstart` | 用户开始输入 IME 候选（按下首键，IME 进入 composition 模式） | `""` |
| `compositionupdate` | 候选字符串变化（每次按键，候选实时变化） | 当前 preedit 字符串 |
| `compositionend` | 用户确认（Space / Enter / 鼠标选 candidate） | 最终确认的字符串 |
| `input` | 紧跟 `compositionend` 一次（commit 字符插入） | InputEvent，`inputType=insertCompositionText` |

期间 `keydown` / `keypress` 事件**不应**触发（IME 拦截了原始按键）—— SDL `SDL_StartTextInput` 后即此行为，dong 直接复用即可。

### 3.2 C ABI 扩展

```c
// dong/include/dong.h（新增）

// 开始 composition：text 通常为 ""；某些平台（macOS / Windows）合并 start+update
void dong_engine_send_composition_start(dong_engine_t* eng);

// composition 字符串更新（preedit）
//   text:                当前 preedit utf8
//   cursor_byte:         caret 在 preedit 中的字节位置
//   selection_start_byte: preedit 中候选高亮起点
//   selection_end_byte:   preedit 中候选高亮终点
void dong_engine_send_composition_update(dong_engine_t* eng,
                                         const char* text,
                                         int32_t cursor_byte,
                                         int32_t selection_start_byte,
                                         int32_t selection_end_byte);

// composition 结束：text 是最终确认字符串（可空 = 取消）
void dong_engine_send_composition_end(dong_engine_t* eng, const char* text);

// caret rect 查询（供宿主 IME 定位候选窗）
//   返回 0=有效；1=无 active editable
typedef struct {
    int32_t x, y;       // 像素坐标，相对窗口左上
    int32_t width;      // caret 宽度（通常 = 1 或 2）
    int32_t height;     // caret 高度（含 line-height）
} dong_caret_rect_t;

int dong_engine_query_caret_rect(dong_engine_t* eng, dong_caret_rect_t* out);
```

### 3.3 SDL backend 改造

```cpp
// sdl_input_adapter.cpp

case SDL_EVENT_TEXT_EDITING: {
    auto& ev = sdl_event.edit;
    if (!in_composition_) {
        dong_engine_send_composition_start(engine);
        in_composition_ = true;
    }
    // SDL 的 cursor / selection 单位已经是字节
    dong_engine_send_composition_update(engine,
        ev.text, ev.start, ev.start, ev.start + ev.length);
    break;
}

case SDL_EVENT_TEXT_INPUT: {
    if (in_composition_) {
        dong_engine_send_composition_end(engine, sdl_event.text.text);
        in_composition_ = false;
    } else {
        dong_engine_send_text(engine, sdl_event.text.text);  // 直接键入，无 IME
    }
    break;
}
```

**Caret rect → 系统 IME**：`dong_app` 主循环每帧调一次 `dong_engine_query_caret_rect`，若有变化则 `SDL_SetTextInputArea(window, rect, cursor)`，让系统 IME 把候选窗贴到 caret 位置。

### 3.4 Core 内部：CompositionContext

```cpp
// dong/src/dom/composition_context.hpp（新增）
struct CompositionContext {
    DOMNode* target = nullptr;          // 当前输入元素
    std::string preedit;                // 当前未提交的 IME 字符串
    int32_t cursor_byte = 0;
    int32_t sel_start_byte = 0;
    int32_t sel_end_byte = 0;
    bool active = false;

    // 渲染相关
    Rect caret_rect;                    // 由 InputElementState 计算
    std::vector<UnderlineSegment> underlines;  // preedit 下划线段（区分高亮 / 普通）
};
```

挂在 `EngineView` / `Context` 上。

### 3.5 处理流程

#### `dong_engine_send_composition_start`

```
1. context.active_element = focus_manager.activeElement
   - 若不是 input/textarea/contenteditable → 忽略，return
2. composition.target = active_element
3. composition.active = true
4. composition.preedit.clear()
5. dispatch DOM event "compositionstart" on target
```

#### `dong_engine_send_composition_update`

```
1. if (!composition.active) → 隐式 start
2. composition.preedit = text
3. composition.cursor_byte/sel_* 更新
4. element.preedit_render = preedit + underline 信息
5. markNeedsRepaint()
6. dispatch "compositionupdate" on target, data = preedit
7. 不修改元素 value（preedit 不进 value）
```

#### `dong_engine_send_composition_end`

```
1. composition.active = false
2. element.preedit_render.clear()
3. dispatch "compositionend" on target, data = final_text
4. if (final_text non-empty):
     element.insertText(final_text)
     dispatch "input" event with inputType="insertCompositionText"
5. markNeedsRepaint()
```

### 3.6 Caret rect 计算

```cpp
// InputElementState / ContentEditable 已有 textLayout
Rect getCaretRect() const {
    auto pos = layout_->positionForByteOffset(cursor_byte_);
    return { pos.x, pos.y - line_ascent_, 2, line_height_ };
}
```

`dong_engine_query_caret_rect` 简单封装：

```cpp
1. focused = focus_manager.activeElement
2. if (focused 是 InputElement / Textarea / contenteditable):
     out_rect = focused.getCaretRect()
     // 转换：local → window pixels（应用滚动 + 父级 transform）
     out_rect = transformLocalToWindow(focused, out_rect)
     return 0
3. return 1
```

### 3.7 Preedit 渲染

文本绘制时：

```
Painter::paintText(element):
  text = element.value
  if (composition.target == element && composition.active):
      // 在 cursor_byte 位置插入 preedit
      text = text.substr(0, cursor) + composition.preedit + text.substr(cursor)
      // 给 preedit 范围加下划线 / 高亮背景
      addPreeditUnderlines(composition.underlines)
```

所有 preedit 字符走与正常字符同一 Slug / MSDF 路径，无新 shader。下划线复用 `text-decoration` 渲染路径。

---

## 4. 实施步骤

| Step | 范围 |
|---|---|
| **S1 — C ABI + SDL 接入** | composition_start/update/end / query_caret_rect 全部上线；headless tool `--composition-script` 可注入 |
| **S2 — Core CompositionContext + DOM 事件** | 三件套事件可监听；JS 侧测试用例 |
| **S3 — InputElement preedit 渲染** | preedit 显示 + 下划线 |
| **S4 — `dong_app` 集成 `SDL_SetTextInputArea`** | 真实 IME 候选窗贴 caret |
| **S5 — Textarea / contenteditable 完整覆盖** | 含多行换行、选区、scroll 偏移 |

---

## 5. 通过验证规则

### 5.1 Hard

| 项 | 阈值 |
|---|---|
| `<input>` `<textarea>` `<contenteditable>` 三类元素，IME 候选窗贴在 caret 位置（误差 ≤ 2px） | 必须 |
| `compositionstart/update/end` 三个事件按序触发 | JS 监听验证 |
| `compositionend` 后必须紧跟 `input` 事件，且 `inputType=insertCompositionText` | 必须 |
| 期间 `keydown`/`keypress` 不触发（按键被 IME 拦截） | 必须 |
| preedit 字符串显示在 caret 位置，确认前不写入 `element.value` | 必须 |
| ESC 取消 composition：`compositionend` 触发但 `data=""`、不插字 | 必须 |
| 三种渲染轨道（DOM 输入、Direct Draw 不涉及 IME；Scene Graph 仅当含 input 时同 DOM）兼容 | DOM 必须；Direct Draw N/A |
| 不破坏既有 `dong_engine_send_text` 路径 | 既有 baseline 像素回归 ≥ 99% |

### 5.2 Soft

| 项 | 阈值 |
|---|---|
| 候选窗位置更新延迟 | < 16 ms |
| preedit 渲染下划线 | 高亮段 vs 非高亮段视觉可区分 |
| Emoji 输入（macOS Ctrl+Cmd+Space）| 走相同流程，能正确插入 |

### 5.3 必须新增的测试

由于 IME 是平台事件，不能用 SDL headless 模拟真实 IME。**用 C ABI 直接驱动**：

#### 单元测试（headless）

```cpp
// dong/tests/test_composition.cpp（新增）
TEST(Composition, BasicFlow) {
    auto eng = create_test_engine();
    load_html(eng, R"(<input id="x">)");
    focus_element(eng, "#x");

    expect_event_log_clear();

    dong_engine_send_composition_start(eng);
    EXPECT_EVENT("compositionstart", target="#x", data="");

    dong_engine_send_composition_update(eng, "你", 3, 0, 3);
    EXPECT_EVENT("compositionupdate", data="你");
    EXPECT_VALUE("#x", "");  // 未确认，value 未变

    dong_engine_send_composition_update(eng, "你好", 6, 0, 6);
    EXPECT_EVENT("compositionupdate", data="你好");

    dong_engine_send_composition_end(eng, "你好");
    EXPECT_EVENT("compositionend", data="你好");
    EXPECT_EVENT("input", inputType="insertCompositionText");
    EXPECT_VALUE("#x", "你好");
}

TEST(Composition, EscCancels) {
    // ... 同上但 end 传 "" ...
    EXPECT_VALUE("#x", "");  // value 不变
}

TEST(Composition, CaretRectFollowsInput) {
    // 不同 input 位置，query_caret_rect 返回不同 y
}

TEST(Composition, MultilineTextarea) {
    // textarea 第三行输入，caret_rect.y 在第三行
}
```

#### Headless 多帧渲染测试

```bash
# html_render_test 扩展支持 composition script
html_render_test data/tests/test_input_chinese.html out.bmp 800 600 \
  --frames 5 \
  --frame1 "focus #x; composition_start; composition_update '你' 3 0 3"  \
  --frame2 "composition_update '你好' 6 0 6" \
  --frame3 "composition_end '你好'" \
  --frame4 "render"
```

期望：frame1-3 显示 preedit + 下划线；frame4 显示已 commit 的 "你好" 文字。

#### 集成测试（手动）

| 平台 | 验证 |
|---|---|
| Windows MS-IME（中文）| `<input>` 输入"你好世界"，候选窗贴 caret |
| macOS 拼音 | 同上 + Ctrl+Cmd+Space Emoji |
| Linux IBus / Fcitx | 同上 |
| 日文 IME（macOS / Windows）| 同上 |

每个 release 必跑一遍，结果记录到 `doc/qa_log.md`（新建）。

---

## 6. 风险与回退

| 风险 | 缓解 |
|---|---|
| 平台 IME 行为差异（macOS 一次性传 commit；Windows 分段传 update + commit）| C ABI 设计已支持两种 → start 是 idempotent；update 可被 commit 直接替代 |
| `SDL_SetTextInputArea` 在 SDL3 各平台覆盖度不一致 | 文档列出已知限制；不可用时 fallback 候选窗位置不准但不崩 |
| ContentEditable 情况下 caret 计算复杂（多 inline + transform）| v1 仅保证简单单 block；嵌套 inline / transform 留 issue |
| Direct Draw 模式不涉及 IME | 文档明示；Direct Draw 无 input 元素，不存在 |
| Scene Graph 模式禁用 IME | 文档明示（Scene Graph 当前不含 input；以后若加再扩） |
| 性能：caret query 每帧调 | 缓存上一次结果，仅 dirty 时重算 |

---

## 7. 不在本方案范围

- ❌ Composition 期间允许鼠标点击其他元素（行为：cancel 当前 composition + 切焦点）
- ❌ 多语言输入栈管理（应用自己装 IME 后系统已切，dong 不参与）
- ❌ 软键盘（移动端 `inputmode` 提示，留 Phase 1）
- ❌ 自绘 IME 候选窗（极少需求；除非主机平台无系统 IME 才考虑）
- ❌ Bidi / RTL 文本下的 IME（基础 LTR 先做扎实；RTL 与 IME 同时是大坑）

---

## 8. 完成后更新

- [ ] `doc/specific/html_css_dom_gap_analysis.md` § 4.5 三件套事件标 ✅
- [ ] `doc/specific/html_css_dom_草案.md` § 4.4 `CompositionEvent` 标 ✅
- [ ] `doc/重要特性.md` 新增 § "IME 输入"
- [ ] `doc/qa_log.md`（新建）记录每次 release 的多平台 IME 手动测试结果
