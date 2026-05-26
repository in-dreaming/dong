# Dong 定位与边界

> 决策日期：2026-04-17  
> 状态：approved

## 一句话定位

**Dong 是一套用 HTML/CSS/DOM 子集描述、GPU 直接渲染的游戏 UI 工具，面向游戏 editor 与 runtime（含 3D HUD/世界空间 UI），不是浏览器。**

## 路线选择：Ultralight，不是 Coherent

| 路线 | 代表 | 思路 | 取舍 |
|------|------|------|------|
| Coherent (Gameface) | 改造 Chromium / Blink | HTML 完整兼容 | 内存大、依赖重、构建复杂 |
| Ultralight | 自研裁剪引擎 | 强裁剪 + 嵌入友好 + 极致性能 | 牺牲冷门 web 特性 |

**Dong 选 Ultralight 路线**：

- 不追求 web 标准完整性，缺口按「游戏 UI 是否需要」排序
- 不追求「任意 web 页面能跑」——输入应为「为 Dong 写的 HTML」
- 不追求 Web 平台 API 完整性（无 Service Worker / WebGL / IndexedDB 等）
- 追求：单 frame budget 可控、嵌入宿主不带 RT、首屏延迟低、多 view 共享资源、Slug 清晰字体

## 目标场景

| 场景 | Dong 必须做好 |
|------|---------------|
| **游戏 Editor** | 复杂 panel、高频局部更新、可嵌入引擎主程序 |
| **2D 游戏 HUD** | 低 draw call、Slug 文字、手柄导航 |
| **3D 中的 UI** | 多 view 共享 atlas、纹理输出可被宿主采样 |
| **世界空间 UI** | 高频 instanced 绘制、与宿主 3D 管线协同 |

**非目标**：通用浏览器、第三方 web 应用、Chrome 扩展、完整 Web 平台 API。

## 三轨渲染模式

通过 `<meta name="dong-render-mode">` 与 JS API 选择。详见 [渲染模式](../guide/render-modes.md)。

| 模式 | 启用 | 适用 |
|------|------|------|
| **DOM**（默认） | 不写 meta 或 `content="dom"` | Editor、React/Preact、复杂面板 |
| **Scene Graph** | `content="scene"` | 高密度静态 HUD（背包、技能栏） |
| **Direct Draw** | `dong.drawRect` 等 JS API | 伤害浮字、粒子文字、debug overlay |

**边界规则**：

- 同一 view 可叠加 DOM + Direct Draw
- DOM 与 Scene Graph 互斥
- 三轨共用 GlyphAtlas / ImageAtlas / GPU Pipeline

## 3D 集成策略

`dong_scene3d_*`（自带相机 + WASD）定位为 **demo / 工具 API**，不作为生产级 3D 入口。

生产集成走宿主可调用的 Primitive：

| Primitive | 用途 |
|-----------|------|
| HTML → GPU Texture（已有） | View 渲染到纹理，宿主采样贴 quad |
| `dong_world_text_t` | 世界空间 billboard 文字 |
| `dong_decal_t` | UI 投影到世界几何 |
| `dong_world_overlay_t` | 世界空间 HTML overlay |
| `DongDrawList` C ABI | RT-free draw command 导出 |

**原则**：Dong 不假设拥有相机和 swapchain。

## 明确不做

| 类别 | 不做 |
|------|------|
| HTML | iframe、embed、object、portal |
| CSS | :visited、@page、多列 |
| Web 平台 | Service Worker、Web Worker、WebGL、WebRTC、IndexedDB |
| 安全 | 沙箱、CSP、同源策略（游戏资产自带，非浏览器模型） |

## 相关文档

- [路线图](../roadmap.md)
- [架构概览](../architecture/overview.md)
- [Dong 扩展](../reference/extensions.md)
- 完整决策记录与 Phase spec → [docs/developer/](../../docs/developer/)
