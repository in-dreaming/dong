#include "gpu_gpu_driver.hpp"

#include "../../src/core/log.h"
#include "../../src/render/gpu_ir.hpp"

namespace dong::gpu_backend {

void gpu_register_full_vtable(DongGPUDriverVTable* vtable);

static int gpu_initialize(DongGPUDriver* driver) {
    auto* impl = gpu_driver_impl(driver);
    return impl && impl->initialize() ? 1 : 0;
}

static void gpu_shutdown(DongGPUDriver* driver) {
    if (auto* impl = gpu_driver_impl(driver)) {
        impl->shutdown();
    }
}

static DongGPUTexture gpu_create_texture(DongGPUDriver* driver, const DongGPUTextureDesc* desc) {
    auto* impl = gpu_driver_impl(driver);
    return impl ? impl->resources().createTexture(desc) : nullptr;
}

static void gpu_destroy_texture(DongGPUDriver* driver, DongGPUTexture texture) {
    if (auto* impl = gpu_driver_impl(driver)) {
        impl->resources().destroyTexture(texture);
    }
}

static DongGPUBuffer gpu_create_buffer(DongGPUDriver* driver, const DongGPUBufferDesc* desc) {
    auto* impl = gpu_driver_impl(driver);
    return impl ? impl->resources().createBuffer(desc) : nullptr;
}

static void gpu_destroy_buffer(DongGPUDriver* driver, DongGPUBuffer buffer) {
    if (auto* impl = gpu_driver_impl(driver)) {
        impl->resources().destroyBuffer(buffer);
    }
}

static DongGPUSampler gpu_create_sampler(DongGPUDriver* driver, const DongGPUSamplerDesc* desc) {
    auto* impl = gpu_driver_impl(driver);
    return impl ? impl->resources().createSampler(desc) : nullptr;
}

static void gpu_destroy_sampler(DongGPUDriver* driver, DongGPUSampler sampler) {
    if (auto* impl = gpu_driver_impl(driver)) {
        impl->resources().destroySampler(sampler);
    }
}

static int gpu_upload_texture(DongGPUDriver* driver, DongGPUTexture texture, const void* data, uint32_t width,
                              uint32_t height, uint32_t x, uint32_t y) {
    auto* impl = gpu_driver_impl(driver);
    return impl ? impl->resources().uploadTexture(texture, data, width, height, x, y) : -1;
}

static int gpu_upload_buffer(DongGPUDriver* driver, DongGPUBuffer buffer, const void* data, uint32_t size,
                             uint32_t offset) {
    auto* impl = gpu_driver_impl(driver);
    return impl ? impl->resources().uploadBuffer(buffer, data, size, offset) : -1;
}

static int gpu_upload_texture_subrect(DongGPUDriver* driver, DongGPUTexture texture, const void* data,
                                      uint32_t dest_x, uint32_t dest_y, uint32_t width, uint32_t height,
                                      uint32_t src_stride_bytes, void** out_fence) {
    auto* impl = gpu_driver_impl(driver);
    return impl ? impl->resources().uploadTextureSubrect(texture, data, dest_x, dest_y, width, height,
                                                         src_stride_bytes, out_fence)
                : -1;
}

static int gpu_query_fence(DongGPUDriver* driver, void* fence) {
    auto* impl = gpu_driver_impl(driver);
    return impl ? impl->resources().queryFence(fence) : 1;
}

static void gpu_release_fence(DongGPUDriver* driver, void* fence) {
    if (auto* impl = gpu_driver_impl(driver)) {
        impl->resources().releaseFence(fence);
    }
}

static void gpu_wait_for_gpu(DongGPUDriver* driver) {
    if (auto* impl = gpu_driver_impl(driver)) {
        impl->resources().waitForGpu();
    }
}

static int gpu_begin_frame(DongGPUDriver* driver) {
    return gpu_driver_impl(driver) ? gpu_driver_impl(driver)->beginFrame() : 0;
}

static int gpu_end_frame(DongGPUDriver* driver) {
    return gpu_driver_impl(driver) ? gpu_driver_impl(driver)->endFrame() : 0;
}

static int gpu_execute(DongGPUDriver* driver, const void* command_list) {
    return gpu_driver_impl(driver) ? gpu_driver_impl(driver)->execute(command_list) : 0;
}

static int gpu_begin_frame_offscreen(DongGPUDriver* driver, DongGPUTexture target, uint32_t width, uint32_t height) {
    return gpu_driver_impl(driver) ? gpu_driver_impl(driver)->beginFrameOffscreen(target, width, height) : 0;
}

static int gpu_end_frame_offscreen(DongGPUDriver* driver) {
    return gpu_driver_impl(driver) ? gpu_driver_impl(driver)->endFrameOffscreen() : 0;
}

static void gpu_prepare_resources(DongGPUDriver* driver, const void* command_list) {
    if (auto* impl = gpu_driver_impl(driver)) {
        impl->prepareResources(command_list);
    }
}

static int gpu_update_external_image_rgba(DongGPUDriver* driver, const char* key, const uint8_t* rgba, uint32_t width,
                                          uint32_t height, uint32_t stride_bytes) {
    auto* impl = gpu_driver_impl(driver);
    return impl && key && impl->updateExternalImageRGBA(key, rgba, width, height, stride_bytes) ? 1 : 0;
}

static int gpu_update_external_image_yuv420p(DongGPUDriver* driver, const char* key, const uint8_t* plane_y,
                                             uint32_t stride_y, const uint8_t* plane_u, uint32_t stride_u,
                                             const uint8_t* plane_v, uint32_t stride_v, uint32_t width,
                                             uint32_t height) {
    auto* impl = gpu_driver_impl(driver);
    return impl && key && plane_y && plane_u && plane_v &&
                   impl->updateExternalImageYUV420P(key, plane_y, stride_y, plane_u, stride_u,
                                                    plane_v, stride_v, width, height)
               ? 1
               : 0;
}

static void gpu_set_resource_root(DongGPUDriver* driver, const char* root) {
    if (auto* impl = gpu_driver_impl(driver)) {
        impl->setResourceRoot(root);
    }
}

static void gpu_get_capabilities(DongGPUDriver* driver, uint32_t* out_max_texture_size) {
    if (auto* impl = gpu_driver_impl(driver)) {
        impl->resources().getCapabilities(out_max_texture_size);
    } else if (out_max_texture_size) {
        *out_max_texture_size = 4096;
    }
}

static void* gpu_get_native_device(DongGPUDriver* driver) {
    auto* impl = gpu_driver_impl(driver);
    return impl ? impl->nativeDevice() : nullptr;
}

static void* gpu_get_native_texture_handle(DongGPUDriver* driver, DongGPUTexture texture) {
    auto* impl = gpu_driver_impl(driver);
    return impl ? impl->resources().nativeTextureHandle(texture) : nullptr;
}

void gpu_register_full_vtable(DongGPUDriverVTable* vtable) {
    if (!vtable) {
        return;
    }
    vtable->initialize = gpu_initialize;
    vtable->shutdown = gpu_shutdown;
    vtable->create_texture = gpu_create_texture;
    vtable->destroy_texture = gpu_destroy_texture;
    vtable->create_buffer = gpu_create_buffer;
    vtable->destroy_buffer = gpu_destroy_buffer;
    vtable->create_sampler = gpu_create_sampler;
    vtable->destroy_sampler = gpu_destroy_sampler;
    vtable->upload_texture = gpu_upload_texture;
    vtable->upload_buffer = gpu_upload_buffer;
    vtable->upload_texture_subrect = gpu_upload_texture_subrect;
    vtable->query_fence = gpu_query_fence;
    vtable->release_fence = gpu_release_fence;
    vtable->wait_for_gpu = gpu_wait_for_gpu;
    vtable->begin_frame = gpu_begin_frame;
    vtable->end_frame = gpu_end_frame;
    vtable->execute = gpu_execute;
    vtable->begin_frame_offscreen = gpu_begin_frame_offscreen;
    vtable->end_frame_offscreen = gpu_end_frame_offscreen;
    vtable->prepare_resources = gpu_prepare_resources;
    vtable->update_external_image_rgba = gpu_update_external_image_rgba;
    vtable->update_external_image_yuv420p = gpu_update_external_image_yuv420p;
    vtable->set_resource_root = gpu_set_resource_root;
    vtable->get_capabilities = gpu_get_capabilities;
    vtable->get_native_device = gpu_get_native_device;
    vtable->get_native_texture_handle = gpu_get_native_texture_handle;
}

} // namespace dong::gpu_backend
