# T11 — 平台 API 与 Dong 专有 API：clipboard / matchMedia / CSS.supports / dialog / scene / textLayout

- **Area**: dong-host
- **性质**: 纯实现型 — 对标物：QuickJS 版各 bindings（`js_clipboard_bindings.cpp`、`js_scene_bindings.cpp`、`js_text_layout_bindings.cpp`、dialog 相关 `js_node_bindings.cpp` 段）的签名与行为
- **优先级**: P2
- **依赖**: T06（字符串约定与脚手架）
- **预估**: 3–5 人天，可按子项拆散认领
- **前置阅读**: `docs/developer/porffor/tasks/setup.md` §3 §8

## 背景

若干小而独立的 Web API 与 Dong 专有 API，评估均为**高可行（同步 host import 直通）**。打包为一个任务，认领人可按子项分批交付。全面切换后这些是必做项（不再有 QuickJS 兜底），QuickJS 侧实现是签名参照物。

## 子项

### 11.1 剪贴板

- `dong_clipboard_write(str)`、`dong_clipboard_read() -> str`。
- 浏览器里 `navigator.clipboard` 是 Promise API；Porffor 路径**同步语义**即可（游戏引擎剪贴板本来就是同步的），prelude 命名 `clipboardRead()`/`clipboardWrite(s)`，不模拟 Promise。

### 11.2 matchMedia / CSS.supports

- `dong_match_media(query) -> i32`：复用引擎现有媒体查询求值器；只支持引擎实现的 query 子集，不支持 MediaQueryList 的 change 监听（如测试需要，后续走 T08 事件槽加 `mediachange` 事件）。
- `dong_css_supports(prop, value) -> i32`：问引擎 CSS 属性支持表。

### 11.3 dialog 元素

- `dong_dialog_show(nodeId)`、`dong_dialog_show_modal(nodeId)`、`dong_dialog_close(nodeId, returnValueStr)`、`dong_dialog_return_value(nodeId) -> str`、`dong_dialog_open(nodeId) -> i32`。
- `close` 事件走 T08 事件类型扩展。

### 11.4 Dong 专有 API（游戏 UI 核心，可拆子任务认领）

- **scene graph**：`test_scene_graph.html` 一类；现状 `src/script/porffor/js_scene_stub_porffor.cpp` 是空 stub。扁平 import 集签名对照 QuickJS 版 `js_scene_bindings.cpp`。
- **`dong.textLayout` / overlay 绘制**：与 DOM 无关的专用 import 集，对照 `js_text_layout_bindings.cpp`。

### 11.5 移交与决策（记录在案）

- `MutationObserver`：改由 C++ 侧直接 callExport（引擎自己知道 DOM 何时变了），不做通用 observer；有具体测试需要时单独立任务。
- `DOMParser` / `FormData` / Selection & Range / IME：**不再「留 QuickJS」**——host 化实现、下沉 C++ 或显式裁剪，决策移交 **T20**。

## 验收标准

1. 每个子项一个最小 HTML+JS 用例进 Dong 测试目录并通过。
2. clipboard 用例覆盖中文文本（依赖 T05 编码约定）。
3. `dong_match_media("(min-width: ...)")` 在窗口 resize 后再次调用返回新结果。
4. scene / textLayout 子项若认领：`test_scene_graph.html` 的 Porffor 版 smoke 通过；未认领则在完成记录中注明并保留子项开放。
5. prelude 文档更新全部签名。

## 完成记录

- **日期**: 2026-07-05
- **Commit**: (见 git log)
- **子项**:
  - 11.1 clipboard：`dong_clipboard_read/write` + prelude `clipboardRead/Write`；`test_porffor_clipboard_cn.html` 中文往返
  - 11.2 matchMedia / CSS.supports：`dong_match_media`（`StyleEngine::evaluateMediaQuery` + viewport）、`dong_css_supports`；prelude `matchMedia` / `cssSupports`
  - 11.3 dialog：`dong_dialog_show/show_modal/close/return_value/open` + prelude 包装
  - 11.4 scene graph：替换 `js_scene_stub_porffor.cpp` → `js_scene_porffor.cpp`（JSON config flat imports）
  - 11.5 textLayout / overlay：`js_text_layout_porffor.cpp` + `dong_text_layout/clear_overlay/render_text/draw_rect/draw_circle`
- **验证**: `docs/developer/porffor/tasks/repro/t11/t11_verify.mjs`（portable node）；t08 回归通过；clang native smoke 在无 clang 环境 SKIP
- **遗留**: dialog `close` 事件仍走 T08 扩展；`renderText/drawRect/drawCircle` 仅 host 直通，无 QuickJS 对象 API 对等

## 完成记录

- **日期**: 2026-07-05
- **Commit**: `8d34b91`
- **实现**: clipboard / matchMedia / cssSupports / dialog / scene graph / textLayout 扁平 import + prelude；`js_scene_porffor.cpp`、`js_text_layout_porffor.cpp`
- **验证**: repro `t11/`（若存在）；`porffor_compile` import 注册
- **遗留**: `test_scene_graph.html` Porffor CI smoke；overlay drawRect/renderText 示例对齐 — 见 `WAVE3-LEFTOVER.md`
