# P2-7 — 可视化编辑器（dong-editor，dogfood）

> 上游：[`doc/roadmap.md`](../roadmap.md) Phase 2 P2-7
> 协同：[`P1-3 DevTools v1`](../phase1/P1-3_devtools_v1.md) [`P2-5 dong-ui`](./P2-5_dong_ui_library.md) [`P2-6 npm`](./P2-6_typescript_npm.md)
>
> 状态：方案 v1 / 2026-04-17

---

## 1. 目标

提供 dong 自家 UI 编辑器 `dong-editor`，让美术 / TA / UI 程序拖拖拽拽就能产出可在 runtime 跑的 HTML/CSS：

- 节点树面板 + 属性 inspector（CSS / 事件）
- 所见即所得 canvas（实时 dong 渲染，不是 web 模拟）
- dong-ui 组件库一键插入
- 多文档管理（一个 project 多个 view）
- 输出 HTML/CSS/JSX 文件，无供应商锁定
- 编辑器**自身用 dong + dong-ui 写**（终极 dogfood）

---

## 2. 现状

| 项 | 现状 |
|---|---|
| 编辑器 | ❌ |
| DevTools v1 | Phase 1 P1-3 完成（节点树 + style 修改 + perf）|
| Live reload | Phase 1 P1-4 完成 |
| dong-ui 组件库 | Phase 2 P2-5 完成 |
| `host-view` 嵌入 | Phase 1 P1-2 完成 |

P1-3 已经完成了大部分调试基础；本任务把它**升级为完整编辑器**。

---

## 3. 设计

### 3.1 与 DevTools 的边界

| 工具 | 用途 | 启动 |
|---|---|---|
| **DevTools (P1-3)** | 调试运行中的 view（任意 dong_app）；只看 / 修 inline style；不保存 | F12 在任意 dong_app 内 |
| **dong-editor（本任务）** | 创作 / 修改 HTML/CSS 源码；多文档；保存到磁盘 | 独立可执行 `dong-editor.exe` |

DevTools 与 editor 共享 inspector C ABI（P1-3）；editor 在其上加：
- 文件 IO（项目管理）
- 拖拽创建 / 删除节点
- 操作历史（undo/redo）
- dong-ui 组件库面板（拖入）
- 多文档 + tab

### 3.2 架构

```
dong-editor.exe（独立 binary）
   │
   ├── 主窗口（dong_app）
   │      └── HTML：用 dong-ui 写编辑器 UI（左：tree，中：canvas，右：inspector）
   │
   ├── Embedded canvas
   │      └── 用 P1-2 <host-view> 嵌入"被编辑文档的 dong view"
   │             - 操作：选中元素 → 高亮 → inspector 改 → 文件 dirty
   │
   └── Project 文件系统
          └── 监听文件变化 → live reload 编辑结果到 canvas
```

dong-editor 的**主 UI 本身是 dong + dong-ui 写的 HTML/CSS/React**（dogfood）。

### 3.3 用户工作流

```
1. dong-editor.exe --new-project ./game-ui
   → 生成 project 模板：
       game-ui/
         ├── dong-editor.json     # 项目配置
         ├── views/
         │   ├── main_menu.html
         │   ├── inventory.html
         │   └── hud.html
         ├── styles/
         │   └── theme.css
         └── assets/

2. 编辑器打开
   → 左侧：项目文件树
   → 中央：选中 main_menu.html → canvas 实时渲染
   → 右侧：当前选中元素的 inspector

3. 拖拽：
   - 从 dong-ui 面板把 <Button> 拖到 canvas
     → editor 在 source HTML 适当位置插入 <Button>...</Button>
     → 文件保存 → live reload → canvas 更新
   - 在 canvas 直接拖元素改位置（仅 absolute 定位时）
     → 修改 inline style 的 left/top

4. 修改：
   - inspector 改 background-color → 修改 source CSS
   - 修改 attributes / event handlers

5. 保存 → 文件已更新；业务方在 IDE 也能看见同步变化（双向）

6. 一键预览：
   - dong-editor 内 "Preview" 按钮 → 弹出独立窗口模拟 game runtime
```

### 3.4 C ABI 扩展（基于 P1-3 Inspector）

