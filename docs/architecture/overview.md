# 架构概览

## 设计原则

**Core 层无 SDL 依赖**：`dong.dll` 平台无关，平台能力通过 Plugin 注入 Backend。

```
Applications (dong_app / 游戏引擎)
         │
    dong.dll (Core: DOM/CSS/Layout/Script)
         │ Platform Singleton (接口注入点)
         │  - DongGPUDriver*
         │  - DongImageDecoder*
         │  - DongFileSystem*
         │
    backends/sdl/ (SDL GPU 实现)
```

## 库职责

| 库 | 职责 |
|----|------|
| **dong.dll** | 平台无关 Core：DOM、CSS、Layout、Script、绘制命令生成 |
| **dong_sdl_backend.dll** | SDL GPU 驱动、着色器、窗口 |
| **dong_appcore.dll** | 高层 C API：`dong_app`、`dong_scene3d` |
| **dong_plugin_sdl.dll** | 平台插件（视频/FFmpeg 等） |

## 渲染管线

1. **HTML 解析** — Lexbor → DOM 树
2. **CSS 处理** — 选择器匹配、样式计算
3. **布局** — Yoga Flexbox / Grid 扩展
4. **Display List** — Painter 遍历生成 GPU 命令
5. **GPU 执行** — SDL_GPU + HLSL 着色器

## Core 模块

| 目录 | 职责 |
|------|------|
| `src/core/` | EngineView、GlobalShared、热重载 |
| `src/dom/` | DOM 树、HTML/CSS 解析 |
| `src/layout/` | Yoga 封装、Grid、sticky、aspect-ratio |
| `src/script/` | QuickJS 绑定 |
| `src/render/` | Painter、DisplayList、OverlayDraw |
| `backends/sdl/` | GPU 驱动、着色器、输入适配 |
| `include/` | 公开 C API |

## 公开 C API

**引擎生命周期**（`dong.h`）：

```c
dong_engine_create(desc, &engine);
dong_engine_load_html(engine, html);
dong_engine_tick(engine);
dong_engine_resize(engine, w, h);
dong_engine_send_mouse_move(engine, x, y);
dong_engine_eval_script(engine, code);
dong_engine_destroy(engine);
```

**AppCore**（`dong_app.h`）：

```c
dong_app_t* app = dong_app_create(&cfg);
dong_app_load_html(app, html);
dong_app_run(app, NULL, NULL);
dong_app_destroy(app);
```

**3D 场景**（`dong_scene3d.h`）：

```c
dong_scene3d_t* scene = dong_scene3d_create(app);
dong_scene3d_add_screen(scene, &scfg);
dong_scene3d_update(scene, dt);
dong_scene3d_render(scene);
```

## GlobalShared

同进程多 View 共享 GlyphAtlas，避免每 View 独立字图集。C API：`dong_global_shared.h`。

## 使用场景

| 场景 | 推荐方案 |
|------|----------|
| 独立应用 | `dong_app` + `dong.dll` + SDL backend |
| 游戏引擎集成 | `dong.dll` + 自定义 plugin |
| 3D 内嵌 UI | 离屏渲染 / DrawList C ABI |
| 快速原型 | `dong_legacy.lib`（SDL 耦合路径） |

## 输入约定

滚轮语义：`delta_y` 正值 = 向下滚动（内容向上移动）。调用方需转换平台原始值。

## 相关文档

- [渲染管线优化](./render-pipeline.md)
- [渲染模式](../guide/render-modes.md)
- [Dong 扩展](../reference/extensions.md)

更详细的架构图与 legacy 库说明见 [developer/arch/libraries-and-legacy.md](../developer/arch/libraries-and-legacy.md)。
