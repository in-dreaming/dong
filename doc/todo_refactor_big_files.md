# 大文件重构 TODO

更新时间: 2026-03-09

目的:
- 为切换到 coding 模型后的连续拆分提供一份可直接执行的清单。
- 优先解决当前仓库里最影响可维护性、最不利于 vibing coding 的大文件和大函数。
- 保持现有公共 API、ABI、功能行为不变，先做结构重组，再做行为优化。

---

## 0. 执行规则

- [ ] 每次只拆一个主题文件组，不并行做多个高风险拆分。
- [ ] 先做函数级提取，再做文件移动；不要一上来跨文件大搬家。
- [ ] 保持公共入口文件存在，先把它收缩成 orchestrator，不直接删除。
- [ ] 新增文件优先放在原模块目录内，避免跨层依赖倒灌。
- [ ] 共享内部声明优先放 `*_internal.hpp` / `*_internal.h`，不要扩大公共头文件暴露面。
- [ ] 所有新 helper 默认 `static` / 匿名命名空间，只有确实跨文件复用时再提升可见性。
- [ ] 每完成一个文件组后至少跑一次 `zig build run-feature-tests`。
- [ ] AppCore 相关拆分后额外跑一次 `zig build examples`，确认示例不被拆坏。
- [ ] 先重构，后优化。不要把“逻辑改动”和“结构拆分”绑在同一次提交里。

---

## 1. 当前大文件清单

本表基于 2026-03-09 对 `src/`、`backends/`、`appcore/` 的行数扫描。

| 优先级 | 文件 | 当前行数 | 处理策略 |
|---|---|---:|---|
| P0 | `src/layout/layout_engine.cpp` | 4557 | 必拆 |
| P0 | `src/script/js_bindings.cpp` | 4079 | 必拆 |
| P0 | `src/dom/css/css_parser.cpp` | 3525 | 必拆 |
| P0 | `src/core/engine_view.cpp` | 2523 | 必拆 |
| P0 | `appcore/src/dock.c` | 2520 | 必拆 |
| P0 | `src/dom/css/style_engine.cpp` | 746 | 已拆 (cascade/apply/inheritance/pseudo) |
| P0 | `src/render/painter.cpp` | 2106 | 必拆 |
| P1 | `appcore/src/scene3d.c` | 1976 | 应拆 |
| P1 | `backends/sdl/sdl_gpu_driver_execute.cpp` | 1735 | 应拆 |
| P1 | `src/script/js_node_bindings.cpp` | 1544 | 应拆 |
| P2 | `src/render/painter/painter_text.cpp` | 1489 | 函数级拆分 |
| P2 | `src/dom/dom/dom_node.cpp` | 1383 | 函数级拆分，暂不移动 |
| P2 | `src/dom/css/selector_matcher.cpp` | 919 | 观察 |
| P2 | `src/render/glyph_atlas.cpp` | 897 | 函数级拆分 |
| P2 | `backends/sdl/dong_sdl_platform.c` | 863 | 观察 |
| P2 | `src/render/font_resolver.cpp` | 801 | 函数级拆分 |

---

## 2. 推荐执行顺序

- [x] Phase 1: `appcore/src/dock.c`
- [x] Phase 2: `src/dom/css/css_parser.cpp` + `src/dom/css/style_engine.cpp`
- [ ] Phase 3: `src/layout/layout_engine.cpp`
- [ ] Phase 4: `src/core/engine_view.cpp`
- [ ] Phase 5: `src/script/js_bindings.cpp` + `src/script/js_node_bindings.cpp`
- [ ] Phase 6: `src/render/painter.cpp`
- [ ] Phase 7: `appcore/src/scene3d.c`
- [ ] Phase 8: `backends/sdl/sdl_gpu_driver_execute.cpp`
- [ ] Phase 9: P2 文件只做函数级缩减，不急着拆文件

