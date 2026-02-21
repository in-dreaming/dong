// SDL Platform Implementation
// Provides SDL-based implementations of the Platform abstraction interfaces.

#include "dong_sdl_platform.h"
#include "dong_sdl_gpu_bridge.h"
#include "dong_sdl_image_decoder.h"
#include "dong_platform.h"
#include "dong_gpu_driver.h"
#include "dong_surface.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


// =============================================================================
// SDL GPU Driver Implementation
// =============================================================================

// Transfer buffer tracking for async cleanup
typedef struct PendingTransferBuffer {
    SDL_GPUTransferBuffer* buffer;
    SDL_GPUFence* fence;
} PendingTransferBuffer;

typedef struct DongSDLGPUDriverImpl {
    DongGPUDriver base;  // Must be first for C polymorphism
    SDL_GPUDevice* device;
    SDL_Window* window;
    int owns_device;
    int initialized;

    DongSDLGPUBridge* bridge;

    // Async transfer buffer lifecycle management
    PendingTransferBuffer* pending_transfers;
    size_t pending_transfer_count;
    size_t pending_transfer_capacity;
} DongSDLGPUDriverImpl;


// Forward declarations of vtable functions
static int sdl_gpu_initialize(DongGPUDriver* driver);
static void sdl_gpu_shutdown(DongGPUDriver* driver);
static DongGPUTexture sdl_gpu_create_texture(DongGPUDriver* driver, const DongGPUTextureDesc* desc);
static void sdl_gpu_destroy_texture(DongGPUDriver* driver, DongGPUTexture texture);
static DongGPUBuffer sdl_gpu_create_buffer(DongGPUDriver* driver, const DongGPUBufferDesc* desc);
static void sdl_gpu_destroy_buffer(DongGPUDriver* driver, DongGPUBuffer buffer);
static DongGPUSampler sdl_gpu_create_sampler(DongGPUDriver* driver, const DongGPUSamplerDesc* desc);
static void sdl_gpu_destroy_sampler(DongGPUDriver* driver, DongGPUSampler sampler);
static int sdl_gpu_upload_texture(DongGPUDriver* driver, DongGPUTexture texture, const void* data,
                                   uint32_t width, uint32_t height, uint32_t x, uint32_t y);
static int sdl_gpu_upload_buffer(DongGPUDriver* driver, DongGPUBuffer buffer,
                                  const void* data, uint32_t size, uint32_t offset);
static int sdl_gpu_upload_texture_subrect(DongGPUDriver* driver, DongGPUTexture texture,
                                           const void* data,
                                           uint32_t dest_x, uint32_t dest_y,
                                           uint32_t width, uint32_t height,
                                           uint32_t src_stride_bytes,
                                           void** out_fence);
static int sdl_gpu_query_fence(DongGPUDriver* driver, void* fence);
static void sdl_gpu_release_fence(DongGPUDriver* driver, void* fence);
static void sdl_gpu_wait_for_gpu(DongGPUDriver* driver);
static int sdl_gpu_begin_frame(DongGPUDriver* driver);
static int sdl_gpu_end_frame(DongGPUDriver* driver);
static int sdl_gpu_execute(DongGPUDriver* driver, const void* command_list);
static int sdl_gpu_begin_frame_offscreen(DongGPUDriver* driver, DongGPUTexture target,
                                          uint32_t width, uint32_t height);
static int sdl_gpu_end_frame_offscreen(DongGPUDriver* driver);
static void sdl_gpu_prepare_resources(DongGPUDriver* driver, const void* command_list);
static int sdl_gpu_update_external_image_rgba(DongGPUDriver* driver, const char* key,
                                               const uint8_t* rgba, uint32_t width,
                                               uint32_t height, uint32_t stride_bytes);
static int sdl_gpu_update_external_image_yuv420p(DongGPUDriver* driver, const char* key,
                                                  const uint8_t* plane_y, uint32_t stride_y,
                                                  const uint8_t* plane_u, uint32_t stride_u,
                                                  const uint8_t* plane_v, uint32_t stride_v,
                                                  uint32_t width, uint32_t height);
static void sdl_gpu_set_resource_root(DongGPUDriver* driver, const char* root);
static void sdl_gpu_get_capabilities(DongGPUDriver* driver, uint32_t* out_max_texture_size);
static void* sdl_gpu_get_native_device(DongGPUDriver* driver);
static void* sdl_gpu_get_native_texture_handle(DongGPUDriver* driver, DongGPUTexture texture);


static const DongGPUDriverVTable g_sdl_gpu_vtable = {
    .initialize = sdl_gpu_initialize,
    .shutdown = sdl_gpu_shutdown,
    .create_texture = sdl_gpu_create_texture,
    .destroy_texture = sdl_gpu_destroy_texture,
    .create_buffer = sdl_gpu_create_buffer,
    .destroy_buffer = sdl_gpu_destroy_buffer,
    .create_sampler = sdl_gpu_create_sampler,
    .destroy_sampler = sdl_gpu_destroy_sampler,
    .upload_texture = sdl_gpu_upload_texture,
    .upload_buffer = sdl_gpu_upload_buffer,
    .upload_texture_subrect = sdl_gpu_upload_texture_subrect,
    .query_fence = sdl_gpu_query_fence,
    .release_fence = sdl_gpu_release_fence,
    .wait_for_gpu = sdl_gpu_wait_for_gpu,
    .begin_frame = sdl_gpu_begin_frame,
    .end_frame = sdl_gpu_end_frame,
    .execute = sdl_gpu_execute,
    .begin_frame_offscreen = sdl_gpu_begin_frame_offscreen,
    .end_frame_offscreen = sdl_gpu_end_frame_offscreen,
    .prepare_resources = sdl_gpu_prepare_resources,
    .update_external_image_rgba = sdl_gpu_update_external_image_rgba,
    .update_external_image_yuv420p = sdl_gpu_update_external_image_yuv420p,
    .set_resource_root = sdl_gpu_set_resource_root,
    .get_capabilities = sdl_gpu_get_capabilities,
    .get_native_device = sdl_gpu_get_native_device,
    .get_native_texture_handle = sdl_gpu_get_native_texture_handle,
};


