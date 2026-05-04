// =============================================================================
// GPU Texture Compressor Implementation
// =============================================================================
// Uses compute shaders for GPU-accelerated BC7/ASTC texture compression
// =============================================================================

#include "gpu_texture_compressor.hpp"
#include "sdl_gpu_device.hpp"
#include "sdl_shader_manager.hpp"
#include "../../src/core/log.h"

#include <SDL3/SDL_gpu.h>
#include <cstring>
#include <fstream>
#include <sstream>

namespace dong::sdl_backend {

// Shader directory path (set via CMake define)
#ifndef DONG_SDL_SHADER_DIR
#define DONG_SDL_SHADER_DIR "backends/sdl/shaders"
#endif

static std::string getShaderPath(const char* filename) {
    std::string base = DONG_SDL_SHADER_DIR;
    if (!base.empty() && (base.back() == '/' || base.back() == '\\')) {
        return base + "cmp/" + filename;
    }
    return base + "/cmp/" + filename;
}

static std::string getShaderIncludeDir() {
    std::string base = DONG_SDL_SHADER_DIR;
    if (!base.empty() && (base.back() == '/' || base.back() == '\\')) {
        return base + "cmp";
    }
    return base + "/cmp";
}

static std::string loadShaderSource(const char* filename) {
    std::string path = getShaderPath(filename);
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) {
        DONG_LOG_WARN("GPUTextureCompressor: Could not load shader file: %s", path.c_str());
        return "";
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

GPUTextureCompressor::GPUTextureCompressor(GPUDevice* gpu_device, ShaderManager* shader_manager)
    : gpu_device_(gpu_device), shader_manager_(shader_manager) {}

GPUTextureCompressor::~GPUTextureCompressor() {
    if (output_buffer_ && gpu_device_ && gpu_device_->isInitialized()) {
        SDL_ReleaseGPUBuffer(gpu_device_->getHandle(), output_buffer_);
        output_buffer_ = nullptr;
    }
}

bool GPUTextureCompressor::initialize() {
    if (initialized_) {
        return true;
    }

    if (!gpu_device_ || !gpu_device_->isInitialized() || !shader_manager_) {
        DONG_LOG_ERROR("GPUTextureCompressor: GPU device or shader manager not ready");
        return false;
    }

    // Load shader sources from files
    std::string bc7_source = loadShaderSource("bc7_encode_cs.hlsl");
    std::string astc_source = loadShaderSource("astc_encode_cs.hlsl");
    std::string include_dir = getShaderIncludeDir();

    // Initialize BC7 pipeline
    if (!bc7_source.empty()) {
        ComputePipelineInfo bc7_info{};
        bc7_info.num_readonly_storage_textures = 1;  // g_srcTexture
        bc7_info.num_readwrite_storage_buffers = 1;  // g_dstBuffer
        bc7_info.num_uniform_buffers = 1;            // CompressParams
        bc7_info.threadcount_x = 8;
        bc7_info.threadcount_y = 8;
        bc7_info.threadcount_z = 1;

        bc7_pipeline_ = shader_manager_->loadComputePipelineFromHLSL(
            "bc7_encode_cs", bc7_source.c_str(), bc7_info, "main", include_dir.c_str());

        if (!bc7_pipeline_) {
            DONG_LOG_WARN("GPUTextureCompressor: Failed to create BC7 compute pipeline");
        }
    }

    // Initialize ASTC pipeline
    if (!astc_source.empty()) {
        ComputePipelineInfo astc_info{};
        astc_info.num_readonly_storage_textures = 1;
        astc_info.num_readwrite_storage_buffers = 1;
        astc_info.num_uniform_buffers = 1;
        astc_info.threadcount_x = 8;
        astc_info.threadcount_y = 8;
        astc_info.threadcount_z = 1;

        astc_4x4_pipeline_ = shader_manager_->loadComputePipelineFromHLSL(
            "astc_encode_cs", astc_source.c_str(), astc_info, "main", include_dir.c_str());

        // All ASTC sizes use the same pipeline with different uniforms
        astc_5x5_pipeline_ = astc_4x4_pipeline_;
        astc_6x6_pipeline_ = astc_4x4_pipeline_;
        astc_8x8_pipeline_ = astc_4x4_pipeline_;

        if (!astc_4x4_pipeline_) {
            DONG_LOG_WARN("GPUTextureCompressor: Failed to create ASTC compute pipeline");
        }
    }

    initialized_ = (bc7_pipeline_ != nullptr || astc_4x4_pipeline_ != nullptr);

    if (initialized_) {
        DONG_LOG_INFO("GPUTextureCompressor: Initialized (BC7=%s, ASTC=%s)",
                     bc7_pipeline_ ? "yes" : "no",
                     astc_4x4_pipeline_ ? "yes" : "no");
    } else {
        DONG_LOG_WARN("GPUTextureCompressor: No compression pipelines available");
    }

    return initialized_;
}

bool GPUTextureCompressor::isFormatSupported(GPUCompressFormat format) const {
    switch (format) {
        case GPUCompressFormat::BC7:
            return bc7_pipeline_ != nullptr;
        case GPUCompressFormat::ASTC_4x4:
        case GPUCompressFormat::ASTC_5x5:
        case GPUCompressFormat::ASTC_6x6:
        case GPUCompressFormat::ASTC_8x8:
            return astc_4x4_pipeline_ != nullptr;
        default:
            return false;
    }
}

size_t GPUTextureCompressor::getCompressedSize(uint32_t width, uint32_t height, GPUCompressFormat format) {
    uint32_t block_w, block_h;
    getBlockDimensions(format, block_w, block_h);

    uint32_t blocks_x = (width + block_w - 1) / block_w;
    uint32_t blocks_y = (height + block_h - 1) / block_h;

    // Both BC7 and ASTC use 16 bytes per block
    return static_cast<size_t>(blocks_x) * blocks_y * 16;
}

void GPUTextureCompressor::getBlockDimensions(GPUCompressFormat format, uint32_t& block_w, uint32_t& block_h) {
    switch (format) {
        case GPUCompressFormat::BC7:
        case GPUCompressFormat::ASTC_4x4:
            block_w = 4; block_h = 4;
            break;
        case GPUCompressFormat::ASTC_5x5:
            block_w = 5; block_h = 5;
            break;
        case GPUCompressFormat::ASTC_6x6:
            block_w = 6; block_h = 6;
            break;
        case GPUCompressFormat::ASTC_8x8:
            block_w = 8; block_h = 8;
            break;
        default:
            block_w = 4; block_h = 4;
            break;
    }
}

SDL_GPUComputePipeline* GPUTextureCompressor::getPipelineForFormat(GPUCompressFormat format) const {
    switch (format) {
        case GPUCompressFormat::BC7:
            return bc7_pipeline_;
        case GPUCompressFormat::ASTC_4x4:
        case GPUCompressFormat::ASTC_5x5:
        case GPUCompressFormat::ASTC_6x6:
        case GPUCompressFormat::ASTC_8x8:
            return astc_4x4_pipeline_;
        default:
            return nullptr;
    }
}

bool GPUTextureCompressor::ensureOutputBuffer(SDL_GPUDevice* dev, size_t required_size) {
    if (output_buffer_ && output_buffer_size_ >= required_size) {
        return true;
    }

    // Release old buffer
    if (output_buffer_) {
        SDL_ReleaseGPUBuffer(dev, output_buffer_);
        output_buffer_ = nullptr;
        output_buffer_size_ = 0;
    }

    // Create new buffer with some headroom
    size_t alloc_size = required_size + (required_size / 4);  // 25% headroom

    SDL_GPUBufferCreateInfo buf_info{};
    buf_info.usage = SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_WRITE | SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_READ;
    buf_info.size = (Uint32)alloc_size;

    output_buffer_ = SDL_CreateGPUBuffer(dev, &buf_info);
    if (!output_buffer_) {
        DONG_LOG_ERROR("GPUTextureCompressor: Failed to create output buffer: %s", SDL_GetError());
        return false;
    }

    output_buffer_size_ = alloc_size;
    return true;
}

bool GPUTextureCompressor::compressTexture(
    SDL_GPUCommandBuffer* cmd_buf,
    SDL_GPUTexture* src_texture,
    uint32_t src_width,
    uint32_t src_height,
    GPUCompressFormat format,
    void* output_data,
    size_t output_size) {

    if (!initialized_ || !gpu_device_ || !gpu_device_->isInitialized()) {
        DONG_LOG_ERROR("GPUTextureCompressor: Not initialized");
        return false;
    }

    SDL_GPUComputePipeline* pipeline = getPipelineForFormat(format);
    if (!pipeline) {
        DONG_LOG_ERROR("GPUTextureCompressor: Format not supported");
        return false;
    }

    size_t required_size = getCompressedSize(src_width, src_height, format);
    if (output_size < required_size) {
        DONG_LOG_ERROR("GPUTextureCompressor: Output buffer too small (%zu < %zu)",
                      output_size, required_size);
        return false;
    }

    SDL_GPUDevice* dev = gpu_device_->getHandle();

    // Ensure output buffer
    if (!ensureOutputBuffer(dev, required_size)) {
        return false;
    }

    // Get block dimensions
    uint32_t block_w, block_h;
    getBlockDimensions(format, block_w, block_h);
    uint32_t blocks_x = (src_width + block_w - 1) / block_w;
    uint32_t blocks_y = (src_height + block_h - 1) / block_h;

    // Prepare uniform data
    struct CompressParams {
        uint32_t srcWidth;
        uint32_t srcHeight;
        uint32_t blocksX;
        uint32_t blocksY;
        uint32_t blockWidth;
        uint32_t blockHeight;
        uint32_t pad0;
        uint32_t pad1;
    };
    CompressParams params = {
        src_width, src_height, blocks_x, blocks_y,
        block_w, block_h, 0, 0
    };

    // Begin compute pass
    SDL_GPUStorageBufferReadWriteBinding buffer_binding{};
    buffer_binding.buffer = output_buffer_;
    buffer_binding.cycle = false;

    SDL_GPUComputePass* compute_pass = SDL_BeginGPUComputePass(
        cmd_buf,
        nullptr, 0,  // No storage textures in read-write mode
        &buffer_binding, 1
    );

    if (!compute_pass) {
        DONG_LOG_ERROR("GPUTextureCompressor: Failed to begin compute pass: %s", SDL_GetError());
        return false;
    }

    // Bind pipeline and resources
    SDL_BindGPUComputePipeline(compute_pass, pipeline);

    // Bind source texture as readonly storage texture
    SDL_GPUTexture* textures[] = { src_texture };
    SDL_BindGPUComputeStorageTextures(compute_pass, 0, textures, 1);

    // Push uniform data
    SDL_PushGPUComputeUniformData(cmd_buf, 0, &params, sizeof(params));

    // Dispatch compute
    uint32_t groups_x = (blocks_x + 7) / 8;
    uint32_t groups_y = (blocks_y + 7) / 8;
    SDL_DispatchGPUCompute(compute_pass, groups_x, groups_y, 1);

    SDL_EndGPUComputePass(compute_pass);

    // Download result to CPU
    // Create transfer buffer
    SDL_GPUTransferBufferCreateInfo transfer_info{};
    transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD;
    transfer_info.size = (Uint32)required_size;

    SDL_GPUTransferBuffer* transfer_buf = SDL_CreateGPUTransferBuffer(dev, &transfer_info);
    if (!transfer_buf) {
        DONG_LOG_ERROR("GPUTextureCompressor: Failed to create transfer buffer: %s", SDL_GetError());
        return false;
    }

    // Begin copy pass to download buffer
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd_buf);
    if (!copy_pass) {
        SDL_ReleaseGPUTransferBuffer(dev, transfer_buf);
        DONG_LOG_ERROR("GPUTextureCompressor: Failed to begin copy pass: %s", SDL_GetError());
        return false;
    }

    SDL_GPUTransferBufferLocation dst_loc{};
    dst_loc.transfer_buffer = transfer_buf;
    dst_loc.offset = 0;

    SDL_GPUBufferRegion src_region{};
    src_region.buffer = output_buffer_;
    src_region.offset = 0;
    src_region.size = (Uint32)required_size;

    SDL_DownloadFromGPUBuffer(copy_pass, &src_region, &dst_loc);
    SDL_EndGPUCopyPass(copy_pass);

    // Submit and wait
    SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(cmd_buf);
    if (!fence) {
        SDL_ReleaseGPUTransferBuffer(dev, transfer_buf);
        DONG_LOG_ERROR("GPUTextureCompressor: Failed to submit command buffer: %s", SDL_GetError());
        return false;
    }

    // Wait for completion
    SDL_WaitForGPUFences(dev, true, &fence, 1);
    SDL_ReleaseGPUFence(dev, fence);

    // Map and copy result
    void* mapped = SDL_MapGPUTransferBuffer(dev, transfer_buf, false);
    if (!mapped) {
        SDL_ReleaseGPUTransferBuffer(dev, transfer_buf);
        DONG_LOG_ERROR("GPUTextureCompressor: Failed to map transfer buffer: %s", SDL_GetError());
        return false;
    }

    std::memcpy(output_data, mapped, required_size);
    SDL_UnmapGPUTransferBuffer(dev, transfer_buf);
    SDL_ReleaseGPUTransferBuffer(dev, transfer_buf);

    DONG_LOG_DEBUG("GPUTextureCompressor: Compressed %ux%u to %zu bytes (format=%d)",
                  src_width, src_height, required_size, (int)format);

    return true;
}

bool GPUTextureCompressor::compressFromPixels(
    const uint8_t* rgba_pixels,
    uint32_t width,
    uint32_t height,
    GPUCompressFormat format,
    void* output_data,
    size_t output_size) {

    if (!initialized_ || !gpu_device_ || !gpu_device_->isInitialized()) {
        DONG_LOG_ERROR("GPUTextureCompressor: Not initialized");
        return false;
    }

    if (!rgba_pixels || width == 0 || height == 0) {
        DONG_LOG_ERROR("GPUTextureCompressor: Invalid input parameters");
        return false;
    }

    SDL_GPUComputePipeline* pipeline = getPipelineForFormat(format);
    if (!pipeline) {
        DONG_LOG_ERROR("GPUTextureCompressor: Format not supported");
        return false;
    }

    size_t required_size = getCompressedSize(width, height, format);
    if (output_size < required_size) {
        DONG_LOG_ERROR("GPUTextureCompressor: Output buffer too small (%zu < %zu)",
                      output_size, required_size);
        return false;
    }

    SDL_GPUDevice* dev = gpu_device_->getHandle();

    // Create temporary texture for source data
    SDL_GPUTextureCreateInfo tex_info{};
    tex_info.type = SDL_GPU_TEXTURETYPE_2D;
    tex_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    tex_info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_READ;
    tex_info.width = width;
    tex_info.height = height;
    tex_info.layer_count_or_depth = 1;
    tex_info.num_levels = 1;

    SDL_GPUTexture* temp_texture = SDL_CreateGPUTexture(dev, &tex_info);
    if (!temp_texture) {
        DONG_LOG_ERROR("GPUTextureCompressor: Failed to create temp texture: %s", SDL_GetError());
        return false;
    }

    // Create transfer buffer and upload pixel data
    size_t pixel_size = (size_t)width * height * 4;

    SDL_GPUTransferBufferCreateInfo upload_info{};
    upload_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    upload_info.size = (Uint32)pixel_size;

    SDL_GPUTransferBuffer* upload_buf = SDL_CreateGPUTransferBuffer(dev, &upload_info);
    if (!upload_buf) {
        SDL_ReleaseGPUTexture(dev, temp_texture);
        DONG_LOG_ERROR("GPUTextureCompressor: Failed to create upload buffer: %s", SDL_GetError());
        return false;
    }

    // Map and copy pixel data
    void* mapped = SDL_MapGPUTransferBuffer(dev, upload_buf, false);
    if (!mapped) {
        SDL_ReleaseGPUTransferBuffer(dev, upload_buf);
        SDL_ReleaseGPUTexture(dev, temp_texture);
        DONG_LOG_ERROR("GPUTextureCompressor: Failed to map upload buffer: %s", SDL_GetError());
        return false;
    }
    std::memcpy(mapped, rgba_pixels, pixel_size);
    SDL_UnmapGPUTransferBuffer(dev, upload_buf);

    // Acquire command buffer for upload and compression
    SDL_GPUCommandBuffer* cmd_buf = gpu_device_->acquireCommandBuffer();
    if (!cmd_buf) {
        SDL_ReleaseGPUTransferBuffer(dev, upload_buf);
        SDL_ReleaseGPUTexture(dev, temp_texture);
        DONG_LOG_ERROR("GPUTextureCompressor: Failed to acquire command buffer");
        return false;
    }

    // Upload pixels to texture
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd_buf);
    if (!copy_pass) {
        SDL_ReleaseGPUTransferBuffer(dev, upload_buf);
        SDL_ReleaseGPUTexture(dev, temp_texture);
        DONG_LOG_ERROR("GPUTextureCompressor: Failed to begin copy pass: %s", SDL_GetError());
        return false;
    }

