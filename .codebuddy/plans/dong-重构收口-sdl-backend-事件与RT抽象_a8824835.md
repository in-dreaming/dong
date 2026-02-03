---
name: dong-重构收口-sdl-backend-事件与RT抽象
overview: 基于现有分层与文档目标，完成“core 去 SDL + SDL 集中到 sdl backend + 事件/RenderTarget 抽象 + AppCore 迁新链路 + engine 复用 View”的系统性重构，并打通可运行渲染闭环与验收路径。
todos:
  - id: public-api-boundary
    content: 收敛 public headers：调整 dong/include/dong.h 与 dong_view.h 边界
    status: completed
  - id: platform-subsystems
    content: 新增 dong_file_system.h/dong_logger.h，并替换 core 资源读取与日志入口
    status: completed
    dependencies:
      - public-api-boundary
  - id: engine-view-converge
    content: 重构 api_bindings.cpp：dong_engine_* 复用 View，删除 EngineImpl 双实现
    status: completed
    dependencies:
      - platform-subsystems
  - id: view-de-sdl
    content: 重构 src/core/view.* 去 SDL；仅生成 GPUCommandList 与 Surface 输出
    status: completed
    dependencies:
      - engine-view-converge
  - id: sdl-backend-execute
    content: 补齐 backends/sdl execute：迁入旧 sdl_render 执行链路并打通渲染闭环
    status: completed
    dependencies:
      - view-de-sdl
  - id: appcore-new-chain
    content: AppCore 迁新链路：事件抽象+RT 抽象，移除 SDL_Event/SDL_GPUTexture 外泄
    status: completed
    dependencies:
      - sdl-backend-execute
---

## User Requirements

- 结合现有源码与 `doc/重构.md`、`doc/重构2.md`、`doc/think.md`，梳理“完成重构”所需的系统性工作项与验收口径。
- 将 **SDL 依赖彻底收口**：`dong`（core）不直接依赖/编译/链接 SDL；SDL 仅存在于 `backends/sdl`、`plugins/sdl`、`appcore`、`examples`。
- 引入并落地 **事件抽象**：AppCore 对外只暴露 `dong_app_event_t`（tag + union）；内部完成 SDL→事件抽象转换。
- 引入并落地 **渲染目标（rt）抽象**：以现有 `DongSurface`/`DongGPUTexture` 为对外抽象（overlay/screen 返回它们），不再外泄 `SDL_GPUTexture*`。
- 以 **View 为主收敛实现**：`dong_engine_*` 复用 View 的 DOM/Layout/Script/Render 命令生成逻辑，并彻底去除 View 对 SDL 的直接依赖（避免 Engine/View 双维护）。
- 打通 “新链路最小闭环”：至少一个 demo 走 `dong_engine_*` + `dong_sdl_backend` 的 `execute()` 完整渲染出可见 UI。

## Product Overview

- 一个平台无关的核心 HTML/CSS 渲染引擎（`dong`）+ 可插拔 GPU/Surface/FS/Logger 平台能力（Platform 注入）+ 可选 SDL 后端与 AppCore 便捷层。

## Core Features

- 平台隔离：core 不包含 SDL 类型/头文件与 SDL 渲染执行链路。
- 后端可插拔：通过 `DongGPUDriver` 执行 `GPUCommandList`；通过 `DongSurface` 表达渲染输出。
- AppCore 易用：统一事件模型、统一渲染输出抽象，降低 demo/集成复杂度。

## Tech Stack Selection（基于现有仓库约束）

- 语言/标准：C++20（core 渲染与 IR），C（Platform/AppCore/后端 glue），必要处保持 C ABI。
- 图形/窗口：SDL3 仅用于 `backends/sdl` 与 `appcore`（不得进入 core）。
- 构建：Zig 作为 orchestrator（`dong/build.zig`），内部通过 CMake 构建各 target。

## Implementation Approach（高层策略）

- 采用“**接口收口 → 核心去 SDL → 后端补 execute → AppCore 迁新链路**”的顺序推进，优先完成 `重构2.md` 的 M1：闭环可运行且分层可验收。
- 以 **View 为唯一主实现**：将 `api_bindings.cpp` 的 `EngineImpl` 收敛为对 View 的薄包装，避免 DOM/Layout/Script 渲染逻辑双份实现导致行为漂移。
- 以现有 `GPUCommandList`（`src/render/gpu_ir.hpp`）作为 core 输出，后端 `execute(const void*)` 直接 cast 执行；对外 ABI 维持 `const void*`，但在内部增加版本/断言与最小字段校验，控制崩溃面。

## Implementation Notes（关键执行细节）

