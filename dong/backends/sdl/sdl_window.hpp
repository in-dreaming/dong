#pragma once

#include <cstdint>
#include <memory>
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

// DLL export/import macros
#if defined(_WIN32) || defined(_WIN64)
    #ifdef DONG_SDL_BUILDING_DLL
        #define DONG_SDL_API __declspec(dllexport)
    #else
        #define DONG_SDL_API __declspec(dllimport)
    #endif
#else
    #define DONG_SDL_API __attribute__((visibility("default")))
#endif

namespace dong::platform {

// SDL3 窗口管理：负责 SDL3 窗口、GPU 设备和事件循环
class DONG_SDL_API SDL3Window {
public:
    struct CreateInfo {
        const char* title;
        uint32_t width;
        uint32_t height;
        bool use_gpu;
        bool debug_mode;
    };

    SDL3Window() = default;
    ~SDL3Window();

    // 初始化 SDL 和窗口
    bool initialize(const CreateInfo& info);

    // 获取 SDL 窗口指针
    SDL_Window* getHandle() const { return window_; }

    // 获取 GPU 设备指针
    SDL_GPUDevice* getGPUDevice() const { return gpu_device_; }

    // 处理事件
    bool processEvents();

    // 是否应该关闭
    bool shouldClose() const { return should_close_; }

    // 调整窗口大小
    void resize(uint32_t width, uint32_t height);

    // 获取当前宽度/高度
    uint32_t getWidth() const { return width_; }
    uint32_t getHeight() const { return height_; }

    // 检查是否初始化
    bool isInitialized() const { return window_ != nullptr; }

private:
    SDL_Window* window_ = nullptr;
    SDL_GPUDevice* gpu_device_ = nullptr;
    uint32_t width_ = 0;
    uint32_t height_ = 0;
    bool should_close_ = false;
    bool use_gpu_ = false;

    bool initializeSDL();
    bool createWindow(const CreateInfo& info);
    bool createGPUDevice(bool debug_mode);
};

} // namespace dong::platform
