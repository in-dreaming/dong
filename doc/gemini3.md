使用 `zig build` 来管理这个混合了 C (Lexbor, QuickJS) 和 C++ (Yoga, Skia) 的项目是极佳的选择，因为 Zig 的构建系统对 C/C++ 交叉编译支持极好。

项目代号：**Dong** (UI Engine)

---

### 1. 工程目录结构 (Project Structure)

我们采用“单体仓库 (Monorepo)”结构，将依赖库源码（或构建好的库）放在 `third_party`，核心逻辑放在 `src`。

```text
dong/
├── build.zig             # Zig 构建脚本 (核心)
├── include/
│   └── dong.h            # 对外暴露唯一的 C ABI 头文件
├── src/
│   ├── core/             # 核心上下文管理
│   ├── dom/              # Lexbor 封装与 DOM 树维护
│   ├── layout/           # Yoga 封装与 CSS -> Flexbox 映射
│   ├── script/           # QuickJS 绑定与 JS 运行时
│   ├── render/           # Skia 绘制指令封装
│   └── platform/         # 平台抽象 (文件IO, 剪贴板等)
├── third_party/          # 依赖库
│   ├── lexbor/           # [Submodule] HTML/CSS 解析
│   ├── yoga/             # [Submodule] 布局引擎
│   ├── quickjs/          # [Submodule] JS 引擎
│   └── skia/             # Skia 比较特殊，建议放预编译好的静态库或头文件
└── examples/             # 示例代码 (使用 GLFW/SDL 调用 dong)
```

---

### 2. 模块划分与职责 (Modules & Responsibilities)

这是一个典型的“流水线”架构。

#### A. `dong_dom` (基于 Lexbor)
*   **职责**：解析 HTML 字符串，维护 DOM 树结构，解析 CSS。
*   **关键逻辑**：
    *   使用 `lxb_html_document_parse` 生成 DOM。
    *   使用 Lexbor 的 CSS 引擎计算**Computed Styles**（计算后样式）。这是最难的一步，需要查询匹配的 CSS 规则并将 `font-size: 1.2em` 转换为具体的像素值。

#### B. `dong_layout` (基于 Yoga)
*   **职责**：将 DOM 的 Computed Styles 映射为 Yoga Nodes。
*   **关键逻辑**：
    *   维护一棵与 DOM 树一一对应的 `YGNodeRef` 树。
    *   **属性映射器**：编写一个转换器，将 Lexbor 解析出的 `display: flex`, `width: 50%`, `margin: 10px` 转换成 Yoga 的 API 调用（如 `YGNodeStyleSetWidthPercent`）。
    *   执行 `YGNodeCalculateLayout()` 计算最终的 `x, y, width, height`。

#### C. `dong_render` (基于 Skia)
*   **职责**：遍历 Layout 树，调用 Skia 进行绘制。
*   **关键逻辑**：
    *   **Painter**：根据 DOM 节点的样式（背景色、圆角、边框、阴影）生成 `SkCanvas` 指令。
    *   **Text Layout**：Skia 的 `SkParagraph` 或基础 `SkTextBlob` 处理文字渲染（需从 Lexbor 获取字体属性，从 Yoga 获取文本框约束）。
    *   **输出**：最终渲染到一个 `SkSurface`，并提供接口导出为 `OpenGL Texture ID` 或 `RGBA Buffer`。

#### D. `dong_script` (基于 QuickJS)
*   **职责**：提供 JS 操纵 DOM 的能力。
*   **关键逻辑**：
    *   **Bindings**：实现 `document.getElementById` 等 API，这些 API 实际上调用 `dong_dom` 的 C++ 函数。
    *   **Event Loop**：将宿主的输入事件转发给 JS 的事件监听器。

---

### 3. 上层架构设计 (Architecture)

内部是 C++ (为了用 Skia/Yoga)，外部暴露纯 C ABI。

