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

    const char* html = R"(
        <!DOCTYPE html>
        <html>
        <head>
            <meta charset="utf-8" />
            <title>Dong GPU View Demo</title>
            <style>
                html, body {
                    margin: 0;
                    padding: 0;
                    width: 100%;
                    height: 100%;
                }

                body {
                    background-color: #20262e;
                    font-family: Arial, sans-serif;
                    display: flex;
                    justify-content: center;
                    align-items: center;
                    color: #ffffff;
                }

                .card {
                    background-color: #2f3640;
                    border-radius: 12px;
                    padding: 24px 28px;
                    box-shadow: 0 8px 20px rgba(0, 0, 0, 0.4);
                    min-width: 520px;
                }

                h1 {
                    margin: 0 0 8px 0;
                    font-size: 24px;
                }

                p {
                    margin: 4px 0;
                    color: #d0d0d0;
                    line-height: 1.5;
                }

                .pill-row {
                    margin: 12px 0 8px 0;
                }

                .pill {
                    display: inline-block;
                    padding: 4px 10px;
                    border-radius: 999px;
                    background-color: #374151;
                    color: #e5e7eb;
                    font-size: 12px;
                    margin-right: 6px;
                }

                .button-row {
                    margin-top: 16px;
                    display: flex;
                    gap: 10px;
                }

                .btn {
                    padding: 8px 16px;
                    border-radius: 6px;
                    border: none;
                    cursor: pointer;
                    font-size: 14px;
                }

                .btn.primary {
                    background-color: #3b82f6;
                    color: #ffffff;
                }

                .btn.secondary {
                    background-color: #4b5563;
                    color: #e5e7eb;
                }

                #status {
                    margin-top: 10px;
                    font-size: 13px;
                    color: #9ca3af;
                }
            </style>
        </head>
        <body>
            <div class="card">
                <h1>Dong GPU View Demo</h1>
                <p>This window is driven by SDL3 + SDL_gpu, while Dong handles HTML/CSS/JS and DOM events.</p>
                <div class="pill-row">
                    <span class="pill">HTML/CSS</span>
                    <span class="pill">DOM Events</span>
                    <span class="pill">SDL_gpu</span>
                </div>
                <p>Move the mouse and click / press keys in this window. Events are forwarded to Dong's JS layer.</p>
                <div class="button-row">
                    <button class="btn primary" id="primaryBtn">Primary</button>
                    <button class="btn secondary" id="secondaryBtn">Secondary</button>
                </div>
                <div id="status">Waiting for input...</div>
            </div>

            <script>
                (function() {
                    console.log('[GPU Demo] Script running');

                    var primary = document.getElementById('primaryBtn');
                    var secondary = document.getElementById('secondaryBtn');
                    var status = document.getElementById('status');

                    function setStatus(text) {
                        if (status) {
                            status.textContent = text;
                        }
                        console.log('[GPU Demo] ' + text);
                    }

                    if (primary) {
                        primary.addEventListener('click', function() {
                            setStatus('Primary button clicked.');
                        });
                    }

                    if (secondary) {
                        secondary.addEventListener('click', function() {
                            setStatus('Secondary button clicked.');
                        });
                    }

                    window.addEventListener('mousemove', function(ev) {
                        setStatus('Mouse move: (' + ev.clientX + ', ' + ev.clientY + ')');
                    });

                    window.addEventListener('keydown', function(ev) {
                        setStatus('Key down: ' + ev.key);
                    });

                    window.addEventListener('keyup', function(ev) {
                        setStatus('Key up: ' + ev.key);
                    });

                    setStatus('GPU view initialized.');
                })();
            </script>
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
        dong_view_update(view);

        // 5. 使用 SDL_gpu 正常渲染一帧（这里先只做清屏，保持典型 GPU 流程）
        SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
        if (!cmd) {
            std::printf("ERROR: SDL_AcquireGPUCommandBuffer failed: %s\n", SDL_GetError());
            break;
        }

        SDL_GPUTexture* swapchain_texture = nullptr;
        Uint32 w = 0;
        Uint32 h = 0;

        if (!SDL_WaitAndAcquireGPUSwapchainTexture(
                cmd,
                window.getHandle(),
                &swapchain_texture,
                &w,
                &h)) {
            std::printf("ERROR: SDL_WaitAndAcquireGPUSwapchainTexture failed: %s\n", SDL_GetError());
            SDL_SubmitGPUCommandBuffer(cmd);
            break;
        }

        SDL_GPUColorTargetInfo color_target{};
        color_target.texture = swapchain_texture;
        color_target.mip_level = 0;
        color_target.layer_or_depth_plane = 0;
        color_target.clear_color = SDL_FColor{0.1f, 0.2f, 0.4f, 1.0f};
        color_target.load_op = SDL_GPU_LOADOP_CLEAR;
        color_target.store_op = SDL_GPU_STOREOP_STORE;
        color_target.resolve_texture = nullptr;
        color_target.resolve_mip_level = 0;
        color_target.resolve_layer = 0;
        color_target.cycle = false;
        color_target.cycle_resolve_texture = false;

        SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(
            cmd,
            &color_target,
            1,
            nullptr
        );

        if (!pass) {
            std::printf("ERROR: SDL_BeginGPURenderPass failed: %s\n", SDL_GetError());
            SDL_SubmitGPUCommandBuffer(cmd);
            break;
        }

        // TODO：后续可以在这里采样 Dong 渲染的 GPU/CPU 结果贴到窗口上
        // 目前先保持一个干净的 GPU 清屏流程

        SDL_EndGPURenderPass(pass);
        SDL_SubmitGPUCommandBuffer(cmd);

        // 简单限帧，避免 CPU 跑满
        SDL_Delay(16);
    }

    std::printf("Exiting GPU view demo.\n");

    dong_view_free(view);
    dong_destroy_context(ctx);
    // SDL3Window 析构时会销毁窗口和 GPU 设备，并调用 SDL_Quit()

    return 0;
}
