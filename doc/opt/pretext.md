1. VSync/GPU 优化

当 vsync=0 时自动设置 DONG_GPU_SWAPCHAIN_NOWAIT
消除 SDL_WaitAndAcquireGPUSwapchainTexture 的阻塞等待
文件: appcore/src/app.c
2. Direct Draw API (dong.renderText)

新增 JS API: dong.clearOverlay() + dong.renderText({lines, font, color})
完全绕过 DOM/CSS/Layout 管线，直接注入 GPU 渲染命令
新增 overlay 机制 (OverlayDraw 单例 → Painter::appendOverlayItems)
文件: overlay_draw.hpp, js_text_layout_bindings.cpp, engine_view.cpp, painter.hpp, display_list.hpp
3. GPU Glyph Run 合并 (最关键的优化)

将 40 行文字合并为单个 DrawGlyphRunData
GPU draw calls: 40 → 1
每行的 glyph 坐标在 design units 空间预偏移
文件: js_text_layout_bindings.cpp