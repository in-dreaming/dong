# P1-3 — DevTools v1（嵌入式 Inspector）

> 上游：[`docs/roadmap.md`](../roadmap.md) Phase 1 P1-3
> 性能门槛：[`docs/developer/perf-budget.md`](../perf_budget.md) § 3.6（DevTools 打开延迟）
>
> 状态：方案 v1 / 2026-04-17

---

## 1. 目标

为 dong 提供**嵌入式开发者工具**，按 F12 在任意 view 上叠加 inspector overlay：

- DOM 树浏览 + 高亮命中元素
- Computed style + box model 可视化
- Layout box overlay（content / padding / border / margin）
- 当前帧 perf HUD（FPS / draw / damage rect / atlas usage）
- Console（JS log / 输入命令）
- 元素属性 / inline style 修改 → 立刻看到效果
- 性能 trace 切换（present-only / damage rect / batch boundary 显示）

不依赖外部浏览器；DevTools 自身用 dong 写（dogfood）。

---

## 2. 现状

| 项 | 状态 |
|---|---|
| 浏览器 inspector | ❌ |
| 命令行 / log 调试 | ✅（`DONG_LOG_LEVEL`） |
| 像素 baseline diff | ✅（offline，需手动） |
| Profiler trace | ✅（offline，需手动跑 `auto_profile_loop`） |
| `dong_engine_get_perf_counters` | P0-7 后可用 |
| Damage rect 元数据 | P0-6 后可用 |

---

## 3. 设计

### 3.1 总体架构：DevTools 是一个 dong 子 view

```
+----------------------------------------------+
|  Main HTML View (用户应用)                    |
|                                              |
|  +----------------------------------------+ |
|  | DevTools Overlay View                   |<-- 独立 dong View 实例
|  |  - dong_app_create + load DevTools.html | |
|  |  - 通过 dong_engine_attach_inspector    | |
|  |    与 main view 双向通信                 | |
|  +----------------------------------------+ |
+----------------------------------------------+
```

DevTools UI 本身就是一份 HTML/CSS/JS（用 React/Preact 任选），由 dong 渲染——**dogfood 闭环**：dong 渲不好 DevTools，团队会第一个发现。

### 3.2 C ABI: Inspector Bridge

```c
// dong/include/dong_inspector.h（新增）
typedef struct DongInspector DongInspector;

typedef struct {
    uint64_t node_id;
    const char* tag;
    const char* id;
    const char* class_list;
    DongRect content_rect;
    DongRect padding_rect;
    DongRect border_rect;
    DongRect margin_rect;
} DongInspectorNodeInfo;

typedef struct {
    const char* property;
    const char* value;
    const char* origin;     // "user" | "ua" | "inline" | "inherited"
    int         important;  // 0/1
    int         active;     // 1 = 命中级联；0 = 被覆盖
} DongInspectorStyleEntry;

DongInspector* dong_engine_attach_inspector(dong_engine_t* main_view);
void           dong_inspector_detach(DongInspector* ins);

// DOM 树
size_t dong_inspector_dom_children(DongInspector* ins, uint64_t parent_node_id,
                                   uint64_t* out_ids, size_t cap);
int    dong_inspector_node_info(DongInspector* ins, uint64_t node_id,
                                DongInspectorNodeInfo* out);

// 样式
size_t dong_inspector_node_styles(DongInspector* ins, uint64_t node_id,
                                  DongInspectorStyleEntry* out, size_t cap);

// 修改
int dong_inspector_set_inline_style(DongInspector* ins, uint64_t node_id,
                                    const char* property, const char* value);
int dong_inspector_set_attribute(DongInspector* ins, uint64_t node_id,
                                 const char* name, const char* value);

// 命中测试（在 main view 像素坐标）
uint64_t dong_inspector_pick(DongInspector* ins, int x, int y);

// 高亮（在 main view 上画 overlay；返回 token，detach 时自动清）
typedef struct { float r, g, b, a; } DongInspectorColor;
int dong_inspector_highlight(DongInspector* ins, uint64_t node_id,
                             DongInspectorColor border, DongInspectorColor fill);
void dong_inspector_clear_highlights(DongInspector* ins);

// Perf
typedef struct {
    float    fps;
    uint64_t draw_calls;
    uint64_t commands_regenerated;
    float    layout_paint_us;
    float    gpu_us;
    uint64_t glyph_atlas_bytes;
    uint64_t image_atlas_bytes;
} DongInspectorPerfSample;

void dong_inspector_perf_sample(DongInspector* ins, DongInspectorPerfSample* out);

// Console
typedef void (*DongInspectorLogCallback)(const char* level, const char* msg, void* user);
void dong_inspector_set_log_callback(DongInspector* ins,
                                     DongInspectorLogCallback cb, void* user);
int  dong_inspector_eval_js(DongInspector* ins, const char* code,
                            char* out_result, size_t cap);
```

