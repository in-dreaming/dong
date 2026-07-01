#include "gpu_gpu_driver_execute_dispatch.hpp"
#include "gpu_gpu_driver.hpp"
#include "gpu_shader_bindings.hpp"

#include "../../src/core/log.h"
#include "../../src/render/font_resolver.hpp"
#include "../../src/render/glyph_atlas.hpp"
#include "../../src/render/text_renderer_mode.hpp"

#include "dong_image_atlas.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace dong::gpu_backend {

#ifdef DONG_HAS_IN_DREAMING_GPU

namespace {

constexpr int kMaxRoundedClipUniforms = 4;

void writeLinearColor(const dong::render::Color& color, float out_rgba[4]) {
    out_rgba[0] = color.r;
    out_rgba[1] = color.g;
    out_rgba[2] = color.b;
    out_rgba[3] = color.a;
}

void writeTransform(float (&out)[8], const float transform[6]) {
    out[0] = transform[0];
    out[1] = transform[1];
    out[2] = transform[2];
    out[3] = 0.0f;
    out[4] = transform[3];
    out[5] = transform[4];
    out[6] = transform[5];
    out[7] = 0.0f;
}

bool isIdentity2D(const float t[6]) {
    return t[0] == 1.0f && t[1] == 0.0f && t[2] == 0.0f && t[3] == 0.0f && t[4] == 1.0f && t[5] == 0.0f;
}

void transformPoint2D(const float t[6], float x, float y, float& ox, float& oy) {
    ox = t[0] * x + t[1] * y + t[2];
    oy = t[3] * x + t[4] * y + t[5];
}

struct RectUniformData {
    float rect[4];
    float color[4];
    float viewport[4];
    float transform[8];
    float clip_rects[4][4];
    float clip_radii[4];
    float clip_meta[4];
};

struct ImageUniformData {
    float rect[4];
    float uv_rect[4];
    float viewport[4];
    float transform[8];
    float tint[4];
    float clip_rects[4][4];
    float clip_radii[4];
    float clip_meta[4];
};

struct NineSliceUniformData {
    ImageUniformData base;
    float nine_slice[4];
    float nine_width[4];
};

struct RoundRectUniformData {
    float rect[4];
    float radius[4];
    float viewport[4];
    float transform[8];
    float color[4];
    float clip_rects[4][4];
    float clip_radii[4];
    float clip_meta[4];
};

struct ShadowUniformData {
    float rect[4];
    float radius[4];
    float viewport[4];
    float transform[8];
    float color[4];
    float clip_rects[4][4];
    float clip_radii[4];
    float clip_meta[4];
};

struct GradientUniformData {
    float rect[4];
    float viewport[4];
    float transform[8];
    float gradient_params[4];
    float stop_colors[8][4];
    float stop_positions[2][4];
    float clip_rects[4][4];
    float clip_radii[4];
    float clip_meta[4];
};

struct ConicGradientUniformData {
    float rect[4];
    float viewport[4];
    float transform[8];
    float conic_meta[4];
    float conic_center_period[4];
    float stop_colors[8][4];
    float stop_positions[2][4];
    float clip_rects[4][4];
    float clip_radii[4];
    float clip_meta[4];
};

struct GlyphInstanceUniform {
    float rect[4];
    float uv_rect[4];
    float color[4];
    float params[4];
};

struct TextBatchUniformData {
    float viewport[4];
    float transform[8];
    float clip_rects[4][4];
    float clip_radii[4];
    float clip_meta[4];
    float glyphs[244][4];
};

} // namespace

void GpuExecuteContext::writeViewport(float out[4]) const {
    out[0] = static_cast<float>(viewport_w);
    out[1] = static_cast<float>(viewport_h);
    out[2] = 0.0f;
    out[3] = 0.0f;
}

const float* GpuExecuteContext::currentTransform() const {
    static const float identity[6] = {1, 0, 0, 0, 1, 0};
    return transform;
}

void GpuExecuteContext::fillClipUniform(float clip_rects[4][4], float clip_radii[4], float clip_meta[4]) const {
    for (int i = 0; i < kMaxRoundedClipUniforms; ++i) {
        std::memset(clip_rects[i], 0, sizeof(clip_rects[i]));
        clip_radii[i] = 0.0f;
    }
    clip_meta[0] = 0.0f;
    clip_meta[1] = clip_meta[2] = clip_meta[3] = 0.0f;

    int clip_index = 0;
    for (const auto& entry : clip_stack) {
        if (!entry.has_rounded) {
            continue;
        }
        const float x0 = entry.rounded_rect.x;
        const float y0 = entry.rounded_rect.y;
        const float x1 = entry.rounded_rect.x + entry.rounded_rect.width;
        const float y1 = entry.rounded_rect.y + entry.rounded_rect.height;
        clip_rects[clip_index][0] = x0;
        clip_rects[clip_index][1] = y0;
        clip_rects[clip_index][2] = x1;
        clip_rects[clip_index][3] = y1;
        clip_radii[clip_index] = entry.rounded_radius;
        ++clip_index;
        if (clip_index >= kMaxRoundedClipUniforms) {
            break;
        }
    }
    clip_meta[0] = static_cast<float>(clip_index);
}

