#include "scene3d_world_draw.hpp"

#include "gpu/gpu.h"
#include "gpu/core/gpu_internal.h"
#include "gpu_shader_bindings.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

extern "C" DONG_GPU_BACKEND_API void* dong_gpu_backend_create_scene3d_render_pipeline(void* device, void* shader_program,
                                                                                      uint32_t color_format,
                                                                                      uint32_t depth_format) {
    if (!device || !shader_program) {
        return nullptr;
    }

    auto* gpu_dev = static_cast<GpuDevice>(device);
    auto* program = static_cast<GpuShaderProgram>(shader_program);
    if (!program->rhiProgram) {
        return nullptr;
    }

    rhi::ColorTargetDesc target{};
    target.format = gpuFormatToRhi(static_cast<GpuFormat>(color_format));
    target.enableBlend = false;

    rhi::RenderPipelineDesc rhi_desc{};
    rhi_desc.program = program->rhiProgram;
    rhi_desc.primitiveTopology = rhi::PrimitiveTopology::TriangleList;
    rhi_desc.label = "scene3d_textured";
    rhi_desc.targets = &target;
    rhi_desc.targetCount = 1;
    rhi_desc.rasterizer.cullMode = rhi::CullMode::None;
    rhi_desc.depthStencil.format = gpuFormatToRhi(static_cast<GpuFormat>(depth_format));
    rhi_desc.depthStencil.depthTestEnable = true;
    rhi_desc.depthStencil.depthWriteEnable = true;
    rhi_desc.depthStencil.depthFunc = rhi::ComparisonFunc::Less;

    rhi::ComPtr<rhi::IRenderPipeline> rhi_pipeline;
    if (SLANG_FAILED(gpu_dev->rhiDevice->createRenderPipeline(rhi_desc, rhi_pipeline.writeRef()))) {
        return nullptr;
    }

    auto* pipeline = new GpuRenderPipeline_t();
    pipeline->rhiPipeline = rhi_pipeline;
    return pipeline;
}

extern "C" DONG_GPU_BACKEND_API void dong_gpu_backend_destroy_scene3d_render_pipeline(void* pipeline) {
    if (!pipeline) {
        return;
    }
    delete static_cast<GpuRenderPipeline_t*>(pipeline);
}

extern "C" DONG_GPU_BACKEND_API int dong_gpu_backend_scene3d_bind_textured_draw(void* device, void* pass_encoder, void* sampler_handle,
                                                         const DongGpuTextureViewHandle* texture,
                                                         const void* uniforms, size_t uniform_size) {
    if (!device || !pass_encoder || !sampler_handle || !texture || !uniforms || uniform_size == 0) {
        return 0;
    }

    GpuTextureHandle tex_view{};
    tex_view.index = texture->index;
    tex_view.generation = texture->generation;
    if (tex_view.index == 0) {
        return 0;
    }

    GpuSamplerHandle sampler{};
    std::memcpy(&sampler, sampler_handle, sizeof(sampler));
    if (sampler.index == 0) {
        return 0;
    }

    auto* gpu_dev = static_cast<GpuDevice>(device);
    auto* pass = static_cast<GpuRenderPassEncoder>(pass_encoder);

    static const bool s_debug = std::getenv("DONG_SCENE3D_DEBUG") != nullptr;
    if (!dong::gpu_backend::gpuPassSetCbuffer(pass, "uniforms", uniforms, uniform_size)) {
        if (s_debug) {
            static int n = 0;
            if (n++ == 0) std::fprintf(stderr, "[Scene3DWorldDraw] gpuPassSetCbuffer(uniforms) failed\n");
        }
        return 0;
    }
    if (!dong::gpu_backend::gpuPassBindTexture(gpu_dev, pass, "screenTexture", tex_view)) {
        if (s_debug) {
            static int n = 0;
            if (n++ == 0) std::fprintf(stderr, "[Scene3DWorldDraw] gpuPassBindTexture(screenTexture) failed\n");
        }
        return 0;
    }
    if (!dong::gpu_backend::gpuPassBindSampler(gpu_dev, pass, "screenSampler", sampler)) {
        if (s_debug) {
            static int n = 0;
            if (n++ == 0) std::fprintf(stderr, "[Scene3DWorldDraw] gpuPassBindSampler(screenSampler) failed\n");
        }
        return 0;
    }
    return 1;
}