// =============================================================================
// SDL GPU Driver VTable Implementations
// =============================================================================

static int sdl_gpu_initialize(DongGPUDriver* driver) {
    DongSDLGPUDriverImpl* impl = (DongSDLGPUDriverImpl*)driver;
    if (!impl || impl->initialized) return impl ? 1 : 0;

    // Device should already be set via adopt or create
    if (!impl->device) {
        fprintf(stderr, "[DongSDLGPUDriver] No device set\n");
        return 0;
    }

    impl->initialized = 1;
    return 1;
}

static void sdl_gpu_shutdown(DongGPUDriver* driver) {
    DongSDLGPUDriverImpl* impl = (DongSDLGPUDriverImpl*)driver;
    if (!impl) return;

    // Release all pending transfer buffers
    if (impl->pending_transfers) {
        for (size_t i = 0; i < impl->pending_transfer_count; i++) {
            PendingTransferBuffer* pending = &impl->pending_transfers[i];
            if (pending->fence && impl->device) {
                SDL_ReleaseGPUFence(impl->device, pending->fence);
            }
            if (pending->buffer && impl->device) {
                SDL_ReleaseGPUTransferBuffer(impl->device, pending->buffer);
            }
        }
        free(impl->pending_transfers);
        impl->pending_transfers = NULL;
        impl->pending_transfer_count = 0;
        impl->pending_transfer_capacity = 0;
    }

    if (impl->bridge) {
        dong_sdl_gpu_bridge_destroy(impl->bridge);
        impl->bridge = NULL;
    }

    if (impl->owns_device && impl->device) {
        SDL_DestroyGPUDevice(impl->device);
    }
    impl->device = NULL;
    impl->window = NULL;
    impl->initialized = 0;
}

// Reap completed transfer buffers (called at beginning of frame)
static void sdl_gpu_reap_transfer_buffers(DongSDLGPUDriverImpl* impl) {
    if (!impl || !impl->device || !impl->pending_transfers) return;

    size_t write_idx = 0;
    for (size_t read_idx = 0; read_idx < impl->pending_transfer_count; read_idx++) {
        PendingTransferBuffer* pending = &impl->pending_transfers[read_idx];

        // Check if fence is signaled
        int signaled = SDL_QueryGPUFence(impl->device, pending->fence);
        if (signaled) {
            // GPU finished reading, safe to release now
            SDL_ReleaseGPUFence(impl->device, pending->fence);
            SDL_ReleaseGPUTransferBuffer(impl->device, pending->buffer);
        } else {
            // Keep this entry
            if (write_idx != read_idx) {
                impl->pending_transfers[write_idx] = impl->pending_transfers[read_idx];
            }
            write_idx++;
        }
    }
    impl->pending_transfer_count = write_idx;
}



static SDL_GPUTextureFormat convert_texture_format(DongGPUTextureFormat format) {
    switch (format) {
        case DONG_GPU_TEXTURE_FORMAT_RGBA8_UNORM: return SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
        case DONG_GPU_TEXTURE_FORMAT_BGRA8_UNORM: return SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM;
        case DONG_GPU_TEXTURE_FORMAT_R8_UNORM: return SDL_GPU_TEXTUREFORMAT_R8_UNORM;
        case DONG_GPU_TEXTURE_FORMAT_DEPTH24_STENCIL8: return SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT;
        default: return SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    }
}

static DongGPUTexture sdl_gpu_create_texture(DongGPUDriver* driver, const DongGPUTextureDesc* desc) {
    DongSDLGPUDriverImpl* impl = (DongSDLGPUDriverImpl*)driver;
    if (!impl || !impl->device || !desc) return NULL;

    SDL_GPUTextureUsageFlags usage = 0;
    if (desc->usage & DONG_GPU_TEXTURE_USAGE_SAMPLER) usage |= SDL_GPU_TEXTUREUSAGE_SAMPLER;
    if (desc->usage & DONG_GPU_TEXTURE_USAGE_COLOR_TARGET) usage |= SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
    if (desc->usage & DONG_GPU_TEXTURE_USAGE_DEPTH_STENCIL_TARGET) usage |= SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;

    SDL_GPUTextureCreateInfo create_info = {0};
    create_info.type = SDL_GPU_TEXTURETYPE_2D;
    create_info.format = convert_texture_format(desc->format);
    create_info.width = desc->width;
    create_info.height = desc->height;
    create_info.layer_count_or_depth = 1;
    create_info.num_levels = desc->mip_levels > 0 ? desc->mip_levels : 1;
    create_info.usage = usage;

    SDL_GPUTexture* texture = SDL_CreateGPUTexture(impl->device, &create_info);
    return (DongGPUTexture)texture;
}

