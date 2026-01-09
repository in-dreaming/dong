#include "sdl3_window.hpp"
#include <SDL3/SDL_log.h>

namespace dong::platform {

SDL3Window::~SDL3Window() {
    if (gpu_device_) {
        SDL_DestroyGPUDevice(gpu_device_);
        gpu_device_ = nullptr;
    }

    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }

    SDL_Quit();
}

bool SDL3Window::initialize(const CreateInfo& info) {
    if (window_) {
        SDL_Log("Window already initialized");
        return false;
    }

    if (!initializeSDL()) {
        return false;
    }

    if (!createWindow(info)) {
        SDL_Quit();
        return false;
    }

    use_gpu_ = info.use_gpu;
    if (use_gpu_) {
        if (!createGPUDevice(info.debug_mode)) {
            SDL_DestroyWindow(window_);
            window_ = nullptr;
            SDL_Quit();
            return false;
        }
    }

    return true;
}

bool SDL3Window::processEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                should_close_ = true;
                return false;

            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                should_close_ = true;
                return false;

            case SDL_EVENT_WINDOW_RESIZED: {
                SDL_GetWindowSize(window_, (int*)&width_, (int*)&height_);
                SDL_Log("Window resized to %u x %u", width_, height_);
                break;
            }

            default:
                break;
        }
    }

    return true;
}

void SDL3Window::resize(uint32_t width, uint32_t height) {
    if (window_ && (width != width_ || height != height_)) {
        SDL_SetWindowSize(window_, (int)width, (int)height);
        width_ = width;
        height_ = height;
    }
}

bool SDL3Window::initializeSDL() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return false;
    }

    SDL_Log("SDL initialized successfully");
    return true;
}

bool SDL3Window::createWindow(const CreateInfo& info) {
    SDL_WindowFlags flags = SDL_WINDOW_HIDDEN;

    // GPU 渲染目前不需要特殊的窗口 flag，SDL 会在 ClaimWindowForGPUDevice 时建立 swapchain
    window_ = SDL_CreateWindow(info.title, (int)info.width, (int)info.height, flags);
    if (!window_) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }

    width_ = info.width;
    height_ = info.height;

    SDL_ShowWindow(window_);
    SDL_Log("Window created successfully: %u x %u", width_, height_);

    return true;
}

bool SDL3Window::createGPUDevice(bool debug_mode) {
    // 选择最优的着色器格式（位掩码）
#ifdef __APPLE__
    SDL_GPUShaderFormat format_flags = SDL_GPU_SHADERFORMAT_MSL;
#else
    SDL_GPUShaderFormat format_flags =
        SDL_GPU_SHADERFORMAT_SPIRV |
        SDL_GPU_SHADERFORMAT_DXIL;
#endif

    gpu_device_ = SDL_CreateGPUDevice(format_flags, debug_mode, nullptr);
    if (!gpu_device_) {
        SDL_Log("Failed to create GPU device: %s", SDL_GetError());
        return false;
    }

    // 将 GPU 设备与窗口关联
    if (!SDL_ClaimWindowForGPUDevice(gpu_device_, window_)) {
        SDL_Log("Failed to claim window for GPU device: %s", SDL_GetError());
        SDL_DestroyGPUDevice(gpu_device_);
        gpu_device_ = nullptr;
        return false;
    }

    // 设置 swapchain 参数：使用 MAILBOX 模式
    // MAILBOX 模式会在有新帧时丢弃旧帧，避免 swapchain 纹理为 null 的问题
    // 如果 MAILBOX 不支持，回退到 VSYNC
    if (!SDL_SetGPUSwapchainParameters(
        gpu_device_,
        window_,
        SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
        SDL_GPU_PRESENTMODE_MAILBOX
    )) {
        SDL_Log("MAILBOX present mode not supported, falling back to VSYNC");
        SDL_SetGPUSwapchainParameters(
            gpu_device_,
            window_,
            SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
            SDL_GPU_PRESENTMODE_VSYNC
        );
    }

    SDL_Log("GPU device created and claimed for window");
    return true;
}

} // namespace dong::platform