原因:
- `dock.c` 是新的复杂度中心，边界清楚，收益最大。
- CSS 和 layout 是高频 feature 落点，拆完最利于后续继续做标准对齐。
- `engine_view.cpp` 是输入/事件/命中测试汇聚点，拆完能显著降低行为修改风险。
- JS 和 painter 适合在核心路由稳定后再拆。

---

## 3. P0: Docking 子系统重构

源文件:
- `appcore/src/dock.c`
- `appcore/src/dock_internal.h`
- `appcore/src/dock_split.c`
- `appcore/CMakeLists.txt`

目标结构:

```text
appcore/src/
  dock.c                 # 公开 API 薄入口
  dock_internal.h        # 内部共享结构
  dock_lifecycle.c       # create/destroy/run/frame timing
  dock_window.c          # window slot/create/destroy/find
  dock_pane.c            # pane add/remove/title/html/script/resource_root
  dock_events.c          # SDL 事件翻译与路由
  dock_drag.c            # tab drag / drop / reorder / attach / detach
  dock_render.c          # offscreen tick + window composite + overlay
  dock_title.c           # title 纹理/文字光栅化
  dock_layout_io.c       # save/load layout
  dock_split.c           # 继续保持纯 split tree
```

TODO:
- [x] 保留 `dock.c` 中所有 `DONG_APPCORE_API` 入口，只让它们转调到新文件实现。
- [x] 把 plugin loading 和公共创建/销毁流程抽到 `dock_lifecycle.c`。
- [x] 把 pane slot 分配、pane 初始化、HTML 加载、标题设置、脚本执行抽到 `dock_pane.c`。
- [x] 把 window slot 分配、window 查找、window 销毁抽到 `dock_window.c`。
- [x] 把 SDL event 翻译、window id 提取、poll loop 路由抽到 `dock_events.c`。
- [x] 把 `dock_update_drag`、`dock_finish_drag`、drop zone 计算、tab reorder 判定抽到 `dock_drag.c`。
- [x] 把 `dock_render_tree`、offscreen texture 确保、solid blit、composite 相关逻辑抽到 `dock_render.c`。
- [x] 把 `dock_render_title_texture` 和 8x16 bitmap font 相关逻辑抽到 `dock_title.c`。
- [x] 把 layout save/load JSON 相关逻辑抽到 `dock_layout_io.c`。
- [x] 收缩 `dock_internal.h`，只保留跨文件必须共享的结构与函数声明。
- [x] 更新 `appcore/CMakeLists.txt`，加入新的 `dock_*.c` 文件。
- [x] 如果 `build.zig` 有显式源文件列表，同步更新。
  当前 `build.zig` 未维护 `appcore/src/dock_*.c` 的显式清单，无需改动。

函数级先拆:
- [x] `dong_dock_create`
- [x] `dong_dock_destroy`
- [x] `dong_dock_poll_events`
- [x] `dong_dock_render`
- [x] `dock_translate_sdl_event`
- [x] `dock_render_title_texture`
- [x] `dock_update_drag`
- [x] `dock_finish_drag`
- [x] `dock_render_tree`

完成标准:
- [x] `dock.c` 控制在 500 行以内。
- [x] 单个 `dock_*.c` 文件不超过 900 行。
- [x] `dock_internal.h` 不出现与外部模块无关的杂项 helper。

---

## 4. P0: CSS Parser 重构

源文件:
- `src/dom/css/css_parser.cpp`

目标结构:

```text
src/dom/css/
  css_parser.cpp               # 顶层 parse 流程
  css_property_handlers.cpp    # 属性注册与 dispatch
  css_value_parser.cpp         # 数值/长度/颜色/关键字
  css_shorthand_parser.cpp     # margin/border/list-style/place-* 等简写
  css_function_parser.cpp      # gradient/filter/color functions
  css_at_rule_parser.cpp       # @layer 等 at-rule
  css_parser_internal.hpp      # 内部共享 helper / tables
```