static void sdl_gpu_destroy_texture(DongGPUDriver* driver, DongGPUTexture texture) {
    DongSDLGPUDriverImpl* impl = (DongSDLGPUDriverImpl*)driver;
    if (!impl || !impl->device || !texture) return;

    SDL_ReleaseGPUTexture(impl->device, (SDL_GPUTexture*)texture);
}

static DongGPUBuffer sdl_gpu_create_buffer(DongGPUDriver* driver, const DongGPUBufferDesc* desc) {
    DongSDLGPUDriverImpl* impl = (DongSDLGPUDriverImpl*)driver;
    if (!impl || !impl->device || !desc) return NULL;

    SDL_GPUBufferUsageFlags usage = 0;
    if (desc->usage & DONG_GPU_BUFFER_USAGE_VERTEX) usage |= SDL_GPU_BUFFERUSAGE_VERTEX;
    if (desc->usage & DONG_GPU_BUFFER_USAGE_INDEX) usage |= SDL_GPU_BUFFERUSAGE_INDEX;

    SDL_GPUBufferCreateInfo create_info = {0};
    create_info.usage = usage;
    create_info.size = desc->size;

    SDL_GPUBuffer* buffer = SDL_CreateGPUBuffer(impl->device, &create_info);
    return (DongGPUBuffer)buffer;
}

static void sdl_gpu_destroy_buffer(DongGPUDriver* driver, DongGPUBuffer buffer) {
    DongSDLGPUDriverImpl* impl = (DongSDLGPUDriverImpl*)driver;
    if (!impl || !impl->device || !buffer) return;

    SDL_ReleaseGPUBuffer(impl->device, (SDL_GPUBuffer*)buffer);
}

static SDL_GPUSamplerAddressMode convert_address_mode(DongGPUSamplerAddressMode mode) {
    switch (mode) {
        case DONG_GPU_SAMPLER_ADDRESS_REPEAT: return SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
        case DONG_GPU_SAMPLER_ADDRESS_CLAMP_TO_EDGE: return SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
        case DONG_GPU_SAMPLER_ADDRESS_CLAMP_TO_BORDER: return SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
        default: return SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
    }
}

static SDL_GPUFilter convert_filter(DongGPUSamplerFilter filter) {
    switch (filter) {
        case DONG_GPU_SAMPLER_FILTER_NEAREST: return SDL_GPU_FILTER_NEAREST;
        case DONG_GPU_SAMPLER_FILTER_LINEAR: return SDL_GPU_FILTER_LINEAR;
        default: return SDL_GPU_FILTER_LINEAR;
    }
}

static DongGPUSampler sdl_gpu_create_sampler(DongGPUDriver* driver, const DongGPUSamplerDesc* desc) {
    DongSDLGPUDriverImpl* impl = (DongSDLGPUDriverImpl*)driver;
    if (!impl || !impl->device || !desc) return NULL;

    SDL_GPUSamplerCreateInfo create_info = {0};
    create_info.min_filter = convert_filter(desc->min_filter);
    create_info.mag_filter = convert_filter(desc->mag_filter);
    create_info.mipmap_mode = (desc->mip_filter == DONG_GPU_SAMPLER_FILTER_LINEAR)
        ? SDL_GPU_SAMPLERMIPMAPMODE_LINEAR : SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
    create_info.address_mode_u = convert_address_mode(desc->address_mode_u);
    create_info.address_mode_v = convert_address_mode(desc->address_mode_v);
    create_info.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;

    SDL_GPUSampler* sampler = SDL_CreateGPUSampler(impl->device, &create_info);
    return (DongGPUSampler)sampler;
}

static void sdl_gpu_destroy_sampler(DongGPUDriver* driver, DongGPUSampler sampler) {
    DongSDLGPUDriverImpl* impl = (DongSDLGPUDriverImpl*)driver;
    if (!impl || !impl->device || !sampler) return;

    SDL_ReleaseGPUSampler(impl->device, (SDL_GPUSampler*)sampler);
}

static int sdl_gpu_upload_texture(DongGPUDriver* driver, DongGPUTexture texture, const void* data,
                                   uint32_t width, uint32_t height, uint32_t x, uint32_t y) {
    DongSDLGPUDriverImpl* impl = (DongSDLGPUDriverImpl*)driver;
    if (!impl || !impl->device || !texture || !data) return 0;

    // Create transfer buffer
    uint32_t size = width * height * 4;
    SDL_GPUTransferBufferCreateInfo tb_info = {0};
    tb_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    tb_info.size = size;

    SDL_GPUTransferBuffer* transfer = SDL_CreateGPUTransferBuffer(impl->device, &tb_info);
    if (!transfer) return 0;

    // Map and copy
    void* mapped = SDL_MapGPUTransferBuffer(impl->device, transfer, false);
    if (mapped) {
        memcpy(mapped, data, size);
        SDL_UnmapGPUTransferBuffer(impl->device, transfer);
    }

    // Upload
    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(impl->device);
    if (cmd) {
        SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd);
        if (copy_pass) {
            SDL_GPUTextureTransferInfo src = {0};
            src.transfer_buffer = transfer;
            src.offset = 0;

            SDL_GPUTextureRegion dst = {0};
            dst.texture = (SDL_GPUTexture*)texture;
            dst.x = x;
            dst.y = y;
            dst.w = width;
            dst.h = height;
            dst.d = 1;

            SDL_UploadToGPUTexture(copy_pass, &src, &dst, false);
            SDL_EndGPUCopyPass(copy_pass);
        }
        SDL_SubmitGPUCommandBuffer(cmd);
    }

    SDL_ReleaseGPUTransferBuffer(impl->device, transfer);
    return 1;
}

