#include "gpu_resource_manager.hpp"

#include "../../src/core/log.h"

#ifdef DONG_HAS_IN_DREAMING_GPU
#include "gpu/core/gpu_internal.h"
#include "gpu/resource/gpu_readback.h"
#endif

#include <algorithm>
#include <cstring>

namespace dong::gpu_backend {

#ifdef DONG_HAS_IN_DREAMING_GPU

GpuFormat GpuResourceManager::mapTextureFormat(DongGPUTextureFormat format) {
    switch (format) {
    case DONG_GPU_TEXTURE_FORMAT_RGBA8_UNORM:
        return GPU_FORMAT_RGBA8_UNORM;
    case DONG_GPU_TEXTURE_FORMAT_BGRA8_UNORM:
        return GPU_FORMAT_BGRA8_UNORM;
    case DONG_GPU_TEXTURE_FORMAT_R8_UNORM:
        return GPU_FORMAT_R8_UNORM;
    case DONG_GPU_TEXTURE_FORMAT_RGBA32_FLOAT:
        return GPU_FORMAT_RGBA32_FLOAT;
    default:
        DONG_LOG_WARN("GpuResourceManager: unsupported texture format %d, using RGBA8", (int)format);
        return GPU_FORMAT_RGBA8_UNORM;
    }
}

uint32_t GpuResourceManager::mapTextureUsage(uint32_t usage) {
    uint32_t out = GPU_TEXTURE_USAGE_SHADER_RESOURCE;
    if (usage & DONG_GPU_TEXTURE_USAGE_COLOR_TARGET) {
        out |= GPU_TEXTURE_USAGE_RENDER_TARGET;
    }
    if (usage & DONG_GPU_TEXTURE_USAGE_DEPTH_STENCIL_TARGET) {
        out |= GPU_TEXTURE_USAGE_DEPTH_STENCIL;
    }
    if (usage & DONG_GPU_TEXTURE_USAGE_TRANSFER_SRC) {
        out |= GPU_TEXTURE_USAGE_COPY_SOURCE;
    }
    if (usage & DONG_GPU_TEXTURE_USAGE_TRANSFER_DST) {
        out |= GPU_TEXTURE_USAGE_COPY_DEST;
    }
    return out;
}

uint32_t GpuResourceManager::mapBufferUsage(uint32_t usage) {
    uint32_t out = GPU_BUFFER_USAGE_NONE;
    if (usage & DONG_GPU_BUFFER_USAGE_VERTEX) {
        out |= GPU_BUFFER_USAGE_VERTEX_BUFFER;
    }
    if (usage & DONG_GPU_BUFFER_USAGE_INDEX) {
        out |= GPU_BUFFER_USAGE_INDEX_BUFFER;
    }
    if (usage & DONG_GPU_BUFFER_USAGE_UNIFORM) {
        out |= GPU_BUFFER_USAGE_CONSTANT_BUFFER;
    }
    if (usage & DONG_GPU_BUFFER_USAGE_TRANSFER_SRC) {
        out |= GPU_BUFFER_USAGE_COPY_SOURCE;
    }
    if (usage & DONG_GPU_BUFFER_USAGE_TRANSFER_DST) {
        out |= GPU_BUFFER_USAGE_COPY_DEST;
    }
    return out;
}

GpuSamplerDesc GpuResourceManager::makeSamplerDesc(const DongGPUSamplerDesc& desc) {
    GpuSamplerDesc out{};
    out.minFilter = desc.min_filter == DONG_GPU_SAMPLER_FILTER_LINEAR ? GPU_FILTER_LINEAR : GPU_FILTER_NEAREST;
    out.magFilter = desc.mag_filter == DONG_GPU_SAMPLER_FILTER_LINEAR ? GPU_FILTER_LINEAR : GPU_FILTER_NEAREST;
    out.mipFilter = desc.mip_filter == DONG_GPU_SAMPLER_FILTER_LINEAR ? GPU_FILTER_LINEAR : GPU_FILTER_NEAREST;
    auto map_addr = [](DongGPUSamplerAddressMode mode) {
        switch (mode) {
        case DONG_GPU_SAMPLER_ADDRESS_REPEAT:
            return GPU_SAMPLER_ADDRESS_MODE_REPEAT;
        case DONG_GPU_SAMPLER_ADDRESS_CLAMP_TO_BORDER:
            return GPU_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        default:
            return GPU_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        }
    };
    out.addressModeU = map_addr(desc.address_mode_u);
    out.addressModeV = map_addr(desc.address_mode_v);
    out.addressModeW = GPU_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    out.label = "dong_gpu_sampler";
    return out;
}

void GpuResourceManager::initialize(GpuDevice device, GpuUploadQueue* upload_queue) {
    shutdown();
    device_ = device;
    upload_queue_ = upload_queue;
    if (device_) {
        gpuGetQueue(device_, GPU_QUEUE_TYPE_GRAPHICS, &queue_);
    }
}

void GpuResourceManager::shutdown() {
    waitForGpu();
    for (auto& [handle, rec] : textures_) {
        if (rec) {
            if (rec->shader_view.index != 0) {
                gpuDestroyTextureView(device_, rec->shader_view);
            }
            if (rec->rt_view.index != 0) {
                gpuDestroyTextureView(device_, rec->rt_view);
            }
            if (rec->handle.index != 0) {
                gpuDestroyTexture(device_, rec->handle);
            }
        }
    }
    for (auto& [handle, rec] : buffers_) {
        if (rec && rec->handle.index != 0) {
            gpuDestroyBuffer(device_, rec->handle);
        }
    }
    for (auto& [handle, rec] : samplers_) {
        if (rec && rec->handle.index != 0) {
            gpuDestroySampler(device_, rec->handle);
        }
    }
    for (auto& fence : fences_) {
        if (fence && fence->fence) {
            gpuDestroyFence(device_, fence->fence);
        }
    }
    textures_.clear();
    buffers_.clear();
    samplers_.clear();
    texture_storage_.clear();
    buffer_storage_.clear();
    sampler_storage_.clear();
    fences_.clear();
    device_ = nullptr;
    upload_queue_ = nullptr;
    queue_ = nullptr;
}

DongGPUTexture GpuResourceManager::createTexture(const DongGPUTextureDesc* desc) {
    if (!device_ || !desc || desc->width == 0 || desc->height == 0) {
        return nullptr;
    }

    auto rec = std::make_unique<TextureRecord>();
    rec->desc = *desc;

    GpuTextureDesc gpu_desc{};
    gpu_desc.type = GPU_TEXTURE_TYPE_2D;
    gpu_desc.width = desc->width;
    gpu_desc.height = desc->height;
    gpu_desc.depth = 1;
    gpu_desc.arrayLength = 1;
    gpu_desc.mipCount = desc->mip_levels > 0 ? desc->mip_levels : 1;
    gpu_desc.format = mapTextureFormat(desc->format);
    gpu_desc.sampleCount = 1;
    gpu_desc.usage = mapTextureUsage(desc->usage);
    gpu_desc.label = desc->debug_name ? desc->debug_name : "dong_texture";

    if (gpuCreateTexture(device_, &gpu_desc, &rec->handle) != GPU_SUCCESS) {
        DONG_LOG_ERROR("GpuResourceManager: gpuCreateTexture failed");
        return nullptr;
    }

    if (gpuCreateTextureView(device_, rec->handle, GPU_TEXTURE_VIEW_TYPE_SHADER_RESOURCE, &rec->shader_view) !=
        GPU_SUCCESS) {
        gpuDestroyTexture(device_, rec->handle);
        return nullptr;
    }

    if (desc->usage & DONG_GPU_TEXTURE_USAGE_COLOR_TARGET) {
        if (gpuCreateTextureView(device_, rec->handle, GPU_TEXTURE_VIEW_TYPE_RENDER_TARGET, &rec->rt_view) !=
            GPU_SUCCESS) {
            gpuDestroyTextureView(device_, rec->shader_view);
            gpuDestroyTexture(device_, rec->handle);
            return nullptr;
        }
    }

    TextureRecord* raw = rec.get();
    DongGPUTexture handle = static_cast<DongGPUTexture>(raw);
    texture_storage_.push_back(std::move(rec));
    textures_[handle] = raw;
    return handle;
}

void GpuResourceManager::destroyTexture(DongGPUTexture texture) {
    auto it = textures_.find(texture);
    if (it == textures_.end()) {
        return;
    }
    TextureRecord* rec = it->second;
    if (rec) {
        if (rec->shader_view.index != 0) {
            gpuDestroyTextureView(device_, rec->shader_view);
        }
        if (rec->rt_view.index != 0) {
            gpuDestroyTextureView(device_, rec->rt_view);
        }
        if (rec->handle.index != 0) {
            gpuDestroyTexture(device_, rec->handle);
        }
    }
    textures_.erase(it);
}

DongGPUBuffer GpuResourceManager::createBuffer(const DongGPUBufferDesc* desc) {
    if (!device_ || !desc || desc->size == 0) {
        return nullptr;
    }
    auto rec = std::make_unique<BufferRecord>();
    rec->desc = *desc;
    GpuBufferDesc gpu_desc{};
    gpu_desc.size = desc->size;
    gpu_desc.usage = mapBufferUsage(desc->usage);
    gpu_desc.label = desc->debug_name ? desc->debug_name : "dong_buffer";
    if (gpuCreateBuffer(device_, &gpu_desc, &rec->handle) != GPU_SUCCESS) {
        return nullptr;
    }
    BufferRecord* raw = rec.get();
    DongGPUBuffer handle = static_cast<DongGPUBuffer>(raw);
    buffer_storage_.push_back(std::move(rec));
    buffers_[handle] = raw;
    return handle;
}

void GpuResourceManager::destroyBuffer(DongGPUBuffer buffer) {
    auto it = buffers_.find(buffer);
    if (it == buffers_.end()) {
        return;
    }
    BufferRecord* rec = it->second;
    if (rec && rec->handle.index != 0) {
        gpuDestroyBuffer(device_, rec->handle);
    }
    buffers_.erase(it);
}

DongGPUSampler GpuResourceManager::createSampler(const DongGPUSamplerDesc* desc) {
    if (!device_ || !desc) {
        return nullptr;
    }
    auto rec = std::make_unique<SamplerRecord>();
    GpuSamplerDesc gpu_desc = makeSamplerDesc(*desc);
    if (gpuCreateSampler(device_, &gpu_desc, &rec->handle) != GPU_SUCCESS) {
        return nullptr;
    }
    SamplerRecord* raw = rec.get();
    DongGPUSampler handle = static_cast<DongGPUSampler>(raw);
    sampler_storage_.push_back(std::move(rec));
    samplers_[handle] = raw;
    return handle;
}

void GpuResourceManager::destroySampler(DongGPUSampler sampler) {
    auto it = samplers_.find(sampler);
    if (it == samplers_.end()) {
        return;
    }
    SamplerRecord* rec = it->second;
    if (rec && rec->handle.index != 0) {
        gpuDestroySampler(device_, rec->handle);
    }
    samplers_.erase(it);
}

int GpuResourceManager::uploadTextureSubrectInternal(TextureRecord* rec, const void* data,
                                                      uint32_t dest_x, uint32_t dest_y,
                                                      uint32_t width, uint32_t height,
                                                      uint32_t src_stride_bytes, void** out_fence) {
    if (!rec || !data || width == 0 || height == 0 || !device_ || !queue_) {
        return -1;
    }

    const uint32_t bytes_per_pixel =
        (rec->desc.format == DONG_GPU_TEXTURE_FORMAT_RGBA32_FLOAT) ? 16u : 4u;
    const uint32_t row_bytes = width * bytes_per_pixel;
    if (src_stride_bytes == 0) {
        src_stride_bytes = row_bytes;
    }
    const uint32_t total_bytes = row_bytes * height;

    GpuUploadQueue::Allocation staging{};
    if (upload_queue_) {
        staging = upload_queue_->acquire(total_bytes);
    }
    if (staging.buffer.index == 0) {
        GpuBufferDesc staging_desc{};
        staging_desc.size = total_bytes;
        staging_desc.usage = GPU_BUFFER_USAGE_COPY_SOURCE;
        staging_desc.label = "dong_upload_temp";
        if (gpuCreateBuffer(device_, &staging_desc, &staging.buffer) != GPU_SUCCESS) {
            return -1;
        }
    }

    std::vector<uint8_t> packed(total_bytes);
    const auto* src = static_cast<const uint8_t*>(data);
    for (uint32_t y = 0; y < height; ++y) {
        std::memcpy(packed.data() + y * row_bytes, src + y * src_stride_bytes, row_bytes);
    }
    if (gpuUploadToBuffer(device_, staging.buffer, packed.data(), total_bytes, staging.offset) != GPU_SUCCESS) {
        if (upload_queue_) {
            upload_queue_->release(staging);
        } else if (staging.buffer.index != 0) {
            gpuDestroyBuffer(device_, staging.buffer);
        }
        return -1;
    }

    rhi::ITexture* rhi_tex = device_->texturePool.resolve(rec->handle.index, rec->handle.generation);
    if (!rhi_tex) {
        if (upload_queue_) {
            upload_queue_->release(staging);
        }
        return -1;
    }

    rhi::ComPtr<rhi::ICommandEncoder> encoder;
    if (SLANG_FAILED(device_->graphicsQueue->createCommandEncoder(encoder.writeRef()))) {
        if (upload_queue_) {
            upload_queue_->release(staging);
        }
        return -1;
    }

    encoder->setTextureState(rhi_tex, rhi::ResourceState::CopyDestination);

    rhi::SubresourceData sub{};
    sub.data = packed.data();
    sub.rowPitch = row_bytes;
    sub.slicePitch = total_bytes;
    // SubresourceRange{} default-initializes to {0,0,0,0} (zero layers/mips),
    // which makes uploadTextureData copy nothing. Explicitly target layer 0, mip 0.
    rhi::SubresourceRange range{};
    range.layer = 0;
    range.layerCount = 1;
    range.mip = 0;
    range.mipCount = 1;
    encoder->uploadTextureData(rhi_tex, range, {dest_x, dest_y, 0}, {width, height, 1}, &sub, 1);
    encoder->setTextureState(rhi_tex, rhi::ResourceState::ShaderResource);

    rhi::ComPtr<rhi::ICommandBuffer> cmd_buf;
    encoder->finish(cmd_buf.writeRef());
    rhi::ICommandBuffer* cmds[] = {cmd_buf.get()};
    rhi::SubmitDesc submit{};
    submit.commandBuffers = cmds;
    submit.commandBufferCount = 1;
    device_->graphicsQueue->submit(submit);
    device_->graphicsQueue->waitOnHost();

    if (upload_queue_) {
        upload_queue_->release(staging);
    } else if (staging.buffer.index != 0) {
        gpuDestroyBuffer(device_, staging.buffer);
    }

    device_->textureStates[rec->handle.index] = GPU_RESOURCE_STATE_SHADER_RESOURCE;

    if (out_fence) {
        *out_fence = nullptr;
    }
    return 0;
}

int GpuResourceManager::uploadTexture(DongGPUTexture texture, const void* data, uint32_t width, uint32_t height,
                                      uint32_t x, uint32_t y) {
    auto it = textures_.find(texture);
    uint32_t bytes_per_pixel = 4;
    if (it != textures_.end() && it->second &&
        it->second->desc.format == DONG_GPU_TEXTURE_FORMAT_RGBA32_FLOAT) {
        bytes_per_pixel = 16;
    }
    return uploadTextureSubrect(texture, data, x, y, width, height, width * bytes_per_pixel, nullptr);
}

int GpuResourceManager::uploadTextureSubrect(DongGPUTexture texture, const void* data,
                                              uint32_t dest_x, uint32_t dest_y,
                                              uint32_t width, uint32_t height,
                                              uint32_t src_stride_bytes, void** out_fence) {
    auto it = textures_.find(texture);
    if (it == textures_.end()) {
        return -1;
    }
    return uploadTextureSubrectInternal(it->second, data, dest_x, dest_y, width, height, src_stride_bytes, out_fence);
}

int GpuResourceManager::uploadBuffer(DongGPUBuffer buffer, const void* data, uint32_t size, uint32_t offset) {
    auto it = buffers_.find(buffer);
    if (it == buffers_.end() || !data || size == 0) {
        return -1;
    }
    return gpuUploadToBuffer(device_, it->second->handle, data, size, offset) == GPU_SUCCESS ? 0 : -1;
}

int GpuResourceManager::queryFence(void* fence) {
    auto* rec = static_cast<GpuFenceRecord*>(fence);
    if (!rec || !rec->fence || !device_) {
        return 1;
    }
    return gpuFenceGetCurrentValue(rec->fence) >= rec->value ? 1 : 0;
}

void GpuResourceManager::releaseFence(void* fence) {
    auto* rec = static_cast<GpuFenceRecord*>(fence);
    if (!rec) {
        return;
    }
    for (auto it = fences_.begin(); it != fences_.end(); ++it) {
        if (it->get() == rec) {
            if (rec->fence) {
                gpuDestroyFence(device_, rec->fence);
            }
            fences_.erase(it);
            break;
        }
    }
}

void GpuResourceManager::waitForGpu() {
    if (queue_) {
        gpuQueueWaitOnHost(queue_);
    }
}

void* GpuResourceManager::nativeTextureHandle(DongGPUTexture texture) const {
    auto it = textures_.find(texture);
    if (it == textures_.end() || !it->second) {
        return nullptr;
    }
    return reinterpret_cast<void*>(static_cast<uintptr_t>(it->second->handle.index));
}

GpuTextureHandle GpuResourceManager::gpuTextureHandle(DongGPUTexture texture) const {
    auto it = textures_.find(texture);
    return it != textures_.end() && it->second ? it->second->handle : GpuTextureHandle{};
}

GpuTextureHandle GpuResourceManager::gpuTextureShaderView(DongGPUTexture texture) const {
    auto it = textures_.find(texture);
    return it != textures_.end() && it->second ? it->second->shader_view : GpuTextureHandle{};
}

GpuBufferHandle GpuResourceManager::gpuBufferHandle(DongGPUBuffer buffer) const {
    auto it = buffers_.find(buffer);
    return it != buffers_.end() && it->second ? it->second->handle : GpuBufferHandle{};
}

GpuSamplerHandle GpuResourceManager::gpuSamplerHandle(DongGPUSampler sampler) const {
    auto it = samplers_.find(sampler);
    return it != samplers_.end() && it->second ? it->second->handle : GpuSamplerHandle{};
}

void GpuResourceManager::getCapabilities(uint32_t* out_max_texture_size) const {
    if (out_max_texture_size) {
        *out_max_texture_size = 8192;
    }
}

int GpuResourceManager::readbackTextureRGBA(DongGPUTexture texture, uint32_t width, uint32_t height,
                                           uint8_t* out_rgba, size_t out_rgba_bytes) const {
    if (!device_ || !queue_ || !out_rgba || out_rgba_bytes < (size_t)width * height * 4) {
        return -1;
    }
    const GpuTextureHandle tex = gpuTextureHandle(texture);
    if (tex.index == 0) {
        return -1;
    }

    const uint32_t row_pitch = gpuGetReadbackRowPitch(tex, device_);
    const uint64_t buf_size = (uint64_t)row_pitch * height;
    GpuBufferHandle readback = GPU_NULL_HANDLE;
    if (gpuCreateReadbackBuffer(device_, buf_size, &readback) != GPU_SUCCESS) {
        return -1;
    }

    GpuCommandEncoder enc = gpuBeginCommandEncoder(device_, queue_);
    if (!enc) {
        return -1;
    }
    if (gpuCmdCopyTextureToBuffer(enc, tex, 0, 0, readback, 0) != GPU_SUCCESS) {
        gpuFinishCommandEncoder(enc);
        return -1;
    }
    GpuCommandBuffer cmd = gpuFinishCommandEncoder(enc);
    if (cmd) {
        gpuQueueSubmit(queue_, 1, &cmd);
    }
    gpuQueueWaitOnHost(queue_);

    void* mapped = nullptr;
    if (gpuMapReadbackBuffer(device_, readback, &mapped) != GPU_SUCCESS || !mapped) {
        return -1;
    }

    const uint8_t* src = static_cast<const uint8_t*>(mapped);
    for (uint32_t y = 0; y < height; ++y) {
        std::memcpy(out_rgba + y * width * 4, src + y * row_pitch, width * 4);
    }
    gpuUnmapReadbackBuffer(device_, readback);
    return 0;
}

#endif

} // namespace dong::gpu_backend
