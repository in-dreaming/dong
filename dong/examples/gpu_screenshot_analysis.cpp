#include <cstdio>
#include <cstdint>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <dong.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

// 简单的 BMP 写入函数
bool writeBMP(const char* filename, uint32_t width, uint32_t height, const uint8_t* rgba_data) {
    FILE* f = fopen(filename, "wb");
    if (!f) return false;
    
    uint32_t filesize = 54 + width * height * 3;
    uint8_t bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
    bmpfileheader[2] = (uint8_t)(filesize);
    bmpfileheader[3] = (uint8_t)(filesize>>8);
    bmpfileheader[4] = (uint8_t)(filesize>>16);
    bmpfileheader[5] = (uint8_t)(filesize>>24);
    
    uint8_t bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
    bmpinfoheader[4] = (uint8_t)(width);
    bmpinfoheader[5] = (uint8_t)(width>>8);
    bmpinfoheader[6] = (uint8_t)(width>>16);
    bmpinfoheader[7] = (uint8_t)(width>>24);
    bmpinfoheader[8] = (uint8_t)(height);
    bmpinfoheader[9] = (uint8_t)(height>>8);
    bmpinfoheader[10] = (uint8_t)(height>>16);
    bmpinfoheader[11] = (uint8_t)(height>>24);
    
    fwrite(bmpfileheader, 1, 14, f);
    fwrite(bmpinfoheader, 1, 40, f);
    
    for (int y = height - 1; y >= 0; y--) {
        for (uint32_t x = 0; x < width; x++) {
            uint32_t idx = (y * width + x) * 4;
            uint8_t rgb[3] = {rgba_data[idx + 2], rgba_data[idx + 1], rgba_data[idx + 0]};
            fwrite(rgb, 3, 1, f);
        }
    }
    
    fclose(f);
    return true;
}

