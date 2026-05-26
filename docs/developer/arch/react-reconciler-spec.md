# React-Dong: 基于 react-reconciler 的自定义渲染器

## 1. 概述

Dong 引擎内嵌 QuickJS 并实现了浏览器级 DOM/CSS/Layout 管线。
借助 `react-reconciler`，可以将 React 组件模型直接映射到 Dong 的 DOM 树，
让用户用 JSX 编写游戏 UI，同时享受 React 生态（hooks、状态管理、组件库）。

这条路径已被多个项目验证：React Ink（终端）、React Native（移动端）、
React Three Fiber（3D）、React UMG（Unreal Engine）。

## 2. 架构分层

```
┌─────────────────────────────────────────────────────┐
│  用户 JSX 组件 (App.jsx, components/*.jsx)          │  ← 开发者编写
├─────────────────────────────────────────────────────┤
│  react + react-reconciler (~40KB minified)          │  ← npm 包，随 bundle 打入
├─────────────────────────────────────────────────────┤
│  dong-react-renderer.js (Host Config 适配层)        │  ← 本项目实现，~200 行
├─────────────────────────────────────────────────────┤
│  Dong DOM API (QuickJS JS bindings)                 │  ← dong.dll 已有
│  document.createElement / appendChild / style ...   │
├─────────────────────────────────────────────────────┤
│  Dong Core: DOM → CSS → Yoga Layout → Display List  │  ← 无需修改
├─────────────────────────────────────────────────────┤
│  GPU Rendering (SDL / D3D12 backend)                │  ← 无需修改
└─────────────────────────────────────────────────────┘
```

数据流向：

```
JSX render() → reconciler diff → Host Config 调用 → Dong DOM 操作
                                                      ↓
                                               markLayoutDirty()
                                                      ↓
                                               Yoga 重新布局
                                                      ↓
                                               Painter 重建 DisplayList
                                                      ↓
                                               GPU 渲染新帧
```

核心设计决策：
- **react-reconciler 而非 ReactDOM**：跳过浏览器 quirks（合成事件、legacy context、DOM diffing），直接操作 Dong 原生 DOM
- **JS 侧 bundle 打包**：用户代码、React、Host Config 在外部用 esbuild 打成单个 IIFE，通过 `<script src>` 加载到 QuickJS
- **Dong Core 零修改**：所有适配在 JS 层完成；仅需补齐少量 JS binding（见第 4 节）

## 3. Host Config 完整规格

`react-reconciler` 要求宿主实现约 25 个函数。下表逐一映射到 Dong DOM API。

### 3.1 核心树操作

| Host Config 函数 | 实现 | Dong API |
|---|---|---|
| `createInstance(type, props, root, ctx, fiber)` | 创建元素节点，设置初始属性 | `document.createElement(type)` + `setAttribute` / `style.setProperty` |
| `createTextInstance(text, root, ctx, fiber)` | 创建文本节点 | `document.createTextNode(text)` |
| `appendInitialChild(parent, child)` | 构建阶段追加子节点 | `parent.appendChild(child)` |
| `appendChild(parent, child)` | 提交阶段追加子节点 | `parent.appendChild(child)` |
| `appendChildToContainer(container, child)` | 追加到根容器 | `container.appendChild(child)` |
| `removeChild(parent, child)` | 移除子节点 | `parent.removeChild(child)` |
| `removeChildFromContainer(container, child)` | 从根容器移除 | `container.removeChild(child)` |
| `insertBefore(parent, child, before)` | 在指定节点前插入 | `parent.insertBefore(child, before)` |
| `insertInContainerBefore(container, child, before)` | 在根容器中指定位置插入 | `container.insertBefore(child, before)` |
| `clearContainer(container)` | 清空容器 | `container.innerHTML = ''` |

### 3.2 属性更新

| Host Config 函数 | 实现 |
|---|---|
| `prepareUpdate(node, type, oldProps, newProps)` | 对比 props，返回需要更新的字段列表（纯 JS diff） |
| `commitUpdate(node, payload, type, oldProps, newProps)` | 应用 payload：逐字段调用 `setAttribute` / `style.setProperty` / `removeAttribute` |
| `commitTextUpdate(node, oldText, newText)` | `node.textContent = newText` |
| `resetTextContent(node)` | `node.textContent = ''` |
| `shouldSetTextContent(type, props)` | `return typeof props.children === 'string' \|\| typeof props.children === 'number'` |
| `finalizeInitialChildren(node, type, props)` | 设置需要在 commit 阶段处理的属性（如 `autoFocus`），返回 boolean |

