目标：**可跨窗口 / 跨进程 / 跨应用 / 支持 OS 文件拖入拖出**，同时保持实现复杂度可控、性能可控、可扩展。

这不是 HTML5 全量 DnD，而是**工程可落地的子集标准**。

---

# 🧭 Game/UI Editor Drag & Drop Spec v2

## 🎯 设计目标

✅ 支持游戏内拖拽（背包、节点、层级）
✅ 支持多子窗口
✅ 支持跨进程
✅ 支持 OS 文件拖入
✅ 支持拖出到系统
✅ 支持跨应用
✅ 高性能（逐帧渲染安全）
✅ API 简洁
✅ 与 HTML 思想兼容但不绑定

---

# 🧱 总体架构

DnD 分三层（非常关键）：

```
┌─────────────────────────────┐
│   OS Drag Manager           │  ← 系统级（文件/跨应用）
├─────────────────────────────┤
│   Engine Drag Service       │  ← 跨窗口/跨进程
├─────────────────────────────┤
│   Local UI Drag Layer       │  ← 游戏UI内
└─────────────────────────────┘
```

---

# 🧩 层级职责

## 🥇 Level 1：Local UI Drag（最快路径）

用于：

* 背包拖物品
* UI 排序
* 节点拖动
* timeline 拖动

特点：

* 🚀 无 IPC
* 🚀 无 OS 参与
* 🚀 每帧更新
* 🚀 最低延迟

---

## 🥈 Level 2：Engine Drag Service（跨窗口/跨进程）

用于：

* 编辑器多窗口
* 多 viewport
* dock window
* 工具面板

特点：

* 进程内或跨进程共享
* 使用轻量消息
* 不触发 OS drag

👉 这是编辑器必须有的一层

---

## 🥉 Level 3：OS Drag（系统级）

用于：

* 拖入文件
* 拖出资源
* 跨应用拖拽

特点：

* 调用系统 API
* 低频
* 慢路径

⚠️ 永远不要用于游戏内部拖拽

---

# 🔥 核心数据模型（统一）

---

## DragPayload

```ts
type DragPayload = {
  types: string[]        // mime-like
  data: Map<string, any>
  sourceId?: string
  sessionId: string
}
```

示例：

### 🎮 游戏内

```ts
{
  types: ["game/item"],
  data: {
    itemId: 1001,
    count: 3
  }
}
```

### 🧩 编辑器节点

```ts
{
  types: ["editor/node"],
  data: {
    nodeId: "abc"
  }
}
```

### 📁 OS 文件

```ts
{
  types: ["files"],
  data: {
    paths: [...]
  }
}
```

---

# 🧭 拖拽生命周期（统一事件）

这是你实现时最重要的状态机。

---

## 1️⃣ dragstart

触发：

* pointer down + threshold

事件：

```ts
onDragStart(payload, source)
```

必须做：

* 创建 sessionId
* 锁定 source
* 创建 drag ghost

---

## 2️⃣ dragmove（每帧）

```ts
onDragMove(position)
```

内部必须做：

* 命中测试（hit test）
* drop target 变化检测
* hover 状态切换

⚠️ 高频路径，必须无 GC

---

## 3️⃣ dragenter

当 target 改变：

```ts
onDragEnter(target, payload)
```

target 返回：

```ts
boolean accept
```

---

## 4️⃣ dragover（可选）

用于：

* 排序预览
* 插入线
* 高亮

---

## 5️⃣ drop

释放鼠标：

```ts
onDrop(target, payload)
```

返回：

```ts
DropEffect
```

---

## 6️⃣ dragend

清理：

* ghost
* 状态
* hover

---

# 🧠 命中测试（编辑器级关键）

你的引擎必须提供：

```ts
hitTest(x, y) -> UIElement[]
```

DnD 只取：

```
topmost droppable
```

建议缓存：

```
lastHoverTarget
```

避免重复 enter/leave。

---

# 🪟 多窗口 / 跨进程设计（重点）

这是你刚刚提的 hardest 部分。

---

## 🧩 Engine Drag Service

全局唯一：

```
DragCoordinator
```

职责：

* 全局 session
* 跨窗口广播
* 跨进程桥接
* OS fallback

---

## 📡 IPC 消息（推荐最小集）

```ts
DragBegin
DragUpdate
DragEnd
DropCommit
```

payload 必须：

* 可序列化
* 小
* 无大 blob

⚠️ 大数据走引用（assetId）

---

## 🪟 Window 注册

每个窗口：

```ts
registerDragSurface(windowId, bounds)
```

Coordinator 负责：

* 指针路由
* target window 判定

---

# 🖥️ OS 文件拖入（必须支持）

来源：

* Finder / Explorer
* 外部应用

---

## 接口

```ts
onExternalDragEnter(files)
onExternalDragOver()
onExternalDrop(files)
```

files：

```ts
type OSFile = {
  path: string
  name: string
  mime?: string
}
```

---

## 平台实现建议

### Windows

* OLE DragDrop
* IDropTarget

### macOS

* NSDraggingDestination

### Linux

* XDND

👉 SDL3 / winit 已有封装，强烈建议复用。

---

# 📤 拖出到系统（很多引擎没做）

这是高级编辑器必须功能。

---

## 触发条件

当拖拽离开所有 engine window：

```
→ 升级为 OS drag
```

---

## 接口

```ts
beginExternalDrag(payload)
```

支持：

* 文件
* 文本
* uri-list

---

## ⚠️ 关键难点

你必须提供真实文件路径，或：

### 虚拟文件方案（推荐）

编辑器生成临时文件：

```
/tmp/editor-dnd/xxx.png
```

拖出后：

* 可延迟生成
* 或使用 promise file（高级）

---

# 🧪 DragEffect（统一）

```ts
enum DropEffect {
  none
  copy
  move
  link
}
```

target 决定最终 effect。

---

# 🚀 性能红线（必须遵守）

DnD 高频路径必须：

❌ 不分配内存
❌ 不走 IPC（除非跨窗口）
❌ 不走 OS
❌ 不创建字符串
❌ 不做复杂布局

否则你 editor 会卡。

---

# 🧩 推荐实现优先级（非常实战）

如果你按 ROI：

---

## 🥇 Phase 1

仅 Local UI：

* dragstart
* dragmove
* drop
* ghost
* hitTest

👉 立即可用背包拖拽

---

## 🥈 Phase 2

Engine Drag：

* DragCoordinator
* 多窗口
* IPC

👉 编辑器 usable

---

## 🥉 Phase 3

OS drag in：

* 文件拖入
* URI 支持

👉 专业度大幅提升

---

## 🏆 Phase 4

OS drag out：

* promise file
* 跨应用

👉 工业级编辑器

---

# ✅ 一句话架构原则

> **游戏内走本地，编辑器走 Engine，出入系统才走 OS。**