static int sdl_gpu_upload_buffer(DongGPUDriver* driver, DongGPUBuffer buffer,
                                  const void* data, uint32_t size, uint32_t offset) {
    DongSDLGPUDriverImpl* impl = (DongSDLGPUDriverImpl*)driver;
    if (!impl || !impl->device || !buffer || !data) return 0;

    // Create transfer buffer
    SDL_GPUTransferBufferCreateInfo tb_info = {0};
    tb_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    tb_info.size = size;

    SDL_GPUTransferBuffer* transfer = SDL_CreateGPUTransferBuffer(impl->device, &tb_info);
    if (!transfer) return 0;

    // Map and copy
    void* mapped = SDL_MapGPUTransferBuffer(impl->device, transfer, false);
    if (mapped) {
        memcpy(mapped, data, size);
        SDL_UnmapGPUTransferBuffer(impl->device, transfer);
    }

    // Upload
    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(impl->device);
    if (cmd) {
        SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd);
        if (copy_pass) {
            SDL_GPUTransferBufferLocation src = {0};
            src.transfer_buffer = transfer;
            src.offset = 0;

            SDL_GPUBufferRegion dst = {0};
            dst.buffer = (SDL_GPUBuffer*)buffer;
            dst.offset = offset;
            dst.size = size;

            SDL_UploadToGPUBuffer(copy_pass, &src, &dst, false);
            SDL_EndGPUCopyPass(copy_pass);
        }
        SDL_SubmitGPUCommandBuffer(cmd);
    }

    SDL_ReleaseGPUTransferBuffer(impl->device, transfer);
    return 1;
}

static int sdl_gpu_begin_frame(DongGPUDriver* driver) {
    DongSDLGPUDriverImpl* impl = (DongSDLGPUDriverImpl*)driver;
    if (!impl || !impl->device) return 0;

    // Reap completed transfer buffers
    sdl_gpu_reap_transfer_buffers(impl);

    // Frame begin is handled implicitly in execute
    return 1;
}

static int sdl_gpu_end_frame(DongGPUDriver* driver) {
    DongSDLGPUDriverImpl* impl = (DongSDLGPUDriverImpl*)driver;
    if (!impl || !impl->device) return 0;
    // Frame end is handled implicitly in execute
    return 1;
}

static int sdl_gpu_execute(DongGPUDriver* driver, const void* command_list) {
    DongSDLGPUDriverImpl* impl = (DongSDLGPUDriverImpl*)driver;
    if (!impl || !impl->device || !command_list) {
        return 0;
    }

    if (!impl->bridge) {
        impl->bridge = dong_sdl_gpu_bridge_create((void*)impl->device, (void*)impl->window, driver);
        if (!impl->bridge) {
            fprintf(stderr, "[DongSDLGPUDriver] failed to create bridge\n");
            return 0;
        }
    }

    return dong_sdl_gpu_bridge_execute(impl->bridge, command_list);
}


static int sdl_gpu_begin_frame_offscreen(DongGPUDriver* driver, DongGPUTexture target,
                                          uint32_t width, uint32_t height) {
    DongSDLGPUDriverImpl* impl = (DongSDLGPUDriverImpl*)driver;
    if (!impl || !impl->device || !target) {
        return 0;
    }

    if (!impl->bridge) {
        impl->bridge = dong_sdl_gpu_bridge_create((void*)impl->device, (void*)impl->window, driver);
        if (!impl->bridge) {
            return 0;
        }
    }

    return dong_sdl_gpu_bridge_begin_frame_offscreen(impl->bridge, target, width, height);
}

static int sdl_gpu_end_frame_offscreen(DongGPUDriver* driver) {
    DongSDLGPUDriverImpl* impl = (DongSDLGPUDriverImpl*)driver;
    if (!impl || !impl->bridge) {
        return 0;
    }

    return dong_sdl_gpu_bridge_end_frame_offscreen(impl->bridge);
}

static void sdl_gpu_prepare_resources(DongGPUDriver* driver, const void* command_list) {
    DongSDLGPUDriverImpl* impl = (DongSDLGPUDriverImpl*)driver;
    if (!impl || !impl->device || !command_list) {
        return;
    }

    if (!impl->bridge) {
        impl->bridge = dong_sdl_gpu_bridge_create((void*)impl->device, (void*)impl->window, driver);
        if (!impl->bridge) {
            return;
        }
    }

    dong_sdl_gpu_bridge_prepare_resources(impl->bridge, command_list);
}

static int sdl_gpu_update_external_image_rgba(DongGPUDriver* driver, const char* key,
                                               const uint8_t* rgba, uint32_t width,
                                               uint32_t height, uint32_t stride_bytes) {
    DongSDLGPUDriverImpl* impl = (DongSDLGPUDriverImpl*)driver;
    if (!impl || !impl->device || !key || !rgba) {
        return 0;
    }

    if (!impl->bridge) {
        impl->bridge = dong_sdl_gpu_bridge_create((void*)impl->device, (void*)impl->window, driver);
        if (!impl->bridge) {
            return 0;
        }
    }

    return dong_sdl_gpu_bridge_update_external_image_rgba(impl->bridge, key, rgba, width, height, stride_bytes);
}