static float effectiveAlpha(const GpuExecuteContext& ctx, float a) {
    return a * ctx.layer_opacity;
}

static void drawRect(GpuGPUDriverImpl& driver, GpuExecuteContext& ctx, const dong::render::GPUCommand& cmd) {
    if (!ctx.pass || ctx.skip_draw_depth > 0) {
        return;
    }
    auto pipe = driver.pipelines().pipeline(GpuPipelineKind::Rect);
    if (!pipe) {
        return;
    }

    RectUniformData u{};
    u.rect[0] = cmd.rect.x;
    u.rect[1] = cmd.rect.y;
    u.rect[2] = cmd.rect.width;
    u.rect[3] = cmd.rect.height;
    writeLinearColor(cmd.color, u.color);
    u.color[3] = effectiveAlpha(ctx, u.color[3]);
    ctx.writeViewport(u.viewport);
    writeTransform(u.transform, ctx.currentTransform());
    ctx.fillClipUniform(u.clip_rects, u.clip_radii, u.clip_meta);

    gpuCmdBindRenderPipeline(ctx.pass, pipe);
    gpuPassSetCbuffer(ctx.pass, "RectUniforms", &u, sizeof(u));
    gpuCmdDraw(ctx.pass, 4, 1, 0, 0);
}

static void drawRoundedRect(GpuGPUDriverImpl& driver, GpuExecuteContext& ctx, const dong::render::GPUCommand& cmd) {
    if (!ctx.pass || ctx.skip_draw_depth > 0) {
        return;
    }
    auto pipe = driver.pipelines().pipeline(GpuPipelineKind::RoundRect);
    if (!pipe) {
        return;
    }

    RoundRectUniformData u{};
    u.rect[0] = cmd.rect.x;
    u.rect[1] = cmd.rect.y;
    u.rect[2] = cmd.rect.width;
    u.rect[3] = cmd.rect.height;
    u.radius[0] = cmd.radius;
    u.radius[1] = cmd.stroke_width;
    u.radius[2] = 0.0f;
    u.radius[3] = 0.0f;
    ctx.writeViewport(u.viewport);
    writeTransform(u.transform, ctx.currentTransform());
    writeLinearColor(cmd.color, u.color);
    u.color[3] = effectiveAlpha(ctx, u.color[3]);
    ctx.fillClipUniform(u.clip_rects, u.clip_radii, u.clip_meta);

    gpuCmdBindRenderPipeline(ctx.pass, pipe);
    gpuPassSetCbuffer(ctx.pass, "RoundRectUniforms", &u, sizeof(u));
    gpuCmdDraw(ctx.pass, 4, 1, 0, 0);
}

static void drawShadow(GpuGPUDriverImpl& driver, GpuExecuteContext& ctx, const dong::render::GPUCommand& cmd) {
    if (!ctx.pass || ctx.skip_draw_depth > 0) {
        return;
    }
    auto pipe = driver.pipelines().pipeline(GpuPipelineKind::Shadow);
    if (!pipe) {
        return;
    }

    ShadowUniformData u{};
    u.rect[0] = cmd.rect.x;
    u.rect[1] = cmd.rect.y;
    u.rect[2] = cmd.rect.width;
    u.rect[3] = cmd.rect.height;
    u.radius[0] = cmd.radius;
    u.radius[1] = cmd.blur;
    u.radius[2] = 0.0f;
    u.radius[3] = 0.0f;
    ctx.writeViewport(u.viewport);
    writeTransform(u.transform, ctx.currentTransform());
    writeLinearColor(cmd.color, u.color);
    u.color[3] = effectiveAlpha(ctx, u.color[3]);
    ctx.fillClipUniform(u.clip_rects, u.clip_radii, u.clip_meta);

    gpuCmdBindRenderPipeline(ctx.pass, pipe);
    gpuPassSetCbuffer(ctx.pass, "ShadowUniforms", &u, sizeof(u));
    gpuCmdDraw(ctx.pass, 4, 1, 0, 0);
}