### 3.3 调度与容器

| Host Config 函数 | 实现 |
|---|---|
| `getRootHostContext(rootContainer)` | 返回 `null`（无命名空间区分） |
| `getChildHostContext(parentCtx, type)` | 返回 `parentCtx`（透传） |
| `getPublicInstance(instance)` | 返回 `instance`（Dong DOM 节点即公开实例） |
| `prepareForCommit(container)` | noop，返回 `null` |
| `resetAfterCommit(container)` | noop（Dong 自动重新布局/渲染） |
| `scheduleTimeout(fn, delay)` | `setTimeout(fn, delay)` |
| `cancelTimeout(id)` | `clearTimeout(id)` |
| `noTimeout` | `-1` |
| `isPrimaryRenderer` | `true` |
| `supportsMutation` | `true` |
| `supportsPersistence` | `false` |
| `supportsHydration` | `false` |
| `getCurrentEventPriority()` | 返回 `DefaultEventPriority`（`0b0000000000000000000000000100000`） |
| `getInstanceFromNode(node)` | 从 DOM 节点取回 fiber（可选，高级事件用） |
| `beforeActiveInstanceBlur()` | noop |
| `afterActiveInstanceBlur()` | noop |
| `prepareScopeUpdate()` | noop |
| `getInstanceFromScope()` | noop |
| `detachDeletedInstance(node)` | noop（GC 自动回收） |

### 3.4 props 应用逻辑

`commitUpdate` 中的 props diff 策略：

```javascript
function applyProps(node, key, value) {
    if (key === 'children') {                      // 文本子节点由 shouldSetTextContent 控制
        if (typeof value === 'string' || typeof value === 'number')
            node.textContent = String(value);
        return;
    }
    if (key === 'style') {
        for (const [k, v] of Object.entries(value || {}))
            node.style.setProperty(k, String(v));
        return;
    }
    if (key === 'className') {
        node.setAttribute('class', value || '');
        return;
    }
    if (key.startsWith('on')) {
        // 事件处理，见第 6 节
        return;
    }
    if (value === null || value === undefined || value === false) {
        node.removeAttribute(key);
    } else {
        node.setAttribute(key, String(value));
    }
}
```

## 4. Dong DOM API 差距分析与补齐清单

### 4.1 已有 API（react-reconciler 直接可用）

| 类别 | API | 来源 |
|---|---|---|
| 节点创建 | `document.createElement`, `document.createTextNode` | `js_bindings.cpp` |
| 树操作 | `appendChild`, `removeChild`, `insertBefore` | `js_node_bindings.cpp` |
| 属性 | `setAttribute`, `getAttribute`, `removeAttribute`, `hasAttribute` | `js_node_bindings.cpp` |
| 样式 | `style.setProperty`, `style.getPropertyValue`, `style.removeProperty` | `js_bindings.cpp` |
| 内容 | `textContent` (get/set), `innerHTML` (get/set) | `js_node_bindings.cpp` |
| 查询 | `getElementById`, `querySelector`, `querySelectorAll` | `js_bindings.cpp` |
| 事件 | `addEventListener`, `removeEventListener`, `dispatchEvent` | `js_bindings.cpp` |
| 定时器 | `setTimeout`, `clearTimeout`, `setInterval`, `clearInterval` | `js_bindings.cpp` |
| 类列表 | `classList.add/remove/toggle/contains` | `js_node_bindings.cpp` |
| 遍历 | `parentNode`, `childNodes`, `children`, `firstChild`, `lastChild` | `js_node_bindings.cpp` |
| 几何 | `getBoundingClientRect`, `scrollTop/Left`, `offsetWidth/Height` | `js_node_bindings.cpp` |
| 焦点 | `focus()`, `blur()` | `js_bindings.cpp` |
| Observer | `MutationObserver`, `ResizeObserver`, `IntersectionObserver` | `js_observer_bindings.cpp` |
| 构造函数 | `Event`, `MouseEvent`, `KeyboardEvent` | `js_bindings.cpp` |
| ES 内置 | `Promise`, `Map`, `Set`, `Symbol`, `Object.is` | QuickJS 引擎内置 |