TODO:
- [x] 保留 `css_parser.cpp` 作为主入口，收缩成 parse orchestrator。
- [x] 把 `getPropertyHandlers` 相关内容集中到 `css_property_handlers.cpp`。
- [x] 把 `parseValue` 的值类型分支拆到 `css_value_parser.cpp`。
- [x] 把 shorthand 解析统一移到 `css_shorthand_parser.cpp`。
- [x] 把 `parseGradient`、`parseFilter`、颜色函数解析移到 `css_function_parser.cpp`。
  - [x] 把 `@layer` 及后续 at-rule 解析整理到 `css_at_rule_parser.cpp`。
- [x] 为 parser 内部共享 token helper 建立 `css_parser_internal.hpp`，避免重复静态函数。

函数级先拆:
- [x] `getPropertyHandlers`
- [x] `CSSParser::parseValue`
- [x] `CSSParser::parseGradient`
- [x] `CSSParser::parseFilter`
- [x] `CSSParser::parseLayerRules`

完成标准:
- [x] `css_parser.cpp` 控制在 700 行以内。
- [x] 属性注册、值解析、函数解析、at-rule 四类逻辑不再混在一个文件中。

---

## 5. P0: Style Engine 重构

源文件:
- `src/dom/css/style_engine.cpp`

目标结构:

```text
src/dom/css/
  style_engine.cpp             # 对外入口与总编排
  style_engine_cascade.cpp     # rule 排序、layer/cascade
  style_engine_inheritance.cpp # 默认值、继承、逻辑属性解析
  style_engine_apply.cpp       # applyRuleProperties 分组逻辑
  style_engine_pseudo.cpp      # pseudo-element / marker / generated content
  style_engine_internal.hpp
```

TODO:
- [x] 保留 `style_engine.cpp` 作为总编排入口。
- [x] 把 rule 排序、优先级、layer 级联集中到 `style_engine_cascade.cpp`。
- [x] 把 logical properties、direction/text-align resolve、inherit/default 逻辑集中到 `style_engine_inheritance.cpp`。
- [x] 把 `applyRuleProperties` 的大量子类分支移到 `style_engine_apply.cpp`。
- [x] 把 `::before` / `::after` / `::marker` / generated content 相关逻辑移到 `style_engine_pseudo.cpp`。

函数级先拆:
- [x] `applyRuleProperties`
- [x] 现有 `applyRule*` 族函数按文本、盒模型、边框、背景、布局、动画再细分
- [x] 逻辑属性映射 helper 从主入口剥离

完成标准:
- [x] `style_engine.cpp` 控制在 600 行以内。
- [x] `applyRuleProperties` 不再是总垃圾桶函数。

---

## 6. P0: Layout Engine 重构

源文件:
- `src/layout/layout_engine.cpp`

目标结构:

```text
src/layout/
  layout_engine.cpp            # Engine 入口与总流程
  layout_intrinsic.cpp         # intrinsic width/height / text measure
  layout_block.cpp             # block formatting / 常规块布局
  layout_inline.cpp            # inline formatting context
  layout_yoga_bridge.cpp       # CSS -> Yoga 样式映射
  layout_positioned.cpp        # absolute/fixed/sticky
  layout_engine_internal.hpp
```

TODO:
- [ ] 让 `layout_engine.cpp` 只保留流程函数和对外成员函数。
- [ ] 把 intrinsic text width/height 与相关 cache glue 移到 `layout_intrinsic.cpp`。
- [ ] 把常规 block 布局、child node 构建相关逻辑移到 `layout_block.cpp`。
- [ ] 把 IFC 相关逻辑移到 `layout_inline.cpp`。
- [ ] 把 `applyDOMStylesToYoga` 和周边 helper 移到 `layout_yoga_bridge.cpp`。
- [ ] 把 positioned/sticky post-process 逻辑移到 `layout_positioned.cpp`。

