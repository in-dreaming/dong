#include "../gpu_driver.hpp"


#include "../gpu_driver_sdl.hpp"

#include <cstdlib>

namespace dong::render {

std::unique_ptr<GPUDriver> CreateGPUDriver(
    GPUBackendType backend,
    GPUDevice* device,
    SDL_Window* window,
    ShaderManager* shader_manager
) {
    if (backend == GPUBackendType::SDL_GPU) {
        if (!device || !window || !shader_manager) {
            return nullptr;
        }
        auto driver = std::make_unique<GPUDriverSDL>(device, window, shader_manager);

        // 通过环境变量控制调试日志：
        //   DONG_DEBUG_DRAW_BATCHES=1       → 打印 draw_batches 日志
        //   DONG_DEBUG_LAYER_CACHE=1        → 打印图层缓存（重栅格 / 复用）日志
        if (const char* env_batches = std::getenv("DONG_DEBUG_DRAW_BATCHES")) {
            if (env_batches[0] == '1') {
                driver->setDebugLogDrawBatches(true);
            }
        }
        if (const char* env_layer_cache = std::getenv("DONG_DEBUG_LAYER_CACHE")) {
            if (env_layer_cache[0] == '1') {
                driver->setDebugLogLayerCache(true);
            } else if (env_layer_cache[0] == '0') {
                driver->setDebugLogLayerCache(false);
            }
        }
        if (const char* env_subpixel = std::getenv("DONG_MSDF_SUBPIXEL")) {
            if (env_subpixel[0] == '1') {
                driver->setMsdfSubpixelEnabled(true);
            }
        }
        // 可选启用图层缓存（默认关闭，避免影响渲染正确性）。
        // 通过环境变量 DONG_LAYER_CACHE=1 显式开启。
        if (const char* env_layer_cache_enable = std::getenv("DONG_LAYER_CACHE")) {
            if (env_layer_cache_enable[0] == '1') {
                driver->setLayerCacheEnabled(true);
            } else if (env_layer_cache_enable[0] == '0') {
                driver->setLayerCacheEnabled(false);
            }
        }

        return driver;
    }
    return nullptr;
}

} // namespace dong::render
