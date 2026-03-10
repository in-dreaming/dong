你要做的是一套**Dockable Window System（可停靠窗口系统）**——也就是 Unity Editor、VS Code、Qt Creator 那种。
这不是简单 UI 控件，而是一个**布局 + 拖拽 + 容器树 + 命中系统**的综合子系统。

下面给你一套**工程可落地的架构蓝图**（按引擎实现视角）。

---

# 🧠 一、先理解 Unity 的本质模型

Unity Editor 的窗口系统本质是一个**树状容器布局**：

```
Root
 ├─ Split (horizontal)
 │   ├─ DockArea (tabs)
 │   │   ├─ Scene
 │   │   └─ Game
 │   └─ DockArea
 │       └─ Inspector
```

核心不是“窗口”，而是三种节点：

### ✅ 三大核心节点类型

| 类型               | 作用     | 是否可见 |
| ---------------- | ------ | ---- |
| **DockArea**     | tab 容器 | ✅    |
| **SplitNode**    | 分割布局   | ❌    |
| **Window(View)** | 真正内容   | ✅    |

👉 Unity 内部叫：

* `DockArea`
* `SplitView`
* `EditorWindow`

---

# 🏗️ 二、推荐你的引擎架构（工业级）

## ⭐ 核心数据结构（极重要）

```cpp
enum NodeType {
    Split,
    DockArea,
    FloatingWindow
};

struct LayoutNode {
    NodeType type;

    Rect rect;

    union {
        SplitData split;
        DockData dock;
        FloatData floating;
    };
};
```

---

## ⭐ Split 节点（布局骨架）

```cpp
struct SplitData {
    Axis axis; // Horizontal / Vertical
    float ratio;
    LayoutNode* childA;
    LayoutNode* childB;
};
```

✅ 职责：

* 控制分割
* 计算子区域
* 不直接绘制内容

---

## ⭐ DockArea（tab 容器）

```cpp
struct DockData {
    vector<View*> tabs;
    int activeIndex;
};
```

✅ 职责：

* tab 管理
* tab 绘制
* tab 拖出入口

🚨 **注意**

DockArea 是系统的“稳定点”，
所有停靠都发生在它身上。

---

## ⭐ Floating Window（漂浮窗）

用于：

* 拖出后悬浮
* 多显示器支持
* 类似 Unity 的独立窗口

```cpp
struct FloatData {
    LayoutNode* root; // 自己也有一棵树
};
```

---

# 🎯 三、拖拽系统（成败关键）

Unity 那种丝滑体验，核心在：

> ❗拖拽是“预览驱动”的，而不是释放时才计算

你必须实现 3 层：

---

## 🥇 第1层：Drag Context

```cpp
struct DragContext {
    View* draggingView;
    DockArea* sourceDock;
    LayoutNode* hoverTarget;
    DockPosition previewPos;
};
```

生命周期：

```
mouse down → begin drag
mouse move → update hover
mouse up → commit docking
```

---

## 🥈 第2层：Dock 命中算法（🔥核心难点）

Unity 的停靠不是简单矩形判断，而是：

### 命中区域分 5 块：

![Image](https://docs.unity3d.com/6000.3/Documentation/uploads/Main/DockZones.gif)

![Image](https://cdn.prod.website-files.com/633b006c17a3aa08511210ac/633b006c17a3aa8d5c121533_w-overview_abstract%20-%20L.png)

![Image](https://europe1.discourse-cdn.com/unity/original/4X/6/2/2/622027e7857d6c1b3a01c99d53138815d4d4ad12.jpeg)

![Image](https://users.infragistics.com/GeorgeA/Blog/Docking-and-Guides/dockingPE.gif)

```
      top
 left center right
     bottom
```

你需要函数：

```cpp
DockPosition HitTestDock(DockArea*, mousePos)
```

返回：

* Center（合并为 tab）
* Left / Right / Top / Bottom（split）

---

## 🥉 第3层：Preview Overlay（体验灵魂）

Unity 的高级感 80% 来自这里。

拖拽时要画：

* 半透明高亮
* 箭头指示
* preview 矩形

建议做一个独立层：

```cpp
DockPreviewLayer
```

特点：

* 不参与布局
* 始终最上层
* GPU 绘制

---

# 🧩 四、布局变换算法（真正难点）

当用户 drop 时，本质是：

> ❗对 Layout Tree 做一次结构变换

例如：

### 情况1：tab 合并

```
DockA + ViewB → DockA.tabs.push()
```

---

### 情况2：左右停靠（🔥最关键）

原来：

```
DockA
```

变成：

```
Split
 ├─ DockA
 └─ NewDock(ViewB)
```

伪代码：

```cpp
LayoutNode* SplitDock(DockArea* target, DockPosition pos) {
    auto newDock = CreateDock(view);

    auto split = CreateSplit(axis);

    ReplaceNode(target, split);

    split->childA = target;
    split->childB = newDock;
}
```

🚨 **这一步是系统复杂度爆炸点**

---

# ⚡ 五、你必须提前决定的 5 个架构选择

这是很多引擎后期重写的原因。

---

## ❶ 布局是否可序列化（强烈建议）

Unity 能保存 layout 是因为：

```json
{
  "type": "split",
  "axis": "horizontal",
  "children": [...]
}
```

✅ 必做，否则编辑器体验不完整

---

## ❷ 是否支持嵌套 floating（建议支持）

Unity 支持：

* 主窗口 dock
* 独立浮窗
* 浮窗里再 split

很多自研 UI 死在这里。

---

## ❸ 拖拽是否跨窗口（🔥高级特性）

Unity 支持：

* dock → 浮窗
* 浮窗 → dock
* 跨主窗口

这需要：

> ❗全局 Drag Manager

而不是每个窗口自己处理。

---

## ❹ 是否做延迟布局（强烈建议）

拖拽时不要频繁重排。

推荐：

```
drag move → 只更新 preview
mouse up → 才修改 layout tree
```

性能差一个数量级。

---

## ❺ 是否支持最小尺寸约束（必须）

否则 split 会被拖崩。

每个 DockArea 要有：

```cpp
MinSize {x, y}
```

---

# 🚀 六、给你的工程落地路线（最稳）

按这个顺序做，成功率最高：

### Phase 1（1周）

* LayoutNode 树
* Split 布局
* DockArea + tabs
* 手动构造布局

✅ 目标：能显示 Unity 类似布局

---

### Phase 2（2周）

* tab 拖出 → 浮窗
* tab 合并
* 基础命中

✅ 目标：可用但不丝滑

---

### Phase 3（🔥质变）

* 五向 dock 命中
* preview overlay
* 动画
* layout 序列化

✅ 目标：接近 Unity

---

### Phase 4（工业级）

* 跨窗口拖拽
* 多显示器
* DPI 适配
* 性能优化