    SDL_GPUTextureTransferInfo src_transfer{};
    src_transfer.transfer_buffer = upload_buf;
    src_transfer.offset = 0;
    src_transfer.pixels_per_row = width;
    src_transfer.rows_per_layer = height;

    SDL_GPUTextureRegion dst_region{};
    dst_region.texture = temp_texture;
    dst_region.x = 0;
    dst_region.y = 0;
    dst_region.z = 0;
    dst_region.w = width;
    dst_region.h = height;
    dst_region.d = 1;

    SDL_UploadToGPUTexture(copy_pass, &src_transfer, &dst_region, false);
    SDL_EndGPUCopyPass(copy_pass);

    // Ensure output buffer is ready
    if (!ensureOutputBuffer(dev, required_size)) {
        SDL_ReleaseGPUTransferBuffer(dev, upload_buf);
        SDL_ReleaseGPUTexture(dev, temp_texture);
        return false;
    }

    // Get block dimensions
    uint32_t block_w, block_h;
    getBlockDimensions(format, block_w, block_h);
    uint32_t blocks_x = (width + block_w - 1) / block_w;
    uint32_t blocks_y = (height + block_h - 1) / block_h;

    // Prepare uniform data
    struct CompressParams {
        uint32_t srcWidth;
        uint32_t srcHeight;
        uint32_t blocksX;
        uint32_t blocksY;
        uint32_t blockWidth;
        uint32_t blockHeight;
        uint32_t pad0;
        uint32_t pad1;
    };
    CompressParams params = {
        width, height, blocks_x, blocks_y,
        block_w, block_h, 0, 0
    };