static void writeGradientStops(const dong::render::GPUCommand& cmd, float stop_colors[8][4],
                                float stop_positions[2][4]) {
    for (int i = 0; i < cmd.gradient_stop_count && i < 8; ++i) {
        writeLinearColor(cmd.gradient_stops[i].color, stop_colors[i]);
        const int vec_idx = i / 4;
        const int comp_idx = i % 4;
        stop_positions[vec_idx][comp_idx] = cmd.gradient_stops[i].position;
    }
}

static void drawGradient(GpuGPUDriverImpl& driver, GpuExecuteContext& ctx, const dong::render::GPUCommand& cmd) {
    if (!ctx.pass || ctx.skip_draw_depth > 0) {
        return;
    }
    auto pipe = driver.pipelines().pipeline(GpuPipelineKind::Gradient);
    if (!pipe) {
        return;
    }

    const float* current_transform = ctx.currentTransform();
    const bool fold = !isIdentity2D(current_transform);

    GradientUniformData u{};
    float rect_x = cmd.rect.x;
    float rect_y = cmd.rect.y;
    if (fold) {
        transformPoint2D(current_transform, rect_x, rect_y, rect_x, rect_y);
    }
    u.rect[0] = rect_x;
    u.rect[1] = rect_y;
    u.rect[2] = cmd.rect.width;
    u.rect[3] = cmd.rect.height;

    ctx.writeViewport(u.viewport);
    if (fold) {
        static const float identity[6] = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
        writeTransform(u.transform, identity);
    } else {
        writeTransform(u.transform, current_transform);
    }

    u.gradient_params[0] = cmd.gradient_angle_deg * 3.14159265358979f / 180.0f;
    u.gradient_params[1] = static_cast<float>(cmd.gradient_stop_count);
    u.gradient_params[2] = cmd.radius;
    u.gradient_params[3] = 0.0f;
    writeGradientStops(cmd, u.stop_colors, u.stop_positions);
    ctx.fillClipUniform(u.clip_rects, u.clip_radii, u.clip_meta);

    gpuCmdBindRenderPipeline(ctx.pass, pipe);
    gpuPassSetCbuffer(ctx.pass, "GradientUniforms", &u, sizeof(u));
    gpuCmdDraw(ctx.pass, 4, 1, 0, 0);
}

static void drawConicGradient(GpuGPUDriverImpl& driver, GpuExecuteContext& ctx, const dong::render::GPUCommand& cmd,
                              bool mask_apply) {
    if (!ctx.pass || ctx.skip_draw_depth > 0) {
        return;
    }
    auto pipe = driver.pipelines().pipeline(mask_apply ? GpuPipelineKind::MaskApplyConic
                                                        : GpuPipelineKind::ConicGradient);
    if (!pipe) {
        return;
    }

    const float* current_transform = ctx.currentTransform();
    const bool fold = !isIdentity2D(current_transform);

    ConicGradientUniformData u{};
    float rect_x = cmd.rect.x;
    float rect_y = cmd.rect.y;
    if (fold) {
        transformPoint2D(current_transform, rect_x, rect_y, rect_x, rect_y);
    }
    u.rect[0] = rect_x;
    u.rect[1] = rect_y;
    u.rect[2] = cmd.rect.width;
    u.rect[3] = cmd.rect.height;

    ctx.writeViewport(u.viewport);
    if (fold) {
        static const float identity[6] = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
        writeTransform(u.transform, identity);
    } else {
        writeTransform(u.transform, current_transform);
    }

    u.conic_meta[0] = cmd.conic_from_angle_deg * 3.14159265358979f / 180.0f;
    u.conic_meta[1] = static_cast<float>(cmd.gradient_stop_count);
    u.conic_meta[2] = cmd.radius;
    u.conic_meta[3] = cmd.conic_repeating;

    float center_x = cmd.conic_center_x_px;
    float center_y = cmd.conic_center_y_px;
    if (fold) {
        transformPoint2D(current_transform, center_x, center_y, center_x, center_y);
    }
    u.conic_center_period[0] = center_x;
    u.conic_center_period[1] = center_y;
    u.conic_center_period[2] = 0.0f;
    u.conic_center_period[3] = 0.0f;
    writeGradientStops(cmd, u.stop_colors, u.stop_positions);
    ctx.fillClipUniform(u.clip_rects, u.clip_radii, u.clip_meta);

    gpuCmdBindRenderPipeline(ctx.pass, pipe);
    gpuPassSetCbuffer(ctx.pass, "ConicGradientUniforms", &u, sizeof(u));
    gpuCmdDraw(ctx.pass, 4, 1, 0, 0);
}

