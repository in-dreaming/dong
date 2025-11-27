#include <cstdio>
#include <cstdint>

#include "platform/sdl3_window.hpp"

using dong::platform::SDL3Window;

int main() {
    std::printf("=== Dong SDL3 GPU Demo (clear window) ===\n");

    SDL3Window::CreateInfo ci{};
    ci.title = "Dong SDL3 GPU Demo";
    ci.width = 800;
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

    bool running = true;
    std::printf("Entering event/render loop... Close window to exit.\n");

    while (running && !window.shouldClose()) {
        if (!window.processEvents()) {
            break;
        }

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

        // 目前只做清屏，将来可以在这里调用 GPU pipeline 进行真正绘制
        SDL_EndGPURenderPass(pass);
        SDL_SubmitGPUCommandBuffer(cmd);
    }

    std::printf("Exiting SDL3 GPU demo.\n");
    return 0;
}