static int sdl_gpu_update_external_image_yuv420p(DongGPUDriver* driver, const char* key,
                                                  const uint8_t* plane_y, uint32_t stride_y,
                                                  const uint8_t* plane_u, uint32_t stride_u,
                                                  const uint8_t* plane_v, uint32_t stride_v,
                                                  uint32_t width, uint32_t height) {
    DongSDLGPUDriverImpl* impl = (DongSDLGPUDriverImpl*)driver;
    if (!impl || !impl->device || !key || !plane_y || !plane_u || !plane_v) {
        return 0;
    }

    if (!impl->bridge) {
        impl->bridge = dong_sdl_gpu_bridge_create((void*)impl->device, (void*)impl->window, driver);
        if (!impl->bridge) {
            return 0;
        }
    }

    return dong_sdl_gpu_bridge_update_external_image_yuv420p(impl->bridge,
                                                             key,
                                                             plane_y, stride_y,
                                                             plane_u, stride_u,
                                                             plane_v, stride_v,
                                                             width, height);
}

static void sdl_gpu_set_resource_root(DongGPUDriver* driver, const char* root) {
    DongSDLGPUDriverImpl* impl = (DongSDLGPUDriverImpl*)driver;
    if (!impl || !impl->device) {
        return;
    }

    if (!impl->bridge) {
        impl->bridge = dong_sdl_gpu_bridge_create((void*)impl->device, (void*)impl->window, driver);
        if (!impl->bridge) {
            return;
        }
    }

    dong_sdl_gpu_bridge_set_resource_root(impl->bridge, root);
}

static void sdl_gpu_get_capabilities(DongGPUDriver* driver, uint32_t* out_max_texture_size) {
    (void)driver;
    if (out_max_texture_size) {
        *out_max_texture_size = 8192;  // Conservative default
    }
}


static void* sdl_gpu_get_native_device(DongGPUDriver* driver) {
    DongSDLGPUDriverImpl* impl = (DongSDLGPUDriverImpl*)driver;
    return impl ? impl->device : NULL;
}

// =============================================================================
// Extended Atlas Upload API (for GlyphAtlas)
// =============================================================================

static int sdl_gpu_upload_texture_subrect(DongGPUDriver* driver, DongGPUTexture texture,
                                           const void* data,
                                           uint32_t dest_x, uint32_t dest_y,
                                           uint32_t width, uint32_t height,
                                           uint32_t src_stride_bytes,
                                           void** out_fence) {
    DongSDLGPUDriverImpl* impl = (DongSDLGPUDriverImpl*)driver;
    if (!impl || !impl->device || !texture || !data) return -1;

    // Preferred path: route through the C++ SDLGPUDriver so uploads reuse the current frame
    // command buffer when available. This guarantees upload ordering relative to draw calls.
    // IMPORTANT: do NOT create the bridge here. This function can be called during bridge creation
    // (GlyphAtlas tier initialization), and creating the bridge recursively would stack overflow.
    if (impl->bridge && dong_sdl_gpu_bridge_is_in_frame(impl->bridge)) {
        int ok = dong_sdl_gpu_bridge_upload_texture_subrect_rgba(impl->bridge,
                                                                texture,
                                                                data,
                                                                dest_x,
                                                                dest_y,
                                                                width,
                                                                height,
                                                                src_stride_bytes);
        if (ok) {
            if (out_fence) {
                *out_fence = NULL; // ordered within active frame
            }
            return 0;
        }
    }

    SDL_GPUDevice* dev = impl->device;

    // Fallback path: standalone command buffer + fence.
    // Note: this does NOT guarantee ordering with SDLGPUDriver draws in the same frame.
    // It is kept as a fallback for early init or when bridge is unavailable.

    // Calculate buffer size based on stride
    uint32_t buffer_size = src_stride_bytes * height;

    // Create transfer buffer
    SDL_GPUTransferBufferCreateInfo tb_info = {0};
    tb_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    tb_info.size = buffer_size;

    SDL_GPUTransferBuffer* transfer = SDL_CreateGPUTransferBuffer(dev, &tb_info);
    if (!transfer) return -1;

    // Map and copy data (respecting source stride)
    void* mapped = SDL_MapGPUTransferBuffer(dev, transfer, false);
    if (mapped) {
        const uint8_t* src = (const uint8_t*)data;
        uint8_t* dst = (uint8_t*)mapped;
        uint32_t row_size = width * 4;  // RGBA

        if (src_stride_bytes == row_size) {
            // Fast path: contiguous data
            memcpy(dst, src, buffer_size);
        } else {
            // Slow path: copy row by row
            for (uint32_t y = 0; y < height; ++y) {
                memcpy(dst + y * row_size, src + y * src_stride_bytes, row_size);
            }
        }
        SDL_UnmapGPUTransferBuffer(dev, transfer);
    }

    // Upload via command buffer
    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(dev);
    if (!cmd) {
        SDL_ReleaseGPUTransferBuffer(dev, transfer);
        return -1;
    }

    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd);
    if (copy_pass) {
        SDL_GPUTextureTransferInfo src = {0};
        src.transfer_buffer = transfer;
        src.offset = 0;
        src.pixels_per_row = width;
        src.rows_per_layer = height;

        SDL_GPUTextureRegion dst = {0};
        dst.texture = (SDL_GPUTexture*)texture;
        dst.x = dest_x;
        dst.y = dest_y;
        dst.w = width;
        dst.h = height;
        dst.d = 1;

        SDL_UploadToGPUTexture(copy_pass, &src, &dst, false);
        SDL_EndGPUCopyPass(copy_pass);
    }

    // Submit and acquire fence
    SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(cmd);
    if (!fence) {
        // Failed to acquire fence - must wait synchronously (fallback path)
        SDL_SubmitGPUCommandBuffer(cmd);
        SDL_WaitForGPUIdle(dev);  // CAUTION: May deadlock on some platforms
        SDL_ReleaseGPUTransferBuffer(dev, transfer);
        return 0;
    }

    // Store (fence, buffer) pair for async cleanup
    // Expand capacity if needed
    if (impl->pending_transfer_count >= impl->pending_transfer_capacity) {
        size_t new_capacity = impl->pending_transfer_capacity ? impl->pending_transfer_capacity * 2 : 8;
        PendingTransferBuffer* new_array = (PendingTransferBuffer*)realloc(
            impl->pending_transfers,
            new_capacity * sizeof(PendingTransferBuffer)
        );
        if (!new_array) {
            // Out of memory - must wait synchronously (fallback path)
            SDL_WaitForGPUFences(dev, true, &fence, 1);
            SDL_ReleaseGPUFence(dev, fence);
            SDL_ReleaseGPUTransferBuffer(dev, transfer);
            return 0;
        }
        impl->pending_transfers = new_array;
        impl->pending_transfer_capacity = new_capacity;
    }

    // Add to pending list
    impl->pending_transfers[impl->pending_transfer_count].buffer = transfer;
    impl->pending_transfers[impl->pending_transfer_count].fence = fence;
    impl->pending_transfer_count++;

    // If caller requested fence, also return it (they can poll it separately)
    if (out_fence) {
        *out_fence = fence;
    }

    return 0;
}

