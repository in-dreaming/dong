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
    std::printf("=== GPU Screenshot Demo - MSDF Offscreen ===\n");

    // 1. 创建带 GPU 的 SDL 窗口（复用现有 SDL3Window 封装）
    SDL3Window::CreateInfo ci{};
    ci.title = "GPU Screenshot Demo";
    ci.width = 800;
    ci.height = 400;
    ci.use_gpu = true;
    ci.debug_mode = false;

    SDL3Window window;
    if (!window.initialize(ci)) {
        std::printf("ERROR: Failed to initialize SDL3Window: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GPUDevice* device = window.getGPUDevice();
    if (!device) {
        std::printf("ERROR: GPU device is null\n");
        return 1;
    }

    // 2. 创建 Dong 上下文 + 视图
    std::printf("[Init] Creating dong context...\n");
    dong_context_t* ctx = dong_create_context();
    if (!ctx) {
        std::printf("ERROR: Failed to create dong context\n");
        return 1;
    }

    std::printf("[Init] Creating dong view...\n");
    dong_view_t* view = dong_view_create(ctx, ci.width, ci.height);
    if (!view) {
        std::printf("ERROR: Failed to create dong view\n");
        dong_destroy_context(ctx);
        return 1;
    }

    // 3. 切换到 GPU 渲染模式
    std::printf("[Init] Enabling GPU render mode...\n");
    dong_view_set_external_gpu_device(view,
                                      static_cast<void*>(device),
                                      static_cast<void*>(window.getHandle()));

    // 4. 加载测试 HTML - 大字号 MSDF 文字（与原 demo 一致）
    const char* html = R"(
        <!DOCTYPE html>
        <html>
        <head>
            <style>
                body {
                    margin: 0;
                    padding: 20px;
                    background-color: #ffffff;
                    font-family: Arial, sans-serif;
                }
                
                .test-line {
                    margin: 15px 0;
                    padding: 10px;
                    background: #f0f0f0;
                }
                
                .size-72 { font-size: 72px; color: #000000; }
                .size-48 { font-size: 48px; color: #ff0000; }
                .size-36 { font-size: 36px; color: #0000ff; }
            </style>
        </head>
        <body>
            <div class="test-line size-72">ABC</div>
            <div class="test-line size-48">Test</div>
            <div class="test-line size-36">Hello</div>
        </body>
        </html>
    )";

    std::printf("[Load] Loading HTML...\n");
    dong_view_load_html(view, html);

    // 5. 给 Dong 一点时间做首次布局/渲染（可选）
    std::printf("[Render] Letting view update once...\n");
    dong_view_update(view);
    SDL_Delay(200);

    // 6. 使用 dong_view_render_offscreen 渲染到 RGBA 缓冲
    const uint32_t width = ci.width;
    const uint32_t height = ci.height;
    std::vector<uint8_t> pixels(width * height * 4);

    std::printf("[Render] Rendering offscreen via dong_view_render_offscreen(%ux%u)...\n",
                width, height);
    if (!dong_view_render_offscreen(view,
                                    static_cast<void*>(device),
                                    width,
                                    height,
                                    pixels.data())) {
        std::printf("ERROR: dong_view_render_offscreen failed\n");

        std::printf("[Cleanup] Shutting down...\n");
        dong_view_destroy(view);
        dong_destroy_context(ctx);
        return 1;
    }

    // 7. 保存 BMP
    const char* output_file = "gpu_screenshot.bmp";
    if (writeBMP(output_file, width, height, pixels.data())) {
        std::printf("[Save] Saved to %s\n", output_file);
    } else {
        std::printf("ERROR: Failed to save BMP\n");
    }

    // 8. 简单像素统计（参考 gpu_screenshot_analysis 的风格）
    std::printf("\n=== PIXEL ANALYSIS ===\n");
    int total_pixels = static_cast<int>(width * height);
    int total_black = 0, total_white = 0, total_gray = 0;

    for (int i = 0; i < total_pixels; ++i) {
        int idx = i * 4;
        int brightness = (pixels[idx] + pixels[idx + 1] + pixels[idx + 2]) / 3;
        if (brightness < 50) total_black++;
        else if (brightness > 200) total_white++;
        else total_gray++;
    }

    std::printf("Total pixels: %d\n", total_pixels);
    std::printf("  Black (<50): %d (%.1f%%)\n", total_black, 100.0 * total_black / total_pixels);
    std::printf("  Gray (50-200): %d (%.1f%%)\n", total_gray, 100.0 * total_gray / total_pixels);
    std::printf("  White (>200): %d (%.1f%%)\n", total_white, 100.0 * total_white / total_pixels);

    if (total_white == total_pixels) {
        std::printf("\n⚠️  WARNING: Image is all white! Offscreen rendering produced blank output.\n");
    } else if (total_black > 100 || total_gray > 100) {
        std::printf("\n✅ SUCCESS: Text pixels detected in offscreen screenshot!\n");
    }

    // 9. 清理
    std::printf("\n[Cleanup] Shutting down...\n");
    dong_view_destroy(view);
    dong_destroy_context(ctx);

    std::printf("[Done] Check %s for the captured MSDF text.\n", output_file);
    return 0;
}