### 4.2 需补齐：调度器 polyfill（优先级：高）

React scheduler 依赖以下 API 选择调度策略（按降级顺序）：

| API | 用途 | 补齐方案 |
|---|---|---|
| `requestAnimationFrame` | 帧对齐调度 | **C++ 实现**：在 `js_bindings.cpp` 注册全局函数，回调列表存入 `JSBindings`，在 `EngineView::tick()` 中批量执行（与 `tickTimers` 模式一致） |
| `cancelAnimationFrame` | 取消 rAF 回调 | 配套实现，通过 ID 从回调列表移除 |
| `MessageChannel` | 微任务调度 | **JS polyfill**：`setTimeout(fn, 0)` 足够（React scheduler 已有此降级路径） |
| `queueMicrotask` | 微任务入队 | **JS polyfill**：`globalThis.queueMicrotask = (fn) => Promise.resolve().then(fn)` |

`requestAnimationFrame` 实现要点：

```cpp
// js_bindings.cpp 中添加

// 回调存储
struct RAFCallback { int32_t id; JSValue func; };
std::vector<RAFCallback> raf_callbacks_;
int32_t next_raf_id_ = 1;

// JS 函数注册
JSValue js_requestAnimationFrame(JSContext* ctx, JSValueConst this_val,
                                  int argc, JSValueConst* argv) {
    // 将 argv[0] (function) 存入 raf_callbacks_，返回 id
}

// 在 EngineView::tick() 中调用
void JSBindings::tickAnimationFrames(double timestamp) {
    auto callbacks = std::move(raf_callbacks_);  // swap 避免递归问题
    for (auto& cb : callbacks) {
        JSValue ts = JS_NewFloat64(ctx_, timestamp);
        JS_Call(ctx_, cb.func, JS_UNDEFINED, 1, &ts);
        JS_FreeValue(ctx_, ts);
        JS_FreeValue(ctx_, cb.func);
    }
}
```

### 4.3 需补齐：JS binding（优先级：中）

这些 API 在 C++ DOM 层已实现，仅缺少 JS 绑定：

| API | C++ 实现位置 | 绑定方式 |
|---|---|---|
| `node.replaceChild(new, old)` | `DOMNode::replaceChild` (`dom_node.cpp`) | 在 `js_node_bindings.cpp` 的 `bindNodeProperties` 中添加 `JS_NewCFunction` |
| `node.compareDocumentPosition(other)` | `DOMNode::compareDocumentPosition` (`dom_node.cpp:373`) | 同上 |
| `node.ownerDocument` | 返回 `document` 对象 | 在 `bindNodeProperties` 中添加 getter |

### 4.4 需补齐：新增实现（优先级：中）

| API | 用途 | 实现方案 |
|---|---|---|
| `document.createComment(text)` | React 用作占位符节点（Suspense fallback 等） | 在 `dom_node.cpp` 中支持 `NodeType::COMMENT`，内容不参与渲染、不参与布局（Yoga 不创建节点） |
| `node.isConnected` | 检查节点是否在文档树中 | getter：从当前节点沿 `parentNode` 链向上查找，到达 `document` 则为 `true` |

### 4.5 可选补齐（优先级：低）

以下 API 不是 react-reconciler 必需的，但部分 React 生态库可能用到：

| API | 场景 |
|---|---|
| `document.createDocumentFragment` | 批量 DOM 操作优化（reconciler 不用，但用户代码可能用） |
| `element.namespaceURI` | SVG 元素支持（Dong 暂无 SVG 渲染） |
| `window.getSelection` | 文本选择 API（已有 `initializeSelectionAPI`，需确认覆盖度） |

## 5. 构建管线

### 5.1 工具链