static int sdl_gpu_query_fence(DongGPUDriver* driver, void* fence) {
    DongSDLGPUDriverImpl* impl = (DongSDLGPUDriverImpl*)driver;
    if (!impl || !impl->device || !fence) return 1;  // Return signaled if invalid

    return SDL_QueryGPUFence(impl->device, (SDL_GPUFence*)fence) ? 1 : 0;
}

static void sdl_gpu_release_fence(DongGPUDriver* driver, void* fence) {
    DongSDLGPUDriverImpl* impl = (DongSDLGPUDriverImpl*)driver;
    if (!impl || !impl->device || !fence) return;

    SDL_ReleaseGPUFence(impl->device, (SDL_GPUFence*)fence);
}

static void sdl_gpu_wait_for_gpu(DongGPUDriver* driver) {
    DongSDLGPUDriverImpl* impl = (DongSDLGPUDriverImpl*)driver;
    if (!impl || !impl->device) return;

    SDL_WaitForGPUIdle(impl->device);
}

static void* sdl_gpu_get_native_texture_handle(DongGPUDriver* driver, DongGPUTexture texture) {
    (void)driver;
    // The DongGPUTexture is already the SDL_GPUTexture* handle
    return texture;
}

// =============================================================================
// SDL Surface Implementation
// =============================================================================

typedef struct DongSDLSurfaceImpl {
    DongSurface base;
    SDL_GPUDevice* device;
    SDL_GPUTexture* texture;
    uint32_t width;
    uint32_t height;
    DongSurfaceType type;
    int dirty;
    DongDirtyBounds dirty_bounds;
} DongSDLSurfaceImpl;

static DongSurfaceType sdl_surface_get_type(DongSurface* s) {
    DongSDLSurfaceImpl* impl = (DongSDLSurfaceImpl*)s;
    return impl ? impl->type : DONG_SURFACE_TYPE_CPU_BUFFER;
}

static uint32_t sdl_surface_get_width(DongSurface* s) {
    DongSDLSurfaceImpl* impl = (DongSDLSurfaceImpl*)s;
    return impl ? impl->width : 0;
}

static uint32_t sdl_surface_get_height(DongSurface* s) {
    DongSDLSurfaceImpl* impl = (DongSDLSurfaceImpl*)s;
    return impl ? impl->height : 0;
}

static uint32_t sdl_surface_get_stride(DongSurface* s) {
    DongSDLSurfaceImpl* impl = (DongSDLSurfaceImpl*)s;
    return impl ? impl->width * 4 : 0;
}

static DongGPUTexture sdl_surface_get_texture(DongSurface* s) {
    DongSDLSurfaceImpl* impl = (DongSDLSurfaceImpl*)s;
    return impl ? (DongGPUTexture)impl->texture : NULL;
}

static void* sdl_surface_lock_pixels(DongSurface* s) {
    (void)s;
    return NULL;  // GPU surfaces don't support direct pixel access
}

static void sdl_surface_unlock_pixels(DongSurface* s) {
    (void)s;
}

static void sdl_surface_mark_dirty(DongSurface* s) {
    DongSDLSurfaceImpl* impl = (DongSDLSurfaceImpl*)s;
    if (impl) {
        impl->dirty = 1;
        impl->dirty_bounds.x = 0;
        impl->dirty_bounds.y = 0;
        impl->dirty_bounds.width = (int32_t)impl->width;
        impl->dirty_bounds.height = (int32_t)impl->height;
        impl->dirty_bounds.valid = 1;
    }
}

