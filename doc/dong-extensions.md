# dong Extensions (Private Web Extensions)

> dong 在 Web 标准之上提供的私有扩展，用于游戏 UI 场景。
> 以下所有扩展不影响标准 HTML/CSS/JS 内容的正确渲染。

---

## HTML 自定义元素

| 元素 | 用途 | 引入版本 |
|---|---|---|
| `<host-view data-host-view-id="N">` | 宿主引擎渲染区域占位 (P1-2) | Phase 1 |
| `<dong-lottie src="..." autoplay loop speed>` | Lottie 矢量动画播放 (P2-4) | Phase 2 |
| `<dong-rive src="..." state-machine="...">` | Rive 矢量动画播放 (P2-4) | Phase 2 |

---

## CSS 私有属性

| 属性 | 用途 | 引入版本 |
|---|---|---|
| `nav-up` / `nav-down` / `nav-left` / `nav-right` | 空间导航显式覆盖 (P0-4) | Phase 0 |
| `--dong-hdr-boost: <float>` | HDR 亮度增益乘数 (P1-6) | Phase 1 |

---

## data-* 属性

| 属性 | 用途 | 引入版本 |
|---|---|---|
| `data-host-view-id` | 宿主视图 ID (P1-2) | Phase 1 |
| `data-nav-trap` | 空间导航限定容器 (P0-4) | Phase 0 |
| `data-dong-native-tooltip` | 原生 tooltip hint | Phase 0 |

---

## JS 全局对象

| API | 用途 | 引入版本 |
|---|---|---|
| `dong.focusNav(direction)` | 编程式空间导航 (P0-4) | Phase 0 |
| `dong.getView(name)` | 多 View 跨视图访问 | Phase 1 |

---

## C API 扩展

| Header | 用途 | 引入版本 |
|---|---|---|
| `dong_drawlist.h` | Draw List C ABI for host engines (P1-1) | Phase 1 |
| `dong_world_text.h` | 世界空间 billboard 文字 (P2-1) | Phase 2 |
| `dong_decal.h` | 世界空间 decal 投影 (P2-2) | Phase 2 |
| `dong_world_overlay.h` | 世界空间 HTML overlay (P2-3) | Phase 2 |
| `dong_vector.h` | Lottie/Rive 矢量动画 (P2-4) | Phase 2 |
| `dong_resource_pack.h` | GPU 资源预打包 (P2-8) | Phase 2 |

---

## 命名约定

- HTML 自定义元素：`dong-*` 前缀
- CSS 自定义属性：`--dong-*` 前缀
- data 属性：`data-dong-*` 或已有的 `data-host-view-id` / `data-nav-trap`
- C API：`dong_*` 前缀
- 环境变量：`DONG_*` 前缀

---

## 与标准 Web 内容的兼容性

所有 dong 扩展是 **纯加法**：
- 标准 HTML/CSS/JS 内容在 dong 中正确渲染（不需要修改）
- 使用 dong 扩展的内容在浏览器中优雅降级（unknown elements 被忽略，CSS 属性被跳过）
- 不修改任何标准 API 的行为
