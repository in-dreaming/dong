// =============================================================================
// GPU Texture Compressor
// =============================================================================
// Uses compute shaders for GPU-accelerated BC7/ASTC texture compression
// =============================================================================

#pragma once

#include <cstdint>
#include <string>
#include <memory>

struct SDL_GPUDevice;
struct SDL_GPUTexture;
struct SDL_GPUBuffer;
struct SDL_GPUComputePipeline;
struct SDL_GPUCommandBuffer;

namespace dong::sdl_backend {

class GPUDevice;
class ShaderManager;

// Compression format enum
enum class GPUCompressFormat {
    BC7,
    ASTC_4x4,
    ASTC_5x5,
    ASTC_6x6,
    ASTC_8x8
};

// GPU Texture Compressor
// Compresses RGBA8 textures to BC7 or ASTC format using compute shaders
class GPUTextureCompressor {
public:
    GPUTextureCompressor(GPUDevice* gpu_device, ShaderManager* shader_manager);
    ~GPUTextureCompressor();

    // Initialize compute pipelines
    bool initialize();

    // Check if a format is supported
    bool isFormatSupported(GPUCompressFormat format) const;

    // Compress a texture (synchronous, blocks until complete)
    // Returns true if successful, false otherwise
    // Output buffer must be pre-allocated with correct size
    bool compressTexture(
        SDL_GPUCommandBuffer* cmd_buf,
        SDL_GPUTexture* src_texture,
        uint32_t src_width,
        uint32_t src_height,
        GPUCompressFormat format,
        void* output_data,
        size_t output_size
    );

    // Compress from RGBA8 pixel data (creates temporary texture internally)
    // This is a convenience method that handles texture creation/upload
    bool compressFromPixels(
        const uint8_t* rgba_pixels,
        uint32_t width,
        uint32_t height,
        GPUCompressFormat format,
        void* output_data,
        size_t output_size
    );

    // Compress RGBA8 pixels directly to a region of a destination texture (GPU-side only)
    // This is the most efficient path - no GPU→CPU→GPU round-trip
    // dst_texture must have COMPUTE_STORAGE_WRITE usage and matching compressed format
    // Returns true if successful
    bool compressToTextureRegion(
        const uint8_t* rgba_pixels,
        uint32_t src_width,
        uint32_t src_height,
        GPUCompressFormat format,
        SDL_GPUTexture* dst_texture,
        uint32_t dst_x,
        uint32_t dst_y
    );

    // Get required output buffer size for given dimensions and format
    static size_t getCompressedSize(uint32_t width, uint32_t height, GPUCompressFormat format);

    // Get block dimensions for a format
    static void getBlockDimensions(GPUCompressFormat format, uint32_t& block_w, uint32_t& block_h);

private:
    GPUDevice* gpu_device_;
    ShaderManager* shader_manager_;

    // Compute pipelines
    SDL_GPUComputePipeline* bc7_pipeline_ = nullptr;
    SDL_GPUComputePipeline* astc_4x4_pipeline_ = nullptr;
    SDL_GPUComputePipeline* astc_5x5_pipeline_ = nullptr;
    SDL_GPUComputePipeline* astc_6x6_pipeline_ = nullptr;
    SDL_GPUComputePipeline* astc_8x8_pipeline_ = nullptr;

    // Working buffer for compression output (reused)
    SDL_GPUBuffer* output_buffer_ = nullptr;
    size_t output_buffer_size_ = 0;

    bool initialized_ = false;

    // Internal helpers
    bool ensureOutputBuffer(SDL_GPUDevice* dev, size_t required_size);
    SDL_GPUComputePipeline* getPipelineForFormat(GPUCompressFormat format) const;
};

} // namespace dong::sdl_backend