```
┌──────────────────────────────────────────────────────────────────┐
│ 开发环境 (Node.js)                                               │
│                                                                  │
│  src/                                                            │
│  ├── dong-react-renderer.js    ← Host Config 适配层              │
│  ├── polyfills.js              ← rAF/queueMicrotask polyfill     │
│  ├── App.jsx                   ← 用户根组件                      │
│  └── components/*.jsx          ← 用户组件                        │
│                                                                  │
│  package.json                                                    │
│  ├── react: "^18.3.0"                                            │
│  ├── react-reconciler: "^0.29.0"                                 │
│  └── esbuild: "^0.21.0"                                          │
│                                                                  │
│  esbuild --bundle src/main.jsx --format=iife --outfile=bundle.js │
└──────────────────────┬───────────────────────────────────────────┘
                       │ 产出
                       ▼
              bundle.js (~60-80KB minified)
                       │
                       ▼
┌──────────────────────────────────────────────────────────────────┐
│ Dong 运行时                                                      │
│                                                                  │
│  <html>                                                          │
│    <body>                                                        │
│      <div id="root"></div>                                       │
│      <script src="bundle.js"></script>                           │
│    </body>                                                       │
│  </html>                                                         │
│                                                                  │
│  QuickJS eval("...bundle content...")                             │
│  → React reconciler 启动                                         │
│  → Host Config 调用 Dong DOM API                                  │
│  → Dong 渲染管线出图                                              │
└──────────────────────────────────────────────────────────────────┘
```

### 5.2 esbuild 配置

```javascript
// build.mjs
import { build } from 'esbuild';

await build({
    entryPoints: ['src/main.jsx'],
    bundle: true,
    format: 'iife',
    outfile: 'dist/bundle.js',
    minify: true,
    target: 'es2020',          // QuickJS 支持 ES2021
    jsx: 'automatic',
    define: {
        'process.env.NODE_ENV': '"production"',
    },
});
```

### 5.3 注意事项

- **脚本超时**：`DONG_SCRIPT_TIMEOUT_MS` 默认 2000ms，大 bundle 首次 eval 可能超时。设为 `0`（禁用）或 `10000`+
- **微任务上限**：`ScriptEngine::processPendingTasks` 单次最多 1000 个 job，React scheduler 正常使用不会触及
- **无 ES modules**：QuickJS 在 Dong 中仅使用 `JS_EVAL_TYPE_GLOBAL`，因此必须打包成 IIFE

## 6. 事件系统集成

### 6.1 推荐方案：直接使用 Dong 原生事件

Dong 已有完整的事件系统（冒泡、捕获、`stopPropagation`），无需重新实现合成事件。

在 Host Config 的 `commitUpdate` / `finalizeInitialChildren` 中处理 `on*` props：

```javascript
const EVENT_MAP = {
    onClick:       'click',
    onMouseDown:   'mousedown',
    onMouseUp:     'mouseup',
    onMouseMove:   'mousemove',
    onMouseEnter:  'mouseenter',
    onMouseLeave:  'mouseleave',
    onKeyDown:     'keydown',
    onKeyUp:       'keyup',
    onInput:       'input',
    onChange:       'change',
    onFocus:       'focus',
    onBlur:        'blur',
    onScroll:      'scroll',
};

// 每个 DOM 节点上挂一个 Map 记录当前注册的 handler
const LISTENER_KEY = Symbol('dongListeners');

function updateEventListener(node, propKey, oldHandler, newHandler) {
    const eventName = EVENT_MAP[propKey];
    if (!eventName) return;

    if (!node[LISTENER_KEY]) node[LISTENER_KEY] = {};

    if (oldHandler) {
        node.removeEventListener(eventName, node[LISTENER_KEY][propKey]);
    }
    if (newHandler) {
        node[LISTENER_KEY][propKey] = newHandler;
        node.addEventListener(eventName, newHandler);
    }
}
```

### 6.2 为什么不做事件委托

React DOM 内部使用根节点事件委托（所有事件监听在 root 上，通过 fiber 树分发）。
对 Dong 来说没有必要：

- Dong 的事件冒泡已经是 C++ 原生实现，性能远高于 JS 重新分发
- 事件委托需要 `getInstanceFromNode`（从 DOM 节点反查 fiber），增加耦合
- 简单的 per-node `addEventListener` 完全够用（游戏 UI 事件密度远低于 Web 应用）

## 7. 完整示例

### 7.1 dong-react-renderer.js

