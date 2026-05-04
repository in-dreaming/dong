#ifndef DONG_GPU_ATLAS_OPS_H
#define DONG_GPU_ATLAS_OPS_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// GlyphAtlas Texture Operations (C ABI)
// =============================================================================
// These operations provide the minimal GPU capabilities needed by GlyphAtlas
// to manage atlas texture pages without directly depending on SDL or any
// specific GPU backend.
//
// Implementation is provided by the backend (e.g., SDL backend) and injected
// into the core engine via dong_platform_set_gpu_atlas_ops().
// =============================================================================

// Opaque handle to a GPU texture (implementation-defined)
typedef struct dong_gpu_texture_t dong_gpu_texture_t;

// Fence for async upload synchronization
typedef struct dong_gpu_fence_t dong_gpu_fence_t;

// Texture format (simplified for atlas use)
typedef enum dong_gpu_texture_format_t {
    DONG_GPU_TEXTURE_FORMAT_R8G8B8A8_UNORM = 0,
    DONG_GPU_TEXTURE_FORMAT_B8G8R8A8_UNORM = 1,
} dong_gpu_texture_format_t;

// Texture creation info
typedef struct dong_gpu_texture_create_info_t {
    uint32_t width;
    uint32_t height;
    dong_gpu_texture_format_t format;
    // If true, texture can be used as render target
    int is_render_target;
    // If true, texture can be sampled from shaders
    int is_sampled;
} dong_gpu_texture_create_info_t;

// Sub-rect upload info (supports stride)
typedef struct dong_gpu_texture_upload_info_t {
    // Destination texture
    dong_gpu_texture_t* texture;
    // Destination region (x, y, width, height)
    uint32_t dst_x;
    uint32_t dst_y;
    uint32_t width;
    uint32_t height;
    // Source data (row-major, may have stride)
    const uint8_t* data;
    // Stride in bytes between rows (0 = tightly packed width * 4)
    uint32_t stride_bytes;
} dong_gpu_texture_upload_info_t;

// =============================================================================
// GlyphAtlasTextureOps VTable
// =============================================================================
typedef struct dong_gpu_atlas_ops_t {
    // Create a texture for atlas use
    // Returns opaque handle or NULL on failure
    dong_gpu_texture_t* (*create_texture)(
        void* user_data,
        const dong_gpu_texture_create_info_t* info
    );

    // Destroy a texture
    void (*destroy_texture)(
        void* user_data,
        dong_gpu_texture_t* texture
    );

    // Upload pixel data to a sub-rectangle of the texture
    // Returns 0 on success, non-zero on failure
    int (*upload_subrect)(
        void* user_data,
        const dong_gpu_texture_upload_info_t* info
    );

    // Upload with async fence (optional, may be NULL)
    // The fence is signaled when the upload is complete on GPU
    int (*upload_subrect_async)(
        void* user_data,
        const dong_gpu_texture_upload_info_t* info,
        dong_gpu_fence_t** out_fence
    );

    // Wait for a fence to be signaled (optional, may be NULL)
    void (*wait_fence)(
        void* user_data,
        dong_gpu_fence_t* fence
    );

    // Release a fence (optional, may be NULL)
    void (*release_fence)(
        void* user_data,
        dong_gpu_fence_t* fence
    );

    // Get the native texture handle (for binding to shaders)
    // For SDL backend, this returns SDL_GPUTexture*
    void* (*get_native_handle)(
        void* user_data,
        dong_gpu_texture_t* texture
    );

} dong_gpu_atlas_ops_t;

// =============================================================================
// Platform API for setting ops
// =============================================================================

// Set the GPU atlas operations (call once during initialization)
void dong_platform_set_gpu_atlas_ops(
    const dong_gpu_atlas_ops_t* ops,
    void* user_data
);

// Get the current GPU atlas operations (returns NULL if not set)
const dong_gpu_atlas_ops_t* dong_platform_get_gpu_atlas_ops(void);

// Get the user data associated with the ops
void* dong_platform_get_gpu_atlas_ops_user_data(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // DONG_GPU_ATLAS_OPS_H
