#pragma once

#include "dong_gpu_driver.h"
#include "gpu_upload_queue.hpp"

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#ifdef DONG_HAS_IN_DREAMING_GPU
#include "gpu/gpu.h"
#endif

namespace dong::gpu_backend {

#ifdef DONG_HAS_IN_DREAMING_GPU
struct GpuFenceRecord {
    GpuFence fence = nullptr;
    uint64_t value = 0;
};

class GpuResourceManager {
public:
    void initialize(GpuDevice device, GpuUploadQueue* upload_queue);
    void shutdown();

    DongGPUTexture createTexture(const DongGPUTextureDesc* desc);
    void destroyTexture(DongGPUTexture texture);

    DongGPUBuffer createBuffer(const DongGPUBufferDesc* desc);
    void destroyBuffer(DongGPUBuffer buffer);

    DongGPUSampler createSampler(const DongGPUSamplerDesc* desc);
    void destroySampler(DongGPUSampler sampler);

    int uploadTexture(DongGPUTexture texture, const void* data, uint32_t width, uint32_t height,
                      uint32_t x, uint32_t y);
    int uploadBuffer(DongGPUBuffer buffer, const void* data, uint32_t size, uint32_t offset);
    int uploadTextureSubrect(DongGPUTexture texture, const void* data,
                             uint32_t dest_x, uint32_t dest_y,
                             uint32_t width, uint32_t height,
                             uint32_t src_stride_bytes, void** out_fence);

    int queryFence(void* fence);
    void releaseFence(void* fence);
    void waitForGpu();

    void* nativeTextureHandle(DongGPUTexture texture) const;
    GpuTextureHandle gpuTextureHandle(DongGPUTexture texture) const;
    GpuTextureHandle gpuTextureShaderView(DongGPUTexture texture) const;
    GpuBufferHandle gpuBufferHandle(DongGPUBuffer buffer) const;
    GpuSamplerHandle gpuSamplerHandle(DongGPUSampler sampler) const;

    void getCapabilities(uint32_t* out_max_texture_size) const;
    int readbackTextureRGBA(DongGPUTexture texture, uint32_t width, uint32_t height,
                            uint8_t* out_rgba, size_t out_rgba_bytes) const;
    GpuDevice device() const { return device_; }

private:
    struct TextureRecord {
        GpuTextureHandle handle{};
        GpuTextureHandle shader_view{};
        GpuTextureHandle rt_view{};
        DongGPUTextureDesc desc{};
    };
    struct BufferRecord {
        GpuBufferHandle handle{};
        DongGPUBufferDesc desc{};
    };
    struct SamplerRecord {
        GpuSamplerHandle handle{};
    };

    static GpuFormat mapTextureFormat(DongGPUTextureFormat format);
    static uint32_t mapTextureUsage(uint32_t usage);
    static uint32_t mapBufferUsage(uint32_t usage);
    static GpuSamplerDesc makeSamplerDesc(const DongGPUSamplerDesc& desc);

    int uploadTextureSubrectInternal(TextureRecord* rec, const void* data,
                                     uint32_t dest_x, uint32_t dest_y,
                                     uint32_t width, uint32_t height,
                                     uint32_t src_stride_bytes, void** out_fence);

    GpuDevice device_ = nullptr;
    GpuUploadQueue* upload_queue_ = nullptr;
    GpuCommandQueue queue_ = nullptr;
    std::unordered_map<DongGPUTexture, TextureRecord*> textures_;
    std::unordered_map<DongGPUBuffer, BufferRecord*> buffers_;
    std::unordered_map<DongGPUSampler, SamplerRecord*> samplers_;
    std::vector<std::unique_ptr<TextureRecord>> texture_storage_;
    std::vector<std::unique_ptr<BufferRecord>> buffer_storage_;
    std::vector<std::unique_ptr<SamplerRecord>> sampler_storage_;
    std::vector<std::unique_ptr<GpuFenceRecord>> fences_;
};
#else
class GpuResourceManager {
public:
    void initialize(void*, GpuUploadQueue*) {}
    void shutdown() {}
    DongGPUTexture createTexture(const DongGPUTextureDesc*) { return nullptr; }
    void destroyTexture(DongGPUTexture) {}
    DongGPUBuffer createBuffer(const DongGPUBufferDesc*) { return nullptr; }
    void destroyBuffer(DongGPUBuffer) {}
    DongGPUSampler createSampler(const DongGPUSamplerDesc*) { return nullptr; }
    void destroySampler(DongGPUSampler) {}
    int uploadTexture(DongGPUTexture, const void*, uint32_t, uint32_t, uint32_t, uint32_t) { return -1; }
    int uploadBuffer(DongGPUBuffer, const void*, uint32_t, uint32_t) { return -1; }
    int uploadTextureSubrect(DongGPUTexture, const void*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, void**) {
        return -1;
    }
    int queryFence(void*) { return 1; }
    void releaseFence(void*) {}
    void waitForGpu() {}
    void* nativeTextureHandle(DongGPUTexture) const { return nullptr; }
    void getCapabilities(uint32_t* out) const {
        if (out) {
            *out = 4096;
        }
    }
};
#endif

} // namespace dong::gpu_backend