    // Begin compute pass
    SDL_GPUStorageBufferReadWriteBinding buffer_binding{};
    buffer_binding.buffer = output_buffer_;
    buffer_binding.cycle = false;

    SDL_GPUComputePass* compute_pass = SDL_BeginGPUComputePass(
        cmd_buf,
        nullptr, 0,
        &buffer_binding, 1
    );

    if (!compute_pass) {
        SDL_ReleaseGPUTransferBuffer(dev, upload_buf);
        SDL_ReleaseGPUTexture(dev, temp_texture);
        DONG_LOG_ERROR("GPUTextureCompressor: Failed to begin compute pass: %s", SDL_GetError());
        return false;
    }

    // Bind pipeline and resources
    SDL_BindGPUComputePipeline(compute_pass, pipeline);

    SDL_GPUTexture* textures[] = { temp_texture };
    SDL_BindGPUComputeStorageTextures(compute_pass, 0, textures, 1);

    SDL_PushGPUComputeUniformData(cmd_buf, 0, &params, sizeof(params));

    // Dispatch compute
    uint32_t groups_x = (blocks_x + 7) / 8;
    uint32_t groups_y = (blocks_y + 7) / 8;
    SDL_DispatchGPUCompute(compute_pass, groups_x, groups_y, 1);

    SDL_EndGPUComputePass(compute_pass);

