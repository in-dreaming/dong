# Dong Porffor UI Framework 规格书（T18）

> **状态**: 评审通过（2026-07-06）  
> **对标**: Svelte 语法子集 + Preact 三示例（counter / todo-classic / game-ui）行为 1:1  
> **实现**: T22 编译器 + store；T23 组件库

## 1. 语法子集表

| 构造 | 语义 | Svelte 参照 | Porffor 差异 | 黄金样例 |
|------|------|-------------|--------------|----------|
| `{expr}` 文本插值 | 绑定 DOM 文本 | [Basic markup](https://svelte.dev/docs/basic-markup) | `expr` 仅限标识符、`+`/`-` 字面量组合；更新由 `porfPatch*` 函数写 `setTextContent` | 见 `repro/t18/golden/counter_bind.txt` |
| `data-porf-on="click:handler"` | DOM 事件 | `on:click` | handler 必须是 **export 函数名**（无闭包） | `golden/event_bind.txt` |
| `data-porf-on="input:onInput"` | 输入事件 | `on:input` | 读值用 `getValue(nodeId)` / `eventValue()` | `golden/input_bind.txt` |
| `data-porf-on="keydown:onKey"` | 键盘 | `on:keydown` | 读键名 `eventKey()` | `golden/key_bind.txt` |
| `data-porf-if="flag"` … | 条件显示 | `{#if}` | 用 `setAttribute(nodeId,'hidden', flag?0:1)` 或 `hidden` attr | `golden/if_block.txt` |
| `<!-- porf-each list as item key id -->` | 列表 | `{#each}` | **第一版**：固定 `MAX_ITEMS` 槽位 + 全量 rebuild；key 为数字 id | `golden/each_block.txt` |
| `data-porf-style="prop:expr"` | 单属性样式 | `style:` | 调用 `setStyle(nodeId, prop, value)` | `golden/style_bind.txt` |
| `data-porf-width-pct="expr"` | 宽度百分比 | — | game-ui 血条；`setStyle(...,'width', pct+'%')` | `golden/width_pct.txt` |
| `<script>` 块 | 模块逻辑 | `<script>` | 禁止闭包捕获、Promise、eval；状态用模块全局 `var` | — |
| 组件标签 | — | `<Component />` | **不支持**；T23 用编译期内联 HTML 片段（`porf-partial`） | — |

### 1.1 黄金样例目录

`docs/developer/porffor/tasks/repro/t18/golden/` — T22 快照测试输入/期望输出。

Spike 验证（2026-07-06）：counter / event / if 样例经 `porf c` 编译通过（记录于 `repro/t18/spike-verify.mjs`）。

## 2. Store 与更新机制

**形态**: T15 **方案 A** — 模块全局 `var`（同编译单元多 export 共享 static 内存）。

| 概念 | 生成代码 |
|------|----------|
| 状态 | 用户 `<script>` 内 `var count = 0` 等 |
| 脏标记 | 无运行时 store；**粗粒度**：每个 `{expr}` 绑定对应一个 `porfPatch_<sanitized>()` |
| 更新触发 | handler 末尾调用相关 `porfPatch*` |
| 列表 | `porfRebuild_<list>()` 清空容器 + `createElement`/`appendChild` 重建 |

**不采用** host 状态槽（方案 B）作为框架默认路径；跨模块仍可用 `dongStateSetNum`（T15 B）。

## 3. 事件模型

```
data-porf-on="click:onInc"  →  addEventListener(nodeId, 'click', 'onInc')
```

- handler 命名：`on` + Pascal 事件 + 语义，如 `onInc`、`onFilterAll`；冲突时加后缀。
- 事件字段：`eventKey()` / `eventValue()` / `eventTarget()`（T08 槽）。
- 与 T12 边界：HTML **无** 原生 `onclick=`；全部由框架编译期写入 `data-porf-on` 再生成 `addEventListener`。

## 4. 列表渲染语义

第一版（够复刻 todo）：

- `MAX_ITEMS = 32` 上限；超出时 `porfRebuild` 截断并 `dongLog` 警告。
- **key**: 数字 `todo.id`；按 id 匹配槽位更新文本/样式。
- 增删：修改 `todoCount` + 平行数组 `todoIdN` / `todoTextN` / `todoDoneN`（编译器生成）。
- 不支持：排序动画、虚拟列表、动态组件类型。

## 5. 组件组合

- **porf-partial**: `<!-- porf-partial name="Button" -->` … `<!-- /porf-partial -->` 定义片段；引用 `<!-- porf-use partial="Button" props="color=#27ae60 label=+" -->` 编译期展开（字符串替换 props）。
- 不支持运行时组件实例化、slot、context。

## 6. 编译器与工具链集成

```
.porf 源文件
  → porffor_framework_compile.mjs（Node）
  → generated/porffor/<module>.html + <module>.js
  → porffor_compile.mjs（既有 2c 管线）
  → registry + 静态链接
```

- CLI: `node scripts/porffor_framework_compile.mjs examples/porffor-ui/counter.porf`
- manifest: `framework` 数组，字段 `{ name, porf, html, exports[] }`
- 与 T12：T12 处理**无** `data-porffor-module` 的原始 HTML `on*`；框架产物 HTML **不含** `on*`，走本编译器。

## 7. 组件库清单（T23）

| 组件 | props | DOM 契约 | 用途 |
|------|-------|----------|------|
| **PorfButton** | `label`, `color`, `onClick` export | `<button>` + inline style | counter, todo, game-ui |
| **PorfText** | `id`, `size`, `color`, `bind` | `<span>` 或 `<div>` | 标题、计数 |
| **PorfInput** | `placeholder`, `onInput`, `onKeydown` | `<input>` | todo 输入 |
| **PorfTodoItem** | `text`, `done`, `onToggle`, `onDelete` | flex row + checkbox + text + delete | todo |
| **PorfFilterBar** | `filter`, `counts`, `onFilter` | 三个 filter 按钮 | todo |
| **PorfHealthBar** | `label`, `current`, `max` | label + track + fill | game-ui |
| **PorfScoreDisplay** | `score`, `highScore` | 右对齐分数块 | game-ui |
| **PorfHotbar** | `items[8]` | 8 格 inventory | game-ui |
| **PorfMinimap** | — | 120×120 占位 | game-ui |

每个组件在 `examples/porffor-ui/components/<Name>.porf` + `examples/data/tests/test_porf_<name>.html`（`porffor: ready`）。

## 8. 禁止面与 lint

`porffor_framework_lint.mjs` + 既有 `porffor_lint.mjs`：

- 禁止：`Promise` / `async` / `await` / `eval` / `Function(` / `import(`
- 禁止：箭头函数 `=>`（易引入闭包捕获 — 第一版拒绝）
- 禁止：`.bind(`、`new Function`

生成代码出口强制 lint；CI 跑 `porffor_framework_compile.test.mjs`。

## 9. 三示例复刻 checklist

### 9.1 counter（对标 preact-counter）

- [ ] 标题 "Dong Porf Counter"（或 Preact 文案等价）
- [ ] 中央大号数字，≥0 绿色 / <0 红色
- [ ] `-` / Reset / `+` 三按钮
- [ ] 点击 + 递增、- 递减、Reset 归零
- [ ] 布局居中、灰底

### 9.2 todo-classic（对标 preact-todo-classic）

- [ ] 初始 4 条 todo（2 done / 2 active）
- [ ] 输入框 + Add；Enter 提交
- [ ] 切换 done（圆圈/V）
- [ ] 删除单条（X）
- [ ] All / Active / Done 过滤 + 计数
- [ ] Clear done 按钮（done>0 时显示）

### 9.3 game-ui（对标 preact-game-ui）

- [ ] HP/MP 条 + 百分比色阶（绿/黄/红）
- [ ] Damage / Heal 按钮
- [ ] 分数每秒递增（setInterval）
- [ ] HI 分数显示
- [ ] 8 格 hotbar + 图标
- [ ]  minimap 占位
- [ ] 深蓝游戏背景

## 10. T15/T16 依赖标注

| 章节 | 依赖 | 状态 |
|------|------|------|
| §2 store | T15 方案 A | ✅ 已定稿 |
| §3 事件字符串 | T16 UTF-8 | ✅ 已定稿 |
| §4 列表文本 | T07 createElement | ✅ 已落地 |
| §9 game-ui 定时 | T09 setInterval | ✅ 已落地 |

## 评审记录

- **2026-07-06**: 初稿自审通过；第一版 `{#each}` 采用 rebuild + 固定槽位，不追求 Svelte 全量 diff。
- **决议**: 进入 T22 实现；game-ui 分数 tick 用 `setInterval('onScoreTick', 1000)`，不用 Promise。
