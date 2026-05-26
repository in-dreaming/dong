# Dong 扩展

> Dong 在 Web 标准之上提供的私有扩展，用于游戏 UI 场景。  
> 以下扩展不影响标准 HTML/CSS/JS 内容的正确渲染。

## HTML 自定义元素

| 元素 | 用途 |
|------|------|
| `<host-view data-host-view-id="N">` | 宿主引擎渲染区域占位 |
| `<dong-lottie src="..." autoplay loop speed>` | Lottie 矢量动画 |
| `<dong-rive src="..." state-machine="...">` | Rive 矢量动画 |

## CSS 私有属性

| 属性 | 用途 |
|------|------|
| `nav-up/down/left/right` | 空间导航显式覆盖 |
| `--dong-hdr-boost: <float>` | HDR 亮度增益 |

## data-* 属性

| 属性 | 用途 |
|------|------|
| `data-host-view-id` | 宿主视图 ID |
| `data-nav-trap` | 空间导航限定容器 |
| `data-dong-native-tooltip` | 原生 tooltip hint |

## JS 全局对象

| API | 用途 |
|-----|------|
| `dong.focusNav(direction)` | 编程式空间导航 |
| `dong.getView(name)` | 多 View 跨视图访问 |

完整 JS API 见 [JavaScript API](./js-api.md)。

## C API 扩展

| Header | 用途 |
|--------|------|
| `dong_drawlist.h` | Draw List C ABI |
| `dong_world_text.h` | 世界空间 billboard 文字 |
| `dong_decal.h` | 世界空间 decal |
| `dong_world_overlay.h` | 世界空间 HTML overlay |
| `dong_vector.h` | Lottie/Rive |
| `dong_resource_pack.h` | GPU 资源预打包 `.dpkg` |

## 命名约定

| 类型 | 前缀 |
|------|------|
| HTML 自定义元素 | `dong-*` |
| CSS 自定义属性 | `--dong-*` |
| data 属性 | `data-dong-*` 或专用名 |
| C API | `dong_*` |
| 环境变量 | `DONG_*` |

## 与标准 Web 的兼容性

所有扩展为**纯加法**：

- 标准 HTML/CSS/JS 在 Dong 中正确渲染
- Dong 扩展在浏览器中优雅降级（unknown elements 忽略，私有 CSS 跳过）
- 不修改标准 API 行为
