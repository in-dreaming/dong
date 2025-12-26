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
    ci.height = 400;
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
                body {
                    background: rgb(240, 240, 255);
                    margin: 0;
                    padding: 10px;
                }
                .title {
                    font-size: 72px;
                    color: rgb(237, 46, 46);
                    margin: 10px 0;
                }
                .subtitle {
                    font-size: 48px;
                    color: rgb(50, 100, 200);
                    margin: 10px 0;
                }
                .text {
                    font-size: 36px;
                    color: rgb(50, 50, 50);
                    margin: 10px 0;
                }
            </style>
        </head>
        <body>
            <div class="title">MSDF OK!</div>
            <div class="subtitle">Font Test</div>
            <div class="text">ABC 123</div>
        </body>
        </html>
    )";

    SDL_Log("[Load] Loading HTML...");
    dong_view_load_html(view, html);
    SDL_Log("[Load] HTML loaded successfully");

    // 5. 给 Dong 一点时间做首次布局/渲染（可选）
    SDL_Log("[Render] Letting view update once...");
    dong_view_update(view);
    SDL_Log("[Render] View updated");
    SDL_Delay(200);

    // 6. 使用 dong_view_render_offscreen 渲染到 RGBA 缓冲
    const uint32_t width = ci.width;
    const uint32_t height = ci.height;
    std::vector<uint8_t> pixels(width * height * 4);

    SDL_Log("[Render] Rendering offscreen via dong_view_render_offscreen(%ux%u)...",
                width, height);
    if (!dong_view_render_offscreen(view,
                                    static_cast<void*>(device),
                                    width,
                                    height,
                                    pixels.data())) {
        SDL_Log("ERROR: dong_view_render_offscreen failed");

        SDL_Log("[Cleanup] Shutting down...");
        dong_view_destroy(view);
        dong_destroy_context(ctx);
        return 1;
    }

    // 7. 保存 BMP
    const char* output_file = "zig-out/tmp/gpu_screenshot.bmp";
    if (writeBMP(output_file, width, height, pixels.data())) {
        SDL_Log("[Save] Saved to %s", output_file);
    } else {
        SDL_Log("ERROR: Failed to save BMP");
    }

    // 8. 简单像素统计（参考 gpu_screenshot_analysis 的风格）
    SDL_Log("=== PIXEL ANALYSIS ===");
    int total_pixels = static_cast<int>(width * height);
    int total_black = 0, total_white = 0, total_gray = 0;
    int total_red = 0;  // 红色像素计数

    // 打印前 10 个像素的 RGBA 值
    SDL_Log("First 10 pixels (RGBA):");
    for (int i = 0; i < 10 && i < total_pixels; ++i) {
        int idx = i * 4;
        SDL_Log("  pixel[%d] = R:%d G:%d B:%d A:%d", 
                i, pixels[idx], pixels[idx+1], pixels[idx+2], pixels[idx+3]);
    }
    
    // 检查 row 10（与 view.cpp 中的调试对应）
    SDL_Log("Row 10 from demo pixels array:");
    for (int x = 0; x < 50; x += 10) {
        int idx = (10 * width + x) * 4;
        SDL_Log("  pixels[%d,10] = R:%d G:%d B:%d A:%d", 
                x, pixels[idx], pixels[idx+1], pixels[idx+2], pixels[idx+3]);
    }
    
    // 检查 (1,1) 附近的像素
    SDL_Log("Checking around (1,1):");
    for (int y = 0; y <= 3; ++y) {
        for (int x = 0; x <= 3; ++x) {
            int idx = (y * width + x) * 4;
            SDL_Log("  pixels[%d,%d] = R:%d G:%d B:%d A:%d", 
                    x, y, pixels[idx], pixels[idx+1], pixels[idx+2], pixels[idx+3]);
        }
    }

    // 找到第一个非白色像素的位置
    SDL_Log("Searching for first non-white pixel...");
    bool found_non_white = false;
    for (int y = 0; y < (int)height && !found_non_white; ++y) {
        for (int x = 0; x < (int)width && !found_non_white; ++x) {
            int idx = (y * width + x) * 4;
            if (pixels[idx] < 255 || pixels[idx+1] < 255 || pixels[idx+2] < 255) {
                SDL_Log("  First non-white at (%d,%d) = R:%d G:%d B:%d A:%d", 
                        x, y, pixels[idx], pixels[idx+1], pixels[idx+2], pixels[idx+3]);
                // 打印周围 5x5 区域
                SDL_Log("  Surrounding 5x5 area:");
                for (int dy = -2; dy <= 2; ++dy) {
                    int py = y + dy;
                    if (py < 0 || py >= (int)height) continue;
                    char line[256] = "    ";
                    for (int dx = -2; dx <= 2; ++dx) {
                        int px = x + dx;
                        if (px < 0 || px >= (int)width) {
                            strcat(line, "--- ");
                            continue;
                        }
                        int pidx = (py * width + px) * 4;
                        char buf[32];
                        snprintf(buf, sizeof(buf), "%3d ", (pixels[pidx] + pixels[pidx+1] + pixels[pidx+2]) / 3);
                        strcat(line, buf);
                    }
                    SDL_Log("%s", line);
                }
                found_non_white = true;
            }
        }
    }
    if (!found_non_white) {
        SDL_Log("  No non-white pixel found!");
    }

    for (int i = 0; i < total_pixels; ++i) {
        int idx = i * 4;
        int brightness = (pixels[idx] + pixels[idx + 1] + pixels[idx + 2]) / 3;
        if (brightness < 50) total_black++;
        else if (brightness > 200) total_white++;
        else total_gray++;
        
        // 检测红色像素 (R > 200, G < 100, B < 100)
        if (pixels[idx] > 200 && pixels[idx+1] < 100 && pixels[idx+2] < 100) {
            total_red++;
        }
    }

    SDL_Log("Total pixels: %d", total_pixels);
    SDL_Log("  Black (<50): %d (%.1f%%)", total_black, 100.0 * total_black / total_pixels);
    SDL_Log("  Gray (50-200): %d (%.1f%%)", total_gray, 100.0 * total_gray / total_pixels);
    SDL_Log("  White (>200): %d (%.1f%%)", total_white, 100.0 * total_white / total_pixels);
    SDL_Log("  Red pixels: %d", total_red);

    if (total_white == total_pixels) {
        SDL_Log("WARNING: Image is all white! Offscreen rendering produced blank output.");
    } else if (total_black > 100 || total_gray > 100) {
        SDL_Log("SUCCESS: Text pixels detected in offscreen screenshot!");
    }


    // 9. 清理
    SDL_Log("[Cleanup] Shutting down...");
    dong_view_destroy(view);
    dong_destroy_context(ctx);

    SDL_Log("[Done] Check %s for the captured MSDF text.", output_file);
    return 0;
}