函数级先拆:
- [ ] `Engine::calculateLayout`
- [ ] `Engine::applyDOMStylesToYoga`
- [ ] `Engine::layoutInlineFormattingContexts`
- [ ] `Engine::layoutPositionedElements`
- [ ] `computeIntrinsicTextWidth`
- [ ] `computeIntrinsicTextHeight`
- [ ] `Engine::buildChildYogaNodes`

完成标准:
- [ ] `layout_engine.cpp` 控制在 800 行以内。
- [ ] `layout_inline.cpp` 独立成为 IFC 的唯一实现落点。
- [ ] intrinsic measure 与 Yoga bridge 不再混在同一个编译单元。

---

## 7. P0: Engine View 重构

源文件:
- `src/core/engine_view.cpp`

目标结构:

```text
src/core/
  engine_view.cpp              # EngineView 对外成员入口
  engine_view_hit_test.cpp     # hit test / resize handle / scroll container 查找
  engine_view_input.cpp        # mouse / keyboard / text 输入入口
  engine_view_scroll.cpp       # scroll / wheel / sticky related routing
  engine_view_forms.cpp        # input/select/textarea/details 行为
  engine_view_navigation.cpp   # anchor/hash/tooltip/navigation
  engine_view_media.cpp        # video sync / media update
  engine_view_internal.hpp
```

TODO:
- [ ] 保留 `engine_view.cpp`，只做生命周期和顶层调度。
- [ ] 把命中测试及几何 helper 移到 `engine_view_hit_test.cpp`。
- [ ] 把鼠标/键盘/文本输入入口移到 `engine_view_input.cpp`。
- [ ] 把滚动容器查找、wheel 路由、scrollIntoView 相关逻辑移到 `engine_view_scroll.cpp`。
- [ ] 把表单元素交互、textarea resize handle、select/details 行为移到 `engine_view_forms.cpp`。
- [ ] 把 hash anchor、tooltip、navigation 逻辑移到 `engine_view_navigation.cpp`。
- [ ] 把 `syncVideoElements` 等媒体相关逻辑移到 `engine_view_media.cpp`。

函数级先拆:
- [ ] 顶层 hit-test helper 族
- [ ] mouse move/button/wheel 路由
- [ ] key/text 输入路由
- [ ] anchor/hash 处理
- [ ] `syncVideoElements`

完成标准:
- [ ] `engine_view.cpp` 控制在 700 行以内。
- [ ] 输入、命中测试、媒体同步三类逻辑完全分离。

---

## 8. P0: Script Bindings 重构

源文件:
- `src/script/js_bindings.cpp`
- `src/script/js_node_bindings.cpp`

目标结构:

```text
src/script/
  js_bindings.cpp               # bootstrap / class registration / globals
  js_console_window_bindings.cpp
  js_webapi_bindings.cpp        # structuredClone/FormData/DOMParser/DOMRect/...
  js_cssom_bindings.cpp         # CSS.supports/matchMedia/cssText/StyleSheet
  js_document_bindings.cpp      # document-level API
  js_node_bindings.cpp          # 作为 node/element 绑定入口壳
  js_node_tree_bindings.cpp     # parent/child/sibling/tree traversal
  js_element_bindings.cpp       # attributes/class/style/geometry
  js_form_bindings.cpp          # input/select/textarea/form
  js_event_bindings.cpp         # native event helper / event object plumbing
  js_bindings_internal.hpp
```

TODO:
- [ ] 把 `js_bindings.cpp` 缩成 bootstrap 和全局注册入口。
- [ ] 把 console/alert/confirm/prompt 等移到 `js_console_window_bindings.cpp`。
- [ ] 把 Web API 移到 `js_webapi_bindings.cpp`。
- [ ] 把 CSSOM API 移到 `js_cssom_bindings.cpp`。
- [ ] 把 document 级 API 移到 `js_document_bindings.cpp`。
- [ ] 让 `js_node_bindings.cpp` 只负责 node/element 注册入口。
- [ ] 把 tree traversal 类 getter/method 移到 `js_node_tree_bindings.cpp`。
- [ ] 把 element 属性/几何/基础 DOM 操作移到 `js_element_bindings.cpp`。
- [ ] 把 input/select/textarea/form 绑定移到 `js_form_bindings.cpp`。
- [ ] 把 event helper 和原生事件补丁逻辑移到 `js_event_bindings.cpp`。