static void sdl_surface_mark_dirty_rect(DongSurface* s, int32_t x, int32_t y, int32_t w, int32_t h) {
    DongSDLSurfaceImpl* impl = (DongSDLSurfaceImpl*)s;
    if (!impl) return;
    impl->dirty = 1;
    if (!impl->dirty_bounds.valid) {
        impl->dirty_bounds.x = x;
        impl->dirty_bounds.y = y;
        impl->dirty_bounds.width = w;
        impl->dirty_bounds.height = h;
        impl->dirty_bounds.valid = 1;
    } else {
        // Union bounds
        int32_t x2 = impl->dirty_bounds.x + impl->dirty_bounds.width;
        int32_t y2 = impl->dirty_bounds.y + impl->dirty_bounds.height;
        int32_t nx2 = x + w;
        int32_t ny2 = y + h;
        if (x < impl->dirty_bounds.x) impl->dirty_bounds.x = x;
        if (y < impl->dirty_bounds.y) impl->dirty_bounds.y = y;
        if (nx2 > x2) x2 = nx2;
        if (ny2 > y2) y2 = ny2;
        impl->dirty_bounds.width = x2 - impl->dirty_bounds.x;
        impl->dirty_bounds.height = y2 - impl->dirty_bounds.y;
    }
}

static int sdl_surface_is_dirty(DongSurface* s) {
    DongSDLSurfaceImpl* impl = (DongSDLSurfaceImpl*)s;
    return impl ? impl->dirty : 0;
}

static void sdl_surface_get_dirty_bounds(DongSurface* s, DongDirtyBounds* out) {
    DongSDLSurfaceImpl* impl = (DongSDLSurfaceImpl*)s;
    if (out) {
        if (impl) {
            *out = impl->dirty_bounds;
        } else {
            memset(out, 0, sizeof(*out));
        }
    }
}

static void sdl_surface_clear_dirty(DongSurface* s) {
    DongSDLSurfaceImpl* impl = (DongSDLSurfaceImpl*)s;
    if (impl) {
        impl->dirty = 0;
        memset(&impl->dirty_bounds, 0, sizeof(impl->dirty_bounds));
    }
}

static void sdl_surface_clear(DongSurface* s, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    (void)s;
    (void)r;
    (void)g;
    (void)b;
    (void)a;
    // Clear would be done via GPU render pass
}

static int sdl_surface_resize(DongSurface* s, uint32_t new_width, uint32_t new_height) {
    DongSDLSurfaceImpl* impl = (DongSDLSurfaceImpl*)s;
    if (!impl || !impl->device) return 0;
    if (impl->width == new_width && impl->height == new_height) return 1;

    // Release old texture
    if (impl->texture) {
        SDL_ReleaseGPUTexture(impl->device, impl->texture);
    }

    // Create new texture
    SDL_GPUTextureCreateInfo info = {0};
    info.type = SDL_GPU_TEXTURETYPE_2D;
    info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    info.width = new_width;
    info.height = new_height;
    info.layer_count_or_depth = 1;
    info.num_levels = 1;
    info.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;

    impl->texture = SDL_CreateGPUTexture(impl->device, &info);
    impl->width = new_width;
    impl->height = new_height;
    sdl_surface_mark_dirty(s);

    return impl->texture ? 1 : 0;
}

static const DongSurfaceVTable g_sdl_surface_vtable = {
    .get_type = sdl_surface_get_type,
    .get_width = sdl_surface_get_width,
    .get_height = sdl_surface_get_height,
    .get_stride = sdl_surface_get_stride,
    .get_texture = sdl_surface_get_texture,
    .lock_pixels = sdl_surface_lock_pixels,
    .unlock_pixels = sdl_surface_unlock_pixels,
    .mark_dirty = sdl_surface_mark_dirty,
    .mark_dirty_rect = sdl_surface_mark_dirty_rect,
    .is_dirty = sdl_surface_is_dirty,
    .get_dirty_bounds = sdl_surface_get_dirty_bounds,
    .clear_dirty = sdl_surface_clear_dirty,
    .clear = sdl_surface_clear,
    .resize = sdl_surface_resize,
};

// =============================================================================
// SDL Surface Factory Implementation
// =============================================================================

typedef struct DongSDLSurfaceFactoryImpl {
    DongSurfaceFactory base;
    SDL_GPUDevice* device;
} DongSDLSurfaceFactoryImpl;

static DongSurface* sdl_factory_create_surface(DongSurfaceFactory* factory, const DongSurfaceDesc* desc) {
    DongSDLSurfaceFactoryImpl* impl = (DongSDLSurfaceFactoryImpl*)factory;
    if (!impl || !impl->device || !desc) return NULL;

    DongSDLSurfaceImpl* surface = (DongSDLSurfaceImpl*)calloc(1, sizeof(DongSDLSurfaceImpl));
    if (!surface) return NULL;

    surface->base.vtable = &g_sdl_surface_vtable;
    surface->device = impl->device;
    surface->type = desc->type;
    surface->width = desc->width;
    surface->height = desc->height;

    if (desc->type == DONG_SURFACE_TYPE_GPU_TEXTURE) {
        SDL_GPUTextureCreateInfo info = {0};
        info.type = SDL_GPU_TEXTURETYPE_2D;
        info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
        info.width = desc->width;
        info.height = desc->height;
        info.layer_count_or_depth = 1;
        info.num_levels = 1;
        info.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;

        surface->texture = SDL_CreateGPUTexture(impl->device, &info);
        if (!surface->texture) {
            free(surface);
            return NULL;
        }
    }

    return (DongSurface*)surface;
}