static void drawImage(GpuGPUDriverImpl& driver, GpuExecuteContext& ctx, const dong::render::GPUCommand& cmd) {
    if (!ctx.pass || ctx.skip_draw_depth > 0) {
        return;
    }
    auto pipe = driver.pipelines().pipeline(GpuPipelineKind::Image);
    if (!pipe) {
        return;
    }

    ImageAtlasEntryGpu entry{};
    DongGPUTexture src_texture = nullptr;
    if (cmd.image_src.rfind("video://", 0) == 0) {
        auto* ext = driver.findExternalImage(cmd.image_src);
        if (!ext) {
            return;
        }
        src_texture = ext->texture;
        entry.width = ext->width;
        entry.height = ext->height;
        entry.entry.u0 = 0;
        entry.entry.v0 = 0;
        entry.entry.u1 = 1;
        entry.entry.v1 = 1;
    } else {
        if (!driver.ensureImageInAtlas(cmd.image_src, entry)) {
            return;
        }
        src_texture = static_cast<DongGPUTexture>(dong_atlas_get_texture(driver.imageAtlas(), entry.entry.atlas_page));
    }
    if (!src_texture) {
        return;
    }

    ImageUniformData u{};
    float draw_x = cmd.rect.x;
    float draw_y = cmd.rect.y;
    float draw_w = cmd.rect.width;
    float draw_h = cmd.rect.height;
    const float img_w = static_cast<float>(entry.width);
    const float img_h = static_cast<float>(entry.height);
    if (img_w > 0 && img_h > 0 && draw_w > 0 && draw_h > 0) {
        const float scale_x = draw_w / img_w;
        const float scale_y = draw_h / img_h;
        if (cmd.image_fit == dong::render::ImageFitMode::Contain) {
            const float scale = std::min(scale_x, scale_y);
            draw_w = img_w * scale;
            draw_h = img_h * scale;
            draw_x = cmd.rect.x + (cmd.rect.width - draw_w) * cmd.image_position_x;
            draw_y = cmd.rect.y + (cmd.rect.height - draw_h) * cmd.image_position_y;
        } else if (cmd.image_fit == dong::render::ImageFitMode::Cover) {
            const float scale = std::max(scale_x, scale_y);
            draw_w = img_w * scale;
            draw_h = img_h * scale;
            draw_x = cmd.rect.x + (cmd.rect.width - draw_w) * cmd.image_position_x;
            draw_y = cmd.rect.y + (cmd.rect.height - draw_h) * cmd.image_position_y;
        }
    }

    u.rect[0] = draw_x;
    u.rect[1] = draw_y;
    u.rect[2] = draw_w;
    u.rect[3] = draw_h;
    u.uv_rect[0] = entry.entry.u0;
    u.uv_rect[1] = entry.entry.v0;
    u.uv_rect[2] = entry.entry.u1;
    u.uv_rect[3] = entry.entry.v1;
    ctx.writeViewport(u.viewport);
    writeTransform(u.transform, ctx.currentTransform());
    u.tint[0] = cmd.color.r;
    u.tint[1] = cmd.color.g;
    u.tint[2] = cmd.color.b;
    u.tint[3] = effectiveAlpha(ctx, cmd.opacity * cmd.color.a);
    ctx.fillClipUniform(u.clip_rects, u.clip_radii, u.clip_meta);

    gpuCmdBindRenderPipeline(ctx.pass, pipe);
    gpuPassSetCbuffer(ctx.pass, "ImageUniforms", &u, sizeof(u));
    auto tex_handle = driver.resources().gpuTextureHandle(src_texture);
    auto sampler = cmd.image_sampling == dong::render::ImageSampling::Nearest
        ? driver.pipelines().nearestSampler()
        : driver.pipelines().linearSampler();
    gpuPassBindTexture(ctx.device, ctx.pass, "imageTexture", tex_handle);
    gpuPassBindSampler(ctx.device, ctx.pass, "imageSampler", sampler);
    gpuCmdDraw(ctx.pass, 4, 1, 0, 0);
}

