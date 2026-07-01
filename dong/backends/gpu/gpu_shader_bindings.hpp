#pragma once

#ifdef DONG_HAS_IN_DREAMING_GPU

#include "gpu/core/gpu_command.h"
#include "gpu/core/gpu_internal.h"
#include <slang-rhi/shader-cursor.h>

namespace dong::gpu_backend {

inline rhi::IShaderObject* gpuPassRootObject(GpuRenderPassEncoder pass) {
    if (!pass) {
        return nullptr;
    }
    return static_cast<GpuRenderPassEncoder_t*>(pass)->rootShaderObject;
}

inline bool gpuPassSetCbuffer(GpuRenderPassEncoder pass, const char* block_name, const void* data, size_t size) {
    rhi::IShaderObject* root = gpuPassRootObject(pass);
    if (!root || !block_name || !data || size == 0) {
        return false;
    }
    rhi::ShaderCursor cursor(root);
    rhi::ShaderCursor block = cursor[block_name];
    if (!block.isValid()) {
        return false;
    }
    // A global `cbuffer Foo { ... }` is reflected as a ConstantBuffer-typed
    // field whose contents live in a sub-object. Writing via the field cursor
    // directly would land in the root's inline storage and never be uploaded.
    // Dereference through the constant buffer / parameter block to obtain the
    // sub-object cursor that actually owns the uniform data.
    rhi::ShaderCursor deref = block.getDereferenced();
    rhi::ShaderCursor target = deref.isValid() ? deref : block;
    return SLANG_SUCCEEDED(target.setData(data, size));
}

inline bool gpuPassBindTexture(GpuDevice device, GpuRenderPassEncoder pass, const char* field,
                               GpuTextureHandle texture) {
    rhi::IShaderObject* root = gpuPassRootObject(pass);
    if (!root || !device || texture.index == 0) {
        return false;
    }
    rhi::ITexture* tex = device->texturePool.resolve(texture.index, texture.generation);
    if (!tex) {
        return false;
    }
    rhi::ShaderCursor cursor(root);
    rhi::ShaderCursor field_cursor = cursor[field];
    if (!field_cursor.isValid()) {
        return false;
    }
    return SLANG_SUCCEEDED(field_cursor.setBinding(tex->getDefaultView()));
}

// gpuCmdSetVertexBuffer()/gpuCmdSetViewport() (third_party/gpu) each build a fresh,
// zero-initialized rhi::RenderState and call setRenderState() with it. slang-rhi's
// RenderPassEncoder::setRenderState() is a full overwrite (`m_renderState = state;`), not a
// merge, so calling either one wipes out whatever the other previously set (vertex buffer vs.
// viewport/scissor). Any pipeline that needs BOTH a bound vertex buffer AND a working viewport in
// the same draw (e.g. Slug text) must set them together in one call, or every draw recorded after
// it in the pass would render into an empty (count=0) viewport and produce no visible output.
inline bool gpuPassSetVertexBufferWithViewport(GpuDevice device, GpuRenderPassEncoder pass, uint32_t slot,
                                               GpuBufferHandle buffer, uint64_t offset, float x, float y,
                                               float width, float height) {
    auto* encoder = static_cast<GpuRenderPassEncoder_t*>(pass);
    if (!device || !encoder || !encoder->rhiPassEncoder || buffer.index == 0) {
        return false;
    }
    rhi::IBuffer* rhi_buf = device->bufferPool.resolve(buffer.index, buffer.generation);
    if (!rhi_buf) {
        return false;
    }
    rhi::RenderState state = {};
    state.vertexBuffers[slot] = rhi::BufferOffsetPair(rhi_buf, offset);
    state.vertexBufferCount = slot + 1;
    state.viewportCount = 1;
    state.viewports[0].originX = x;
    state.viewports[0].originY = y;
    state.viewports[0].extentX = width;
    state.viewports[0].extentY = height;
    state.viewports[0].minZ = 0.0f;
    state.viewports[0].maxZ = 1.0f;
    state.scissorRectCount = 1;
    state.scissorRects[0].minX = static_cast<int32_t>(x);
    state.scissorRects[0].minY = static_cast<int32_t>(y);
    state.scissorRects[0].maxX = static_cast<int32_t>(x + width);
    state.scissorRects[0].maxY = static_cast<int32_t>(y + height);
    encoder->rhiPassEncoder->setRenderState(state);
    return true;
}

inline bool gpuPassBindSampler(GpuDevice device, GpuRenderPassEncoder pass, const char* field,
                               GpuSamplerHandle sampler) {
    rhi::IShaderObject* root = gpuPassRootObject(pass);
    if (!root || !device || sampler.index == 0) {
        return false;
    }
    rhi::ISampler* samp = device->samplerPool.resolve(sampler.index, sampler.generation);
    if (!samp) {
        return false;
    }
    rhi::ShaderCursor cursor(root);
    rhi::ShaderCursor field_cursor = cursor[field];
    if (!field_cursor.isValid()) {
        return false;
    }
    return SLANG_SUCCEEDED(field_cursor.setBinding(samp));
}

} // namespace dong::gpu_backend

#endif
