#ifndef DONG_GPU_DRIVER_H
#define DONG_GPU_DRIVER_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// DLL export/import macros
// =============================================================================
#if defined(_WIN32) || defined(_WIN64)
    #ifdef DONG_BUILDING_DLL
        #define DONG_GPU_API __declspec(dllexport)
    #else
        #define DONG_GPU_API __declspec(dllimport)
    #endif
#else
    #define DONG_GPU_API __attribute__((visibility("default")))
#endif

// =============================================================================
// GPU Driver Abstraction
// =============================================================================
// The GPUDriver interface provides hardware-accelerated rendering capabilities.
// Backends (SDL, custom engines) implement this interface to provide GPU resources.
//
// Resource Ownership:
//   - Textures, buffers, and other GPU resources are owned by the GPUDriver.
//   - Handles returned are opaque and must be destroyed via driver methods.
// =============================================================================

// Forward declarations
typedef struct DongGPUDriver DongGPUDriver;
typedef struct DongGPUCommandList DongGPUCommandList;

// =============================================================================
// Opaque GPU Resource Handles
// =============================================================================
// All handles are owned by the GPUDriver that created them.
typedef void* DongGPUTexture;
typedef void* DongGPUBuffer;
typedef void* DongGPUSampler;
typedef void* DongGPURenderPass;

// =============================================================================
// GPU Resource Descriptors
// =============================================================================

typedef enum DongGPUTextureFormat {
    DONG_GPU_TEXTURE_FORMAT_RGBA8_UNORM = 0,
    DONG_GPU_TEXTURE_FORMAT_BGRA8_UNORM = 1,
    DONG_GPU_TEXTURE_FORMAT_R8_UNORM = 2,
    DONG_GPU_TEXTURE_FORMAT_DEPTH24_STENCIL8 = 10,
} DongGPUTextureFormat;

typedef enum DongGPUTextureUsage {
    DONG_GPU_TEXTURE_USAGE_SAMPLER = 1 << 0,
    DONG_GPU_TEXTURE_USAGE_COLOR_TARGET = 1 << 1,
    DONG_GPU_TEXTURE_USAGE_DEPTH_STENCIL_TARGET = 1 << 2,
    DONG_GPU_TEXTURE_USAGE_TRANSFER_SRC = 1 << 3,
    DONG_GPU_TEXTURE_USAGE_TRANSFER_DST = 1 << 4,
} DongGPUTextureUsage;

typedef struct DongGPUTextureDesc {
    uint32_t width;
    uint32_t height;
    DongGPUTextureFormat format;
    uint32_t usage;  // Bitmask of DongGPUTextureUsage
    uint32_t mip_levels;  // 0 or 1 = single level
    const char* debug_name;  // Optional debug label
} DongGPUTextureDesc;

typedef enum DongGPUBufferUsage {
    DONG_GPU_BUFFER_USAGE_VERTEX = 1 << 0,
    DONG_GPU_BUFFER_USAGE_INDEX = 1 << 1,
    DONG_GPU_BUFFER_USAGE_UNIFORM = 1 << 2,
    DONG_GPU_BUFFER_USAGE_TRANSFER_SRC = 1 << 3,
    DONG_GPU_BUFFER_USAGE_TRANSFER_DST = 1 << 4,
} DongGPUBufferUsage;

typedef struct DongGPUBufferDesc {
    uint32_t size;
    uint32_t usage;  // Bitmask of DongGPUBufferUsage
    const char* debug_name;
} DongGPUBufferDesc;

typedef enum DongGPUSamplerFilter {
    DONG_GPU_SAMPLER_FILTER_NEAREST = 0,
    DONG_GPU_SAMPLER_FILTER_LINEAR = 1,
} DongGPUSamplerFilter;

typedef enum DongGPUSamplerAddressMode {
    DONG_GPU_SAMPLER_ADDRESS_REPEAT = 0,
    DONG_GPU_SAMPLER_ADDRESS_CLAMP_TO_EDGE = 1,
    DONG_GPU_SAMPLER_ADDRESS_CLAMP_TO_BORDER = 2,
} DongGPUSamplerAddressMode;

typedef struct DongGPUSamplerDesc {
    DongGPUSamplerFilter min_filter;
    DongGPUSamplerFilter mag_filter;
    DongGPUSamplerFilter mip_filter;
    DongGPUSamplerAddressMode address_mode_u;
    DongGPUSamplerAddressMode address_mode_v;
} DongGPUSamplerDesc;

// =============================================================================
// GPU Command List (Internal IR)
// =============================================================================
// The GPUCommandList is an opaque structure containing rendering commands.
// It is built by the core engine and passed to execute() for GPU submission.
// The actual structure is defined internally; backends receive a pointer.

// =============================================================================
// GPU Driver Virtual Table
// =============================================================================
// Backends implement this interface to provide GPU functionality.