static void drawNineSlice(GpuGPUDriverImpl& driver, GpuExecuteContext& ctx, const dong::render::GPUCommand& cmd) {
    if (!ctx.pass || ctx.skip_draw_depth > 0) {
        return;
    }
    auto pipe = driver.pipelines().pipeline(GpuPipelineKind::NineSlice);
    if (!pipe) {
        return;
    }

    ImageAtlasEntryGpu entry{};
    DongGPUTexture src_texture = nullptr;
    if (cmd.image_src.rfind("video://", 0) == 0) {
        auto* ext = driver.findExternalImage(cmd.image_src);
        if (!ext) {
            return;
        }
        src_texture = ext->texture;
        entry.width = ext->width;
        entry.height = ext->height;
        entry.entry.u0 = 0;
        entry.entry.v0 = 0;
        entry.entry.u1 = 1;
        entry.entry.v1 = 1;
    } else {
        if (!driver.ensureImageInAtlas(cmd.image_src, entry)) {
            return;
        }
        src_texture = static_cast<DongGPUTexture>(dong_atlas_get_texture(driver.imageAtlas(), entry.entry.atlas_page));
    }
    if (!src_texture) {
        return;
    }

    NineSliceUniformData u{};
    u.base.rect[0] = cmd.rect.x;
    u.base.rect[1] = cmd.rect.y;
    u.base.rect[2] = cmd.rect.width;
    u.base.rect[3] = cmd.rect.height;
    u.base.uv_rect[0] = entry.entry.u0;
    u.base.uv_rect[1] = entry.entry.v0;
    u.base.uv_rect[2] = entry.entry.u1;
    u.base.uv_rect[3] = entry.entry.v1;
    ctx.writeViewport(u.base.viewport);
    writeTransform(u.base.transform, ctx.currentTransform());
    u.base.tint[0] = cmd.color.r;
    u.base.tint[1] = cmd.color.g;
    u.base.tint[2] = cmd.color.b;
    u.base.tint[3] = effectiveAlpha(ctx, cmd.opacity * cmd.color.a);
    ctx.fillClipUniform(u.base.clip_rects, u.base.clip_radii, u.base.clip_meta);

    u.nine_slice[0] = cmd.nine_slice_top;
    u.nine_slice[1] = cmd.nine_slice_right;
    u.nine_slice[2] = cmd.nine_slice_bottom;
    u.nine_slice[3] = cmd.nine_slice_left;
    u.nine_width[0] = cmd.nine_width_top;
    u.nine_width[1] = cmd.nine_width_right;
    u.nine_width[2] = cmd.nine_width_bottom;
    u.nine_width[3] = cmd.nine_width_left;

    gpuCmdBindRenderPipeline(ctx.pass, pipe);
    gpuPassSetCbuffer(ctx.pass, "ImageUniforms", &u, sizeof(u));
    auto tex_handle = driver.resources().gpuTextureHandle(src_texture);
    gpuPassBindTexture(ctx.device, ctx.pass, "imageTexture", tex_handle);
    gpuPassBindSampler(ctx.device, ctx.pass, "imageSampler", driver.pipelines().linearSampler());
    gpuCmdDraw(ctx.pass, 4, 1, 0, 0);
}

static void drawText(GpuGPUDriverImpl& driver, GpuExecuteContext& ctx, const dong::render::GPUCommand& cmd) {
    if (!ctx.pass || ctx.skip_draw_depth > 0 || cmd.glyphs.empty()) {
        return;
    }

    const auto selection = driver.textRendererSelector().resolve(cmd.text_renderer_mode);
    if (selection.resolved == dong::render::TextRendererMode::Slug &&
        driver.pipelines().pipeline(GpuPipelineKind::Slug)) {
        driver.drawTextSlug(ctx, cmd);
    } else {
        drawTextMsdf(driver, ctx, cmd);
    }

    if (selection.fallback_used) {
        DONG_LOG_DEBUG("[DrawText] Fallback from %s to %s: %s",
                       dong::render::textRendererModeToString(selection.requested),
                       dong::render::textRendererModeToString(selection.resolved),
                       selection.reason ? selection.reason : "unknown");
    }
}