### 3.3 DevTools UI 内部结构

`dong/devtools/` 新目录：

```
dong/devtools/
├── README.md
├── package.json                 # 用 Preact + esbuild
├── src/
│   ├── App.jsx
│   ├── panels/
│   │   ├── ElementsPanel.jsx     # DOM 树 + style
│   │   ├── PerfPanel.jsx         # FPS / draw / atlas
│   │   ├── ConsolePanel.jsx      # log + eval
│   │   └── DamagePanel.jsx       # 显示 damage rect / batch boundary
│   ├── api/
│   │   └── inspector.js          # 调 dong_inspector_* （通过 JS bridge）
│   └── index.html
└── build.mjs
```

构建产出 `dong/zig-out/devtools/bundle.html` + `bundle.js`，在 dong_app 里通过 `dong_engine_load_html(devtools_view, "...bundle.html")` 加载。

### 3.4 dong_app 集成

```c
// app.c 主循环新增
if (key_pressed == F12) {
    if (devtools_view == NULL) {
        devtools_view = dong_app_create_overlay_view(app, devtools_w, devtools_h);
        dong_engine_load_html(devtools_view, devtools_html_path);
        DongInspector* ins = dong_engine_attach_inspector(main_view);
        // 把 ins 句柄注入 devtools_view 的 JS 全局
        dong_engine_eval_script(devtools_view, "window.__inspector = ...");
    } else {
        dong_app_destroy_overlay_view(app, devtools_view);
        devtools_view = NULL;
    }
}
```

DevTools view 占主窗口右侧 30%（默认；可拖动；可 detach 到独立窗口，留 v2）。

### 3.5 Element 高亮渲染

DevTools 调 `dong_inspector_highlight(node_id, border, fill)` →
内部把高亮信息存到 `inspector_highlights_` 列表 →
main view 的 `Painter::paintOverlay()` 在 DisplayList 末尾 emit 高亮 quad（border outline + 半透明 fill）。

不入主 DOM 树，避免触发样式 invalidation。

### 3.6 实时编辑

DevTools 改 inline style → `dong_inspector_set_inline_style(node, prop, value)` → 内部走与 JS `element.style.setProperty` 同一路径 → 触发 invalidation → 下一帧像素更新。

**用户友好**：值无效时返回非 0；DevTools UI 把字段标红。

### 3.7 Perf HUD（默认就在 DevTools Perf panel）

每 250ms 调 `dong_inspector_perf_sample` → 渲染 sparkline。

进阶：

- 切换"显示 batch 边界"：DevTools 调 `dong_inspector_set_debug_overlay(BATCH)` → main view 在 batch flush 处画细线。
- 切换"显示 damage rect"：与 P0-6 配合，画 damage 矩形（每帧不同色，2 秒内渐隐）。
- 切换"显示 layer 边界"：画 isolated layer 框。

### 3.8 Console

- 接 `console.log` / `console.warn` / `console.error`：dong 内部 console binding 加 callback；DevTools 收到。
- Eval：DevTools 输入框 → `dong_inspector_eval_js(code)` → 在 **main view** JS context 执行；返回字符串。
- 历史 / 自动补全：留 v2。

### 3.9 与三轨的关系

| 模式 | DevTools 支持 |
|---|---|
| **DOM** | ✅ 完整 |
| **Scene Graph** | ✅ 简化版：DOM 树 panel 显示 SceneNode；属性面板对应 setFloat/setColor |
| **Direct Draw** | Console + Perf；无 DOM 树（无 retained 状态） |

### 3.10 性能影响

DevTools 关闭时**零开销**（无 callback 注册、无 perf counter 拉取）。

打开时：
- main view 多一次 `paintOverlay`：< 0.05 ms
- DevTools view 自身渲染：< 4 ms（Preact + 简单 UI）
- perf sample 拉取：< 0.05 ms / 250ms
- 总额外 budget：~5 ms（与 [`perf_budget.md`](../perf_budget.md) S6 兼容）

---

## 4. 实施步骤

| Step | 范围 |
|---|---|
| **S1 — Inspector C ABI（DOM 浏览 + 命中测试）** | dong_inspector_dom_children / node_info / pick；命令行 demo 验证 |
| **S2 — 高亮 overlay** | dong_inspector_highlight + main view paint 末尾 |
| **S3 — DevTools UI 骨架** | dong/devtools/ Preact + esbuild + 显示 DOM 树 panel |
| **S4 — Style panel + 实时编辑** | computed style + inline style edit |
| **S5 — F12 集成 dong_app** | overlay view + 默认 30% 宽度面板 |
| **S6 — Perf panel + batch / damage / layer overlay 切换** | 与 P0-6 / P0-7 联动 |
| **S7 — Console（log + eval）** | 注入 callback + 输入框 |
| **S8 — Polish + 文档 + 默认打包** | 跟 zig build 集成 |

