# T06 — DOM 属性 / 查询 / classList / style / 几何 扁平 import 集

- **Area**: dong-host（`dong/src/script/porffor/` + prelude）
- **性质**: 纯实现型 — 对标物：QuickJS 绑定层 `js_node_bindings.cpp` 的签名与行为 + T16《字符串通道协议》。不发明新语义，属性行为有疑义以 QuickJS 版为准
- **优先级**: P0（P1 级 Web API 的主体，解锁大部分表单/简单页面测试）
- **依赖**: **T16**（host↔wasm 字符串通道——所有 `-> str` API 的前置，见 setup §3 事实 F3/F4）；T01 合入后可直接多参 import，否则沿用 stage/commit
- **预估**: 4–6 人天
- **前置阅读**: `docs/developer/porffor/tasks/setup.md` §3 §8

## 背景

Porffor 无完整 JS 对象模型，Dong 采取「nodeId + 扁平函数」模型：元素是整数句柄，一切属性读写都是 host import。QuickJS 路径的 `element.value`、`element.style.color`、`classList.add`、几何测量等要在 Porffor 路径重建为扁平 API。评估结论：**高可行，纯 Dong 侧，不改 Porffor**。QuickJS 侧签名对照 `dong/src/script/js_node_bindings.cpp`（T21 退役前是最好的参照物）。

## API 清单（C++ import + prelude 包装）

命名 `dong_` + snake_case；`nodeId: i32`；字符串传递一律遵循 **T16 定稿的通道协议与 T05 的编码约定**（未定稿前沿用现有 stage 槽机制，且不要基于现有 `makeByteString` 铺开新 API——它有已知内存 bug，见 setup §3 F3）。

### 6.1 属性读写

| import | 说明 |
|--------|------|
| `dong_get_value(nodeId) -> str` / `dong_set_value(nodeId, str)` | input/textarea/select 的 `.value` |
| `dong_get_checked(nodeId) -> i32` / `dong_set_checked(nodeId, i32)` | `.checked` |
| `dong_get_disabled` / `dong_set_disabled` | `.disabled` |
| `dong_get_attribute(nodeId, name) -> str` / `dong_set_attribute(nodeId, name, value)` / `dong_remove_attribute` | 通用属性兜底 |
| `dong_set_inner_html(nodeId, html)` | `.innerHTML` 写（读暂缓，序列化成本高） |

### 6.2 查询

| import | 说明 |
|--------|------|
| `dong_query_selector(rootNodeId, selector) -> nodeId(0=null)` | 复用引擎现有 CSS 选择器匹配 |
| `dong_query_selector_all(rootNodeId, selector) -> str` | 返回 JSON 数组字符串 `"[3,7,12]"`（约定 §8：复杂结构走 JSON） |
| `dong_get_elements_by_tag_name(rootNodeId, tag) -> str` | 同上 |

### 6.3 classList（4 个 import 即可）

`dong_class_add(nodeId, cls)` / `dong_class_remove` / `dong_class_toggle -> i32` / `dong_class_contains -> i32`

### 6.4 样式

| import | 说明 |
|--------|------|
| `dong_style_set(nodeId, prop, value)` | `style.setProperty` / `style.color = x` |
| `dong_style_get(nodeId, prop) -> str` | 内联样式读 |
| `dong_computed_style_get(nodeId, prop) -> str` | `getComputedStyle().getPropertyValue` |

### 6.5 几何与滚动（QuickJS 对照 `js_node_bindings.cpp` L1704–1713，测试大量使用）

| import | 说明 |
|--------|------|
| `dong_get_rect(nodeId) -> str` | `getBoundingClientRect`，返回 JSON `{"x":..,"y":..,"w":..,"h":..}` |
| `dong_get_metric(nodeId, metricId) -> f64` | `offsetWidth/Height/Top/Left`、`clientWidth/Height`、`scrollWidth/Height` 统一走 metric 枚举（prelude 定义常量），省 10 个 import |
| `dong_get_scroll_top(nodeId) -> f64` / `dong_set_scroll_top(nodeId, v)` | 滚动读写（游戏列表 UI 必用）；`scroll_left` 同 |

### 6.6 焦点与方法

| import | 说明 |
|--------|------|
| `dong_focus(nodeId)` / `dong_blur(nodeId)` | 引擎已有 FocusManager，直通 |
| `dong_click(nodeId)` | 合成点击（测试常用） |
| `dong_matches(nodeId, selector) -> i32` / `dong_closest(nodeId, selector) -> nodeId` | 复用选择器匹配 |

### 6.7 prelude 包装

prelude 提供与浏览器习惯接近的**扁平**别名（如 `getValue(id)`、`setStyle(id, 'color', 'red')`），**不要**试图用对象/方法模拟 `el.style.color = 'red'`（对象模型在 2c 下不可靠，见 T04；T04 修复验证前保持扁平）。

## 验收标准

1. 上述 import 在 C++ 侧实现并接入 `createImport` 声明（js 桩 + 2c 路径都可用）。
2. Dong 测试集中 `test_simple_bindings.html` 及同类表单用例，改写为 Porffor manifest 版本后通过。
3. 每个 API 有一个最小 HTML+JS 用例（进 Dong 测试目录）。
4. 文档：在 Dong 仓库 prelude 文档列出全部新 API 签名。

## 风险

- `querySelectorAll` 返回 JSON 需要 Porffor 侧 `JSON.parse`（Porffor 有 `compiler/builtins/json.ts`，可用性需 smoke test；不行则先提供 `dong_query_result_count()` + `dong_query_result_at(i)` 迭代式兜底）。
- 字符串编码与返回通道依赖 T05/T16 结论，中文 value 用例要等 T05/T16。
- `dong_get_metric` 的取值需要触发布局（layout flush）——确认引擎在脚本执行点的布局新鲜度，必要时 import 内部先 flush（对照 QuickJS 侧 `elem_getOffsetWidth` 的做法）。

## 完成记录

- **日期**: 2026-07-04
- **commit**: `255ac50`（feature/porffor）
- **摘要**: 实现 T06 扁平 DOM import 全集（属性/查询/classList/style/几何/焦点），遵循 T16 拉取式字符串通道；prelude 提供扁平 helper 与 METRIC_* 常量。
- **变更文件**:
  - `dong/src/script/porffor/js_bindings_porffor.hpp/cpp`
  - `dong/src/script/porffor/dong_porf_host.hpp/cpp`
  - `dong/src/script/porffor/dong_porffor_prelude.js`
  - `dong/scripts/porffor_compile.mjs`
  - `docs/developer/porffor/tasks/repro/t06/`
- **验收命令**: `E:\ws\infra\dong\.tools\node-v22.16.0-win-x64\node.exe docs/developer/porffor/tasks/repro/t06/t06_verify.mjs`
- **遗留问题**: `querySelectorAll` 返回 JSON 字符串（Porffor 无 JSON.parse）；`innerHTML` 只写不读；engine 内集成测试待 T13 迁移。