    // Download compressed data
    SDL_GPUTransferBufferCreateInfo download_info{};
    download_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD;
    download_info.size = (Uint32)required_size;

    SDL_GPUTransferBuffer* download_buf = SDL_CreateGPUTransferBuffer(dev, &download_info);
    if (!download_buf) {
        SDL_ReleaseGPUTransferBuffer(dev, upload_buf);
        SDL_ReleaseGPUTexture(dev, temp_texture);
        DONG_LOG_ERROR("GPUTextureCompressor: Failed to create download buffer: %s", SDL_GetError());
        return false;
    }

    SDL_GPUCopyPass* download_pass = SDL_BeginGPUCopyPass(cmd_buf);
    if (!download_pass) {
        SDL_ReleaseGPUTransferBuffer(dev, download_buf);
        SDL_ReleaseGPUTransferBuffer(dev, upload_buf);
        SDL_ReleaseGPUTexture(dev, temp_texture);
        DONG_LOG_ERROR("GPUTextureCompressor: Failed to begin download pass: %s", SDL_GetError());
        return false;
    }

    SDL_GPUTransferBufferLocation dst_loc{};
    dst_loc.transfer_buffer = download_buf;
    dst_loc.offset = 0;

    SDL_GPUBufferRegion src_region{};
    src_region.buffer = output_buffer_;
    src_region.offset = 0;
    src_region.size = (Uint32)required_size;