---

## 5. 通过验证规则

### 5.1 Hard

| 项 | 阈值 |
|---|---|
| `dong_app` 中按 F12 在 200 ms 内打开 DevTools 面板 | 见 perf_budget S6 Soft / Hard |
| 关闭 DevTools 后 main view perf 与未打开时一致（容差 1%） | 必须 |
| DOM 树面板正确显示主 view 的当前 DOM（含 attribute / class） | 手动 + 单元 |
| 鼠标悬停在 DOM 树某节点 → main view 对应元素被高亮 | 手动 |
| Style panel 改 background-color → 像素立即变化 | 手动 |
| Perf panel FPS 数值与 `dong_engine_get_perf_counters` 一致 | 单元 |
| Console eval `1+1` 返回 `2`；eval `document.title` 正确 | 单元 |
| DevTools 打开期间 main view 不掉帧（< 5 ms 额外开销） | perf_baseline.py |
| 编辑 inline style 不破坏既有 baseline（关闭 DevTools 后） | 像素全集 |

### 5.2 Soft

| 项 | 阈值 |
|---|---|
| DevTools 自身 FPS | ≥ 60 |
| DOM 树 1000 节点滚动流畅 | ≥ 60 FPS |
| Style panel 显示 entries 数 | ≥ 20 默认显示 + 可展开全部 |
| Damage rect overlay 渲染 | 矩形渐隐 2 秒，不卡顿 |

### 5.3 必须新增的测试

| 测试 | 内容 |
|---|---|
| `tests/inspector/test_dom_walk.cpp` | C ABI 单元：枚举 DOM、取 NodeInfo |
| `tests/inspector/test_pick.cpp` | 命中测试与坐标转换 |
| `tests/inspector/test_set_inline_style.cpp` | 设置 inline style 后 ComputedStyle 立即变 |
| `examples/data/devtools/devtools.html` | DevTools 自身 UI 入口 |
| `dong/scripts/test_devtools_perf_overhead.py` | 关 vs 开 DevTools 跑 game_ui2，FPS 差 < 5% |

### 5.4 验证命令

```bash
# C ABI 单元
cd dong && zig build run-feature-tests --filter Inspector

# DevTools build
cd dong/devtools && npm i && npm run build
# 产出 dong/zig-out/devtools/bundle.html

# 集成验证
dong_app.exe --html data/gamelikeui/game_ui2.html
# 按 F12，操作面板，逐项验证 § 5.1

# Perf 影响
python dong/scripts/test_devtools_perf_overhead.py
```

---

## 6. 风险与回退

| 风险 | 缓解 |
|---|---|
| dogfood 鸡蛋问题：DevTools 自身渲染崩 → 看不见 main view 状态 | DevTools 必须能用 fallback "console-only" 模式（仅 stdout，跳 UI） |
| Inspector 持有 DOM 节点指针，节点被 JS 删除时 dangling | 用 stable node_id；`node_info` 找不到时返回 0；避免裸指针外泄 |
| 实时编辑触发整树重建 | 与 P0-6 联动：`set_inline_style` 走 `Style` invalidation kind |
| Eval JS 改坏 main view（如把 body 删了） | 文档警告；不做沙箱（开发工具）|
| DevTools 打开后 perf counter 累积偏移影响 perf_baseline | perf_baseline 跑前自动 detach inspector |
| Preact bundle 太大启动慢 | DevTools 使用 Preact，bundle ≤ 50 KB；首次打开 < 200 ms |

---

## 7. 不在本方案范围

- ❌ Network panel（dong 无 fetch / XHR 详细面板需求；可看 `console.log`）
- ❌ Sources panel（无 JS 调试器；要的话挂 quickjs-debugger 留 v2）
- ❌ Memory profiler（QuickJS GC 自带 stats 即可）
- ❌ Animations panel（动画曲线编辑器）
- ❌ Detach 到独立窗口（v2）
- ❌ 远程 DevTools（跨进程）
- ❌ React DevTools 协议（v2 评估）
- ❌ 真正的 Source Map 解析（v2）

---

## 8. 完成后更新

- [ ] `docs/reference/features-index.md` 新增 § "DevTools (F12)"
- [ ] `docs/roadmap.md` Phase 1 P1-3 状态 ✅
- [ ] `docs/developer/perf-budget.md` § 3.6 DevTools 打开延迟实测填入
- [ ] `dong/include/dong_inspector.h` 文档化
- [ ] `dong/devtools/README.md` 用户使用说明