void drawTextMsdf(GpuGPUDriverImpl& driver, GpuExecuteContext& ctx, const dong::render::GPUCommand& cmd) {
    if (!ctx.pass || ctx.skip_draw_depth > 0 || cmd.glyphs.empty()) {
        return;
    }
    auto pipe = driver.pipelines().pipeline(GpuPipelineKind::Text);
    if (!pipe) {
        return;
    }

    const float font_size = cmd.font_size > 0 ? cmd.font_size : 16.0f;
    render::GlyphAtlas* glyph_atlas = driver.glyphAtlasForFontSize(font_size);
    if (!glyph_atlas) {
        return;
    }

    std::string default_font = cmd.font_path;
    if (default_font.empty() && !cmd.font_paths.empty()) {
        default_font = cmd.font_paths[0];
    }
    if (default_font.empty()) {
        default_font = render::resolveFontPath(cmd.font_family, cmd.font_weight, cmd.font_style);
    }
    if (default_font.empty()) {
        return;
    }

    constexpr int kMaxGlyphsPerBatch = 61;
    TextBatchUniformData batch{};
    ctx.writeViewport(batch.viewport);
    writeTransform(batch.transform, ctx.currentTransform());
    ctx.fillClipUniform(batch.clip_rects, batch.clip_radii, batch.clip_meta);

    struct PreparedGlyph {
        GlyphInstanceUniform inst{};
        uint32_t atlas_page = 0;
    };
    std::vector<PreparedGlyph> prepared;
    prepared.reserve(cmd.glyphs.size());

    const float pixel_scale = cmd.scale_to_pixels;
    const float atlas_range = glyph_atlas->getGlyphDistanceRange();

    for (const auto& glyph : cmd.glyphs) {
        if (glyph.glyph_id == 0) {
            continue;
        }
        std::string glyph_font = default_font;
        if (!cmd.font_paths.empty() && glyph.font_path_index < cmd.font_paths.size()) {
            glyph_font = cmd.font_paths[glyph.font_path_index];
        }

        // Try color bitmap FIRST for color-capable fonts (COLR v0/v1, CBDT, sbix).
        // Color fonts (e.g. Segoe UI Emoji) still carry a monochrome fallback outline
        // in the glyf table, so addGlyph() would succeed but render black/white - we
        // must try color first (mirrors the SDL backend's drawText()).
        const render::AtlasEntry* entry = glyph_atlas->addColorGlyph(glyph.glyph_id, glyph_font);
        if (!entry) {
            entry = glyph_atlas->addGlyph(glyph.glyph_id, glyph_font);
        }
        if (!entry || entry->metrics.width_units <= 0 || entry->metrics.height_units <= 0) {
            continue;
        }

        float glyph_pixel_scale = pixel_scale;
        if (glyph.units_per_em > 0 && glyph.units_per_em != cmd.units_per_em) {
            glyph_pixel_scale = font_size / static_cast<float>(glyph.units_per_em);
        }
        const float msdf_scale = entry->metrics.msdf_scale > 0 ? entry->metrics.msdf_scale : 1.0f;
        const float msdf_size = static_cast<float>(glyph_atlas->getGlyphBitmapSize());
        const float pen_x_px = glyph.pen_x_units * cmd.scale_to_pixels + cmd.baseline_x;
        const float pen_y_px = glyph.pen_y_units * cmd.scale_to_pixels + cmd.baseline_y;

        float glyph_w, glyph_h, glyph_x, glyph_y, px_range_screen, unit_range;
        if (entry->is_color_bitmap) {
            // Pre-rendered color glyph (COLR/CBDT/sbix emoji, etc.): bearing-based
            // positioning, no MSDF decode - see text.slang's screenPxRange<=0 branch.
            glyph_w = entry->metrics.width_units * glyph_pixel_scale;
            glyph_h = entry->metrics.height_units * glyph_pixel_scale;
            glyph_x = pen_x_px + entry->metrics.bearing_x_units * glyph_pixel_scale;
            glyph_y = pen_y_px - entry->metrics.bearing_y_units * glyph_pixel_scale;
            px_range_screen = 0.0f;
            unit_range = 0.0f;
        } else {
            const float render_scale = glyph_pixel_scale / msdf_scale;
            glyph_w = msdf_size * render_scale;
            glyph_h = msdf_size * render_scale;
            glyph_x = pen_x_px - entry->metrics.msdf_translate_x * glyph_pixel_scale;
            glyph_y = pen_y_px - msdf_size * render_scale + entry->metrics.msdf_translate_y * glyph_pixel_scale;
            px_range_screen = atlas_range * (glyph_pixel_scale / msdf_scale);
            unit_range = atlas_range / msdf_size;
        }
        if (glyph_w <= 0 || glyph_h <= 0) {
            continue;
        }

        GlyphInstanceUniform inst{};
        inst.rect[0] = glyph_x;
        inst.rect[1] = glyph_y;
        inst.rect[2] = glyph_w;
        inst.rect[3] = glyph_h;
        inst.uv_rect[0] = entry->u0;
        inst.uv_rect[1] = entry->v0;
        inst.uv_rect[2] = entry->u1;
        inst.uv_rect[3] = entry->v1;
        if (entry->is_color_bitmap) {
            // Color bitmap: texture already has the real colors; only alpha carries
            // text opacity, so the tint must stay white or it double-multiplies colors.
            inst.color[0] = inst.color[1] = inst.color[2] = 1.0f;
            inst.color[3] = effectiveAlpha(ctx, cmd.color.a);
        } else {
            writeLinearColor(cmd.color, inst.color);
            inst.color[3] = effectiveAlpha(ctx, inst.color[3]);
        }
        inst.params[0] = px_range_screen;
        inst.params[1] = unit_range;
        PreparedGlyph pg{};
        pg.inst = inst;
        pg.atlas_page = entry->atlas_page;
        prepared.push_back(pg);
    }

    if (prepared.empty()) {
        return;
    }

    gpuCmdBindRenderPipeline(ctx.pass, pipe);
    uint32_t glyph_cursor = 0;
    while (glyph_cursor < prepared.size()) {
        const uint32_t batch_count = static_cast<uint32_t>(
            std::min<size_t>(kMaxGlyphsPerBatch, prepared.size() - glyph_cursor));
        std::memset(batch.glyphs, 0, sizeof(batch.glyphs));
        for (uint32_t i = 0; i < batch_count; ++i) {
            const auto& inst = prepared[glyph_cursor + i].inst;
            const uint32_t base = i * 4;
            std::memcpy(batch.glyphs[base + 0], inst.rect, sizeof(inst.rect));
            std::memcpy(batch.glyphs[base + 1], inst.uv_rect, sizeof(inst.uv_rect));
            std::memcpy(batch.glyphs[base + 2], inst.color, sizeof(inst.color));
            std::memcpy(batch.glyphs[base + 3], inst.params, sizeof(inst.params));
        }
        gpuPassSetCbuffer(ctx.pass, "TextUniforms", &batch, sizeof(batch));

        const uint32_t page = prepared[glyph_cursor].atlas_page;
        DongGPUTexture atlas_tex = glyph_atlas->getAtlasTextureForPage(page);
        gpuPassBindTexture(ctx.device, ctx.pass, "msdfTexture", driver.resources().gpuTextureHandle(atlas_tex));
        gpuPassBindSampler(ctx.device, ctx.pass, "msdfSampler", driver.pipelines().linearSampler());
        gpuCmdDraw(ctx.pass, 4, batch_count, 0, 0);
        glyph_cursor += batch_count;
    }
}

