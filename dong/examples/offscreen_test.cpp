#include <cstdio>
#include <vector>
#include <dong.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

// 简单的 PPM 写入函数
bool writePPM(const char* filename, uint32_t width, uint32_t height, const uint8_t* rgba_data) {
    FILE* f = fopen(filename, "wb");
    if (!f) return false;
    
    fprintf(f, "P6\n%u %u\n255\n", width, height);
    
    for (uint32_t i = 0; i < width * height; i++) {
        uint8_t rgb[3] = {rgba_data[i*4], rgba_data[i*4+1], rgba_data[i*4+2]};
        fwrite(rgb, 3, 1, f);
    }
    
    fclose(f);
    return true;
}

int main() {
    printf("=== Offscreen Rendering Test ===\n");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("Failed to init SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Offscreen Test", 800, 400, SDL_WINDOW_HIDDEN);
    if (!window) {
        printf("Failed to create window\n");
        SDL_Quit();
        return 1;
    }

    SDL_GPUDevice* device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_MSL, false, nullptr);
    if (!device) {
        printf("Failed to create GPU device\n");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!SDL_ClaimWindowForGPUDevice(device, window)) {
        printf("Failed to claim window\n");
        SDL_DestroyGPUDevice(device);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    dong_context_t* ctx = dong_create_context();
    if (!ctx) {
        printf("Failed to create context\n");
        SDL_DestroyGPUDevice(device);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    dong_view_t* view = dong_view_create(ctx, 800, 400);
    if (!view) {
        printf("Failed to create view\n");
        dong_destroy_context(ctx);
        SDL_DestroyGPUDevice(device);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    printf("[Init] Enabling GPU mode...\n");
    dong_view_set_external_gpu_device(view, device, window);

    printf("[Load] Loading HTML...\n");
    const char* html = R"(
        <html>
        <head><style>
            body { margin: 0; padding: 20px; background: #ffffff; font-family: Arial; }
            h1 { font-size: 48px; color: #000000; }
        </style></head>
        <body><h1>Test</h1></body>
        </html>
    )";
    dong_view_load_html(view, html);

    printf("[Render] Pre-rendering...\n");
    for (int i = 0; i < 3; i++) {
        dong_view_update(view);
        SDL_Delay(50);
    }

    printf("[Render] Offscreen rendering...\n");
    std::vector<uint8_t> pixels(800 * 400 * 4);
    
    if (dong_view_render_offscreen(view, device, 800, 400, pixels.data())) {
        printf("[Success] Rendered offscreen!\n");
        
        // 分析像素
        int black = 0, white = 0;
        for (size_t i = 0; i < 800 * 400; i++) {
            int brightness = (pixels[i*4] + pixels[i*4+1] + pixels[i*4+2]) / 3;
            if (brightness < 50) black++;
            else if (brightness > 200) white++;
        }
        
        printf("  Black pixels: %d (%.1f%%)\n", black, 100.0*black/(800*400));
        printf("  White pixels: %d (%.1f%%)\n", white, 100.0*white/(800*400));
        
        if (writePPM("offscreen_test.ppm", 800, 400, pixels.data())) {
            printf("[Save] Saved to offscreen_test.ppm\n");
        }
    } else {
        printf("[Error] Failed to render offscreen\n");
    }

    dong_view_destroy(view);
    dong_destroy_context(ctx);
    SDL_DestroyGPUDevice(device);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
