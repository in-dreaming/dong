#include "gpu_upload_queue.hpp"

#include "../../src/core/log.h"

namespace dong::gpu_backend {

#ifdef DONG_HAS_IN_DREAMING_GPU

void GpuUploadQueue::initialize(GpuDevice device) {
    shutdown();
    device_ = device;
}

void GpuUploadQueue::shutdown() {
    if (!device_) {
        pool_.clear();
        frame_used_.clear();
        return;
    }
    for (auto& slot : pool_) {
        if (slot.buffer.index != 0) {
            gpuDestroyBuffer(device_, slot.buffer);
        }
    }
    pool_.clear();
    frame_used_.clear();
    device_ = nullptr;
}

GpuUploadQueue::Allocation GpuUploadQueue::acquire(uint32_t size_bytes) {
    Allocation out{};
    if (!device_ || size_bytes == 0) {
        return out;
    }

    uint32_t best = UINT32_MAX;
    for (uint32_t i = 0; i < pool_.size(); ++i) {
        if (pool_[i].in_use) {
            continue;
        }
        if (pool_[i].capacity >= size_bytes &&
            (best == UINT32_MAX || pool_[i].capacity < pool_[best].capacity)) {
            best = i;
        }
    }

    if (best == UINT32_MAX) {
        uint32_t cap = size_bytes < (4u * 1024u * 1024u) ? (4u * 1024u * 1024u) : size_bytes;
        GpuBufferDesc desc{};
        desc.size = cap;
        desc.usage = GPU_BUFFER_USAGE_COPY_SOURCE;
        desc.label = "dong_gpu_upload_staging";
        GpuBufferHandle handle{};
        if (gpuCreateBuffer(device_, &desc, &handle) != GPU_SUCCESS) {
            DONG_LOG_ERROR("GpuUploadQueue: failed to create staging buffer (%u bytes)", cap);
            return out;
        }
        Slot slot{};
        slot.buffer = handle;
        slot.capacity = cap;
        pool_.push_back(slot);
        best = static_cast<uint32_t>(pool_.size() - 1);
    }

    pool_[best].in_use = true;
    frame_used_.push_back(best);
    out.buffer = pool_[best].buffer;
    out.offset = 0;
    out.size = size_bytes;
    out.pool_index = best;
    return out;
}

void GpuUploadQueue::release(const Allocation& alloc) {
    if (alloc.pool_index < pool_.size()) {
        pool_[alloc.pool_index].in_use = false;
    }
}

void GpuUploadQueue::beginFrame() {
    frame_used_.clear();
}

void GpuUploadQueue::endFrame() {
    for (uint32_t idx : frame_used_) {
        if (idx < pool_.size()) {
            pool_[idx].in_use = false;
        }
    }
    frame_used_.clear();
}

#endif

} // namespace dong::gpu_backend