int main() {
    std::printf("=== GPU Screenshot Analysis - Offscreen Render Test ===\n");

    // 1. 初始化 SDL 和 GPU
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Failed to init SDL: %s", SDL_GetError());
        return 1;
    }

    // 创建隐藏窗口（用于初始化 GPU device）
    SDL_Window* window = SDL_CreateWindow("GPU Screenshot", 800, 400, SDL_WINDOW_HIDDEN);
    if (!window) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_GPUDevice* device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_MSL, false, nullptr);
    if (!device) {
        SDL_Log("Failed to create GPU device: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!SDL_ClaimWindowForGPUDevice(device, window)) {
        SDL_Log("Failed to claim window: %s", SDL_GetError());
        SDL_DestroyGPUDevice(device);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // 2. 创建 Dong 上下文和视图
    dong_context_t* ctx = dong_create_context();
    if (!ctx) {
        std::printf("ERROR: Failed to create context\n");
        SDL_DestroyGPUDevice(device);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    const uint32_t width = 800;
    const uint32_t height = 400;
    
    dong_view_t* view = dong_view_create(ctx, width, height);
    if (!view) {
        std::printf("ERROR: Failed to create view\n");
        dong_destroy_context(ctx);
        SDL_DestroyGPUDevice(device);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // 3. 切换到 GPU 渲染模式
    std::printf("[Init] Enabling GPU render mode...\n");
    dong_view_set_external_gpu_device(view, static_cast<void*>(device), static_cast<void*>(window));

    // 4. 加载测试 HTML
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
                    background: #f0f0f0;
                    padding: 10px;
                }
                
                .size-48 { font-size: 48px; color: #000000; }
                .size-32 { font-size: 32px; color: #ff0000; }
                .size-24 { font-size: 24px; color: #0000ff; }
            </style>
        </head>
        <body>
            <div class="test-line size-48">ABCDEFG</div>
            <div class="test-line size-32">Test MSDF</div>
            <div class="test-line size-24">Hello World</div>
        </body>
        </html>
    )";

    std::printf("[Load] Loading HTML...\n");
    dong_view_load_html(view, html);

    SDL_Delay(500);

    // 6. 使用新的离屏渲染 API
    std::printf("[Render] Rendering offscreen...\n");
    std::vector<uint8_t> pixels(width * height * 4);
    
    if (!dong_view_render_offscreen(view, device, width, height, pixels.data())) {
        std::printf("ERROR: Failed to render offscreen\n");
        dong_view_destroy(view);
        dong_destroy_context(ctx);
        SDL_DestroyGPUDevice(device);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // 7. 保存图片
    const char* output_file = "gpu_screenshot.bmp";
    if (writeBMP(output_file, width, height, pixels.data())) {
        std::printf("[Save] Saved to %s\n", output_file);
    } else {
        std::printf("ERROR: Failed to save BMP\n");
    }

    // 8. 分析像素
    std::printf("\n=== PIXEL ANALYSIS ===\n");
    
    int total_black = 0, total_white = 0, total_gray = 0, total_colored = 0;
    int total_pixels = width * height;
    
    for (uint32_t i = 0; i < total_pixels; i++) {
        int idx = i * 4;
        uint8_t r = pixels[idx];
        uint8_t g = pixels[idx + 1];
        uint8_t b = pixels[idx + 2];
        
        int brightness = (r + g + b) / 3;
        
        // 检测彩色像素
        int max_diff = std::max({std::abs(r - g), std::abs(g - b), std::abs(r - b)});
        if (max_diff > 30) {
            total_colored++;
        } else if (brightness < 50) {
            total_black++;
        } else if (brightness > 200) {
            total_white++;
        } else {
            total_gray++;
        }
    }
    
    std::printf("Total pixels: %d\n", total_pixels);
    std::printf("  Black (<50): %d (%.1f%%)\n", total_black, 100.0*total_black/total_pixels);
    std::printf("  Gray (50-200): %d (%.1f%%)\n", total_gray, 100.0*total_gray/total_pixels);
    std::printf("  White (>200): %d (%.1f%%)\n", total_white, 100.0*total_white/total_pixels);
    std::printf("  Colored: %d (%.1f%%)\n", total_colored, 100.0*total_colored/total_pixels);
    
    std::printf("\n=== RENDER QUALITY ASSESSMENT ===\n");
    
    if (total_black == total_pixels) {
        std::printf("❌ FAIL: Image is completely black! Rendering failed.\n");
    } else if (total_white == total_pixels) {
        std::printf("❌ FAIL: Image is completely white! No text rendered.\n");
    } else if (total_black == 0 && total_gray == 0 && total_colored == 0) {
        std::printf("❌ FAIL: No text pixels detected!\n");
    } else {
        std::printf("✓ PASS: Text pixels detected\n");
        
        // 检查是否有足够的文字像素
        int text_pixels = total_black + total_gray + total_colored;
        float text_ratio = 100.0 * text_pixels / total_pixels;
        std::printf("  Text coverage: %.1f%%\n", text_ratio);
        
        if (text_ratio < 1.0) {
            std::printf("⚠️  WARNING: Text coverage is very low (< 1%%)\n");
        } else if (text_ratio > 50.0) {
            std::printf("⚠️  WARNING: Text coverage is very high (> 50%%), possible rendering issue\n");
        } else {
            std::printf("  Text coverage looks reasonable\n");
        }
        
        // 检查是否有彩色文字（红色和蓝色）
        if (total_colored > 0) {
            std::printf("✓ Colored text detected (%d pixels)\n", total_colored);
        }
    }

    // 9. 采样特定点
    std::printf("\n=== SAMPLE POINTS ===\n");
    struct {
        int x, y;
        const char* desc;
    } samples[] = {
        {100, 50, "Line 1 (48px black)"},
        {100, 120, "Line 2 (32px red)"},
        {100, 190, "Line 3 (24px blue)"},
        {400, 50, "Line 1 middle"},
    };

    for (const auto& sample : samples) {
        if (sample.x < (int)width && sample.y < (int)height) {
            int idx = (sample.y * width + sample.x) * 4;
            uint8_t r = pixels[idx];
            uint8_t g = pixels[idx + 1];
            uint8_t b = pixels[idx + 2];
            uint8_t a = pixels[idx + 3];
            
            int brightness = (r + g + b) / 3;
            const char* type = "unknown";
            if (brightness < 50) type = "black";
            else if (brightness > 200) type = "white/bg";
            else type = "gray/text";
            
            std::printf("  (%4d,%4d) %s: RGBA(%3d,%3d,%3d,%3d) [%s]\n",
                       sample.x, sample.y, sample.desc, r, g, b, a, type);
        }
    }

    // 10. 清理
    dong_view_destroy(view);
    dong_destroy_context(ctx);
    SDL_DestroyGPUDevice(device);
    SDL_DestroyWindow(window);
    SDL_Quit();

    std::printf("\n[Done] Check %s for visual verification\n", output_file);
    return 0;
}