```c
// dong/include/dong_editor.h（新增）

// 编辑器层比 inspector 多的能力：源码修改
typedef struct DongEditor DongEditor;

typedef struct {
    const char* project_root;
    const char* current_file;        // 正在编辑的 HTML 路径
} DongEditorContext;

DongEditor* dong_editor_create(dong_engine_t* canvas_view);
void        dong_editor_destroy(DongEditor* ed);

int dong_editor_open_file(DongEditor* ed, const char* path);
int dong_editor_save(DongEditor* ed);

// 在选中节点旁插入子节点（编辑源码）
typedef enum {
    DONG_EDITOR_INSERT_BEFORE,
    DONG_EDITOR_INSERT_AFTER,
    DONG_EDITOR_INSERT_FIRST_CHILD,
    DONG_EDITOR_INSERT_LAST_CHILD,
} DongEditorInsertPos;

int dong_editor_insert_html(DongEditor* ed, uint64_t target_node_id,
                            DongEditorInsertPos pos, const char* html_snippet);
int dong_editor_remove_node(DongEditor* ed, uint64_t node_id);
int dong_editor_set_attribute(DongEditor* ed, uint64_t node_id,
                              const char* name, const char* value);
int dong_editor_set_text(DongEditor* ed, uint64_t node_id, const char* text);

// 修改外部 CSS 文件
int dong_editor_set_css_rule(DongEditor* ed, const char* css_file,
                             const char* selector, const char* property, const char* value);

// 撤销 / 重做
int dong_editor_undo(DongEditor* ed);
int dong_editor_redo(DongEditor* ed);
int dong_editor_history_size(DongEditor* ed);

// 序列化（保存到文件）
const char* dong_editor_serialize_html(DongEditor* ed);
```

### 3.5 编辑器 UI 结构

```
┌──────────────────────────────────────────────────────────────┐
│ Toolbar: New | Open | Save | Undo | Redo | Preview | Theme  │
├────────────┬─────────────────────────────────┬───────────────┤
│ Project    │ Canvas                           │ Inspector     │
│ Tree       │  ┌─────────────────────────┐    │  ┌──────────┐ │
│            │  │  <host-view> 嵌入       │    │  │ Tag      │ │
│ - views/   │  │  被编辑的 dong view      │    │  │ Class    │ │
│   - menu   │  │                          │    │  │ ID       │ │
│   - hud    │  │  渲染当前 HTML           │    │  │ Style    │ │
│ - styles/  │  │                          │    │  │   color  │ │
│ - assets/  │  │  选中 / hover            │    │  │   bg     │ │
│            │  │  实时高亮                │    │  │ Events   │ │
│ DOM Tree   │  │                          │    │  └──────────┘ │
│ - body     │  └─────────────────────────┘    │               │
│   - div    │                                  │  Components   │
│     - btn  │                                  │  Library      │
│            │                                  │  - Button     │
│            │                                  │  - Dialog     │
│            │                                  │  - ...        │
└────────────┴─────────────────────────────────┴───────────────┘
```

实现：完全用 dong + dong-ui 的 React 组件搭建。

### 3.6 双向同步（编辑器 ↔ IDE）

| 方向 | 实现 |
|---|---|
| editor 改 → 文件 → IDE 看到 | editor 写文件 + flush；业务方 IDE 自动 reload |
| IDE 改 → 文件 → editor 看到 | editor 自身的 file watcher（与 P1-4 同机制）触发 reload；选中态尽量保留 |

冲突：双方同时改同一行 → 后写的胜出 + console.warn（v1）；v2 加 merge UI。

### 3.7 操作历史

每次操作打 undo entry：

```
{
  type: 'set_attribute',
  node_path: 'body > div:nth-child(2) > button',
  attr: 'class',
  before: 'btn',
  after: 'btn primary',
}
```

存内存数组，dong_editor_undo 反向应用，dong_editor_redo 正向。

### 3.8 与 dong-ui 集成

dong-ui 组件库面板：

```
[Components ↓]
- Basic
  - Button
  - Input
  ...
- Game
  - HealthBar
  - SkillButton
  ...
```

拖入：editor 把对应组件的 HTML 模板（带默认 props）插入到选中位置：

```html
<!-- 拖入 HealthBar 后插入 -->
<HealthBar value={70} max={100} variant="circular" thickness={8}></HealthBar>
```

或对应 vanilla 形式：

```html
<du-health-bar value="70" max="100" variant="circular" thickness="8"></du-health-bar>
```

### 3.9 多 view 协调

业务方一个 project 可能有 10+ 个 view。编辑器：
- 项目树左侧浏览。
- 每个文件一个 tab（最多并发打开 N 个；其余按需）。
- 切 tab → canvas 切换。
- 不要求同时渲染多 view（只渲染当前 active）。

### 3.10 输出与导出

- 默认 = 直接编辑源 HTML/CSS（不导入 / 不导出）。
- 业务侧 build 工具（esbuild / vite）正常处理；editor 不接管 build。
- 导出对照（debug 用）：右键 file → "Show generated AST"。

---

## 4. 实施步骤