    SDL_DownloadFromGPUBuffer(download_pass, &src_region, &dst_loc);
    SDL_EndGPUCopyPass(download_pass);

    // Submit and wait
    SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(cmd_buf);
    if (!fence) {
        SDL_ReleaseGPUTransferBuffer(dev, download_buf);
        SDL_ReleaseGPUTransferBuffer(dev, upload_buf);
        SDL_ReleaseGPUTexture(dev, temp_texture);
        DONG_LOG_ERROR("GPUTextureCompressor: Failed to submit command buffer: %s", SDL_GetError());
        return false;
    }

    SDL_WaitForGPUFences(dev, true, &fence, 1);
    SDL_ReleaseGPUFence(dev, fence);

    // Map and copy result
    void* result_mapped = SDL_MapGPUTransferBuffer(dev, download_buf, false);
    if (!result_mapped) {
        SDL_ReleaseGPUTransferBuffer(dev, download_buf);
        SDL_ReleaseGPUTransferBuffer(dev, upload_buf);
        SDL_ReleaseGPUTexture(dev, temp_texture);
        DONG_LOG_ERROR("GPUTextureCompressor: Failed to map download buffer: %s", SDL_GetError());
        return false;
    }

    std::memcpy(output_data, result_mapped, required_size);
    SDL_UnmapGPUTransferBuffer(dev, download_buf);

