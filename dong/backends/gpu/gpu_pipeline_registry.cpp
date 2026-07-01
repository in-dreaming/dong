#include "gpu_pipeline_registry.hpp"

#include "gpu/core/gpu_internal.h"

#include "../../src/core/log.h"

#include <cstdio>
#include <cstring>

namespace dong::gpu_backend {

#ifdef DONG_HAS_IN_DREAMING_GPU

namespace {

enum class BlendPreset { None, Alpha, MaskMultiply };

rhi::ColorTargetDesc makeTarget(GpuFormat format, BlendPreset preset) {
    rhi::ColorTargetDesc target{};
    target.format = gpuFormatToRhi(format);
    if (preset == BlendPreset::Alpha) {
        target.enableBlend = true;
        target.color.srcFactor = rhi::BlendFactor::SrcAlpha;
        target.color.dstFactor = rhi::BlendFactor::InvSrcAlpha;
        target.color.op = rhi::BlendOp::Add;
        target.alpha.srcFactor = rhi::BlendFactor::One;
        target.alpha.dstFactor = rhi::BlendFactor::InvSrcAlpha;
        target.alpha.op = rhi::BlendOp::Add;
    } else if (preset == BlendPreset::MaskMultiply) {
        target.enableBlend = true;
        target.color.srcFactor = rhi::BlendFactor::Zero;
        target.color.dstFactor = rhi::BlendFactor::SrcAlpha;
        target.color.op = rhi::BlendOp::Add;
        target.alpha.srcFactor = rhi::BlendFactor::Zero;
        target.alpha.dstFactor = rhi::BlendFactor::SrcAlpha;
        target.alpha.op = rhi::BlendOp::Add;
    }
    return target;
}

GpuRenderPipeline createRhiPipeline(GpuDevice device, GpuShaderProgram program, GpuFormat color_format,
                                    BlendPreset blend, const char* label,
                                    rhi::PrimitiveTopology topology = rhi::PrimitiveTopology::TriangleStrip,
                                    rhi::IInputLayout* input_layout = nullptr) {
    if (!device || !program || !program->rhiProgram) {
        return nullptr;
    }
    rhi::ColorTargetDesc target = makeTarget(color_format, blend);
    rhi::RenderPipelineDesc desc{};
    desc.program = program->rhiProgram;
    desc.primitiveTopology = topology;
    desc.inputLayout = input_layout;
    desc.targets = &target;
    desc.targetCount = 1;
    desc.label = label;
    desc.rasterizer.cullMode = rhi::CullMode::None;

    rhi::ComPtr<rhi::IRenderPipeline> rhi_pipeline;
    if (SLANG_FAILED(device->rhiDevice->createRenderPipeline(desc, rhi_pipeline.writeRef()))) {
        return nullptr;
    }
    auto* pipeline = new GpuRenderPipeline_t();
    pipeline->rhiPipeline = rhi_pipeline;
    return pipeline;
}

rhi::ComPtr<rhi::IInputLayout> createSlugInputLayout(GpuDevice device) {
    if (!device || !device->rhiDevice) {
        return nullptr;
    }
    rhi::InputElementDesc elements[5]{};
    for (int i = 0; i < 5; ++i) {
        elements[i].semanticName = "TEXCOORD";
        elements[i].semanticIndex = static_cast<uint32_t>(i);
        elements[i].format = rhi::Format::RGBA32Float;
        elements[i].offset = static_cast<uint32_t>(i * 16);
        elements[i].bufferSlotIndex = 0;
    }
    rhi::ComPtr<rhi::IInputLayout> layout;
    if (SLANG_FAILED(device->rhiDevice->createInputLayout(80, elements, 5, layout.writeRef()))) {
        return nullptr;
    }
    return layout;
}

GpuRenderPipeline createSlugRhiPipeline(GpuDevice device, GpuShaderProgram program, GpuFormat color_format,
                                        const char* label) {
    auto input_layout = createSlugInputLayout(device);
    if (!input_layout) {
        DONG_LOG_ERROR("PipelineRegistry: Slug input layout creation failed");
        return nullptr;
    }
    auto pipeline = createRhiPipeline(device, program, color_format, BlendPreset::Alpha, label,
                                      rhi::PrimitiveTopology::TriangleList, input_layout.get());
    if (!pipeline) {
        DONG_LOG_ERROR("PipelineRegistry: Slug render pipeline creation failed");
    }
    return pipeline;
}

} // namespace

GpuRenderPipeline PipelineRegistry::pipeline(GpuPipelineKind kind) const {
    switch (kind) {
    case GpuPipelineKind::Rect: return rect_pipeline_;
    case GpuPipelineKind::Image: return image_pipeline_;
    case GpuPipelineKind::Text:
    case GpuPipelineKind::Msdf: return text_pipeline_;
    case GpuPipelineKind::Slug: return slug_pipeline_;
    case GpuPipelineKind::RoundRect: return round_rect_pipeline_;
    case GpuPipelineKind::Shadow: return shadow_pipeline_;
    case GpuPipelineKind::Gradient: return gradient_pipeline_;
    case GpuPipelineKind::ConicGradient: return conic_pipeline_;
    case GpuPipelineKind::MaskApplyConic: return mask_apply_conic_pipeline_;
    case GpuPipelineKind::NineSlice: return nineslice_pipeline_;
    }
    return nullptr;
}

bool PipelineRegistry::compilePipeline(GpuPipelineKind kind, const char* slang_file, const char* vs_entry,
                                       const char* fs_entry, GpuFormat color_format) {
    GpuShaderCompileDesc compile_desc{};
    compile_desc.sourcePath = slang_file;
    compile_desc.entryPoint = vs_entry;
    compile_desc.fragmentEntryPoint = fs_entry;
    compile_desc.target = GPU_SHADER_TARGET_SPIRV;

    GpuShaderProgram* program_slot = nullptr;
    GpuRenderPipeline* pipeline_slot = nullptr;
    BlendPreset preset = BlendPreset::Alpha;
    const char* label = "dong_gpu";

    switch (kind) {
    case GpuPipelineKind::Rect:
        program_slot = &rect_program_;
        pipeline_slot = &rect_pipeline_;
        label = "dong_rect";
        break;
    case GpuPipelineKind::Image:
        program_slot = &image_program_;
        pipeline_slot = &image_pipeline_;
        label = "dong_image";
        break;
    case GpuPipelineKind::Text:
    case GpuPipelineKind::Msdf:
        program_slot = &text_program_;
        pipeline_slot = &text_pipeline_;
        label = "dong_text";
        break;
    case GpuPipelineKind::Slug:
        program_slot = &slug_program_;
        pipeline_slot = &slug_pipeline_;
        label = "dong_slug_text";
        break;
    case GpuPipelineKind::RoundRect:
        program_slot = &round_rect_program_;
        pipeline_slot = &round_rect_pipeline_;
        label = "dong_round_rect";
        break;
    case GpuPipelineKind::Shadow:
        program_slot = &shadow_program_;
        pipeline_slot = &shadow_pipeline_;
        label = "dong_shadow";
        break;
    case GpuPipelineKind::Gradient:
        program_slot = &gradient_program_;
        pipeline_slot = &gradient_pipeline_;
        label = "dong_gradient";
        break;
    case GpuPipelineKind::ConicGradient:
        program_slot = &conic_program_;
        pipeline_slot = &conic_pipeline_;
        label = "dong_conic";
        break;
    case GpuPipelineKind::MaskApplyConic:
        program_slot = &conic_program_;
        pipeline_slot = &mask_apply_conic_pipeline_;
        preset = BlendPreset::MaskMultiply;
        label = "dong_mask_conic";
        break;
    case GpuPipelineKind::NineSlice:
        program_slot = &nineslice_program_;
        pipeline_slot = &nineslice_pipeline_;
        label = "dong_nineslice";
        break;
    }

    if (gpuCompileShader(compiler_, &compile_desc, program_slot) != GPU_SUCCESS) {
        DONG_LOG_ERROR("PipelineRegistry: compile failed for %s: %s", slang_file,
                       gpuGetShaderCompileDiagnostic(compiler_));
        return false;
    }

    *pipeline_slot = (kind == GpuPipelineKind::Slug)
                         ? createSlugRhiPipeline(device_, *program_slot, color_format, label)
                         : createRhiPipeline(device_, *program_slot, color_format, preset, label);
    if (!*pipeline_slot) {
        DONG_LOG_ERROR("PipelineRegistry: pipeline creation failed for %s", slang_file);
        return false;
    }
    return true;
}

bool PipelineRegistry::initialize(GpuDevice device, GpuFormat color_format, const char* shader_dir,
                                  GpuFormat present_color_format) {
    shutdown();
    if (!device || !shader_dir) {
        return false;
    }
    device_ = device;
    if (gpuCreateShaderCompiler(device_, &compiler_) != GPU_SUCCESS) {
        return false;
    }

    std::snprintf(shader_path_rect_, sizeof(shader_path_rect_), "%s/rect.slang", shader_dir);
    std::snprintf(shader_path_image_, sizeof(shader_path_image_), "%s/image.slang", shader_dir);
    std::snprintf(shader_path_text_, sizeof(shader_path_text_), "%s/text.slang", shader_dir);
    std::snprintf(shader_path_slug_, sizeof(shader_path_slug_), "%s/slug_text.slang", shader_dir);
    std::snprintf(shader_path_round_rect_, sizeof(shader_path_round_rect_), "%s/round_rect.slang", shader_dir);
    std::snprintf(shader_path_shadow_, sizeof(shader_path_shadow_), "%s/shadow.slang", shader_dir);
    std::snprintf(shader_path_gradient_, sizeof(shader_path_gradient_), "%s/gradient.slang", shader_dir);
    std::snprintf(shader_path_conic_, sizeof(shader_path_conic_), "%s/conic_gradient.slang", shader_dir);
    std::snprintf(shader_path_nineslice_, sizeof(shader_path_nineslice_), "%s/nineslice.slang", shader_dir);

    GpuSamplerDesc linear_desc{};
    linear_desc.minFilter = GPU_FILTER_LINEAR;
    linear_desc.magFilter = GPU_FILTER_LINEAR;
    linear_desc.mipFilter = GPU_FILTER_NEAREST;
    linear_desc.addressModeU = GPU_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    linear_desc.addressModeV = GPU_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    linear_desc.addressModeW = GPU_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    linear_desc.label = "dong_linear";
    if (gpuCreateSampler(device_, &linear_desc, &linear_sampler_) != GPU_SUCCESS) {
        shutdown();
        return false;
    }

    GpuSamplerDesc nearest_desc = linear_desc;
    nearest_desc.minFilter = GPU_FILTER_NEAREST;
    nearest_desc.magFilter = GPU_FILTER_NEAREST;
    nearest_desc.label = "dong_nearest";
    if (gpuCreateSampler(device_, &nearest_desc, &nearest_sampler_) != GPU_SUCCESS) {
        shutdown();
        return false;
    }

    if (!compilePipeline(GpuPipelineKind::Rect, shader_path_rect_, "vsMain", "fsMain", color_format) ||
        !compilePipeline(GpuPipelineKind::Image, shader_path_image_, "vsMain", "fsMain", color_format) ||
        !compilePipeline(GpuPipelineKind::Text, shader_path_text_, "vsMain", "fsMain", color_format) ||
        !compilePipeline(GpuPipelineKind::RoundRect, shader_path_round_rect_, "vsMain", "fsMain", color_format) ||
        !compilePipeline(GpuPipelineKind::Shadow, shader_path_shadow_, "vsMain", "fsMain", color_format) ||
        !compilePipeline(GpuPipelineKind::Gradient, shader_path_gradient_, "vsMain", "fsMain", color_format) ||
        !compilePipeline(GpuPipelineKind::ConicGradient, shader_path_conic_, "vsMain", "fsMain", color_format) ||
        !compilePipeline(GpuPipelineKind::MaskApplyConic, shader_path_conic_, "vsMain", "fsMain", color_format) ||
        !compilePipeline(GpuPipelineKind::NineSlice, shader_path_nineslice_, "vsMain", "fsMain", color_format)) {
        shutdown();
        return false;
    }

    if (!compilePipeline(GpuPipelineKind::Slug, shader_path_slug_, "vsMain", "fsMain", color_format)) {
        DONG_LOG_WARN("PipelineRegistry: Slug text pipeline unavailable; MSDF fallback only");
    }

    present_image_pipeline_ = nullptr;
    if (present_color_format != GPU_FORMAT_UNDEFINED) {
        if (!image_program_) {
            shutdown();
            return false;
        }
        present_image_pipeline_ =
            createRhiPipeline(device_, image_program_, present_color_format, BlendPreset::Alpha, "dong_present_blit");
        if (!present_image_pipeline_) {
            DONG_LOG_ERROR("PipelineRegistry: present blit pipeline creation failed");
            shutdown();
            return false;
        }
        DONG_LOG_INFO("PipelineRegistry: present blit pipeline format=%u", (unsigned)present_color_format);
    }

    ready_ = true;
    DONG_LOG_INFO("GpuPipelineRegistry: rect/image/text/slug/round/shadow/gradient/conic/mask/nineslice pipelines ready");
    return true;
}

void PipelineRegistry::shutdown() {
    auto destroy = [&](GpuRenderPipeline& p) {
        if (p) {
            delete p;
            p = nullptr;
        }
    };
    destroy(rect_pipeline_);
    destroy(image_pipeline_);
    destroy(present_image_pipeline_);
    destroy(text_pipeline_);
    destroy(slug_pipeline_);
    destroy(round_rect_pipeline_);
    destroy(shadow_pipeline_);
    destroy(gradient_pipeline_);
    destroy(conic_pipeline_);
    destroy(mask_apply_conic_pipeline_);
    destroy(nineslice_pipeline_);

    auto release_program = [&](GpuShaderProgram& p) {
        if (p) {
            gpuDestroyShaderProgram(p);
            p = nullptr;
        }
    };
    release_program(rect_program_);
    release_program(image_program_);
    release_program(text_program_);
    release_program(slug_program_);
    release_program(round_rect_program_);
    release_program(shadow_program_);
    release_program(gradient_program_);
    release_program(conic_program_);
    release_program(nineslice_program_);

    if (compiler_) {
        gpuDestroyShaderCompiler(compiler_);
        compiler_ = nullptr;
    }
    if (linear_sampler_.index != 0) {
        gpuDestroySampler(device_, linear_sampler_);
        linear_sampler_ = {};
    }
    if (nearest_sampler_.index != 0) {
        gpuDestroySampler(device_, nearest_sampler_);
        nearest_sampler_ = {};
    }
    device_ = nullptr;
    ready_ = false;
}

#else

bool PipelineRegistry::initialize(GpuDevice, GpuFormat, const char*, GpuFormat) {
    ready_ = true;
    return true;
}

void PipelineRegistry::shutdown() {
    ready_ = false;
}

GpuRenderPipeline PipelineRegistry::pipeline(GpuPipelineKind) const {
    return nullptr;
}

#endif

} // namespace dong::gpu_backend