```javascript
import Reconciler from 'react-reconciler';
import { DefaultEventPriority } from 'react-reconciler/constants';

function applyProps(node, key, value) {
    if (key === 'children') {
        if (typeof value === 'string' || typeof value === 'number') {
            node.textContent = String(value);
        }
        return;
    }
    if (key === 'style' && typeof value === 'object') {
        for (const [k, v] of Object.entries(value || {})) {
            const cssProp = k.replace(/[A-Z]/g, m => '-' + m.toLowerCase());
            node.style.setProperty(cssProp, String(v));
        }
        return;
    }
    if (key === 'className') {
        node.setAttribute('class', value || '');
        return;
    }
    if (key.startsWith('on') && typeof value === 'function') {
        const event = key.slice(2).toLowerCase();
        if (!node.__listeners) node.__listeners = {};
        if (node.__listeners[key]) {
            node.removeEventListener(event, node.__listeners[key]);
        }
        node.__listeners[key] = value;
        node.addEventListener(event, value);
        return;
    }
    if (value === null || value === undefined || value === false) {
        node.removeAttribute(key);
    } else {
        node.setAttribute(key, String(value));
    }
}

const hostConfig = {
    supportsMutation: true,
    supportsPersistence: false,
    supportsHydration: false,
    isPrimaryRenderer: true,
    noTimeout: -1,

    createInstance(type, props) {
        const el = document.createElement(type);
        for (const [key, value] of Object.entries(props)) {
            applyProps(el, key, value);
        }
        return el;
    },

    createTextInstance(text) {
        return document.createTextNode(text);
    },

    appendInitialChild(parent, child) { parent.appendChild(child); },
    appendChild(parent, child) { parent.appendChild(child); },
    appendChildToContainer(container, child) { container.appendChild(child); },

    removeChild(parent, child) { parent.removeChild(child); },
    removeChildFromContainer(container, child) { container.removeChild(child); },

    insertBefore(parent, child, before) { parent.insertBefore(child, before); },
    insertInContainerBefore(container, child, before) {
        container.insertBefore(child, before);
    },

    prepareUpdate(node, type, oldProps, newProps) {
        const payload = [];
        const allKeys = new Set([
            ...Object.keys(oldProps),
            ...Object.keys(newProps),
        ]);
        for (const key of allKeys) {
            if (key === 'children') continue;
            if (oldProps[key] !== newProps[key]) {
                payload.push(key);
            }
        }
        return payload.length > 0 ? payload : null;
    },

    commitUpdate(node, payload, type, oldProps, newProps) {
        for (const key of payload) {
            if (!(key in newProps) || newProps[key] === undefined) {
                if (key.startsWith('on') && node.__listeners && node.__listeners[key]) {
                    const event = key.slice(2).toLowerCase();
                    node.removeEventListener(event, node.__listeners[key]);
                    delete node.__listeners[key];
                } else {
                    node.removeAttribute(key);
                }
            } else {
                applyProps(node, key, newProps[key]);
            }
        }
    },

    commitTextUpdate(node, oldText, newText) {
        node.textContent = newText;
    },

    resetTextContent(node) { node.textContent = ''; },

    shouldSetTextContent(type, props) {
        return typeof props.children === 'string'
            || typeof props.children === 'number';
    },

    clearContainer(container) { container.innerHTML = ''; },

    getRootHostContext() { return null; },
    getChildHostContext(parentCtx) { return parentCtx; },
    getPublicInstance(instance) { return instance; },
    prepareForCommit() { return null; },
    resetAfterCommit() {},

    finalizeInitialChildren(node, type, props) {
        if (props.autoFocus) {
            setTimeout(() => node.focus(), 0);
            return true;
        }
        return false;
    },

    scheduleTimeout: setTimeout,
    cancelTimeout: clearTimeout,
    getCurrentEventPriority() { return DefaultEventPriority; },

    beforeActiveInstanceBlur() {},
    afterActiveInstanceBlur() {},
    prepareScopeUpdate() {},
    getInstanceFromScope() { return null; },
    detachDeletedInstance() {},
};

const reconciler = Reconciler(hostConfig);

export function render(element, container) {
    let root = container.__reactRoot;
    if (!root) {
        root = reconciler.createContainer(
            container,   // containerInfo
            0,           // tag: LegacyRoot
            null,        // hydrationCallbacks
            false,       // isStrictMode
            null,        // concurrentUpdatesByDefaultOverride
            '',          // identifierPrefix
            (err) => console.error(err),  // onRecoverableError
            null,        // transitionCallbacks
        );
        container.__reactRoot = root;
    }
    reconciler.updateContainer(element, root, null, null);
    return root;
}
```

