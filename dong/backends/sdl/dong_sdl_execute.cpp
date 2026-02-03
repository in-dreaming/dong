#include "dong_sdl_execute.h"

#include "core/log.h"
#include "render/gpu_device.hpp"
#include "render/gpu_driver_sdl.hpp"
#include "render/shader_manager.hpp"
#include "render/gpu_ir.hpp"
#include "render/resource_manager.hpp"


#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_video.h>

#include <memory>
#include <new>
#include <string>

namespace {

static inline SDL_GPUShaderFormat chooseShaderFormat() {
#ifdef __APPLE__
    return SDL_GPU_SHADERFORMAT_MSL;
#else
    return SDL_GPU_SHADERFORMAT_SPIRV;
#endif
}

static inline const dong::render::GPUCommandList* asCommandList(const void* command_list) {
    return static_cast<const dong::render::GPUCommandList*>(command_list);
}

} // namespace

struct DongSDLExecutor {
    std::unique_ptr<dong::render::GPUDevice> gpu_device;
    std::unique_ptr<dong::render::ShaderManager> shader_manager;
    std::unique_ptr<dong::render::GPUDriverSDL> driver;
    std::unique_ptr<dong::render::ResourceManager> resource_manager;

    SDL_Window* window = nullptr;
};


extern "C" {

DongSDLExecutor* dong_sdl_executor_create(void* sdl_device, void* sdl_window) {
    auto* dev = static_cast<SDL_GPUDevice*>(sdl_device);
    auto* win = static_cast<SDL_Window*>(sdl_window);
    if (!dev) {
        DONG_LOG_ERROR("[dong_sdl_executor] create: sdl_device is null");
        return nullptr;
    }

    try {
        auto* exec = new DongSDLExecutor();
        exec->window = win;

        exec->gpu_device = std::make_unique<dong::render::GPUDevice>();
        exec->gpu_device->adoptExternal(dev, chooseShaderFormat());

        exec->shader_manager = std::make_unique<dong::render::ShaderManager>(exec->gpu_device.get());
        exec->driver = std::make_unique<dong::render::GPUDriverSDL>(exec->gpu_device.get(), win, exec->shader_manager.get());
        if (!exec->driver->initialize()) {
            DONG_LOG_ERROR("[dong_sdl_executor] GPUDriverSDL initialize failed");
            delete exec;
            return nullptr;
        }

        exec->resource_manager = std::make_unique<dong::render::ResourceManager>();
        exec->driver->setImageResourceManager(exec->resource_manager.get());

        return exec;

    } catch (...) {
        return nullptr;
    }
}

void dong_sdl_executor_destroy(DongSDLExecutor* exec) {
    delete exec;
}

void dong_sdl_executor_prepare_resources(DongSDLExecutor* exec, const void* command_list) {
    if (!exec || !exec->driver || !command_list) {
        return;
    }

    const auto* list = asCommandList(command_list);
    if (!list) {
        return;
    }

    exec->driver->prepareResources(*list);
}

int dong_sdl_executor_execute(DongSDLExecutor* exec, const void* command_list) {
    if (!exec || !exec->driver || !command_list) {
        return 0;
    }

    const auto* list = asCommandList(command_list);
    if (!list) {
        return 0;
    }

    exec->driver->prepareResources(*list);
    exec->driver->beginFrame();
    exec->driver->execute(*list);
    exec->driver->endFrame();
    return 1;
}

int dong_sdl_executor_update_external_image_rgba(DongSDLExecutor* exec, const char* key,
                                                const uint8_t* rgba, uint32_t width,
                                                uint32_t height, uint32_t stride_bytes) {
    if (!exec || !exec->driver || !key || !rgba) {
        return 0;
    }

    return exec->driver->updateExternalImageRGBA(std::string(key), rgba, width, height, stride_bytes) ? 1 : 0;
}

int dong_sdl_executor_update_external_image_yuv420p(DongSDLExecutor* exec, const char* key,
                                                    const uint8_t* plane_y, uint32_t stride_y,
                                                    const uint8_t* plane_u, uint32_t stride_u,
                                                    const uint8_t* plane_v, uint32_t stride_v,
                                                    uint32_t width, uint32_t height) {
    if (!exec || !exec->driver || !key || !plane_y || !plane_u || !plane_v) {
        return 0;
    }

    return exec->driver->updateExternalImageYUV420P(std::string(key),
                                                    plane_y, stride_y,
                                                    plane_u, stride_u,
                                                    plane_v, stride_v,
                                                    width, height) ? 1 : 0;
}

void dong_sdl_executor_set_resource_root(DongSDLExecutor* exec, const char* root) {
    if (!exec || !exec->resource_manager) {
        return;
    }

    const char* value = root ? root : "";
    exec->resource_manager->setResourceRoot(value);
}

int dong_sdl_executor_begin_frame_offscreen(DongSDLExecutor* exec, void* sdl_target_texture,
                                           uint32_t width, uint32_t height) {
    if (!exec || !exec->driver || !sdl_target_texture) {
        return 0;
    }

    exec->driver->beginFrameOffscreen(static_cast<SDL_GPUTexture*>(sdl_target_texture), width, height);
    return 1;
}


int dong_sdl_executor_end_frame_offscreen(DongSDLExecutor* exec) {
    if (!exec || !exec->driver) {
        return 0;
    }

    exec->driver->endFrameOffscreen();
    return 1;
}

} // extern "C"
