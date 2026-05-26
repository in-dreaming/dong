# JavaScript API 参考

Dong 在标准 DOM API 之上提供 `dong` 全局对象及若干扩展 API。

## dong 全局对象

### 渲染（Direct Draw）

| API | 说明 |
|-----|------|
| `dong.clearOverlay()` | 清除当前 view 的 overlay 绘制 |
| `dong.drawRect(x, y, w, h, color)` | 绘制矩形 |
| `dong.drawCircle(cx, cy, r, color)` | 绘制圆形 |
| `dong.renderText(text, x, y, options)` | 即时模式文字 |

### Scene Graph

| API | 说明 |
|-----|------|
| `dong.scene.find(name)` | 按名称查找节点 |
| `dong.scene.set(id, prop, val)` | O(1) 属性更新 |
| `dong.scene.on(id, event, fn)` | 注册事件 |

### 导航与多 View

| API | 说明 |
|-----|------|
| `dong.focusNav(direction)` | 编程式空间导航（`'up'`/`'down'`/`'left'`/`'right'`） |
| `dong.getView(name)` | 多 View 跨视图访问 |

### 文字排版

Text layout API 通过 `dong.textLayout` 等绑定暴露，用于障碍物绕排等高级排版。示例见 `data/pretext/`。

## 标准 DOM 扩展（已实现）

| API | 说明 |
|-----|------|
| `queueMicrotask` | React Fiber scheduler |
| `MessageChannel` / `MessagePort` | React scheduler 通信 |
| `localStorage` / `sessionStorage` | 内存实现 |
| `CustomEvent` | 含 `detail` |
| `createDocumentFragment` | Fragment-aware appendChild |
| `performance.now()` | 高精度时间 |
| `crypto.randomUUID()` | UUID 生成 |
| `requestIdleCallback` | 空闲回调 |
| `Element.animate()` | Web Animations API |
| `Fetch API` | JSON 请求 |
| `DOMParser` | HTML 解析 |
| `MutationObserver` | DOM 变更观察 |

## HTML 自定义元素

| 元素 | 说明 |
|------|------|
| `<host-view data-host-view-id="N">` | 宿主引擎渲染区域占位 |

详见 [Dong 扩展](./extensions.md)。

## CSS 私有属性

| 属性 | 说明 |
|------|------|
| `nav-up/down/left/right` | 空间导航显式覆盖 |
| `--dong-hdr-boost` | HDR 亮度增益 |

## data-* 属性

| 属性 | 说明 |
|------|------|
| `data-host-view-id` | 宿主视图 ID |
| `data-nav-trap` | 空间导航限定容器 |
| `data-dong-native-tooltip` | 原生 tooltip hint |

## TypeScript

`dong.d.ts` 提供 IDE 自动补全支持。

## 测试用例

| 领域 | Case |
|------|------|
| ContentEditable | `tests/test_contenteditable_basic.html` |
| 表单 | `tests/test_select_keyboard.html` |
| 空间导航 | `tests/test_spatial_nav_grid.html` |
| host-view | `tests/test_host_view.html` |

完整 Case 索引见 [features-index.md](./features-index.md)。

## 相关文档

- [CSS 子集](./css-subset.md)
- [渲染模式](../guide/render-modes.md)
- [Dong 扩展](./extensions.md)
- [环境变量](./env-vars.md)
