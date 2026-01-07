#include <cstdio>
#include <cstdint>
#include <vector>
#include <cstring>

#include <dong.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include "platform/sdl3_window.hpp"

using dong::platform::SDL3Window;

// 简单的 BMP 写入函数（与 gpu_screenshot_analysis 保持一致）
bool writeBMP(const char* filename, uint32_t width, uint32_t height, const uint8_t* rgba_data) {
    FILE* f = fopen(filename, "wb");
    if (!f) return false;

    uint32_t filesize = 54 + width * height * 3;
    uint8_t bmpfileheader[14] = {
        'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0
    };
    bmpfileheader[2] = (uint8_t)(filesize);
    bmpfileheader[3] = (uint8_t)(filesize >> 8);
    bmpfileheader[4] = (uint8_t)(filesize >> 16);
    bmpfileheader[5] = (uint8_t)(filesize >> 24);

    uint8_t bmpinfoheader[40] = {
        40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0
    };
    bmpinfoheader[4]  = (uint8_t)(width);
    bmpinfoheader[5]  = (uint8_t)(width >> 8);
    bmpinfoheader[6]  = (uint8_t)(width >> 16);
    bmpinfoheader[7]  = (uint8_t)(width >> 24);
    bmpinfoheader[8]  = (uint8_t)(height);
    bmpinfoheader[9]  = (uint8_t)(height >> 8);
    bmpinfoheader[10] = (uint8_t)(height >> 16);
    bmpinfoheader[11] = (uint8_t)(height >> 24);

    fwrite(bmpfileheader, 1, 14, f);
    fwrite(bmpinfoheader, 1, 40, f);

    for (int y = static_cast<int>(height) - 1; y >= 0; --y) {
        for (uint32_t x = 0; x < width; ++x) {
            uint32_t idx = (y * width + x) * 4;
            uint8_t rgb[3] = {
                rgba_data[idx + 2],
                rgba_data[idx + 1],
                rgba_data[idx + 0],
            };
            fwrite(rgb, 3, 1, f);
        }
    }

    fclose(f);
    return true;
}

