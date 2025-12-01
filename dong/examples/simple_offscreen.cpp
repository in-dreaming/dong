#include <cstdio>
#include <vector>
#include <dong.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

int main() {
    printf("=== Simple Offscreen Test ===\n");

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Test", 800, 400, SDL_WINDOW_HIDDEN);
    SDL_GPUDevice* device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_MSL, false, nullptr);
    SDL_ClaimWindowForGPUDevice(device, window);

    dong_context_t* ctx = dong_create_context();
    dong_view_t* view = dong_view_create(ctx, 800, 400);
    
    printf("Setting GPU device...\n");
    dong_view_set_external_gpu_device(view, device, window);

    printf("Loading HTML...\n");
    dong_view_load_html(view, "<html><body><h1>Test</h1></body></html>");

    printf("Calling renderOffscreen...\n");
    std::vector<uint8_t> pixels(800 * 400 * 4);
    
    if (dong_view_render_offscreen(view, device, 800, 400, pixels.data())) {
        printf("SUCCESS! Got pixels\n");
        
        int count = 0;
        for (size_t i = 0; i < 800 * 400; i++) {
            if (pixels[i*4] < 200 || pixels[i*4+1] < 200 || pixels[i*4+2] < 200) {
                count++;
            }
        }
        printf("Non-white pixels: %d (%.1f%%)\n", count, 100.0*count/(800*400));
    } else {
        printf("FAILED!\n");
    }

    dong_view_destroy(view);
    dong_destroy_context(ctx);
    SDL_DestroyGPUDevice(device);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