static void sdl_factory_destroy_surface(DongSurfaceFactory* factory, DongSurface* surface) {
    DongSDLSurfaceFactoryImpl* impl = (DongSDLSurfaceFactoryImpl*)factory;
    DongSDLSurfaceImpl* surf = (DongSDLSurfaceImpl*)surface;
    if (!impl || !surf) return;

    if (surf->texture && impl->device) {
        SDL_ReleaseGPUTexture(impl->device, surf->texture);
    }
    free(surf);
}

static DongSurface* sdl_factory_create_from_texture(DongSurfaceFactory* factory,
                                                     DongGPUTexture texture,
                                                     uint32_t width, uint32_t height) {
    DongSDLSurfaceFactoryImpl* impl = (DongSDLSurfaceFactoryImpl*)factory;
    if (!impl || !texture) return NULL;

    DongSDLSurfaceImpl* surface = (DongSDLSurfaceImpl*)calloc(1, sizeof(DongSDLSurfaceImpl));
    if (!surface) return NULL;

    surface->base.vtable = &g_sdl_surface_vtable;
    surface->device = NULL;  // Don't own the texture
    surface->type = DONG_SURFACE_TYPE_GPU_TEXTURE;
    surface->width = width;
    surface->height = height;
    surface->texture = (SDL_GPUTexture*)texture;

    return (DongSurface*)surface;
}

static const DongSurfaceFactoryVTable g_sdl_factory_vtable = {
    .create_surface = sdl_factory_create_surface,
    .destroy_surface = sdl_factory_destroy_surface,
    .create_surface_from_texture = sdl_factory_create_from_texture,
};

// =============================================================================
// Public API Implementation
// =============================================================================

DONG_SDL_PLATFORM_API DongGPUDriver* dong_sdl_create_gpu_driver(void* sdl_device, void* sdl_window) {
    if (!sdl_device) return NULL;

    DongSDLGPUDriverImpl* impl = (DongSDLGPUDriverImpl*)calloc(1, sizeof(DongSDLGPUDriverImpl));
    if (!impl) return NULL;

    impl->base.vtable = &g_sdl_gpu_vtable;
    impl->device = (SDL_GPUDevice*)sdl_device;
    impl->window = (SDL_Window*)sdl_window;
    impl->owns_device = 0;  // Device is externally owned
    impl->bridge = NULL;

    // Initialize transfer buffer tracking
    impl->pending_transfers = NULL;
    impl->pending_transfer_count = 0;
    impl->pending_transfer_capacity = 0;

    return (DongGPUDriver*)impl;
}


DONG_SDL_PLATFORM_API void dong_sdl_destroy_gpu_driver(DongGPUDriver* driver) {
    if (!driver) return;
    DongSDLGPUDriverImpl* impl = (DongSDLGPUDriverImpl*)driver;
    sdl_gpu_shutdown(driver);
    free(impl);
}

DONG_SDL_PLATFORM_API DongSurfaceFactory* dong_sdl_create_surface_factory(void* sdl_device) {
    if (!sdl_device) return NULL;

    DongSDLSurfaceFactoryImpl* impl = (DongSDLSurfaceFactoryImpl*)calloc(1, sizeof(DongSDLSurfaceFactoryImpl));
    if (!impl) return NULL;

    impl->base.vtable = &g_sdl_factory_vtable;
    impl->device = (SDL_GPUDevice*)sdl_device;

    return (DongSurfaceFactory*)impl;
}

DONG_SDL_PLATFORM_API void dong_sdl_destroy_surface_factory(DongSurfaceFactory* factory) {
    if (!factory) return;
    free(factory);
}

DONG_SDL_PLATFORM_API int dong_sdl_platform_init(void* sdl_device, void* sdl_window) {
    DongPlatform* platform = dong_platform_get();

    // Create and register GPU driver
    DongGPUDriver* driver = dong_sdl_create_gpu_driver(sdl_device, sdl_window);
    if (driver) {
        dong_platform_set_gpu_driver(platform, driver);
    }

    // Create and register surface factory
    DongSurfaceFactory* factory = dong_sdl_create_surface_factory(sdl_device);
    if (factory) {
        dong_platform_set_surface_factory(platform, factory);
    }

    // Create and register image decoder
    DongImageDecoder* decoder = dong_sdl_image_decoder_create();
    if (decoder) {
        dong_platform_set_image_decoder(platform, decoder);
    }

    return (driver && factory && decoder) ? 1 : 0;
}

DONG_SDL_PLATFORM_API void dong_sdl_platform_shutdown(void) {
    DongPlatform* platform = dong_platform_get();

    // Get and destroy registered subsystems
    DongGPUDriver* driver = dong_platform_get_gpu_driver(platform);
    DongSurfaceFactory* factory = dong_platform_get_surface_factory(platform);
    DongImageDecoder* decoder = dong_platform_get_image_decoder(platform);

    // Clear platform registrations
    dong_platform_reset();

    // Destroy implementations
    dong_sdl_destroy_gpu_driver(driver);
    dong_sdl_destroy_surface_factory(factory);
    dong_sdl_image_decoder_destroy(decoder);
}