static void drawHostView(GpuGPUDriverImpl& driver, GpuExecuteContext& ctx, const dong::render::GPUCommand& cmd) {
    if (!ctx.pass || ctx.skip_draw_depth > 0) {
        return;
    }
    if (cmd.rect.width <= 0.0f || cmd.rect.height <= 0.0f) {
        return;
    }
    const float a = std::clamp(effectiveAlpha(ctx, cmd.opacity), 0.0f, 1.0f);
    const float border = 2.0f;

    dong::render::GPUCommand fill = cmd;
    fill.type = dong::render::GPUCommandType::DrawInstancedQuads;
    fill.color = dong::render::Color{34.0f / 255.0f, 34.0f / 255.0f, 34.0f / 255.0f, a};
    drawRect(driver, ctx, fill);

    const dong::render::Color border_color{85.0f / 255.0f, 85.0f / 255.0f, 85.0f / 255.0f, a};
    dong::render::GPUCommand side = cmd;
    side.type = dong::render::GPUCommandType::DrawInstancedQuads;
    side.color = border_color;

    side.rect = {cmd.rect.x, cmd.rect.y, cmd.rect.width, border};
    drawRect(driver, ctx, side);
    side.rect = {cmd.rect.x, cmd.rect.y + std::max(0.0f, cmd.rect.height - border), cmd.rect.width, border};
    drawRect(driver, ctx, side);
    side.rect = {cmd.rect.x, cmd.rect.y, border, cmd.rect.height};
    drawRect(driver, ctx, side);
    side.rect = {cmd.rect.x + std::max(0.0f, cmd.rect.width - border), cmd.rect.y, border, cmd.rect.height};
    drawRect(driver, ctx, side);
}

