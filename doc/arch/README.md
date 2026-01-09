# Dong Engine 架构概览

## 库职责划分

```
┌─────────────────────────────────────────────────────────────────┐
│                        应用层 (Applications)                      │
│  dong_app / examples / 游戏引擎集成                               │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                      dong_plugin_sdl.dll                         │
│  平台抽象层：窗口创建、GPU 设备、事件轮询、渲染提交                   │
└─────────────────────────────────────────────────────────────────┘
                              │
          ┌───────────────────┴───────────────────┐
          ▼                                       ▼
┌──────────────────────┐              ┌──────────────────────┐
│      dong.dll        │              │    dong_legacy.lib   │
│   平台无关核心库       │              │   SDL 耦合兼容库      │
│                      │              │                      │
│ • DOM 管理           │              │ • View API           │
│ • HTML/CSS 解析      │              │ • GPU 渲染管线        │
│ • 布局引擎 (Yoga)    │              │ • Shader 管理        │
│ • 脚本引擎 (QuickJS) │              │ • 字形图集           │
│ • 事件分发           │              │ • 平台窗口封装        │
│ • 焦点管理           │              │                      │
│ • 绘制命令生成       │              │                      │
└──────────────────────┘              └──────────────────────┘
          │                                       │
          └───────────────────┬───────────────────┘
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                        第三方依赖                                 │
│  lexbor (HTML解析) │ yoga (Flexbox布局) │ quickjs (JS引擎)       │
│  freetype (字体)   │ harfbuzz (文本整形) │ msdfgen (SDF字体)     │
└─────────────────────────────────────────────────────────────────┘
```

## 核心库说明

### dong.dll (平台无关核心)

**职责**：提供完整的 UI 引擎核心功能，不依赖任何平台 API。

**主要模块**：
- `dom/` - DOM 树管理、HTML 解析、CSS 样式计算
- `layout/` - 基于 Yoga 的 Flexbox 布局引擎
- `script/` - QuickJS 脚本引擎和 DOM 绑定
- `render/` - 绘制命令生成（Painter）、资源管理

**公开 API** (`dong.h`):
```c
dong_engine_create()      // 创建引擎实例
dong_engine_tick()        // 驱动更新循环
dong_engine_load_html()   // 加载 HTML 内容
dong_engine_resize()      // 调整视口
dong_engine_send_*()      // 输入事件
dong_engine_eval_script() // 执行 JS
```

### dong_legacy.lib (SDL 耦合兼容库)

**职责**：提供完整的 GPU 渲染能力，依赖 SDL3。用于需要直接 GPU 渲染的场景。

**主要模块**：
- `render/gpu_*` - SDL GPU 渲染管线
- `render/shader_manager` - 着色器编译管理
- `render/glyph_atlas` - 字形纹理图集
- `platform/` - SDL 窗口封装
- `input/` - SDL 输入适配器

**公开 API** (`dong_legacy_view.h`):
```c
dong_view_create()                 // 创建视图
dong_view_update()                 // 更新并渲染
dong_view_set_external_gpu_device() // 绑定 GPU 设备
dong_view_send_*()                 // 输入事件
```

### dong_plugin_sdl.dll (平台插件)

**职责**：为 `dong.dll` 提供平台抽象，实现窗口、GPU、事件等接口。

**实现接口**：
- 窗口创建/销毁
- GPU 设备初始化
- 事件轮询
- 渲染命令提交

## 使用场景

| 场景 | 推荐方案 |
|------|---------|
| 独立应用 | `dong_app` + `dong.dll` + `dong_plugin_sdl.dll` |
| 游戏引擎集成 | `dong.dll` + 自定义插件 |
| 快速原型/示例 | `dong_legacy.lib` (直接 GPU 渲染) |
| 3D 场景内嵌 UI | `dong_legacy.lib` + 离屏渲染 |

## 输入事件约定

**滚轮语义**：`delta_y` 正值 = 向下滚动（内容向上移动）

调用方需将平台原始值转换为此语义后再传入 API。