int main() {
    SDL_Log("=== GPU Screenshot Demo - MSDF Offscreen ===");

    // 1. 创建带 GPU 的 SDL 窗口（复用现有 SDL3Window 封装）
    SDL3Window::CreateInfo ci{};
    ci.title = "GPU Screenshot Demo";
    ci.width = 800;
    ci.height = 1200;
    ci.use_gpu = true;
    ci.debug_mode = false;

    SDL3Window window;
    if (!window.initialize(ci)) {
        SDL_Log("ERROR: Failed to initialize SDL3Window: %s", SDL_GetError());
        return 1;
    }

    SDL_GPUDevice* device = window.getGPUDevice();
    if (!device) {
        SDL_Log("ERROR: GPU device is null");
        return 1;
    }

    // 2. 创建 Dong 上下文 + 视图
    SDL_Log("[Init] Creating dong context...");
    dong_context_t* ctx = dong_create_context();
    if (!ctx) {
        SDL_Log("ERROR: Failed to create dong context");
        return 1;
    }

    SDL_Log("[Init] Creating dong view...");
    dong_view_t* view = dong_view_create(ctx, ci.width, ci.height);
    if (!view) {
        SDL_Log("ERROR: Failed to create dong view");
        dong_destroy_context(ctx);
        return 1;
    }

    // 3. 切换到 GPU 渲染模式
    SDL_Log("[Init] Enabling GPU render mode...");
    dong_view_set_external_gpu_device(view,
                                      static_cast<void*>(device),
                                      static_cast<void*>(window.getHandle()));
    SDL_Log("[Init] GPU render mode enabled");

    // 4. 加载测试 HTML - 大字号 MSDF 文字（与原 demo 一致）
    // <!font-family: Arial, sans-serif;/>
    const char* html = R"(
<!DOCTYPE html>
        <html>
        <head>
            <style>
                * {
                    box-sizing: border-box;
                }
                body {
                    margin: 0;
                    padding: 20px;
                    background-color: #1a1a2e;
                    color: #eaeaea;
                    font-family: Arial, sans-serif;
                }
                
                h1 {
                    font-size: 28px;
                    color: #00d4ff;
                    margin-bottom: 20px;
                }
                
                .section {
                    background-color: #16213e;
                    border-radius: 8px;
                    padding: 16px;
                    margin-bottom: 16px;
                }
                
                .section-title {
                    font-size: 18px;
                    color: #e94560;
                    margin-bottom: 12px;
                }
                
                /* Button Section */
                .button-row {
                    display: flex;
                    gap: 12px;
                }
                
                button {
                    background-color: #0f3460;
                    color: #ffffff;
                    padding: 12px 24px;
                    border-radius: 6px;
                    font-size: 16px;
                }
                
                #click-count {
                    margin-top: 12px;
                    font-size: 14px;
                    color: #94a3b8;
                }
                
                /* ScrollView Section */
                .scroll-container {
                    overflow: scroll;
                    height: 150px;
                    background-color: #0f3460;
                    border-radius: 6px;
                    padding: 8px;
                }
                
                .scroll-item {
                    padding: 10px;
                    margin-bottom: 8px;
                    background-color: #1a1a2e;
                    border-radius: 4px;
                }
                
                /* Input Section */
                .input-group {
                    margin-bottom: 12px;
                }
                
                .input-label {
                    font-size: 14px;
                    color: #94a3b8;
                    margin-bottom: 6px;
                }
                
                input {
                    width: 100%;
                    padding: 12px;
                    background-color: #0f3460;
                    color: #ffffff;
                    border-radius: 6px;
                    font-size: 16px;
                }
                
                #input-preview {
                    margin-top: 12px;
                    padding: 12px;
                    background-color: #0f3460;
                    border-radius: 6px;
                    font-size: 14px;
                    color: #94a3b8;
                }
                
                /* Status Bar */
                .status-bar {
                    position: fixed;
                    bottom: 0;
                    left: 0;
                    right: 0;
                    padding: 8px 20px;
                    background-color: #0f3460;
                    font-size: 12px;
                    color: #64748b;
                }
            </style>
        </head>
        <body>
            <h1>Dong Interactive Demo</h1>
            
            <!-- Button Section -->
            <div class="section">
                <div class="section-title">Button Clicks</div>
                <div class="button-row">
                    <button id="btn-primary">Primary Button</button>
                    <button id="btn-secondary">Secondary</button>
                    <button id="btn-reset">Reset</button>
                </div>
                <div id="click-count">Clicks: 0</div>
            </div>
            
            <!-- ScrollView Section -->
            <div class="section">
                <div class="section-title">ScrollView (use mouse wheel)</div>
                <div class="scroll-container" id="scroll-area">
                    <div class="scroll-item">Item 1 - Scroll down to see more</div>
                    <div class="scroll-item">Item 2 - Lorem ipsum dolor sit amet</div>
                    <div class="scroll-item">Item 3 - Consectetur adipiscing elit</div>
                    <div class="scroll-item">Item 4 - Sed do eiusmod tempor</div>
                    <div class="scroll-item">Item 5 - Incididunt ut labore</div>
                    <div class="scroll-item">Item 6 - Et dolore magna aliqua</div>
                    <div class="scroll-item">Item 7 - Ut enim ad minim veniam</div>
                    <div class="scroll-item">Item 8 - Quis nostrud exercitation</div>
                </div>
            </div>
            
            <!-- Input Section -->
            <div class="section">
                <div class="section-title">Text Input</div>
                <div class="input-group">
                    <div class="input-label">Enter your name:</div>
                    <input type="text" id="name-input" placeholder="Type here..." />
                </div>
                <div class="input-group">
                    <div class="input-label">Enter your message:</div>
                    <input type="text" id="message-input" placeholder="Your message..." />
                </div>
                <div id="input-preview">Preview: (type something above)</div>
            </div>
            
            <div class="status-bar" id="status">
                Ready. Press Tab to navigate, click buttons, scroll the list, or type in inputs.
            </div>
        </body>
        </html>
    )";

    SDL_Log("[Load] Loading HTML...");
    dong_view_load_html(view, html);
    SDL_Log("[Load] HTML loaded successfully");

    // 辅助函数：渲染一帧并保存截图
    auto renderAndSave = [&](int frame_num, const char* filename) {
        const uint32_t w = ci.width;
        const uint32_t h = ci.height;
        std::vector<uint8_t> pixels(w * h * 4);

        SDL_Log("[Frame %d] Rendering offscreen...", frame_num);
        // 注意：不调用 dong_view_update，因为它会尝试渲染到窗口
        // dong_view_render_offscreen 内部会更新布局
        
        if (!dong_view_render_offscreen(view, static_cast<void*>(device), w, h, pixels.data())) {
            SDL_Log("ERROR: dong_view_render_offscreen failed for frame %d", frame_num);
            return false;
        }

        if (writeBMP(filename, w, h, pixels.data())) {
            SDL_Log("[Frame %d] Saved to %s", frame_num, filename);
        }

        // 像素统计
        int total = static_cast<int>(w * h);
        int black = 0, gray = 0, white = 0;
        for (int i = 0; i < total; ++i) {
            int idx = i * 4;
            int brightness = (pixels[idx] + pixels[idx + 1] + pixels[idx + 2]) / 3;
            if (brightness < 50) black++;
            else if (brightness > 200) white++;
            else gray++;
        }
        SDL_Log("[Frame %d] Pixels: black=%d(%.1f%%) gray=%d(%.1f%%) white=%d(%.1f%%)",
                frame_num, black, 100.0*black/total, gray, 100.0*gray/total, white, 100.0*white/total);
        
        return true;
    };



    // 渲染第1帧
    if (!renderAndSave(1, "frame1.bmp")) {
        dong_view_destroy(view);
        dong_destroy_context(ctx);
        return 1;
    }

    // 渲染第2帧
    if (!renderAndSave(2, "frame2.bmp")) {
        dong_view_destroy(view);
        dong_destroy_context(ctx);
        return 1;
    }

    // 渲染第3帧
    if (!renderAndSave(3, "frame3.bmp")) {
        dong_view_destroy(view);
        dong_destroy_context(ctx);
        return 1;
    }



    // 清理
    SDL_Log("[Cleanup] Shutting down...");
    dong_view_destroy(view);
    dong_destroy_context(ctx);

    SDL_Log("[Done] Check frame1.bmp, frame2.bmp, frame3.bmp");
    return 0;
}