- 分层验收必须可自动验证：
- `dong` target：编译单元与链接依赖中不出现 `SDL_` 符号/头文件（重点：`src/core/view.*`、`src/input/*`、`src/platform/*`）。
- SDL 相关执行代码（含 shadercross/dxc 依赖）必须迁移或重归属到 `backends/sdl` target。
- 性能：
- `GPUCompiler::compile` 是热点（O(N) display items）；保持当前“按需 build batches 可选”策略，避免为抽象引入额外拷贝。
- 图层缓存 dirty 标记清理（`clearIsolatedLayerDirtyFlags`）保持在 core，后端只做执行与资源缓存。
- 可靠性：
- `DongFileSystem`/`DongLogger` 未注入时必须有确定行为（no-op 或明确错误码），禁止隐式 fallback 到 SDL filesystem / std::ifstream。
- 代码规范：避免大函数；拆分为多个小函数（单函数尽量 <70 行），尤其是事件转换与 execute 执行分发。

## Architecture Design

- Core（dong）：
- View：DOM/CSS/Layout/JS + DisplayList 构建 + `GPUCommandList` 生成（不做 SDL 执行）
- Platform：注入 `DongGPUDriver` / `DongSurfaceFactory` / `DongFileSystem` / `DongLogger`
- Backend（backends/sdl）：
- 资源创建/上传（已有）
- `execute()`：真正解释并提交 `GPUCommandList`（目前 stub，必须补齐）
- AppCore：
- SDL 窗口与事件循环
- 事件抽象（对外 `dong_app_event_t`）
- 驱动 `dong_engine_*` 并通过后端执行命令；overlay/screen 输出以 `DongSurface`/`DongGPUTexture` 传递

## Directory Structure（将被修改/新增的文件）

dong/
├── include/
│   ├── dong.h                       # [MODIFY] 默认不强制暴露 view；明确新/旧 API 边界
│   ├── dong_view.h                  # [MODIFY] 去 SDL 语义（external SDL device/texture），改为后端无关能力
│   ├── dong_platform.h              # [MODIFY] 引入完整类型定义头，移除“只声明不定义”的对外缺口
│   ├── dong_gpu_driver.h            # [MODIFY] 明确 command_list 版本/校验策略与 execute 约定
│   ├── dong_surface.h               # [MODIFY] 作为 RT 抽象统一出口（overlay/screen/engine）
│   ├── dong_file_system.h           # [NEW] 对外可用 vtable + 约定（exists/read/read_partial 等）
│   └── dong_logger.h                # [NEW] 对外可用 vtable + level/category 约定
├── src/
│   ├── core/
│   │   ├── platform.c               # [MODIFY] 保持注入存储；补齐默认行为辅助（可选小工具函数）
│   │   └── view.cpp/view.hpp        # [MODIFY] 彻底去 SDL include；仅生成命令与管理平台抽象资源
│   ├── api_bindings.cpp             # [MODIFY] EngineImpl 收敛为复用 View；资源加载走 FileSystem
│   └── render/
│       └── gpu_ir.hpp               # [MODIFY] 仅补必要的版本/调试校验点（避免破坏现有结构）
├── backends/
│   └── sdl/
│       ├── dong_sdl_platform.c      # [MODIFY] execute()/prepare_resources 不再 stub；与执行器衔接
│       ├── sdl_execute/*.cpp        # [NEW] 从旧 sdl_render 执行链路迁入的可复用执行器实现（按模块拆分）
│       └── sdl_execute/*.hpp        # [NEW] 执行器私有头（仅 backend 内部使用）
└── appcore/
├── include/
│   ├── dong_app.h               # [MODIFY] 事件回调改为 dong_app_event_t；移除 sdl_event 外泄
│   ├── dong_overlay.h           # [MODIFY] get_texture 改为返回 DongSurface 或 DongGPUTexture
│   └── dong_scene3d.h           # [MODIFY] process_event 改为消费 dong_app_event_t
└── src/
├── app.c                    # [MODIFY] 内部从 view 链路迁到 engine 链路；SDL→事件抽象转换
└── overlay.c                # [MODIFY] 从 view_render_to_gpu_texture 迁到 engine+backend+surface

（构建系统）

- dong/CMakeLists.txt                # [MODIFY] target 依赖重归属：core 不链接 SDL；SDL 执行仅归 backend
- dong/build.zig                      # [MODIFY] zig 目标与安装头文件同步新增/改动

## Key Code Structures（必要的关键接口，建议仅定义签名）

- `dong_app_event_t`：AppCore 对外事件（tag + union，覆盖 mouse/key/text/wheel/window resize/focus）
- `DongFileSystemVTable` / `DongLoggerVTable`：Platform 对外注入能力（含默认行为约定）

# Agent Extensions

（本任务不需要使用已提供的扩展；不创建/更新任何 skill，避免引入额外复杂度）