#### 数据流向图 (Pipeline)
```mermaid
graph TD
    Input[HTML String] -->|1. Lexbor| DOM[DOM Tree]
    DOM -->|2. CSS Cascade| Computed[Computed Styles]
    Computed -->|3. Map Props| YogaTree[Yoga Node Tree]
    YogaTree -->|4. Calculate| Layout[Geometry (x,y,w,h)]
    Layout -->|5. Paint Traverse| SkiaCmds[Skia Draw Commands]
    SkiaCmds -->|6. Rasterize| Texture[GPU Texture / Pixel Buffer]
    
    JS[QuickJS] -->|Modify| DOM
    JS -->|Callback| Events[Click/Hover]
```

#### C ABI 接口设计 (`include/dong.h`)
这是你的引擎对外暴露的唯一契约：

```c
#ifndef DONG_H
#define DONG_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dong_context_t dong_context_t;
typedef struct dong_view_t dong_view_t;

// 1. 初始化
dong_context_t* dong_create_context();
void dong_destroy_context(dong_context_t* ctx);

// 2. 视图管理 (对应一个浏览器 Tab 或 UI 层)
dong_view_t* dong_view_create(dong_context_t* ctx, uint32_t width, uint32_t height);
void dong_view_load_html(dong_view_t* view, const char* html);
void dong_view_resize(dong_view_t* view, uint32_t width, uint32_t height);

// 3. 驱动更新
// 执行 JS 任务队列，处理布局，触发重绘
void dong_view_update(dong_view_t* view);

// 4. 渲染输出
// 获取渲染后的像素数据（RGBA）或 纹理句柄
void* dong_view_get_pixel_buffer(dong_view_t* view); 
// 或者如果集成 GPU 上下文
uint32_t dong_view_get_texture_id(dong_view_t* view);

// 5. 输入事件注入 (宿主 -> UI)
void dong_view_send_mouse_move(dong_view_t* view, int x, int y);
void dong_view_send_mouse_down(dong_view_t* view, int button);
void dong_view_send_mouse_up(dong_view_t* view, int button);

// 6. JS 交互
void dong_view_eval(dong_view_t* view, const char* script);

#ifdef __cplusplus
}
#endif

#endif
```

---

### 4. Zig Build 系统设计

这是实现的难点，特别是混合编译。你需要编写 `build.zig` 来编排这一切。

**关键策略：**
1.  **Lexbor & QuickJS**: 直接编译源码（C）。
2.  **Yoga**: 编译源码（C++）。
3.  **Skia**: **强烈建议使用预编译库**。在 `build.zig` 中编译 Skia 源码极其痛苦（因为它依赖 GN）。你可以去下载 `skia-binaries` 或者自己用 GN 编译好 `.a` 文件后，用 Zig 链接。
4.  **Dong**: 编译为动态库 (`.dll`/`.so`/`.dylib`) 或静态库。

#### `build.zig` 参考模板

