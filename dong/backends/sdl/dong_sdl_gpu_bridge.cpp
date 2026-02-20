#include "dong_sdl_gpu_bridge.h"

#include "sdl_gpu_driver.hpp"

#include "core/log.h"

#include "sdl_gpu_device.hpp"
#include "sdl_shader_manager.hpp"
#include "render/resource_manager.hpp"
#include "render/gpu_ir.hpp"

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

struct DongSDLGPUBridge {
    SDL_GPUDevice* sdl_device = nullptr;
    SDL_Window* sdl_window = nullptr;

    std::unique_ptr<dong::sdl_backend::GPUDevice> gpu_device;
    std::unique_ptr<dong::sdl_backend::ShaderManager> shader_manager;
    std::unique_ptr<dong::render::SDLGPUDriver> driver;
    std::unique_ptr<dong::render::ResourceManager> resource_manager;
};

extern "C" {

DongSDLGPUBridge* dong_sdl_gpu_bridge_create(void* sdl_device, void* sdl_window, DongGPUDriver* driver) {
    auto* dev = static_cast<SDL_GPUDevice*>(sdl_device);
    auto* win = static_cast<SDL_Window*>(sdl_window);
    if (!dev) {
        DONG_LOG_ERROR("[dong_sdl_gpu_bridge] create: sdl_device is null");
        return nullptr;
    }

    try {
        auto* bridge = new DongSDLGPUBridge();
        bridge->sdl_device = dev;
        bridge->sdl_window = win;

        bridge->gpu_device = std::make_unique<dong::sdl_backend::GPUDevice>();
        bridge->gpu_device->adoptExternal(dev, chooseShaderFormat());

        bridge->shader_manager = std::make_unique<dong::sdl_backend::ShaderManager>(bridge->gpu_device.get());
        bridge->driver = std::make_unique<dong::render::SDLGPUDriver>(bridge->gpu_device.get(), win, bridge->shader_manager.get());

        // Set the C API driver for GlyphAtlas creation
        if (driver) {
            bridge->driver->setDongGPUDriver(driver);
        }

        if (!bridge->driver->initialize()) {
            DONG_LOG_ERROR("[dong_sdl_gpu_bridge] SDLGPUDriver initialize failed");
            delete bridge;
            return nullptr;
        }

        bridge->resource_manager = std::make_unique<dong::render::ResourceManager>();
        bridge->driver->setImageResourceManager(bridge->resource_manager.get());

        return bridge;
    } catch (...) {
        return nullptr;
    }
}

void dong_sdl_gpu_bridge_destroy(DongSDLGPUBridge* bridge) {
    delete bridge;
}

int dong_sdl_gpu_bridge_execute(DongSDLGPUBridge* bridge, const void* command_list) {
    if (!bridge || !bridge->driver || !command_list) {
        return 0;
    }

    const auto* list = asCommandList(command_list);
    if (!list) {
        return 0;
    }

    // Check if already in a frame (e.g., offscreen rendering)
    bool already_in_frame = bridge->driver->isInFrame();

    if (!already_in_frame) {
        bridge->driver->beginFrame();
        // Check if beginFrame succeeded
        if (!bridge->driver->isInFrame()) {
            // beginFrame failed (likely GPU not fully initialized yet)
            // This is expected on first frame - just return success
            // The next frame will work once initialization completes
            return 1;
        }
    }
    bridge->driver->prepareResources(*list);
    bridge->driver->execute(*list);
    if (!already_in_frame) {
        bridge->driver->endFrame();
    }
    return 1;
}

int dong_sdl_gpu_bridge_prepare_resources(DongSDLGPUBridge* bridge, const void* command_list) {
    if (!bridge || !bridge->driver || !command_list) {
        return 0;
    }

    const auto* list = asCommandList(command_list);
    if (!list) {
        return 0;
    }

    // Ensure there is a command buffer for uploads.
    bridge->driver->beginFrame();
    bridge->driver->prepareResources(*list);
    bridge->driver->endFrame();
    return 1;
}

int dong_sdl_gpu_bridge_update_external_image_rgba(DongSDLGPUBridge* bridge, const char* key,
                                                  const uint8_t* rgba, uint32_t width,
                                                  uint32_t height, uint32_t stride_bytes) {
    if (!bridge || !bridge->driver || !key || !rgba) {
        return 0;
    }

    // Must be safe to call outside execute(); SDLGPUDriver will acquire a temporary command buffer if needed.
    bool ok = bridge->driver->updateExternalImageRGBA(std::string(key), rgba, width, height, stride_bytes);
    return ok ? 1 : 0;
}

int dong_sdl_gpu_bridge_update_external_image_yuv420p(DongSDLGPUBridge* bridge, const char* key,
                                                      const uint8_t* plane_y, uint32_t stride_y,
                                                      const uint8_t* plane_u, uint32_t stride_u,
                                                      const uint8_t* plane_v, uint32_t stride_v,
                                                      uint32_t width, uint32_t height) {
    if (!bridge || !bridge->driver || !key || !plane_y || !plane_u || !plane_v) {
        return 0;
    }

    bool ok = bridge->driver->updateExternalImageYUV420P(std::string(key),
                                                         plane_y, stride_y,
                                                         plane_u, stride_u,
                                                         plane_v, stride_v,
                                                         width, height);
    return ok ? 1 : 0;
}


void dong_sdl_gpu_bridge_set_resource_root(DongSDLGPUBridge* bridge, const char* root) {
    if (!bridge || !bridge->resource_manager) {
        return;
    }

    bridge->resource_manager->setResourceRoot(root ? root : "");
}

int dong_sdl_gpu_bridge_begin_frame_offscreen(DongSDLGPUBridge* bridge, void* sdl_target_texture,
                                             uint32_t width, uint32_t height) {
    if (!bridge || !bridge->driver || !sdl_target_texture) {
        return 0;
    }

    bridge->driver->beginFrameOffscreen(static_cast<SDL_GPUTexture*>(sdl_target_texture), width, height);
    return 1;
}

int dong_sdl_gpu_bridge_end_frame_offscreen(DongSDLGPUBridge* bridge) {
    if (!bridge || !bridge->driver) {
        return 0;
    }

    bridge->driver->endFrameOffscreen();
    return 1;
}

} // extern "C"