| Step | 范围 |
|---|---|
| **S1 — Editor C ABI（基于 inspector 加 file IO + insert/remove）** | dong_editor_* 全集；命令行 demo |
| **S2 — Editor app 骨架（dong + dong-ui，主 layout 三栏）** | 三栏可调整大小 |
| **S3 — Project 文件树 + open file → canvas 渲染** | 单文件编辑 |
| **S4 — Inspector 双向：选中 canvas 元素 → 改属性 → 写回文件** | 含 inline style 编辑 |
| **S5 — 拖拽：从组件库拖入 + 在 canvas 内拖元素** | 改位置 / 排序 |
| **S6 — Undo/Redo + history** | 必须 |
| **S7 — File watcher 双向同步** | 复用 P1-4 |
| **S8 — Preview 模式 + 多文档 tab** | 完成 |
| **S9 — 文档站 + 教程视频** | 推广 |

---

## 5. 通过验证规则

### 5.1 Hard

| 项 | 阈值 |
|---|---|
| `dong-editor.exe --new-project foo` 在 30 秒内创建可编辑项目 | 必须 |
| 在 canvas 选中元素 → 改 background-color → 100 ms 内 canvas 反映 | 必须 |
| 改动保存后业务方 IDE 看到等价的 source 改动 | 必须 |
| 拖入 dong-ui 组件后正确渲染 | 必须 |
| Undo / Redo 在所有操作上工作 | 单元 |
| 编辑器自身 60 FPS（拖拽时不卡） | perf |
| 关闭编辑器不丢未保存改动（提示保存） | 必须 |
| 项目可由 vanilla / React / Preact 项目模板（来自 P2-6 cli）创建并编辑 | 必须 |
| 编辑器自身用 dong + dong-ui 实现（dogfood） | 必须 |

### 5.2 Soft

| 项 | 阈值 |
|---|---|
| 同时打开 5 个 tab 流畅切换 | 期望 |
| 拖元素重排响应延迟 | < 16 ms |
| 文件保存到 IDE 看到的延迟 | < 200 ms |
| 编辑器启动时间 | < 1 s |
| 文件 > 100 KB 仍能编辑 | 期望 |
| 含 React/Preact 项目编辑 | 期望 |

### 5.3 必须新增的产物

| 产物 | 说明 |
|---|---|
| `dong/dong-editor/` 仓库目录 | 编辑器源码 |
| `dong/dong-editor/dist/dong-editor.exe`（CI 构建） | binary 产物 |
| `dong/include/dong_editor.h` | C ABI |
| `examples/editor_demo_project/` | 示例项目，跟随 editor 安装 |
| `dong/dong-editor/README.md` + 教程 | 用户文档 |

### 5.4 验证命令

```bash
# 启动
dong-editor.exe --new-project /tmp/test
# 期望：创建项目 + 打开默认文件 + canvas 显示

# 端到端 sanity（自动）
python dong/scripts/test_editor_e2e.py
# 自动：创建 → 打开 → 改属性 → 保存 → 关闭 → 重开验证内容
```

---

## 6. 风险与回退

| 风险 | 缓解 |
|---|---|
| 编辑器自身用 dong 写 → dong bug 直接影响编辑体验 | dogfood 是优势：bug 早发现；dong 重大缺陷可临时切外部浏览器渲染 fallback（v2） |
| HTML 源码格式化破坏 git diff | 用 prettier 风格保持；含 noFormat 选项 |
| 拖拽生成的 HTML 不符合业务规范 | 模板可配置 `editor-templates.json` |
| Undo/Redo 与 file watcher 触发冲突 | undo 期间 disable file watcher |
| React JSX 文件 vs HTML 文件双格式 | v1 仅 HTML/Vanilla；JSX 留 v2 |
| 性能：高频选中切换导致 canvas 重渲 | 与 P0-6 damage rect 协同；inspector 改 inline style 走 Style invalidation |
| 中途崩溃丢失未保存编辑 | 自动 30 秒备份 .swp 文件 |

---

## 7. 不在本方案范围

- ❌ JSX 编辑（v1 仅 HTML / Vanilla；JSX 用 IDE）
- ❌ Animation timeline 编辑（@keyframes 拼写没问题，可视化编辑留 v3）
- ❌ Asset pipeline 编辑（图片压缩、atlas 打包；走 P2-8）
- ❌ 协同编辑（多人同时改）
- ❌ 云存储 / 远程项目
- ❌ 完整的代码编辑（用 IDE；编辑器仅可视化）
- ❌ 移动端 editor

---

## 8. 完成后更新

- [ ] `doc/重要特性.md` 新增 § "dong-editor 可视化编辑器"
- [ ] `doc/positioning.md` § 5.2 P2-7 行从"待做"改"已交付"
- [ ] `dong/include/dong_editor.h` 文档化
- [ ] dong 官网首页加 editor 截图与 download