    // Cleanup
    SDL_ReleaseGPUTransferBuffer(dev, download_buf);
    SDL_ReleaseGPUTransferBuffer(dev, upload_buf);
    SDL_ReleaseGPUTexture(dev, temp_texture);

    DONG_LOG_DEBUG("GPUTextureCompressor: GPU compressed %ux%u pixels to %zu bytes (format=%d)",
                  width, height, required_size, (int)format);

    return true;
}

bool GPUTextureCompressor::compressToTextureRegion(
    const uint8_t* rgba_pixels,
    uint32_t src_width,
    uint32_t src_height,
    GPUCompressFormat format,
    SDL_GPUTexture* dst_texture,
    uint32_t dst_x,
    uint32_t dst_y) {

    if (!initialized_ || !gpu_device_ || !gpu_device_->isInitialized()) {
        DONG_LOG_ERROR("GPUTextureCompressor: Not initialized");
        return false;
    }

    if (!rgba_pixels || src_width == 0 || src_height == 0 || !dst_texture) {
        DONG_LOG_ERROR("GPUTextureCompressor: Invalid input parameters");
        return false;
    }

    SDL_GPUComputePipeline* pipeline = getPipelineForFormat(format);
    if (!pipeline) {
        DONG_LOG_ERROR("GPUTextureCompressor: Format not supported");
        return false;
    }

    SDL_GPUDevice* dev = gpu_device_->getHandle();

    // Get block dimensions
    uint32_t block_w, block_h;
    getBlockDimensions(format, block_w, block_h);

    // Align dimensions to block size
    uint32_t aligned_w = ((src_width + block_w - 1) / block_w) * block_w;
    uint32_t aligned_h = ((src_height + block_h - 1) / block_h) * block_h;

    uint32_t blocks_x = aligned_w / block_w;
    uint32_t blocks_y = aligned_h / block_h;

    // Compressed size = 16 bytes per block for both BC7 and ASTC
    size_t compressed_size = (size_t)blocks_x * blocks_y * 16;

    // Ensure output buffer
    if (!ensureOutputBuffer(dev, compressed_size)) {
        return false;
    }

    // Create temporary texture for source RGBA data
    SDL_GPUTextureCreateInfo tex_info{};
    tex_info.type = SDL_GPU_TEXTURETYPE_2D;
    tex_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    tex_info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_READ;
    tex_info.width = src_width;
    tex_info.height = src_height;
    tex_info.layer_count_or_depth = 1;
    tex_info.num_levels = 1;

    SDL_GPUTexture* temp_texture = SDL_CreateGPUTexture(dev, &tex_info);
    if (!temp_texture) {
        DONG_LOG_ERROR("GPUTextureCompressor: Failed to create temp texture: %s", SDL_GetError());
        return false;
    }

    // Create upload transfer buffer
    size_t pixel_size = (size_t)src_width * src_height * 4;

    SDL_GPUTransferBufferCreateInfo upload_info{};
    upload_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    upload_info.size = (Uint32)pixel_size;

    SDL_GPUTransferBuffer* upload_buf = SDL_CreateGPUTransferBuffer(dev, &upload_info);
    if (!upload_buf) {
        SDL_ReleaseGPUTexture(dev, temp_texture);
        DONG_LOG_ERROR("GPUTextureCompressor: Failed to create upload buffer: %s", SDL_GetError());
        return false;
    }

    // Map and copy pixel data
    void* mapped = SDL_MapGPUTransferBuffer(dev, upload_buf, false);
    if (!mapped) {
        SDL_ReleaseGPUTransferBuffer(dev, upload_buf);
        SDL_ReleaseGPUTexture(dev, temp_texture);
        DONG_LOG_ERROR("GPUTextureCompressor: Failed to map upload buffer: %s", SDL_GetError());
        return false;
    }
    std::memcpy(mapped, rgba_pixels, pixel_size);
    SDL_UnmapGPUTransferBuffer(dev, upload_buf);

    // Acquire command buffer
    SDL_GPUCommandBuffer* cmd_buf = gpu_device_->acquireCommandBuffer();
    if (!cmd_buf) {
        SDL_ReleaseGPUTransferBuffer(dev, upload_buf);
        SDL_ReleaseGPUTexture(dev, temp_texture);
        DONG_LOG_ERROR("GPUTextureCompressor: Failed to acquire command buffer");
        return false;
    }

    // === Pass 1: Upload RGBA pixels to temp texture ===
    SDL_GPUCopyPass* upload_pass = SDL_BeginGPUCopyPass(cmd_buf);
    if (!upload_pass) {
        SDL_ReleaseGPUTransferBuffer(dev, upload_buf);
        SDL_ReleaseGPUTexture(dev, temp_texture);
        DONG_LOG_ERROR("GPUTextureCompressor: Failed to begin upload pass: %s", SDL_GetError());
        return false;
    }

    SDL_GPUTextureTransferInfo src_transfer{};
    src_transfer.transfer_buffer = upload_buf;
    src_transfer.offset = 0;
    src_transfer.pixels_per_row = src_width;
    src_transfer.rows_per_layer = src_height;

    SDL_GPUTextureRegion tex_dst_region{};
    tex_dst_region.texture = temp_texture;
    tex_dst_region.x = 0;
    tex_dst_region.y = 0;
    tex_dst_region.z = 0;
    tex_dst_region.w = src_width;
    tex_dst_region.h = src_height;
    tex_dst_region.d = 1;

    SDL_UploadToGPUTexture(upload_pass, &src_transfer, &tex_dst_region, false);
    SDL_EndGPUCopyPass(upload_pass);

    // === Pass 2: Compute shader compression ===
    struct CompressParams {
        uint32_t srcWidth;
        uint32_t srcHeight;
        uint32_t blocksX;
        uint32_t blocksY;
        uint32_t blockWidth;
        uint32_t blockHeight;
        uint32_t pad0;
        uint32_t pad1;
    };
    CompressParams params = {
        src_width, src_height, blocks_x, blocks_y,
        block_w, block_h, 0, 0
    };

    SDL_GPUStorageBufferReadWriteBinding buffer_binding{};
    buffer_binding.buffer = output_buffer_;
    buffer_binding.cycle = false;

    SDL_GPUComputePass* compute_pass = SDL_BeginGPUComputePass(
        cmd_buf,
        nullptr, 0,
        &buffer_binding, 1
    );

    if (!compute_pass) {
        SDL_ReleaseGPUTransferBuffer(dev, upload_buf);
        SDL_ReleaseGPUTexture(dev, temp_texture);
        DONG_LOG_ERROR("GPUTextureCompressor: Failed to begin compute pass: %s", SDL_GetError());
        return false;
    }

    SDL_BindGPUComputePipeline(compute_pass, pipeline);

    SDL_GPUTexture* textures[] = { temp_texture };
    SDL_BindGPUComputeStorageTextures(compute_pass, 0, textures, 1);

    SDL_PushGPUComputeUniformData(cmd_buf, 0, &params, sizeof(params));

    uint32_t groups_x = (blocks_x + 7) / 8;
    uint32_t groups_y = (blocks_y + 7) / 8;
    SDL_DispatchGPUCompute(compute_pass, groups_x, groups_y, 1);

    SDL_EndGPUComputePass(compute_pass);

    // === Pass 3: Copy compressed buffer to destination texture region ===
    // Use a transfer buffer as intermediary (buffer→transfer→texture for compressed formats)
    SDL_GPUTransferBufferCreateInfo download_info{};
    download_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD;
    download_info.size = (Uint32)compressed_size;

    SDL_GPUTransferBuffer* staging_buf = SDL_CreateGPUTransferBuffer(dev, &download_info);
    if (!staging_buf) {
        SDL_ReleaseGPUTransferBuffer(dev, upload_buf);
        SDL_ReleaseGPUTexture(dev, temp_texture);
        DONG_LOG_ERROR("GPUTextureCompressor: Failed to create staging buffer: %s", SDL_GetError());
        return false;
    }

    // Download from GPU buffer to staging transfer buffer
    SDL_GPUCopyPass* download_pass = SDL_BeginGPUCopyPass(cmd_buf);
    if (!download_pass) {
        SDL_ReleaseGPUTransferBuffer(dev, staging_buf);
        SDL_ReleaseGPUTransferBuffer(dev, upload_buf);
        SDL_ReleaseGPUTexture(dev, temp_texture);
        DONG_LOG_ERROR("GPUTextureCompressor: Failed to begin download pass: %s", SDL_GetError());
        return false;
    }

    SDL_GPUTransferBufferLocation staging_loc{};
    staging_loc.transfer_buffer = staging_buf;
    staging_loc.offset = 0;

    SDL_GPUBufferRegion buf_region{};
    buf_region.buffer = output_buffer_;
    buf_region.offset = 0;
    buf_region.size = (Uint32)compressed_size;

    SDL_DownloadFromGPUBuffer(download_pass, &buf_region, &staging_loc);
    SDL_EndGPUCopyPass(download_pass);

    // Submit and wait for download to complete
    SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(cmd_buf);
    if (!fence) {
        SDL_ReleaseGPUTransferBuffer(dev, staging_buf);
        SDL_ReleaseGPUTransferBuffer(dev, upload_buf);
        SDL_ReleaseGPUTexture(dev, temp_texture);
        DONG_LOG_ERROR("GPUTextureCompressor: Failed to submit command buffer: %s", SDL_GetError());
        return false;
    }

    SDL_WaitForGPUFences(dev, true, &fence, 1);
    SDL_ReleaseGPUFence(dev, fence);

    // Now create upload transfer buffer from staging data
    SDL_GPUTransferBufferCreateInfo final_upload_info{};
    final_upload_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    final_upload_info.size = (Uint32)compressed_size;

    SDL_GPUTransferBuffer* final_upload_buf = SDL_CreateGPUTransferBuffer(dev, &final_upload_info);
    if (!final_upload_buf) {
        SDL_ReleaseGPUTransferBuffer(dev, staging_buf);
        SDL_ReleaseGPUTransferBuffer(dev, upload_buf);
        SDL_ReleaseGPUTexture(dev, temp_texture);
        DONG_LOG_ERROR("GPUTextureCompressor: Failed to create final upload buffer: %s", SDL_GetError());
        return false;
    }

    // Copy staging to upload buffer
    void* staging_mapped = SDL_MapGPUTransferBuffer(dev, staging_buf, false);
    void* upload_mapped = SDL_MapGPUTransferBuffer(dev, final_upload_buf, false);
    if (staging_mapped && upload_mapped) {
        std::memcpy(upload_mapped, staging_mapped, compressed_size);
    }
    SDL_UnmapGPUTransferBuffer(dev, staging_buf);
    SDL_UnmapGPUTransferBuffer(dev, final_upload_buf);

    // Acquire new command buffer for final upload
    SDL_GPUCommandBuffer* final_cmd = gpu_device_->acquireCommandBuffer();
    if (!final_cmd) {
        SDL_ReleaseGPUTransferBuffer(dev, final_upload_buf);
        SDL_ReleaseGPUTransferBuffer(dev, staging_buf);
        SDL_ReleaseGPUTransferBuffer(dev, upload_buf);
        SDL_ReleaseGPUTexture(dev, temp_texture);
        DONG_LOG_ERROR("GPUTextureCompressor: Failed to acquire final command buffer");
        return false;
    }

    // Upload compressed data to destination texture
    SDL_GPUCopyPass* final_copy = SDL_BeginGPUCopyPass(final_cmd);
    if (!final_copy) {
        SDL_ReleaseGPUTransferBuffer(dev, final_upload_buf);
        SDL_ReleaseGPUTransferBuffer(dev, staging_buf);
        SDL_ReleaseGPUTransferBuffer(dev, upload_buf);
        SDL_ReleaseGPUTexture(dev, temp_texture);
        DONG_LOG_ERROR("GPUTextureCompressor: Failed to begin final copy pass: %s", SDL_GetError());
        return false;
    }

    SDL_GPUTextureTransferInfo compressed_transfer{};
    compressed_transfer.transfer_buffer = final_upload_buf;
    compressed_transfer.offset = 0;
    compressed_transfer.pixels_per_row = blocks_x;  // For compressed, this is in blocks
    compressed_transfer.rows_per_layer = blocks_y;

    SDL_GPUTextureRegion dst_region{};
    dst_region.texture = dst_texture;
    dst_region.x = dst_x;
    dst_region.y = dst_y;
    dst_region.z = 0;
    dst_region.w = aligned_w;
    dst_region.h = aligned_h;
    dst_region.d = 1;

    SDL_UploadToGPUTexture(final_copy, &compressed_transfer, &dst_region, false);
    SDL_EndGPUCopyPass(final_copy);

    // Submit and wait
    SDL_GPUFence* final_fence = SDL_SubmitGPUCommandBufferAndAcquireFence(final_cmd);
    if (final_fence) {
        SDL_WaitForGPUFences(dev, true, &final_fence, 1);
        SDL_ReleaseGPUFence(dev, final_fence);
    }

    // Cleanup
    SDL_ReleaseGPUTransferBuffer(dev, final_upload_buf);
    SDL_ReleaseGPUTransferBuffer(dev, staging_buf);
    SDL_ReleaseGPUTransferBuffer(dev, upload_buf);
    SDL_ReleaseGPUTexture(dev, temp_texture);

    DONG_LOG_DEBUG("GPUTextureCompressor: Compressed %ux%u to texture region (%u,%u) (format=%d)",
                  src_width, src_height, dst_x, dst_y, (int)format);

    return true;
}

} // namespace dong::sdl_backend
