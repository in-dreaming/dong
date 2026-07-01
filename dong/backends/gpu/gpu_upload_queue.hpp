#pragma once

#include <cstdint>
#include <vector>

#ifdef DONG_HAS_IN_DREAMING_GPU
#include "gpu/gpu.h"
#endif

namespace dong::gpu_backend {

#ifdef DONG_HAS_IN_DREAMING_GPU
class GpuUploadQueue {
public:
    struct Allocation {
        GpuBufferHandle buffer{};
        uint64_t offset = 0;
        uint64_t size = 0;
        uint32_t pool_index = UINT32_MAX;
    };

    void initialize(GpuDevice device);
    void shutdown();

    Allocation acquire(uint32_t size_bytes);
    void release(const Allocation& alloc);
    void beginFrame();
    void endFrame();

    GpuDevice device() const { return device_; }

private:
    struct Slot {
        GpuBufferHandle buffer{};
        uint32_t capacity = 0;
        bool in_use = false;
    };

    GpuDevice device_ = nullptr;
    std::vector<Slot> pool_;
    std::vector<uint32_t> frame_used_;
};
#else
class GpuUploadQueue {
public:
    void initialize(void*) {}
    void shutdown() {}
    void beginFrame() {}
    void endFrame() {}
};
#endif

} // namespace dong::gpu_backend
