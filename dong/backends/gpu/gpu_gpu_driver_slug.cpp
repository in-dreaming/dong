#include "gpu_gpu_driver.hpp"
#include "gpu_gpu_driver_execute_dispatch.hpp"
#include "gpu_shader_bindings.hpp"

#include "../../src/core/log.h"
#include "../../src/render/font_resolver.hpp"
#include "../../src/render/slug/slug_vertex_builder.hpp"
#include "../../src/render/text_renderer_mode.hpp"

#include "gpu/core/gpu_command.h"

#include <algorithm>
#include <cstring>
#include <vector>

namespace dong::gpu_backend {

#ifdef DONG_HAS_IN_DREAMING_GPU

namespace {

struct SlugVertexUniformData {
    float slug_matrix[4][4];
    float slug_viewport[4];
    float clip_rects[4][4];
    float clip_radii[4];
    float clip_meta[4];
};

struct SlugFragUniformData {
    float viewport[4];
    float clip_rects[4][4];
    float clip_radii[4];
    float clip_meta[4];
    float texture_sizes[4];
};

std::vector<render::slug::SlugVertex> expandSlugMeshTriangleList(const render::slug::SlugMeshData& mesh) {
    std::vector<render::slug::SlugVertex> expanded;
    expanded.reserve(mesh.indices.size());
    for (uint16_t index : mesh.indices) {
        expanded.push_back(mesh.vertices[index]);
    }
    return expanded;
}

constexpr uint32_t kSlugVertexBufferAlignment = 16;

uint32_t alignUp(uint32_t value, uint32_t alignment) {
    return (value + alignment - 1) & ~(alignment - 1);
}

// Grows (never shrinks) the shared Slug vertex buffer to fit `needed_bytes` starting at the
// current bump offset. The previous buffer, if any, is kept alive in `retired` (destroyed only at
// driver shutdown) since draws already recorded this frame may still reference it once the render
// graph is actually submitted to the GPU.
bool ensureSlugVertexBufferCapacity(GpuGPUDriverImpl& driver, DongGPUBuffer& buffer, uint32_t& capacity,
                                    uint32_t& write_offset, uint32_t needed_bytes,
                                    std::vector<DongGPUBuffer>& retired) {
    if (buffer && write_offset + needed_bytes <= capacity) {
        return true;
    }
    uint32_t new_capacity = std::max(needed_bytes, capacity * 2u);
    new_capacity = std::max(new_capacity, 64u * 1024u);

    DongGPUBufferDesc desc{};
    desc.size = new_capacity;
    desc.usage = DONG_GPU_BUFFER_USAGE_VERTEX | DONG_GPU_BUFFER_USAGE_TRANSFER_DST;
    desc.debug_name = "slug_text_vb";
    DongGPUBuffer new_buffer = driver.resources().createBuffer(&desc);
    if (!new_buffer) {
        DONG_LOG_ERROR("GpuGPUDriver: failed to grow Slug vertex buffer to %u bytes", new_capacity);
        return false;
    }
    if (buffer) {
        // Old buffer may still be referenced by draws already recorded earlier this frame; keep
        // it alive until shutdown rather than risk a use-after-free once the graph is submitted.
        retired.push_back(buffer);
    }
    buffer = new_buffer;
    capacity = new_capacity;
    write_offset = 0;
    return true;
}

DongGPUTexture ensureSlugTexture(GpuGPUDriverImpl& driver, DongGPUTexture& texture, uint32_t& alloc_height,
                                 uint32_t width, uint32_t height, const char* debug_name) {
    if (height == 0) {
        return texture;
    }
    if (!texture || alloc_height < height) {
        if (texture) {
            driver.resources().destroyTexture(texture);
            texture = nullptr;
        }
        uint32_t new_h = std::max(height, alloc_height * 2u);
        new_h = std::max(new_h, 64u);

        DongGPUTextureDesc desc{};
        desc.width = width;
        desc.height = new_h;
        desc.format = DONG_GPU_TEXTURE_FORMAT_RGBA32_FLOAT;
        desc.usage = DONG_GPU_TEXTURE_USAGE_SAMPLER | DONG_GPU_TEXTURE_USAGE_TRANSFER_DST;
        desc.mip_levels = 1;
        desc.debug_name = debug_name;
        texture = driver.resources().createTexture(&desc);
        if (!texture) {
            DONG_LOG_ERROR("GpuGPUDriver: failed to create %s", debug_name ? debug_name : "slug texture");
            alloc_height = 0;
            return nullptr;
        }
        alloc_height = new_h;
        DONG_LOG_INFO("GpuGPUDriver: created %s %ux%u", debug_name ? debug_name : "slug texture", width, new_h);
    }
    return texture;
}

} // namespace

void GpuGPUDriverImpl::prepareSlugResources(const render::GPUCommandList& commands) {
    if (!slug_font_cache_ || !pipelines().pipeline(GpuPipelineKind::Slug)) {
        return;
    }

    for (const auto& cmd : commands.commands) {
        if (cmd.type != render::GPUCommandType::DrawText || cmd.glyphs.empty()) {
            continue;
        }

        const auto selection = text_renderer_selector_.resolve(cmd.text_renderer_mode);
        if (selection.resolved != render::TextRendererMode::Slug) {
            continue;
        }

        std::string resolved_primary;
        const std::string* font_path = nullptr;
        if (!cmd.font_paths.empty()) {
            font_path = &cmd.font_paths[0];
        } else if (!cmd.font_path.empty()) {
            font_path = &cmd.font_path;
        } else {
            resolved_primary = render::resolveFontPath(cmd.font_family, cmd.font_weight, cmd.font_style);
            font_path = &resolved_primary;
        }
        if (!font_path || font_path->empty()) {
            continue;
        }

        for (const auto& glyph : cmd.glyphs) {
            if (glyph.glyph_id == 0) {
                continue;
            }
            const std::string* glyph_font = font_path;
            if (!cmd.font_paths.empty() && glyph.font_path_index < cmd.font_paths.size()) {
                glyph_font = &cmd.font_paths[glyph.font_path_index];
            }
            slug_font_cache_->prepareGlyph(*glyph_font, glyph.glyph_id);
        }
    }

    if (slug_font_cache_->isCurveTextureDirty() || slug_font_cache_->isBandTextureDirty()) {
        uploadSlugTextures();
    }
}

void GpuGPUDriverImpl::uploadSlugTextures() {
    if (!slug_font_cache_) {
        return;
    }

    if (slug_font_cache_->isCurveTextureDirty()) {
        const auto& data = slug_font_cache_->getCurveTextureData();
        const uint32_t width = slug_font_cache_->getCurveTextureWidth();
        const uint32_t height = slug_font_cache_->getCurveTextureHeight();
        if (height > 0 && !data.empty()) {
            ensureSlugTexture(*this, slug_curve_texture_, slug_curve_texture_height_, width, height,
                              "slug_curve_texture");
            if (slug_curve_texture_) {
                resources_.uploadTexture(slug_curve_texture_, data.data(), width, height, 0, 0);
            }
        }
    }

    if (slug_font_cache_->isBandTextureDirty()) {
        const auto& data = slug_font_cache_->getBandTextureData();
        const uint32_t width = slug_font_cache_->getBandTextureWidth();
        const uint32_t height = slug_font_cache_->getBandTextureHeight();
        if (height > 0 && !data.empty()) {
            ensureSlugTexture(*this, slug_band_texture_, slug_band_texture_height_, width, height,
                              "slug_band_texture");
            if (slug_band_texture_) {
                const uint32_t texel_count = width * height;
                std::vector<float> expanded(texel_count * 4);
                for (uint32_t i = 0; i < texel_count; ++i) {
                    expanded[i * 4 + 0] = static_cast<float>(data[i].u[0]);
                    expanded[i * 4 + 1] = static_cast<float>(data[i].u[1]);
                    expanded[i * 4 + 2] = 0.0f;
                    expanded[i * 4 + 3] = 0.0f;
                }
                resources_.uploadTexture(slug_band_texture_, expanded.data(), width, height, 0, 0);
            }
        }
    }

    slug_font_cache_->clearDirtyFlags();
}

void GpuGPUDriverImpl::drawTextSlug(GpuExecuteContext& ctx, const render::GPUCommand& cmd) {
    if (!ctx.pass || cmd.glyphs.empty()) {
        return;
    }
    auto slug_pipe = pipelines().pipeline(GpuPipelineKind::Slug);
    if (!slug_pipe || !slug_font_cache_ || !slug_curve_texture_ || !slug_band_texture_) {
        drawTextMsdf(*this, ctx, cmd);
        return;
    }

    std::string resolved_primary;
    const std::string* default_font_path = nullptr;
    if (!cmd.font_paths.empty()) {
        default_font_path = &cmd.font_paths[0];
    } else if (!cmd.font_path.empty()) {
        default_font_path = &cmd.font_path;
    } else {
        resolved_primary = render::resolveFontPath(cmd.font_family, cmd.font_weight, cmd.font_style);
        default_font_path = &resolved_primary;
    }
    if (!default_font_path || default_font_path->empty()) {
        return;
    }

    const float font_size = cmd.font_size > 0.0f ? cmd.font_size : 16.0f;
    const float scale_to_px = cmd.scale_to_pixels;

    std::vector<render::slug::SlugGlyphParams> glyph_params;
    glyph_params.reserve(cmd.glyphs.size());
    bool has_unhandled_glyphs = false;

    for (const auto& glyph : cmd.glyphs) {
        if (glyph.glyph_id == 0) {
            continue;
        }

        const std::string* glyph_font = default_font_path;
        if (!cmd.font_paths.empty() && glyph.font_path_index < cmd.font_paths.size()) {
            glyph_font = &cmd.font_paths[glyph.font_path_index];
        }

        const float pen_x = glyph.pen_x_units * scale_to_px + cmd.baseline_x;
        const float pen_y = glyph.pen_y_units * scale_to_px + cmd.baseline_y;

        const auto* prepared = slug_font_cache_->getGlyph(*glyph_font, glyph.glyph_id);
        if (prepared) {
            render::slug::SlugGlyphParams p{};
            p.prepared = prepared;
            p.pos_x = pen_x;
            p.pos_y = pen_y;
            p.scale = font_size;
            p.color_r = cmd.color.r;
            p.color_g = cmd.color.g;
            p.color_b = cmd.color.b;
            p.color_a = cmd.color.a;
            glyph_params.push_back(p);
        } else {
            has_unhandled_glyphs = true;
        }
    }

    if (glyph_params.empty()) {
        if (has_unhandled_glyphs) {
            drawTextMsdf(*this, ctx, cmd);
        }
        return;
    }

    auto mesh = render::slug::buildSlugMesh(glyph_params.data(), static_cast<uint32_t>(glyph_params.size()));
    if (mesh.vertices.empty() || mesh.indices.empty()) {
        if (has_unhandled_glyphs) {
            drawTextMsdf(*this, ctx, cmd);
        }
        return;
    }

    auto expanded = expandSlugMeshTriangleList(mesh);
    const uint32_t vb_size = static_cast<uint32_t>(expanded.size() * sizeof(render::slug::SlugVertex));

    if (!ensureSlugVertexBufferCapacity(*this, slug_vertex_buffer_, slug_vertex_buffer_capacity_,
                                        slug_vertex_write_offset_, vb_size, retired_slug_vertex_buffers_)) {
        if (has_unhandled_glyphs) {
            drawTextMsdf(*this, ctx, cmd);
        }
        return;
    }
    const uint32_t vb_offset = slug_vertex_write_offset_;

    if (resources().uploadBuffer(slug_vertex_buffer_, expanded.data(), vb_size, vb_offset) != 0) {
        if (has_unhandled_glyphs) {
            drawTextMsdf(*this, ctx, cmd);
        }
        return;
    }
    slug_vertex_write_offset_ = alignUp(vb_offset + vb_size, kSlugVertexBufferAlignment);

    const float* transform = ctx.currentTransform();
    const float vw = static_cast<float>(ctx.viewport_w);
    const float vh = static_cast<float>(ctx.viewport_h);

    SlugVertexUniformData vu{};
    const float a = transform[0], b = transform[1], tx = transform[2];
    const float c = transform[3], d = transform[4], ty = transform[5];

    vu.slug_matrix[0][0] = 2.0f * a / vw;
    vu.slug_matrix[0][1] = 2.0f * b / vw;
    vu.slug_matrix[0][2] = 0.0f;
    vu.slug_matrix[0][3] = 2.0f * tx / vw - 1.0f;

    vu.slug_matrix[1][0] = -2.0f * c / vh;
    vu.slug_matrix[1][1] = -2.0f * d / vh;
    vu.slug_matrix[1][2] = 0.0f;
    vu.slug_matrix[1][3] = 1.0f - 2.0f * ty / vh;

    vu.slug_matrix[2][0] = 0.0f;
    vu.slug_matrix[2][1] = 0.0f;
    vu.slug_matrix[2][2] = 1.0f;
    vu.slug_matrix[2][3] = 0.0f;

    vu.slug_matrix[3][0] = 0.0f;
    vu.slug_matrix[3][1] = 0.0f;
    vu.slug_matrix[3][2] = 0.0f;
    vu.slug_matrix[3][3] = 1.0f;

    vu.slug_viewport[0] = vw;
    vu.slug_viewport[1] = vh;
    vu.slug_viewport[2] = 0.0f;
    vu.slug_viewport[3] = 0.0f;

    ctx.fillClipUniform(vu.clip_rects, vu.clip_radii, vu.clip_meta);

    SlugFragUniformData fu{};
    fu.viewport[0] = vw;
    fu.viewport[1] = vh;
    std::memcpy(fu.clip_rects, vu.clip_rects, sizeof(fu.clip_rects));
    std::memcpy(fu.clip_radii, vu.clip_radii, sizeof(fu.clip_radii));
    std::memcpy(fu.clip_meta, vu.clip_meta, sizeof(fu.clip_meta));
    fu.texture_sizes[0] = static_cast<float>(slug_font_cache_->getCurveTextureWidth());
    fu.texture_sizes[1] = static_cast<float>(slug_curve_texture_height_);
    fu.texture_sizes[2] = static_cast<float>(slug_font_cache_->getBandTextureWidth());
    fu.texture_sizes[3] = static_cast<float>(slug_band_texture_height_);

    gpuCmdBindRenderPipeline(ctx.pass, slug_pipe);
    gpuPassSetCbuffer(ctx.pass, "SlugVertexUniforms", &vu, sizeof(vu));
    gpuPassSetCbuffer(ctx.pass, "SlugFragUniforms", &fu, sizeof(fu));
    gpuPassBindTexture(ctx.device, ctx.pass, "curveTexture", resources().gpuTextureHandle(slug_curve_texture_));
    gpuPassBindTexture(ctx.device, ctx.pass, "bandTexture", resources().gpuTextureHandle(slug_band_texture_));
    gpuPassBindSampler(ctx.device, ctx.pass, "curveSampler", pipelines().nearestSampler());
    gpuPassBindSampler(ctx.device, ctx.pass, "bandSampler", pipelines().nearestSampler());
    // Plain gpuCmdSetVertexBuffer() would clobber the pass's viewport/scissor (see helper doc
    // comment in gpu_shader_bindings.hpp) and blank out every draw recorded after this one.
    gpuPassSetVertexBufferWithViewport(ctx.device, ctx.pass, 0, resources().gpuBufferHandle(slug_vertex_buffer_),
                                       vb_offset, 0.0f, 0.0f, vw, vh);
    gpuCmdDraw(ctx.pass, static_cast<uint32_t>(expanded.size()), 1, 0, 0);

    if (has_unhandled_glyphs) {
        drawTextMsdf(*this, ctx, cmd);
    }
}

#endif

} // namespace dong::gpu_backend
