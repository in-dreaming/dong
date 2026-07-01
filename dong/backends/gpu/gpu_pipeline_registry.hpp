#pragma once

#ifdef DONG_HAS_IN_DREAMING_GPU
#include "gpu/gpu.h"
#endif

namespace dong::gpu_backend {

enum class GpuPipelineKind {
    Rect,
    Image,
    Text,
    Msdf,
    Slug,
    RoundRect,
    Shadow,
    Gradient,
    ConicGradient,
    MaskApplyConic,
    NineSlice,
};

class PipelineRegistry {
public:
    bool initialize(GpuDevice device, GpuFormat color_format, const char* shader_dir,
                    GpuFormat present_color_format = GPU_FORMAT_UNDEFINED);
    void shutdown();
    bool isReady() const { return ready_; }

#ifdef DONG_HAS_IN_DREAMING_GPU
    GpuRenderPipeline pipeline(GpuPipelineKind kind) const;
    GpuRenderPipeline presentImagePipeline() const { return present_image_pipeline_; }
    GpuSamplerHandle linearSampler() const { return linear_sampler_; }
    GpuSamplerHandle nearestSampler() const { return nearest_sampler_; }
#endif

private:
#ifdef DONG_HAS_IN_DREAMING_GPU
    bool compilePipeline(GpuPipelineKind kind, const char* slang_file, const char* vs_entry,
                         const char* fs_entry, GpuFormat color_format);

    GpuDevice device_ = nullptr;
    GpuShaderCompiler compiler_ = nullptr;
    GpuShaderProgram rect_program_ = nullptr;
    GpuShaderProgram image_program_ = nullptr;
    GpuShaderProgram text_program_ = nullptr;
    GpuShaderProgram slug_program_ = nullptr;
    GpuShaderProgram round_rect_program_ = nullptr;
    GpuShaderProgram shadow_program_ = nullptr;
    GpuShaderProgram gradient_program_ = nullptr;
    GpuShaderProgram conic_program_ = nullptr;
    GpuShaderProgram nineslice_program_ = nullptr;
    GpuRenderPipeline rect_pipeline_ = nullptr;
    GpuRenderPipeline image_pipeline_ = nullptr;
    GpuRenderPipeline present_image_pipeline_ = nullptr;
    GpuRenderPipeline text_pipeline_ = nullptr;
    GpuRenderPipeline slug_pipeline_ = nullptr;
    GpuRenderPipeline round_rect_pipeline_ = nullptr;
    GpuRenderPipeline shadow_pipeline_ = nullptr;
    GpuRenderPipeline gradient_pipeline_ = nullptr;
    GpuRenderPipeline conic_pipeline_ = nullptr;
    GpuRenderPipeline mask_apply_conic_pipeline_ = nullptr;
    GpuRenderPipeline nineslice_pipeline_ = nullptr;
    GpuSamplerHandle linear_sampler_{};
    GpuSamplerHandle nearest_sampler_{};
    char shader_path_rect_[512]{};
    char shader_path_image_[512]{};
    char shader_path_text_[512]{};
    char shader_path_slug_[512]{};
    char shader_path_round_rect_[512]{};
    char shader_path_shadow_[512]{};
    char shader_path_gradient_[512]{};
    char shader_path_conic_[512]{};
    char shader_path_nineslice_[512]{};
#endif
    bool ready_ = false;
};

} // namespace dong::gpu_backend