```zig
const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // 1. 定义主库 dong
    const lib = b.addStaticLibrary(.{
        .name = "dong",
        .target = target,
        .optimize = optimize,
    });

    // 2. 添加 C++ 标准支持
    lib.linkLibCpp();

    // 3. 添加头文件路径
    lib.addIncludePath(b.path("include"));
    lib.addIncludePath(b.path("src"));
    lib.addIncludePath(b.path("third_party/lexbor/src"));
    lib.addIncludePath(b.path("third_party/yoga"));
    lib.addIncludePath(b.path("third_party/quickjs"));
    // 假设 Skia 头文件在这里
    lib.addIncludePath(b.path("third_party/skia/include/core")); 

    // 4. 添加 Dong 自身源码
    lib.addCSourceFiles(.{
        .files = &.{
            "src/core/context.cpp",
            "src/dom/dom_manager.cpp",
            "src/layout/layout_engine.cpp",
            "src/render/skia_backend.cpp",
            "src/script/js_env.cpp",
            "src/api_bindings.cpp", // 实现 C ABI
        },
        .flags = &.{"-std=c++17"},
    });

    // 5. 编译并链接 QuickJS (纯 C)
    lib.addCSourceFiles(.{
        .files = &.{
            "third_party/quickjs/quickjs.c",
            "third_party/quickjs/libregexp.c",
            "third_party/quickjs/libunicode.c",
            "third_party/quickjs/cutils.c",
            "third_party/quickjs/libbf.c",
        },
        .flags = &.{"-D_GNU_SOURCE"}, // QJS 需要这个
    });

    // 6. 编译并链接 Lexbor (纯 C)
    // Lexbor 文件很多，实际项目中建议编写辅助函数扫描目录
    lib.addCSourceFiles(.{
        .files = &.{
            "third_party/lexbor/source/lexbor/html/parser.c",
            "third_party/lexbor/source/lexbor/html/tokenizer.c",
            // ... 需要添加 Lexbor 核心、CSS、DOM 相关的所有 .c 文件
        },
        .flags = &.{"-DLEXBOR_STATIC"},
    });

    // 7. 编译并链接 Yoga (C++)
    lib.addCSourceFiles(.{
        .files = &.{
            "third_party/yoga/yoga/Yoga.cpp",
            "third_party/yoga/yoga/event/event.cpp",
            // ... 其他 Yoga 源文件
        },
        .flags = &.{"-std=c++17"},
    });

    // 8. 链接 Skia (预编译静态库)
    // 假设你在 lib/skia/ 下有 libskia.a
    lib.addLibraryPath(b.path("third_party/skia/out/Release")); 
    lib.linkSystemLibrary("skia"); 
    // Skia 在不同平台依赖系统库
    if (target.result.os.tag == .windows) {
        lib.linkSystemLibrary("opengl32");
        lib.linkSystemLibrary("user32");
        // ... Skia Windows 依赖
    } else if (target.result.os.tag == .macos) {
        lib.linkFramework("CoreFoundation");
        lib.linkFramework("CoreGraphics");
        lib.linkFramework("CoreText");
    }

    // 安装产物
    b.installArtifact(lib);
}
```

---

### 5. 核心技术难点攻关策略

在开发 `dong` 时，你会遇到三个主要 Boss，我给你预先的攻略：

1.  **Computed Style 的同步 (Lexbor -> Yoga)**
    *   Lexbor 解析 CSS 很快，但它不会自动变成 Yoga 的样式。你需要写一个巨大的 `switch-case` 或者 `HashMap`。
    *   **建议**：只支持 Flexbox 和部分 Box Model 属性。不要试图支持所有 CSS。如果不认识的属性，直接丢弃。
    *   *难点*：`width: 50%`。你需要区分 Yoga 的 `YGUnitPercent` 和 `YGUnitPoint`。

2.  **脏矩形与重排 (Reflow/Repaint)**
    *   不要每一帧都重新计算布局！
    *   **策略**：
        *   在 C++ 的 DOM 节点类中增加 `bool is_dirty`。
        *   JS 修改属性 -> 标记 `is_dirty = true` -> 向上冒泡标记父节点。
        *   在 `dong_view_update` 开始时，只有根节点 dirty，才调用 `YGNodeCalculateLayout`。
        *   渲染时，只有 dirty 的节点才重新生成 Skia 指令（或者重绘到缓存层）。

3.  **资源加载 (图片/字体)**
    *   Lexbor 只是解析出 `<img src="a.png">` 的字符串。它不负责下载。
    *   **设计**：在 `dong_context` 中允许宿主注册 `ResourceLoader` 回调。
    *   C++ 解析到 img 标签 -> 调用回调 -> 宿主加载字节流 -> 传回给 Skia 解码。

### 总结

这个架构（Dong Engine）一旦做成，将是**性能与标准的最佳平衡点**。
*   **Lexbor** 保证了你能解析现代前端框架生成的丑陋 HTML。
*   **Yoga** 保证了你的布局逻辑和 React Native 一样稳健。
*   **Skia** 保证了你的渲染上限极高（Lottie, Shader, 路径动画）。
*   **Zig Build** 让你从 C/C++ 的构建地狱中解脱出来，拥有统一的工程管理。

现在，你可以先建立文件夹，初始化 `build.zig`，尝试先把 Yoga 和 QuickJS 编译通过，这算是一个 Hello World。