### 7.2 polyfills.js（调度器所需）

```javascript
// requestAnimationFrame polyfill（若 C++ 侧未实现）
if (typeof globalThis.requestAnimationFrame === 'undefined') {
    let rafId = 0;
    globalThis.requestAnimationFrame = function(cb) {
        return setTimeout(() => cb(performance.now()), 16);
    };
    globalThis.cancelAnimationFrame = function(id) {
        clearTimeout(id);
    };
}

// queueMicrotask polyfill
if (typeof globalThis.queueMicrotask === 'undefined') {
    globalThis.queueMicrotask = function(fn) {
        Promise.resolve().then(fn);
    };
}
```

### 7.3 用户应用示例 (main.jsx)

```jsx
import React, { useState } from 'react';
import { render } from './dong-react-renderer';
import './polyfills';

function Counter() {
    const [count, setCount] = useState(0);

    return (
        <div style={{
            display: 'flex',
            flexDirection: 'column',
            alignItems: 'center',
            padding: '20px',
        }}>
            <h1 style={{ color: '#333', fontSize: '24px' }}>
                Count: {count}
            </h1>
            <button
                onClick={() => setCount(c => c + 1)}
                style={{
                    padding: '10px 20px',
                    backgroundColor: '#4CAF50',
                    color: 'white',
                    border: 'none',
                    borderRadius: '4px',
                    cursor: 'pointer',
                    fontSize: '16px',
                }}
            >
                Increment
            </button>
        </div>
    );
}

function App() {
    return (
        <div style={{ fontFamily: 'sans-serif' }}>
            <Counter />
        </div>
    );
}

render(<App />, document.getElementById('root'));
```

### 7.4 Dong 侧 HTML

```html
<!DOCTYPE html>
<html>
<head>
    <style>
        body { margin: 0; background: #f5f5f5; }
        #root { width: 100%; height: 100%; }
    </style>
</head>
<body>
    <div id="root"></div>
    <script src="bundle.js"></script>
</body>
</html>
```

### 7.5 构建与运行

```bash
# 安装依赖
npm init -y
npm install react react-reconciler esbuild

# 打包
npx esbuild src/main.jsx --bundle --format=iife \
    --outfile=dist/bundle.js --minify --target=es2020 \
    --jsx=automatic --define:process.env.NODE_ENV=\"production\"

# 运行（将 HTML 和 bundle.js 放入 Dong 资源目录）
# 环境变量调大脚本超时
set DONG_SCRIPT_TIMEOUT_MS=10000
zig-out/bin/minimal_dong_demo
```

## 8. 性能考量

### 8.1 运行时开销

| 环节 | 预估 | 说明 |
|---|---|---|
| Bundle eval（首次） | 100-500ms | ~60-80KB JS 在 QuickJS 中编译+执行，受 CPU 单核速度影响 |
| React reconciler diff | <1ms（小树）/ 5-20ms（大树） | QuickJS 解释器约为 V8 JIT 的 1/10-1/50，但游戏 UI 通常节点数 <500 |
| DOM 操作 | <0.1ms/次 | JS→C++ 跨界调用，直接操作内存中的 DOM 树 |
| Yoga 布局 | 1-5ms | 仅脏节点重算，与 React 无关 |
| GPU 渲染 | <2ms | Display list 执行，与 React 无关 |

### 8.2 优化策略

- **避免大型组件树**：游戏 UI 通常 100-300 个节点，reconciler diff 在 QuickJS 上完全可接受
- **useMemo/useCallback**：减少不必要的 reconciler diff
- **脚本超时**：设置 `DONG_SCRIPT_TIMEOUT_MS=0` 禁用超时限制
- **requestAnimationFrame 原生实现**：比 `setTimeout(fn, 16)` polyfill 更精确，避免帧抖动
- **批量更新**：React 18 自动批量状态更新，减少 DOM 操作次数

### 8.3 内存预算

| 组件 | 增量 |
|---|---|
| React + reconciler JS heap | ~2-5MB |
| 每组件 fiber 节点 | ~1KB |
| 500 节点应用 | ~3-6MB 总 JS 内存增量 |