typedef struct DongGPUDriverVTable {
    // Lifecycle
    int (*initialize)(DongGPUDriver* driver);
    void (*shutdown)(DongGPUDriver* driver);

    // Resource Creation
    DongGPUTexture (*create_texture)(DongGPUDriver* driver, const DongGPUTextureDesc* desc);
    void (*destroy_texture)(DongGPUDriver* driver, DongGPUTexture texture);

    DongGPUBuffer (*create_buffer)(DongGPUDriver* driver, const DongGPUBufferDesc* desc);
    void (*destroy_buffer)(DongGPUDriver* driver, DongGPUBuffer buffer);

    DongGPUSampler (*create_sampler)(DongGPUDriver* driver, const DongGPUSamplerDesc* desc);
    void (*destroy_sampler)(DongGPUDriver* driver, DongGPUSampler sampler);

    // Resource Updates
    int (*upload_texture)(DongGPUDriver* driver, DongGPUTexture texture,
                          const void* data, uint32_t width, uint32_t height,
                          uint32_t x, uint32_t y);
    int (*upload_buffer)(DongGPUDriver* driver, DongGPUBuffer buffer,
                         const void* data, uint32_t size, uint32_t offset);

    // Frame Management
    int (*begin_frame)(DongGPUDriver* driver);
    int (*end_frame)(DongGPUDriver* driver);

    // Command Execution
    // The command_list is the internal GPUCommandList from gpu_ir.hpp.
    // Backends that understand the internal format can cast and execute directly.
    int (*execute)(DongGPUDriver* driver, const void* command_list);

    // Offscreen Rendering
    int (*begin_frame_offscreen)(DongGPUDriver* driver, DongGPUTexture target,
                                  uint32_t width, uint32_t height);
    int (*end_frame_offscreen)(DongGPUDriver* driver);

    // Resource Preparation (upload glyphs, images before render pass)
    void (*prepare_resources)(DongGPUDriver* driver, const void* command_list);

    // External Image Support (for video, etc.)
    int (*update_external_image_rgba)(DongGPUDriver* driver, const char* key,
                                       const uint8_t* rgba, uint32_t width,
                                       uint32_t height, uint32_t stride_bytes);
    int (*update_external_image_yuv420p)(DongGPUDriver* driver, const char* key,
                                          const uint8_t* plane_y, uint32_t stride_y,
                                          const uint8_t* plane_u, uint32_t stride_u,
                                          const uint8_t* plane_v, uint32_t stride_v,
                                          uint32_t width, uint32_t height);

    // Query
    void (*get_capabilities)(DongGPUDriver* driver, uint32_t* out_max_texture_size);

    // Native Handle Access (for advanced integration)
    void* (*get_native_device)(DongGPUDriver* driver);
} DongGPUDriverVTable;

// =============================================================================
// GPU Driver Structure
// =============================================================================
// Backend implementations embed this struct and provide the vtable.

struct DongGPUDriver {
    const DongGPUDriverVTable* vtable;
    void* user_data;  // Backend-specific data
};

// =============================================================================
// Convenience Functions (call through vtable)
// =============================================================================

static inline int dong_gpu_driver_initialize(DongGPUDriver* d) {
    return d && d->vtable && d->vtable->initialize ? d->vtable->initialize(d) : 0;
}

static inline void dong_gpu_driver_shutdown(DongGPUDriver* d) {
    if (d && d->vtable && d->vtable->shutdown) d->vtable->shutdown(d);
}

static inline DongGPUTexture dong_gpu_create_texture(DongGPUDriver* d, const DongGPUTextureDesc* desc) {
    return (d && d->vtable && d->vtable->create_texture) ? d->vtable->create_texture(d, desc) : NULL;
}

static inline void dong_gpu_destroy_texture(DongGPUDriver* d, DongGPUTexture t) {
    if (d && d->vtable && d->vtable->destroy_texture) d->vtable->destroy_texture(d, t);
}

static inline DongGPUBuffer dong_gpu_create_buffer(DongGPUDriver* d, const DongGPUBufferDesc* desc) {
    return (d && d->vtable && d->vtable->create_buffer) ? d->vtable->create_buffer(d, desc) : NULL;
}

static inline void dong_gpu_destroy_buffer(DongGPUDriver* d, DongGPUBuffer b) {
    if (d && d->vtable && d->vtable->destroy_buffer) d->vtable->destroy_buffer(d, b);
}

static inline int dong_gpu_begin_frame(DongGPUDriver* d) {
    return (d && d->vtable && d->vtable->begin_frame) ? d->vtable->begin_frame(d) : 0;
}

static inline int dong_gpu_end_frame(DongGPUDriver* d) {
    return (d && d->vtable && d->vtable->end_frame) ? d->vtable->end_frame(d) : 0;
}

static inline int dong_gpu_execute(DongGPUDriver* d, const void* cmd_list) {
    return (d && d->vtable && d->vtable->execute) ? d->vtable->execute(d, cmd_list) : 0;
}

static inline void* dong_gpu_get_native_device(DongGPUDriver* d) {
    return (d && d->vtable && d->vtable->get_native_device) ? d->vtable->get_native_device(d) : NULL;
}

#ifdef __cplusplus
}
#endif

#endif // DONG_GPU_DRIVER_H