函数级先拆:
- [ ] `JSBindings::createJSElement`
- [ ] console API 族
- [ ] `structuredClone` 相关
- [ ] `window.matchMedia`
- [ ] `CSS.supports`
- [ ] `cssText` getter/setter
- [ ] `node_event_*` / `installBasicEventMethods`

完成标准:
- [ ] `js_bindings.cpp` 控制在 700 行以内。
- [ ] `js_node_bindings.cpp` 控制在 800 行以内。
- [ ] Window/Web API/CSSOM/Node/Form 四类绑定不再交叉。

---

## 9. P0: Painter 重构

源文件:
- `src/render/painter.cpp`
- `src/render/painter/painter_text.cpp`
- `src/render/painter/painter_children.cpp`
- `src/render/painter/painter_marker.cpp`
- `src/render/painter/painter_media.cpp`

目标结构:

```text
src/render/
  painter.cpp                         # 顶层遍历与 orchestrator
  painter/painter_backgrounds.cpp
  painter/painter_borders.cpp
  painter/painter_effects.cpp
  painter/painter_generated.cpp       # quotes/counters/generated content
  painter/painter_text.cpp
  painter/painter_media.cpp
  painter/painter_children.cpp
  painter/painter_marker.cpp
  painter/painter_style_utils.hpp
  painter/painter_internal.hpp
```

TODO:
- [ ] 让 `painter.cpp` 只负责 display list 构建流程和节点遍历编排。
- [ ] 把背景绘制移到 `painter_backgrounds.cpp`。
- [ ] 把边框/圆角/outline 绘制移到 `painter_borders.cpp`。
- [ ] 把 filter/opacity/layer promotion/will-change 相关逻辑移到 `painter_effects.cpp`。
- [ ] 把 quotes/counters/generated content 逻辑移到 `painter_generated.cpp`。
- [ ] 保持 `painter_text.cpp`、`painter_media.cpp`、`painter_marker.cpp`、`painter_children.cpp` 的职责单一，不再回灌其他杂项。

函数级先拆:
- [ ] `parseCssColor` 如仍在 `painter.cpp` 主文件出现，彻底收口到 style utils 或专用实现文件
- [ ] 背景图位置/尺寸解析 helper
- [ ] layer promotion / will-change helper
- [ ] generated content / counters / quotes helper

完成标准:
- [ ] `painter.cpp` 控制在 700 行以内。
- [ ] 文本、媒体、背景、边框、生成内容五条渲染路径彼此独立。

---

## 10. P1: Scene3D 重构

源文件:
- `appcore/src/scene3d.c`

目标结构:

```text
appcore/src/
  scene3d.c               # 公开 API 薄入口
  scene3d_internal.h
  scene3d_lifecycle.c     # create/destroy/common state
  scene3d_screen.c        # screen add/remove/update/html load
  scene3d_input.c         # hit test / raycast / input routing
  scene3d_render.c        # GPU resources / draw path
  scene3d_resource.c      # shader/plugin/path/file helpers
```

TODO:
- [ ] 保留 `scene3d.c` 作为 API 转发层。
- [ ] 把 plugin loading 和文件路径 helper 抽到 `scene3d_resource.c`。
- [ ] 把 screen 生命周期和配置处理抽到 `scene3d_screen.c`。
- [ ] 把鼠标/焦点/命中测试相关逻辑抽到 `scene3d_input.c`。
- [ ] 把 shader、uniform、GPU render path 抽到 `scene3d_render.c`。
- [ ] 引入 `scene3d_internal.h` 承载内部结构。

完成标准:
- [ ] `scene3d.c` 控制在 600 行以内。

---