## 9. 与 Preact 对比

| 维度 | react-reconciler | Preact |
|---|---|---|
| 包体大小 | ~40KB (react + reconciler) | ~3KB |
| 生态兼容 | 完整 React 生态 | 需要 preact/compat（+5KB） |
| Hooks | 完整支持 | 完整支持 |
| Concurrent features | 支持（Suspense、Transition） | 不支持 |
| QuickJS 性能 | reconciler diff 较重 | 更轻量 |
| 实现复杂度 | ~200 行 Host Config | 直接跑在 DOM 上，零适配 |

**结论**：如果不需要 React 并发特性（Suspense/Transition），Preact 是更轻量的选择。
如果需要完整 React 生态兼容（第三方组件库、状态管理），选 react-reconciler。

## 10. 实施路线

### Phase 1：最小可用 [DONE]

1. [x] 编写 `dong-react-renderer.js`（Host Config 适配层）→ `dong/react/src/dong-react-renderer.js`
2. [x] 编写 JS polyfill（rAF、queueMicrotask）→ `dong/react/src/polyfills.js`
3. [x] 用 esbuild 打包 Counter 示例 → `dong/react/examples/counter/`
4. [x] Game UI 示例（血条、分数、小地图、物品栏）→ `dong/react/examples/game-ui/`
5. [ ] 在 Dong 中加载运行，验证基本渲染和事件

### Phase 2：补齐 Dong DOM binding [DONE]

1. [x] C++ 侧实现 `requestAnimationFrame` / `cancelAnimationFrame` → `js_bindings.hpp/cpp`
2. [x] `tickAnimationFrames` 在 `engine_view.cpp` tick 中调用
3. [x] 添加 `document.createComment` 支持 → `js_bindings.cpp`
4. [x] 添加 `node.replaceChild` JS binding → `js_node_bindings.cpp`
5. [x] 添加 `node.ownerDocument` getter → `js_node_bindings.cpp`
6. [x] 添加 `node.isConnected` getter → `js_node_bindings.cpp`

### Phase 3：生产就绪（待做）

1. 性能 profiling（QuickJS 中 reconciler diff 耗时）
2. 更多事件类型支持（wheel、touch、pointer）
3. CSS-in-JS 方案验证（styled-components 等是否可在 QuickJS 运行）
4. 开发工具支持（React DevTools 协议适配，可选）
5. 文档与示例完善

## 10.1 使用模式管理

Dong 支持两种 UI 开发模式，共享同一个 Core 引擎：

```
用户选择路径
├── 直接 Dong（现有方式，零依赖）
│   写 HTML/CSS/JS → dong_app_load_html[_file] → Dong 渲染
│
└── React+Dong（需要 Node.js + npm）
    写 JSX → npm run build → bundle.js → <script src> → Dong 渲染
```

关键点：**Dong Core 不依赖 React**。React 是纯 JS 层插件，
通过 `react-reconciler` Host Config 调用 Dong 已有的 DOM API。
两种模式在引擎层面完全相同，差异仅在用户侧的开发工具链。

项目结构：
```
dong/
├── react/                 ← React 渲染器（npm 包，可选）
│   ├── src/               ← Host Config + polyfills
│   ├── examples/          ← React 示例
│   └── build.mjs          ← esbuild 打包脚本
├── examples/              ← 直接 Dong 示例（HTML/CSS/JS）
└── src/                   ← Dong Core（两种模式共用）
```

## 11. 参考实现

| 项目 | 宿主环境 | Host Config 行数 | 参考价值 |
|---|---|---|---|
| [React Ink](https://github.com/vadimdemedes/ink) | 终端 (Node.js) | ~300 行 | 最接近的非浏览器渲染器 |
| [React Three Fiber](https://github.com/pmndrs/react-three-fiber) | Three.js (WebGL) | ~500 行 | 3D 场景渲染参考 |
| [React Native](https://github.com/facebook/react-native) | iOS/Android | ~2000 行 | 完整移动端方案 |
| [React UMG](https://github.com/nicktrigger/react-umg) | Unreal Engine | ~400 行 | 游戏引擎集成参考 |
| [react-reconciler docs](https://github.com/facebook/react/tree/main/packages/react-reconciler) | — | — | 官方 Host Config 接口文档 |
