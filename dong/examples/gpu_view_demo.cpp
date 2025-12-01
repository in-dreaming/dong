#include <cstdio>
#include <cstdint>

#include <dong.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include "platform/sdl3_window.hpp"

using dong::platform::SDL3Window;

int main() {
    std::printf("=== Dong GPU View SDL Demo ===\n");

    // 1. 创建带 GPU 的 SDL 窗口
    SDL3Window::CreateInfo ci{};
    ci.title = "Dong GPU View Demo";
    ci.width = 960;
    ci.height = 600;
    ci.use_gpu = true;
    ci.debug_mode = false;

    SDL3Window window;
    if (!window.initialize(ci)) {
        std::printf("ERROR: Failed to initialize SDL3Window: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GPUDevice* device = window.getGPUDevice();
    if (!device) {
        std::printf("ERROR: GPU device is null after initialization.\n");
        return 1;
    }

    // 2. 创建 Dong 上下文 + 视图（仍然使用 Dong 的完整 HTML/CSS/JS 流水线）
    std::printf("[gpu_view_demo] Creating dong context...\n");
    dong_context_t* ctx = dong_create_context();
    if (!ctx) {
        std::printf("ERROR: Failed to create dong context\n");
        return 1;
    }
    std::printf("[gpu_view_demo] Dong context created.\n");

    std::printf("[gpu_view_demo] Creating dong view...\n");
    dong_view_t* view = dong_view_create(ctx, ci.width, ci.height);
    if (!view) {
        std::printf("ERROR: Failed to create dong view\n");
        dong_destroy_context(ctx);
        return 1;
    }
    std::printf("[gpu_view_demo] Dong view created.\n");

    // 切换到 GPU 渲染模式（让 View 使用 SDL_gpu 后端）
    std::printf("[gpu_view_demo] Enabling GPU render mode on dong view...\n");
    dong_view_set_external_gpu_device(view, static_cast<void*>(device), static_cast<void*>(window.getHandle()));

    // 使用与 gpu_texture_demo 相同的HTML内容进行对比测试
    const char* html = R"(
        <!DOCTYPE html>
        <html>
        <head>
            <style>
                body {
                    margin: 0;
                    padding: 30px;
                    background-color: #f5f5f5;
                    font-family: Arial, sans-serif;
                }
                
                h1 {
                    font-size: 48px;
                    color: #333333;
                    margin: 20px 0;
                }
                
                .info {
                    font-size: 24px;
                    color: #666666;
                    margin: 10px 0;
                }
                
                .highlight {
                    font-size: 32px;
                    color: #ff6600;
                    font-weight: bold;
                }
            </style>
        </head>
        <body>
            <h1>GPU Texture Demo</h1>
            <div class="info">Two-layer offscreen rendering API</div>
            <div class="highlight">renderToGPUTexture() + renderOffscreen()</div>
        </body>
        </html>
    )";

    std::printf("[gpu_view_demo] Loading HTML into dong view...\n");
    dong_view_load_html(view, html);
    std::printf("[gpu_view_demo] HTML loaded. Running first update...\n");
    dong_view_update(view);
    std::printf("[gpu_view_demo] First update done. Entering main loop...\n");

    bool running = true;
    std::printf("Entering main loop... Close window to exit.\n");

    while (running) {
        // 3. 处理 SDL 事件，并转发到 Dong 的输入 API
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_EVENT_QUIT:
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                running = false;
                break;

            case SDL_EVENT_WINDOW_RESIZED: {
                int w = 0, h = 0;
                SDL_GetWindowSize(window.getHandle(), &w, &h);
                if (w > 0 && h > 0) {
                    dong_view_resize(view, (uint32_t)w, (uint32_t)h);
                }
                break;
            }

            case SDL_EVENT_MOUSE_MOTION:
                dong_view_send_mouse_move(view,
                    (int32_t)ev.motion.x,
                    (int32_t)ev.motion.y);
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                dong_view_send_mouse_move(view,
                    (int32_t)ev.button.x,
                    (int32_t)ev.button.y);
                dong_view_send_mouse_down(view, (int32_t)ev.button.button);
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                dong_view_send_mouse_move(view,
                    (int32_t)ev.button.x,
                    (int32_t)ev.button.y);
                dong_view_send_mouse_up(view, (int32_t)ev.button.button);
                break;

            case SDL_EVENT_KEY_DOWN:
                dong_view_send_key_down(view, (uint32_t)ev.key.key);
                break;

            case SDL_EVENT_KEY_UP:
                dong_view_send_key_up(view, (uint32_t)ev.key.key);
                break;

            default:
                break;
            }
        }

        // 4. 驱动 Dong 的 JS / 布局 / 渲染流水线
        // Dong 已通过 setExternalGPUDevice 拥有 GPU device 的控制权
        // 包括命令缓冲的生命周期和 swapchain 纹理的获取/呈现
        dong_view_update(view);

        // 简单限帧，避免 CPU 跑满
        SDL_Delay(16);
    }

    std::printf("Exiting GPU view demo.\n");

    dong_view_free(view);
    dong_destroy_context(ctx);
    // SDL3Window 析构时会销毁窗口和 GPU 设备，并调用 SDL_Quit()

    return 0;
}