static void drawUberBatch(GpuGPUDriverImpl& driver, GpuExecuteContext& ctx, const dong::render::GPUCommand& cmd,
                          const dong::render::GPUCommandList& list) {
    if (!ctx.pass || cmd.uber_instance_count == 0) {
        return;
    }
    const uint64_t end = static_cast<uint64_t>(cmd.uber_instance_offset) +
                         static_cast<uint64_t>(cmd.uber_instance_count);
    if (end > list.uber_instance_pool.size()) {
        DONG_LOG_ERROR("GpuExecute: uber batch invalid range offset=%u count=%u pool=%zu",
                       cmd.uber_instance_offset, cmd.uber_instance_count, list.uber_instance_pool.size());
        return;
    }
    const dong::render::UberQuadInstance* base = list.uber_instance_pool.data() + cmd.uber_instance_offset;
    for (uint32_t i = 0; i < cmd.uber_instance_count; ++i) {
        const dong::render::UberQuadInstance& inst = base[i];
        dong::render::GPUCommand synthetic{};
        synthetic.rect = dong::render::Rect{inst.rect[0], inst.rect[1], inst.rect[2], inst.rect[3]};
        synthetic.color = dong::render::Color{inst.color[0], inst.color[1], inst.color[2], inst.color[3]};
        synthetic.radius = inst.params[1];
        synthetic.stroke_width = inst.params[2];
        synthetic.blur = inst.params[3];
        const float material = inst.params[0];
        if (material == dong::render::UberQuadMaterial::kSolid) {
            synthetic.type = dong::render::GPUCommandType::DrawInstancedQuads;
            drawRect(driver, ctx, synthetic);
        } else if (material == dong::render::UberQuadMaterial::kRounded) {
            synthetic.type = dong::render::GPUCommandType::DrawRoundedRectQuad;
            drawRoundedRect(driver, ctx, synthetic);
        } else if (material == dong::render::UberQuadMaterial::kShadow) {
            synthetic.type = dong::render::GPUCommandType::DrawShadowQuad;
            drawShadow(driver, ctx, synthetic);
        }
    }
}

void ExecuteDispatcher::dispatch(GpuGPUDriverImpl& driver, GpuExecuteContext& ctx,
                                 const dong::render::GPUCommandList& list) {
    using dong::render::GPUCommandType;
    for (const auto& cmd : list.commands) {
        switch (cmd.type) {
        case GPUCommandType::BeginFrame:
            break;
        case GPUCommandType::EndFrame:
            break;
        case GPUCommandType::BeginPass:
            ctx.in_pass = true;
            break;
        case GPUCommandType::EndPass:
            ctx.in_pass = false;
            break;
        case GPUCommandType::PushClipRect: {
            GpuExecuteContext::ClipEntry entry{};
            entry.x = static_cast<int>(std::floor(cmd.rect.x));
            entry.y = static_cast<int>(std::floor(cmd.rect.y));
            entry.w = static_cast<int>(std::ceil(cmd.rect.width));
            entry.h = static_cast<int>(std::ceil(cmd.rect.height));
            if (cmd.radius > 0.0f) {
                entry.has_rounded = true;
                entry.rounded_rect = cmd.rect;
                entry.rounded_radius = cmd.radius;
            }
            ctx.clip_stack.push_back(entry);
            break;
        }
        case GPUCommandType::PopClip:
            if (!ctx.clip_stack.empty()) {
                ctx.clip_stack.pop_back();
            }
            break;
        case GPUCommandType::BeginIsolatedLayer:
            ctx.layer_opacity_stack.push_back(ctx.layer_opacity);
            ctx.layer_opacity *= cmd.layer_opacity;
            if (!cmd.layer_dirty) {
                ++ctx.skip_draw_depth;
            }
            break;
        case GPUCommandType::EndIsolatedLayer:
            if (!ctx.layer_opacity_stack.empty()) {
                ctx.layer_opacity = ctx.layer_opacity_stack.back();
                ctx.layer_opacity_stack.pop_back();
            }
            if (ctx.skip_draw_depth > 0) {
                --ctx.skip_draw_depth;
            }
            break;
        case GPUCommandType::DrawInstancedQuads:
            drawRect(driver, ctx, cmd);
            break;
        case GPUCommandType::DrawImageQuad:
            drawImage(driver, ctx, cmd);
            break;
        case GPUCommandType::DrawNineSliceQuad:
            drawNineSlice(driver, ctx, cmd);
            break;
        case GPUCommandType::DrawRoundedRectQuad:
            drawRoundedRect(driver, ctx, cmd);
            break;
        case GPUCommandType::DrawShadowQuad:
            drawShadow(driver, ctx, cmd);
            break;
        case GPUCommandType::DrawText:
            drawText(driver, ctx, cmd);
            break;
        case GPUCommandType::DrawGradientQuad:
            drawGradient(driver, ctx, cmd);
            break;
        case GPUCommandType::DrawConicGradientQuad:
            drawConicGradient(driver, ctx, cmd, false);
            break;
        case GPUCommandType::ApplyMaskConicGradient:
            drawConicGradient(driver, ctx, cmd, true);
            break;
        case GPUCommandType::DrawHostView:
            drawHostView(driver, ctx, cmd);
            break;
        case GPUCommandType::UberQuadBatch:
            drawUberBatch(driver, ctx, cmd, list);
            break;
        default:
            break;
        }
    }
}

#endif

} // namespace dong::gpu_backend