## 11. P1: SDL GPU Execute 重构

源文件:
- `backends/sdl/sdl_gpu_driver_execute.cpp`

目标结构:

```text
backends/sdl/
  sdl_gpu_driver_execute.cpp
  sdl_gpu_driver_dispatch.cpp
  sdl_gpu_driver_draw_shapes.cpp
  sdl_gpu_driver_draw_images.cpp
  sdl_gpu_driver_draw_text.cpp
  sdl_gpu_driver_layers.cpp
  sdl_gpu_driver_execute_internal.hpp
```

TODO:
- [ ] 让 `sdl_gpu_driver_execute.cpp` 只保留 execute 入口与 pass orchestration。
- [ ] 把 command dispatch table 抽到 `sdl_gpu_driver_dispatch.cpp`。
- [ ] 把形状类命令移到 `sdl_gpu_driver_draw_shapes.cpp`。
- [ ] 把 image/video 类命令移到 `sdl_gpu_driver_draw_images.cpp`。
- [ ] 把 text/glyph/shadow/decorations 逻辑移到 `sdl_gpu_driver_draw_text.cpp`。
- [ ] 把 isolated layer begin/end/composite 逻辑移到 `sdl_gpu_driver_layers.cpp`。

完成标准:
- [ ] `sdl_gpu_driver_execute.cpp` 控制在 500 行以内。
- [ ] 热路径拆分后不引入额外虚函数和不必要对象层级。

---

## 12. P2: 只做函数级缩减，暂不强制拆文件

### `src/render/painter/painter_text.cpp`
- [ ] 把文本行构建、对齐、装饰、selection、shadow 分为独立 helper
- [ ] 入口函数只做 line loop 和 emit orchestration

### `src/dom/dom/dom_node.cpp`
- [ ] 把 DOM attribute、direction/lang、event helpers、tree mutation helper 分组提取
- [ ] 暂不移动文件，先降低单函数尺寸

### `src/render/glyph_atlas.cpp`
- [ ] 把 glyph 查找、光栅化、上传、批处理分成 helper
- [ ] 目标是缩短 `addGlyph` 和 `addGlyphsBatched`

### `src/render/font_resolver.cpp`
- [ ] 把 family candidate 收集、weight 正规化、best match 评分分开

### `src/dom/css/selector_matcher.cpp`
- [ ] 如继续增长超过 1200 行，再拆成 pseudo-class / structural selector 两个文件

---

## 13. 统一内部文件命名规范

- [ ] 入口文件继续保留原名，例如 `layout_engine.cpp`、`css_parser.cpp`、`dock.c`
- [ ] 实现文件用 “模块_职责” 命名，例如 `dock_render.c`、`engine_view_input.cpp`
- [ ] 共享内部声明用 `*_internal.hpp` / `*_internal.h`
- [ ] 不新增 `util.cpp`、`misc.cpp`、`helpers.cpp` 这种垃圾桶文件

---

## 14. 每个 Phase 的验收步骤

### 通用
- [ ] 编译通过
- [ ] 头文件依赖没有环
- [ ] 入口文件显著变薄
- [ ] 没有新增跨层 include

### Core / CSS / Layout / Render
- [x] `zig build run-feature-tests`

### AppCore
- [x] `zig build examples`
- [ ] 手工 smoke: `minimal_dong_demo`
- [ ] 手工 smoke: `interactive_demo_new`
- [ ] 手工 smoke: `3d_screens_simple`
- [ ] 如 docking 被接入 Zig 示例，额外 smoke `dock_demo`

---

## 15. 完成判定

当以下条件全部满足时，视为大文件拆分阶段完成:

- [ ] 所有 P0 文件都被拆到目标结构
- [ ] 没有任何 `.cpp` / `.c` 文件超过 2000 行
- [ ] 没有任何核心入口文件超过 1000 行
- [ ] 所有公共 API 入口文件都变成薄入口
- [ ] `doc/重构遗留.md` 与本清单状态同步
