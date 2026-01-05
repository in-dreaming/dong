#include <cstdio>
#include <cstdint>
#include <vector>
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
    std::printf("=== GPU Texture Rendering Demo ===\n");
    std::printf("This demo shows the two-layer offscreen rendering API:\n");
    std::printf("  1. renderToGPUTexture() - render to GPU texture only\n");
    std::printf("  2. renderOffscreen() - render + readback to memory\n\n");

    // 1. 初始化 SDL 和 GPU
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Failed to init SDL: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("GPU Texture Demo", 800, 400, SDL_WINDOW_HIDDEN);
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

    // 3. 启用 GPU 渲染模式
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

    std::printf("[Load] Loading HTML...\n");
    dong_view_load_html(view, html);

    // 5. 测试底层接口：渲染到 GPU 纹理
    std::printf("\n=== Test 1: renderToGPUTexture() ===\n");
    SDL_GPUTexture* gpu_texture = static_cast<SDL_GPUTexture*>(
        dong_view_render_to_gpu_texture(view, device, width, height)
    );
    
    if (gpu_texture) {
        std::printf("[Success] Rendered to GPU texture\n");
        std::printf("  - GPU texture can be used for compositing, effects, etc.\n");
        std::printf("  - No memory copy overhead\n");
        
        // 纹理可以在这里用于其他GPU操作...
        // 例如：作为另一个渲染通道的输入，应用后处理效果等
        
        // 用完后需要释放
        SDL_ReleaseGPUTexture(device, gpu_texture);
        std::printf("  - GPU texture released\n");
    } else {
        std::printf("[Failed] Could not render to GPU texture\n");
    }

    // 6. 测试上层接口：渲染并回读到内存
    std::printf("\n=== Test 2: renderOffscreen() ===\n");
    std::vector<uint8_t> pixels(width * height * 4);
    
    if (dong_view_render_offscreen(view, device, width, height, pixels.data())) {
        std::printf("[Success] Rendered and read back to memory\n");
        
        // 分析像素
        int total_black = 0, total_white = 0, total_gray = 0, total_colored = 0;
        int total_pixels = width * height;
        
        for (uint32_t i = 0; i < total_pixels; i++) {
            int idx = i * 4;
            uint8_t r = pixels[idx];
            uint8_t g = pixels[idx + 1];
            uint8_t b = pixels[idx + 2];
            
            int brightness = (r + g + b) / 3;
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
        
        std::printf("\n  Pixel Analysis:\n");
        std::printf("    Total: %d pixels\n", total_pixels);
        std::printf("    Black: %d (%.1f%%)\n", total_black, 100.0*total_black/total_pixels);
        std::printf("    Gray:  %d (%.1f%%)\n", total_gray, 100.0*total_gray/total_pixels);
        std::printf("    White: %d (%.1f%%)\n", total_white, 100.0*total_white/total_pixels);
        std::printf("    Color: %d (%.1f%%)\n", total_colored, 100.0*total_colored/total_pixels);
        
        int text_pixels = total_black + total_gray + total_colored;
        if (text_pixels > 0) {
            std::printf("\n  ✓ Text rendered successfully (%d pixels)\n", text_pixels);
        }
        
        // 保存到文件
        const char* output_file = "gpu_texture_output.bmp";
        if (writeBMP(output_file, width, height, pixels.data())) {
            std::printf("  ✓ Saved to %s\n", output_file);
        }
    } else {
        std::printf("[Failed] Could not render offscreen\n");
    }

    // 7. 清理
    std::printf("\n[Cleanup] Releasing resources...\n");
    dong_view_destroy(view);
    dong_destroy_context(ctx);
    SDL_DestroyGPUDevice(device);
    SDL_DestroyWindow(window);
    SDL_Quit();

    std::printf("\n=== Demo Complete ===\n");
    std::printf("Summary:\n");
    std::printf("  - renderToGPUTexture(): Fast, GPU-only, for compositing\n");
    std::printf("  - renderOffscreen(): Includes readback, for CPU processing\n");
    
    return 0;
}
