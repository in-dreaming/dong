#include "sdl_window.hpp"
#include "dong_sdl_gpu_formats.h"
#include "../src/core/log.h"

#include <cstdlib>
#include <cstring>


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
        DONG_LOG_WARN("Window already initialized");
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
    enable_hdr_request_ = info.enable_hdr;
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
                int pix_w = 0;
                int pix_h = 0;
                if (SDL_GetWindowSizeInPixels(window_, &pix_w, &pix_h) && pix_w > 0 && pix_h > 0) {
                    width_ = (uint32_t)pix_w;
                    height_ = (uint32_t)pix_h;
                } else {
                    SDL_GetWindowSize(window_, (int*)&width_, (int*)&height_);
                }
                DONG_LOG_INFO("Window resized to %u x %u", width_, height_);
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
        DONG_LOG_ERROR("Failed to initialize SDL: %s", SDL_GetError());
        return false;
    }

    DONG_LOG_INFO("SDL initialized successfully");
    return true;
}

bool SDL3Window::createWindow(const CreateInfo& info) {
    SDL_WindowFlags flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;

    // GPU 渲染目前不需要特殊的窗口 flag，SDL 会在 ClaimWindowForGPUDevice 时建立 swapchain
    window_ = SDL_CreateWindow(info.title, (int)info.width, (int)info.height, flags);

    if (!window_) {
        DONG_LOG_ERROR("Failed to create window: %s", SDL_GetError());
        return false;
    }

    width_ = info.width;
    height_ = info.height;

    SDL_ShowWindow(window_);
    DONG_LOG_INFO("Window created successfully: %u x %u", width_, height_);

    return true;
}

bool SDL3Window::createGPUDevice(bool debug_mode) {
    const SDL_GPUShaderFormat format_flags = dong_sdl_default_shader_formats();

    gpu_device_ = SDL_CreateGPUDevice(format_flags, debug_mode, nullptr);
    if (!gpu_device_) {
        DONG_LOG_ERROR("Failed to create GPU device: %s", SDL_GetError());
        return false;
    }

    // 将 GPU 设备与窗口关联
    if (!SDL_ClaimWindowForGPUDevice(gpu_device_, window_)) {
        DONG_LOG_ERROR("Failed to claim window for GPU device: %s", SDL_GetError());
        SDL_DestroyGPUDevice(gpu_device_);
        gpu_device_ = nullptr;
        return false;
    }

    // 允许通过环境变量控制 swapchain present mode / frames-in-flight（性能排查常用）。
    // - DONG_GPU_PRESENT_MODE=mailbox|vsync|immediate
    // - DONG_GPU_FRAMES_IN_FLIGHT=1|2|3
    // - DONG_HDR=1 to force HDR mode
    {
        if (const char* v = std::getenv("DONG_GPU_FRAMES_IN_FLIGHT")) {
            const int n = std::atoi(v);
            if (n >= 1 && n <= 3) {
                SDL_SetGPUAllowedFramesInFlight(gpu_device_, (Uint32)n);
                DONG_LOG_INFO("[GPU] AllowedFramesInFlight=%d", n);
            } else {
                DONG_LOG_WARN("[GPU] Invalid DONG_GPU_FRAMES_IN_FLIGHT=%s (expected 1..3)", v);
            }
        }

        // Check if HDR is requested (via CreateInfo or environment variable)
        bool request_hdr = enable_hdr_request_;
        if (const char* hdr_env = std::getenv("DONG_HDR")) {
            request_hdr = (std::strcmp(hdr_env, "1") == 0);
        }

        // Determine swapchain composition (SDR or HDR)
        SDL_GPUSwapchainComposition composition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR;
        hdr_enabled_ = false;

        if (request_hdr) {
            // Try HDR Extended Linear (R16G16B16A16_FLOAT)
            if (SDL_WindowSupportsGPUSwapchainComposition(gpu_device_, window_,
                                                          SDL_GPU_SWAPCHAINCOMPOSITION_HDR_EXTENDED_LINEAR)) {
                composition = SDL_GPU_SWAPCHAINCOMPOSITION_HDR_EXTENDED_LINEAR;
                hdr_enabled_ = true;
                DONG_LOG_INFO("[GPU] HDR Extended Linear supported and enabled");
            } else {
                DONG_LOG_INFO("[GPU] HDR Extended Linear not supported, using SDR");
            }
        }

        SDL_GPUPresentMode mode = SDL_GPU_PRESENTMODE_MAILBOX;
        bool mode_from_env = false;
        if (const char* pm = std::getenv("DONG_GPU_PRESENT_MODE")) {
            mode_from_env = true;
            if (std::strcmp(pm, "mailbox") == 0) {
                mode = SDL_GPU_PRESENTMODE_MAILBOX;
            } else if (std::strcmp(pm, "vsync") == 0) {
                mode = SDL_GPU_PRESENTMODE_VSYNC;
            } else if (std::strcmp(pm, "immediate") == 0) {
                mode = SDL_GPU_PRESENTMODE_IMMEDIATE;
            } else {
                DONG_LOG_WARN("[GPU] Invalid DONG_GPU_PRESENT_MODE=%s (use mailbox|vsync|immediate)", pm);
                mode_from_env = false;
                mode = SDL_GPU_PRESENTMODE_MAILBOX;
            }
        }

        if (mode_from_env) {
            if (!SDL_SetGPUSwapchainParameters(
                gpu_device_,
                window_,
                composition,
                mode
            )) {
                DONG_LOG_WARN("[GPU] PresentMode request failed, falling back to VSYNC");
                if (hdr_enabled_) {
                    // Try HDR with VSYNC
                    if (!SDL_SetGPUSwapchainParameters(
                        gpu_device_,
                        window_,
                        composition,
                        SDL_GPU_PRESENTMODE_VSYNC
                    )) {
                        // Fall back to SDR
                        hdr_enabled_ = false;
                        composition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR;
                        SDL_SetGPUSwapchainParameters(
                            gpu_device_,
                            window_,
                            composition,
                            SDL_GPU_PRESENTMODE_VSYNC
                        );
                        DONG_LOG_WARN("[GPU] HDR failed, fell back to SDR");
                    }
                } else {
                    SDL_SetGPUSwapchainParameters(
                        gpu_device_,
                        window_,
                        composition,
                        SDL_GPU_PRESENTMODE_VSYNC
                    );
                }
            }
        } else {
            // 默认策略：优先 MAILBOX，不支持则回退 VSYNC。
            if (!SDL_SetGPUSwapchainParameters(
                gpu_device_,
                window_,
                composition,
                SDL_GPU_PRESENTMODE_MAILBOX
            )) {
                DONG_LOG_INFO("MAILBOX present mode not supported, falling back to VSYNC");
                if (!SDL_SetGPUSwapchainParameters(
                    gpu_device_,
                    window_,
                    composition,
                    SDL_GPU_PRESENTMODE_VSYNC
                )) {
                    // If HDR failed, try SDR
                    if (hdr_enabled_) {
                        hdr_enabled_ = false;
                        composition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR;
                        SDL_SetGPUSwapchainParameters(
                            gpu_device_,
                            window_,
                            composition,
                            SDL_GPU_PRESENTMODE_VSYNC
                        );
                        DONG_LOG_WARN("[GPU] HDR swapchain failed, fell back to SDR");
                    }
                }
            }
        }
    }

    DONG_LOG_INFO("GPU device created and claimed for window");

    return true;
}

} // namespace dong::platform
