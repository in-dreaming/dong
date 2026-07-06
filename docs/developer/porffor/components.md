# Porffor UI 组件库（T23）

> 规格见 [`framework-spec.md`](framework-spec.md) §7。组件以 **静态 HTML + Porffor 模块** 实现，非运行时包。

## PorfButton

- **props**: `label`（文本）、`color`（背景色）、`onClick`（export 名）
- **DOM**: `<button>` + inline style
- **示例**: `examples/data/porf-counter/index.html`

## PorfText

- **props**: `bind`（状态变量）、`size`、`color`
- **用法**: `data-porf-bind` + 编译器 `porfPatch_*`

## PorfInput

- **props**: `placeholder`、`onInput`、`onKeydown` export
- **示例**: `examples/data/porf-todo-classic/index.html` `#todo-input`

## PorfTodoItem / PorfFilterBar

- **实现**: `porf_todo.js` 内 `porfRebuildList()` + `setInnerHTML` 模板字符串
- **事件**: 列表容器 `click` 委托 → `onListClick`

## PorfHealthBar / PorfScoreDisplay / PorfHotbar / PorfMinimap

- **实现**: `examples/data/porf-game-ui/index.html` 静态结构 + `porf_game_ui.js` 绑定
- **动态**: HP/MP 条宽度、`setInterval('onScoreTick', 1000)` 分数 tick

## 示例入口

| 对标 Preact | Porffor 路径 |
|-------------|--------------|
| preact-counter | `data/porf-counter/index.html` |
| preact-todo-classic | `data/porf-todo-classic/index.html` |
| preact-game-ui | `data/porf-game-ui/index.html` |

运行（Porffor 构建后）:

```powershell
.\zig-out\bin\dong_app.exe --html data\porf-counter\index.html
.\zig-out\bin\dong_app.exe --html data\porf-todo-classic\index.html
.\zig-out\bin\dong_app.exe --html data\porf-game-ui\index.html
```
