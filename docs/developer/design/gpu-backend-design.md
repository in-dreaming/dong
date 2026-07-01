# GPU Backend 设计

Dong GPU backend 基于 [in-dreaming/gpu](https://github.com/in-dreaming/gpu)（RenderGraph + Slang 反射），UI 绘制采用 data-driven PSO 表与 `GpuShaderObject` 字段绑定。

## 三层架构

| 层级 | 拥有者 | 职责 |
|------|--------|------|
| L0 | in-dreaming/gpu | Device、GpuGraph、barrier |
| L1 | 游戏图形层 | 帧图生命周期、场景/后处理/Present |
| L2 | Dong UI | `dong_ui_graph_add_passes` 注入 UI pass |

## UI Pass Bundle

- `Dong.ResourceUpload` — Copy
- `Dong.TextureBake` / `Dong.TextureCompress` — Compute（gpu_texture_compress）
- `Dong.LayerRaster` — Render
- `Dong.UIMain` — Render

## 关键 API

- [`dong/include/dong_ui_graph.h`](../../dong/include/dong_ui_graph.h)
- [`dong/include/dong_gtc.h`](../../dong/include/dong_gtc.h)
- [`dong/backends/gpu/`](../../dong/backends/gpu/)

## 构建

```bash
cd dong
zig build -Dbackend=gpu
```

嵌入式模式默认开启：`dong_gpu_driver_set_embedded_mode(driver, 1)`。

## 实现状态（2026-07）

| 模块 | 状态 |
|------|------|
| vtable / 资源管理 | 完整（`gpu_resource_manager`、upload/fence/offscreen） |
| UI Execute | rect/image/text/gradient/圆角/阴影/nineslice/uber 等均为真实实现 |
| Slang PSO | rect、image、text、gradient、conic_gradient、round_rect、shadow、nineslice、scene3d_textured |
| UiGraph pass | ResourceUpload、UIMain 真实 callback |
| 窗口呈现（Tier-1 standalone） | UI 渲染到中间 RGBA8_UNORM RT，再 blit 到 swapchain（`Dong.SwapchainBlit`），而非直接绘制到 backbuffer；避免与 swapchain 格式/时序耦合 |
| GTC | `DONG_GTC_BACKEND_IN_DREAMING_GPU` 注册；BC7/ASTC compute encode **仍为 stub**（`gpu_gtc_service.cpp`，仅在 `DONG_ATLAS_FORMAT=BC7/ASTC` 时才会被调用，默认 RGBA atlas 路径不受影响） |
| AppCore / Scene3D | `dong_gpu_app_platform` + `scene3d_gpu`（含 per-screen resource root；`scene3d_world_gpu_render` 已验证可正确采样离屏屏幕纹理并合成到 3D 世界，见下方"三续"记录） |
| ImageDecoder | `dong_gpu_image_decoder`（stb_image 解码 + `DongGtc` 压缩纹理编码，`dong_app`/`3d_screens_simple` 等均已注册） |
| SDL 委托 | **已移除**（GPU 构建不产 `dong_sdl_backend.dll`，也不允许 `DONG_GPU_HTML_VIA_SDL`） |

### 2026-07-01 修复的关键问题

之前窗口模式出现"闪烁 + 画面蒙一层"，根因是两个独立 bug：

1. **RenderGraph barrier 缺失**：`Dong.SwapchainBlit` pass 直接在 callback 里 `gpuPassBindTexture` 采样中间 RT，但没有调用 `gpuGraphPassRead` 声明依赖，自动 barrier 系统不知道该 pass 依赖这块纹理，从未插入 RENDER_TARGET→SHADER_RESOURCE 转换屏障，导致采样到未定义/过渡态内容。修复：在 `gpu_gpu_driver_execute.cpp` 的 blit pass 上补充 `gpuGraphPassRead(blit, color_target)`。
2. **sRGB 二次伽马**：`swapchain_format_` 直接采用 `gpuSurfaceGetPreferredFormat()` 的返回值，多数平台默认给出 `_SRGB` 变体；而 Dong UI 管线产出的已经是"显示就绪"（伽马编码）色值，写入 `_SRGB` RTV 会被硬件再做一次线性→sRGB 编码，造成整体发白/蒙层。修复：新增 `stripSrgbFormat`（`gpu_gpu_driver.hpp`，及 C 侧 `dong_gpu_app_platform.c`/`demo_gpu_platform.c`/`gpu_ui_inject/main.c` 的本地等价函数），在配置 surface 前把 `_SRGB` 格式转换为对应的普通 UNORM 格式。

验证方式：新增 `DONG_GPU_DUMP_WINDOW_DIR`/`DONG_GPU_DUMP_WINDOW_EVERY` 环境变量，对窗口模式下的中间 RT 做 GPU readback 落盘为 BMP（见 `gpu_gpu_driver_execute.cpp::maybeDumpWindowFrame`）。用它验证过：修复后同一帧的中间 RT 与离屏 `html_render_test` 基准逐字节一致（max diff = 0），且连续多帧之间也逐字节一致（无闪烁）。注意：本机截图工具（`PrintWindow`、甚至真实屏幕 `BitBlt`）在当前环境下均不可信（尺寸/内容与实际窗口不符，疑似远程桌面会话下 flip-model swapchain 不进入普通桌面合成层），因此优先使用 GPU 侧 readback 作为 ground truth，而非窗口截图。

另外修复了一批纯 CMake 构建树（`build-cmake-gpu`，不经过 `cmake --install`）下的部署问题：`dong_gpu_backend`/`dong_appcore` 之前输出到各自子目录而非顶层构建目录，加上 `SDL3.dll` 从未被拷贝到构建树，导致所有 GPU 示例在 `build-cmake-gpu` 里直接运行都会报 `STATUS_DLL_NOT_FOUND`（`zig build` 产出的 `zig-out/bin` 因为是打平目录所以不受影响）。现通过 `set_target_properties(... RUNTIME/LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})` 和一个去重的 `dong_gpu_runtime_dlls` 自定义 target 解决。

### 2026-07-01（续）黑屏根因：`Dong.SwapchainBlit` 仿射矩阵写错分量

上述两个 bug 修复后，中间 RT 的 GPU readback 已验证 pixel-perfect，但用户反馈窗口**仍然黑屏**。说明问题在"中间 RT 渲染完成"之后、"呈现到屏幕"之前的这一段——此前从未被独立验证过（`maybeDumpWindowFrame` 只读回中间 RT，不读回真正呈现的 backbuffer）。

**排查方法**：在 [`gpu/platform/gpu_surface.{h,cpp}`](../../dong/third_party/gpu/src/gpu/platform/gpu_surface.cpp) 新增 `gpuSurfaceTextureReadbackRGBA()`，用 slang-rhi 的 `IDevice::readTexture()` 对**已 acquire、已跑完 blit pass、尚未 present** 的 backbuffer 做同步 CPU 读回（BGRA→RGBA 转换）。在 `execute()` 里于 `gpuSurfacePresent`/`gpuSurfaceTextureRelease` 之前，用与中间 RT 相同的 `frame_index` 同时导出两张 BMP（`gpu_gpu_driver_execute.cpp::maybeDumpBackbuffer`），确保是同一帧的两个阶段快照。

结果：中间 RT 有正常像素（`extrema=(6,255)/(22,255)/(42,255)`），但 backbuffer **纯黑**（`extrema=(0,0)/(0,0)/(0,0)`）——证实 `executeSwapchainBlitPass` 的绘制没有在 backbuffer 上留下任何像素，问题被锁定在 blit draw call 本身。

**根因**：`ImageUniforms.uTransform` 在 `image.slang` 里被打包成两个 `float4`：`row0=(a,b,tx,_)`、`row1=(c,d,ty,_)`（对应 C++ 侧 8 元素数组下标 `[0..3]`/`[4..7]`）。单位矩阵要求 `transform[0]=a=1` **且** `transform[5]=d=1`。`executeSwapchainBlitPass` 里写成了 `transform[0]=1; transform[4]=1;`——`transform[4]` 实际是 `c`（x 分量对 y 的剪切系数），而真正的 y 缩放系数 `transform[5]`（`d`）保持默认值 0 未被设置。代入顶点着色器：

```
transformed.y = uTransform[1].x * pos.x + uTransform[1].y * pos.y + uTransform[1].z
              = 1.0 * pos.x + 0.0 * pos.y + 0.0   // 应该是 pos.y，被错误替换成了 pos.x
```

全屏 quad 的 4 个顶点因此退化成一条零面积的线（顶点 0/2 重合于 `(0,0)`，顶点 1/3 重合于 `(w,w)`），triangle strip 光栅化不出任何像素，backbuffer 全程停留在 clear color（纯黑）。这与"中间 RT 正确、backbuffer 全黑"的现象完全吻合。

**修复**：`gpu_gpu_driver_execute.cpp::executeSwapchainBlitPass` 改为 `transform[0]=1.0f; transform[5]=1.0f;`（对照 `gpu_gpu_driver_execute_dispatch.cpp::writeTransform` 里已经正确的 6→8 打包逻辑）。修复后重新用 `maybeDumpBackbuffer` 验证：backbuffer 与中间 RT 逐字节一致（`diff extrema = (0,0)/(0,0)/(0,0)`），且 BMP 内容确认是正确渲染的 `CSS Text Decoration Test` 页面。

同时补充了几处此前静默失败会导致"黑屏但无任何日志"的诊断日志（`gpuSurfaceAcquireNextImage`/`gpuGraphImportSurfaceTexture`/`gpuGraphCompile` 失败、blit pipeline 为空、`executeSwapchainBlitPass` 前置条件不满足），后续如再出现类似问题可直接从 `DONG_LOG_LEVEL=debug` 日志定位，无需重新搭建 readback 基础设施。

详见 [host-graphics-integration.md](host-graphics-integration.md)。

### 2026-07-01（再续）图片资源加载：GPU 后端缺 ImageDecoder + 3D 多屏缺 per-screen resource root

黑屏问题修复后，`dong_app.exe --html data/video/video_acceptance.html` 与 `3d_screens_simple.exe` 均能正常渲染 UI，但涉及 `<img>`/`background-image`/`poster` 等真实图片资源的页面（video 系列测试页、`background_origin_test.html` 等）仍无法显示图片，日志分别报：

```
[WARN] [ResourceManager] no image decoder registered, cannot decode: data\video/../images/bg.png
[ERROR] [ResourceManager] failed to read file: ../images/bg.png (error=1)
```

排查（`dong/src/render/resource_manager.cpp::getImagePixelsRGBA`）发现是两个独立缺口，且都只影响纯 GPU 构建（`DONG_BACKEND=gpu`），SDL 构建不受影响：

1. **GPU 后端从未注册 `DongImageDecoder`**：`dong_platform_set_image_decoder()` 在整个仓库里只被 `backends/sdl/dong_sdl_platform.c` 调用过一次；`backends/gpu/*` 和 `examples/shared/demo_gpu_platform.c` 从未注册任何解码器，导致即使文件读取成功也永远拿不到解码后的像素（对应上面的 WARN）。
   **修复**：新增 `backends/gpu/dong_gpu_image_decoder.{h,c}`——照搬 `dong_sdl_image_decoder.c` 的 stb_image 解码逻辑（PNG/JPEG/BMP/GIF/TGA→premultiplied RGBA8）与 `DongGtc` 编码转发路径，但去掉未被实际用到的 `<SDL3/SDL.h>` 依赖，纯粹基于 `dong_image_decoder.h` 接口，因此 GPU 后端可以独立链接它而不需要 SDL。分别在 `dong_gpu_app_platform.c`（AppCore 路径：`3d_screens_simple`/`interactive_demo_new` 等）和 `examples/shared/demo_gpu_platform.c`（独立示例路径：`dong_app`/`minimal_dong_demo`/`gpu_ui_inject`）的 init/shutdown 里创建并注册/反注册该解码器。
   Windows 下额外发现一个坑：新头文件里的 `dong_gpu_image_decoder_create/destroy` 若不加 `__declspec(dllexport)`（`DONG_GPU_BUILDING_DLL` 时）/`dllimport`，MSVC/lld-link 默认不从 `dong_gpu_backend.dll` 导出符号，会在依赖它的 `dong_app.exe`/`dong_appcore.dll` 等目标出现 `undefined symbol` 链接错误；照抄 `dong_gpu_backend_driver.h` 里已有的 `DONG_GPU_BACKEND_API` 导出宏模式解决。

2. **`scene3d_gpu.c` 的 `dong_scene3d_add_screen` 从未设置 per-screen resource root**：对照 SDL 版 `scene3d.c`（会在加载每个屏幕的 HTML 前，用该 HTML 文件所在目录调用 `dong_engine_set_resource_root`），GPU 版只是把 `scene->resource_root` 拼进 HTML 文件路径去读取文件内容，但从未把任何 root 传给对应屏幕的 `dong_engine_t`。结果每个屏幕 engine 的 resource root 始终是空串，HTML 里的相对路径（如 `../images/bg.png`）在 `ResourceManager::resolvePath` 里原样传给 `fopen`（相对进程 CWD，不做 `..` 规范化），几乎总是找不到文件（对应上面的 ERROR）。
   **修复**：在 `dong_scene3d_add_screen` 里补充 `extract_dir_from_path`/`set_screen_resource_root`（逻辑与 SDL 版一致），加载 HTML 前用文件所在目录（回退到场景级 `resource_root`）调用 `dong_engine_set_resource_root`。

3. **`dong/examples/data/images/bg.png` 缺失**：仓库里 13 个测试/示例 HTML（`tests/*.html`、`video/*.html`）都引用 `../images/bg.png`，但源码树 `examples/data/images/` 目录本身不存在——不过 `zig-out/bin/data/images/bg.png`（早期某次手动放入、未回填到源码树）确实存在，说明这是资源随 `zig-out` 产物"漂移"、从未提交回 `examples/data` 的遗留问题。已将该文件复制回 `dong/examples/data/images/bg.png`，随 `install(DIRECTORY examples/data ...)` / `zig build` 一并分发。

验证：修复后重新跑 `dong_app.exe --html data\video\video_acceptance.html` 与 `3d_screens_simple.exe`（均带 `DONG_AUTO_QUIT=N` 便于非交互验证），完整 stdout/stderr 里再也没有 `no image decoder registered` / `failed to read file` 相关的 WARN/ERROR。

### 2026-07-01（三续）`3d_screens_simple` 黑屏：offscreen 屏幕纹理从未被 3D 世界正确采样

图片资源问题修复后，日志（WARN/ERROR）已完全干净，但用户实测反馈 `3d_screens_simple.exe` 打开后仍是黑屏——这与"日志无报错=渲染正确"的假设直接矛盾，说明问题发生在日志无法覆盖的层面（GPU 资源状态/句柄语义），必须靠像素级验证才能发现。由于该场景的最终合成/呈现（`scene3d_world_gpu_render`，把每块屏幕的离屏纹理画到 3D 世界的浮空板上再 present）完全绕开了 `GpuGPUDriverImpl::execute()` 已有的 `DONG_GPU_DUMP_WINDOW_DIR` 抓帧点（那个抓帧点只覆盖 2D UI 的 window-mode 路径），新增了一套等价的抓帧工具：`scene3d_world_gpu.cpp` 里 `maybeDumpWorldBackbuffer()`，由 `DONG_GPU_DUMP_WORLD_DIR=<dir>`（+ 可选 `DONG_GPU_DUMP_WORLD_EVERY=<N>`，默认 30）驱动，在 `gpuSurfacePresent()` 之前对刚渲染完的 backbuffer 做 `gpuQueueWaitOnHost` + `gpuSurfaceTextureReadbackRGBA` 落盘为 BMP。首次抓帧证实：整张图只有背景清屏色一种像素（`(25,25,38)`，正是 `scene->bg_r/g/b` 的值），没有任何浮空板被画出来——即真正的"黑屏"（准确说是纯背景色）。

顺藤摸瓜发现三个独立叠加的 bug，缺一个都无法让画面正确出现：

1. **离屏渲染目标缺 GpuGraph 读依赖，纹理停留在 `RENDER_TARGET` 布局** —— 每块屏幕的 HTML 内容由各自的 `dong_engine_t` 通过 `dong_gpu_begin/end_frame_offscreen()` 渲染到一张纹理（`GpuGraphImportTexture(..., GPU_RESOURCE_STATE_RENDER_TARGET, ...)`），但 `execute()` 的 offscreen 分支在 UI pass 写完之后从不声明任何"读"依赖（对比 window-mode 分支里 `Dong.SwapchainBlit` 有 `gpuGraphPassRead(blit, color_target)`）。`gpuGraphExecute()` 结束时资源就停在 `RENDER_TARGET`，而稍后 `scene3d_world_gpu_render()` 是**绕开 GpuGraph 的裸命令**去采样它——没有任何 barrier 把它转到 `SHADER_RESOURCE`。这是与"仿射矩阵"那次同类的"隐式布局未转换"问题，只是发生在 3D 合成路径而非 2D blit 路径。
   **修复**：在 `execute()` 里为 offscreen 分支追加一个空的 `Dong.OffscreenExport` render pass，仅调用 `gpuGraphPassRead(export_pass, color_target)`——不需要挂 attachment/callback，效仿已有的零 attachment 的 `Dong.Present` pass 写法，让图的屏障调度器在 `gpuGraphExecute()` 返回前把纹理转到 `GPU_RESOURCE_STATE_SHADER_RESOURCE`。
2. **`scene3d_textured.slang` 的 `float4x4` 未声明内存布局，与 CPU 端列主序矩阵不匹配** —— `dong_math.h` 里 `dong_mat4_t` 明确是列主序（`m: Column-major order`），且仓库里所有已验证工作的 HLSL 着色器（DXC 编译）都依赖 DXC 默认的 `column_major` 内存布局隐式对齐；但 `scene3d_textured.slang` 是本仓库第一个、也是唯一一个使用 `float4x4` 的 Slang 着色器，通过 Slang C++ API（非 `slangc` CLI）编译，[Slang 官方文档](https://docs.shader-slang.org/en/stable/external/slang/docs/user-guide/a1-01-matrix-layout.html)明确写明"Default matrix layout in memory for Slang is row-major...Except when running the compiler through the `slangc` tool"——即通过 API 编译时默认是**行主序**，与 CPU 侧数据的列主序不匹配会导致 GPU 侧把上传的字节重新解释成转置后的矩阵，顶点变换结果错乱（退化/移出可视范围）。
   **修复**：给 `uMVP`/`uModel` 显式加 `column_major` 限定符（`column_major float4x4 uMVP;`），与 CPU 端约定对齐。
3. **（真正的直接病因）纹理句柄类型不匹配：把 `TextureView` 句柄当 `Texture` 句柄用** —— `dong_gpu_backend_get_texture_shader_view()`（`gpu_gpu_driver.cpp`）返回的是 `GpuResourceManager::gpuTextureShaderView()`，也就是 `rec->shader_view`——一个通过 `gpuCreateTextureView(..., GPU_TEXTURE_VIEW_TYPE_SHADER_RESOURCE, ...)` 创建的、落在 `GpuDevice::textureViewPool`（`rhi::ITextureView` 池）里的句柄。但消费方 `gpuPassBindTexture()`（`gpu_shader_bindings.hpp`）的实现是 `device->texturePool.resolve(texture.index, texture.generation)`——查的是完全独立的 `texturePool`（`rhi::ITexture` 池），拿到指针后自己再调 `tex->getDefaultView()`。两个池的 index/generation 命名空间互不相通，传错池的结果是 `resolve()` 每次都返回 `nullptr`，`gpuPassBindTexture` 恒定失败——这正是加调试计数后实测到的 `screen_count=28 drawn=0 ... bind_fail=28`（100% 绑定失败，且与前两个 bug 无关，即使不修前两个，这一个不修画面依旧全黑）。这个函数名本身就有误导性："_shader_view" 暗示应该返回 view，但实际唯一调用方 `scene3d_world_gpu_render` 依赖的绑定路径需要的是 plain texture handle（与 2D UI pass 里 `resources_.gpuTextureHandle(intermediate)` 用法一致）。
   **修复**：把 `dong_gpu_backend_get_texture_shader_view()` 内部改成调用 `resources().gpuTextureHandle(texture)`（而非 `gpuTextureShaderView`），返回值/参数不变，函数名暂未重命名以保持 ABI 稳定，但加了详细注释说明"despite the name"的原因。

**诊断方法论**：这次问题的关键教训是——GPU 资源状态/句柄语义类的 bug 不会在日志里留下任何痕迹（`no WARN/no ERROR` 但画面全黑），必须靠 `DONG_GPU_DUMP_*` 系列像素级 readback 工具配合"逐段插桩计数"（`drawn`/`null_tex`/`view_fail`/`bind_fail`）才能定位到具体哪一步失败；纯粹看 stdout 日志干净就断言"渲染正确"是不可靠的，这也是用户本次反馈与前一轮验证结论矛盾的根本原因。三个修复分别独立生效验证：只修 1（barrier）+2（column_major）时 `bind_fail=28` 依旧、画面依旧是纯背景色；加上 3（句柄池修复）后 `drawn=28`，`world_backbuffer_*.bmp` 像素分布从 1 种颜色变为 2489 种颜色，肉眼可见完整的 3D 楼层画廊（分区标题、22 块浮空屏幕内容、正确的透视/倾斜角度）。

**验证结果**：`3d_screens_simple.exe`（`DONG_SCENE3D_DEBUG=1` + `DONG_GPU_DUMP_WORLD_DIR`）确认 28/28 屏幕绘制成功、backbuffer 像素正常；回归复测 `dong_app.exe --html data\tests\text_decoration_test.html` 与 `dong_app.exe --html data\video\video_acceptance.html`（`DONG_GPU_DUMP_WINDOW_DIR`）像素输出与本次改动前一致，无回归。

### 已知问题（暂未修复）：GPU 示例退出时堆损坏崩溃

调试上述问题时发现：**所有** GPU 后端可执行文件（`dong_app.exe`/`3d_screens_simple.exe`，与图片解码无关——关掉新增的 decoder 反注册代码后复现依旧）在正常退出路径（无论是窗口 `WM_CLOSE` 关闭还是 `DONG_AUTO_QUIT` 达到帧数）都会在打印完最后一行 `[Context] Shutting down Dong UI Engine` 后，以 `STATUS_ACCESS_VIOLATION`（配合 CRT 调试堆检测到的 `HEAP: Free Heap block ... modified after it was freed`，即 use-after-free/double-free）崩溃退出（进程返回码 `0xC0000005`）。用 `cdb -c "g;g;kb;q"` 抓到的调用栈只落在 `dong_app.exe` 自身极小的地址范围内（`+0x1ace`/+0x135c`/+0x3f54`），没有可读符号，尚未定位到具体是哪个对象被过早释放。由于窗口内容此前已确认渲染正确、且崩溃只发生在教程/引擎已经完成全部工作之后的进程收尾阶段，未在本次改动范围内继续深挖；后续若需要处理这个问题，建议给 `dong.dll`/`dong_gpu_backend.dll` 打开调试符号后用 WinDbg/cdb 复现并 `kb` 拿到完整带符号的调用栈。
