#include "gpu_gpu_driver.hpp"
#include "gpu_gpu_driver_ui_graph.hpp"
#include "gpu_shader_bindings.hpp"

#include "../../src/core/log.h"
#include "../../src/render/gpu_ir.hpp"
#include "../../src/render/glyph_atlas.hpp"
#include "../../src/render/font_resolver.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>

#include "dong_image_atlas.h"

#include <cmath>
#include <unordered_set>

#ifdef DONG_HAS_IN_DREAMING_GPU
#include "gpu/gpu.h"
#include "gpu/rendergraph/gpu_render_graph.h"
#include "gpu/platform/gpu_surface.h"
#endif

namespace dong::gpu_backend {

#ifdef DONG_HAS_IN_DREAMING_GPU
void GpuGPUDriverImpl::queryFramebufferSize(uint32_t& out_w, uint32_t& out_h) const {
    out_w = 1;
    out_h = 1;
    if (offscreen_.active) {
        out_w = offscreen_.width ? offscreen_.width : 1;
        out_h = offscreen_.height ? offscreen_.height : 1;
        return;
    }
    if (gpu_window_) {
        GpuWindow window = static_cast<GpuWindow>(gpu_window_);
        out_w = gpuWindowGetWidthInPixels(window);
        out_h = gpuWindowGetHeightInPixels(window);
    } else if (gpu_surface_) {
        GpuSurface surf = static_cast<GpuSurface>(gpu_surface_);
        out_w = gpuSurfaceGetWidth(surf);
        out_h = gpuSurfaceGetHeight(surf);
    }
    if (out_w == 0) {
        out_w = 1;
    }
    if (out_h == 0) {
        out_h = 1;
    }
}

void GpuGPUDriverImpl::syncSurfaceToWindow() {
    if (!gpu_surface_ || !gpu_window_ || offscreen_.active) {
        return;
    }
    GpuWindow window = static_cast<GpuWindow>(gpu_window_);
    GpuSurface surf = static_cast<GpuSurface>(gpu_surface_);
    const uint32_t pw = gpuWindowGetWidthInPixels(window);
    const uint32_t ph = gpuWindowGetHeightInPixels(window);
    if (pw == 0 || ph == 0) {
        return;
    }
    if (pw == gpuSurfaceGetWidth(surf) && ph == gpuSurfaceGetHeight(surf)) {
        return;
    }
    gpuSurfaceConfigure(surf, pw, ph, swapchain_format_, true);
}

const ExternalImageGpu* GpuGPUDriverImpl::findExternalImage(const std::string& key) const {
    auto it = external_images_.find(key);
    return it != external_images_.end() ? &it->second : nullptr;
}

render::GlyphAtlas* GpuGPUDriverImpl::glyphAtlasForFontSize(float font_size) {
    uint32_t best_tier = 64;
    float best_diff = 1e9f;
    for (const auto& atlas : glyph_atlas_tiers_) {
        const uint32_t tier = atlas->getGlyphBitmapSize();
        const float diff = std::fabs(static_cast<float>(tier) - font_size);
        if (diff < best_diff) {
            best_diff = diff;
            best_tier = tier;
        }
    }
    for (auto& atlas : glyph_atlas_tiers_) {
        if (atlas->getGlyphBitmapSize() == best_tier) {
            return atlas.get();
        }
    }
    return glyph_atlas_tiers_.empty() ? nullptr : glyph_atlas_tiers_.front().get();
}

bool GpuGPUDriverImpl::ensureImageInAtlas(const std::string& src, ImageAtlasEntryGpu& out_entry) {
    if (!image_atlas_ || src.empty()) {
        return false;
    }
    auto it = image_atlas_entries_.find(src);
    if (it != image_atlas_entries_.end()) {
        out_entry = it->second;
        return true;
    }

    std::vector<uint8_t> pixels;
    uint32_t width = 0;
    uint32_t height = 0;
    if (!image_resource_manager_.getImagePixelsRGBA(src, pixels, width, height)) {
        DONG_LOG_ERROR("GpuGPUDriver: ensureImageInAtlas failed to get pixels for '%s'", src.c_str());
        return false;
    }
    if (width == 0 || height == 0 || pixels.empty()) {
        return false;
    }

    // Downscale oversized images so they can still fit a single atlas page (mirrors the SDL
    // backend's behavior in sdl_gpu_driver_resources.cpp). Without this, images larger than the
    // atlas page (e.g. a 2560x1707 poster into a 2048x2048 atlas) fail allocation outright and
    // silently disappear from rendering.
    const uint32_t atlas_w = image_atlas_->config.width;
    const uint32_t atlas_h = image_atlas_->config.height;
    if (width > atlas_w || height > atlas_h) {
        const uint32_t src_w = width;
        const uint32_t src_h = height;
        uint32_t new_w = width;
        uint32_t new_h = height;
        if (new_w > atlas_w) {
            new_h = static_cast<uint32_t>((static_cast<uint64_t>(new_h) * atlas_w) / new_w);
            new_w = atlas_w;
        }
        if (new_h > atlas_h) {
            new_w = static_cast<uint32_t>((static_cast<uint64_t>(new_w) * atlas_h) / new_h);
            new_h = atlas_h;
        }
        new_w = std::max<uint32_t>(1, new_w);
        new_h = std::max<uint32_t>(1, new_h);

        DONG_LOG_INFO("GpuGPUDriver: ensureImageInAtlas downscaling '%s' from %ux%u to %ux%u to fit atlas",
                      src.c_str(), src_w, src_h, new_w, new_h);

        std::vector<uint8_t> scaled(static_cast<size_t>(new_w) * new_h * 4);
        for (uint32_t y = 0; y < new_h; ++y) {
            const uint32_t sy = static_cast<uint32_t>((static_cast<uint64_t>(y) * src_h) / new_h);
            for (uint32_t x = 0; x < new_w; ++x) {
                const uint32_t sx = static_cast<uint32_t>((static_cast<uint64_t>(x) * src_w) / new_w);
                const size_t src_idx = (static_cast<size_t>(sy) * src_w + sx) * 4;
                const size_t dst_idx = (static_cast<size_t>(y) * new_w + x) * 4;
                scaled[dst_idx + 0] = pixels[src_idx + 0];
                scaled[dst_idx + 1] = pixels[src_idx + 1];
                scaled[dst_idx + 2] = pixels[src_idx + 2];
                scaled[dst_idx + 3] = pixels[src_idx + 3];
            }
        }
        pixels = std::move(scaled);
        width = new_w;
        height = new_h;
    }

    DongAtlasEntry entry{};
    DongAtlasResult alloc_result = dong_atlas_alloc(image_atlas_, width, height, &entry);
    if (alloc_result != DONG_ATLAS_OK) {
        DONG_LOG_ERROR("GpuGPUDriver: ensureImageInAtlas atlas allocation failed for '%s' (%ux%u), error=%d",
                       src.c_str(), width, height, (int)alloc_result);
        return false;
    }
    DongAtlasResult upload_result = dong_atlas_upload(image_atlas_, &entry, pixels.data(), pixels.size());
    if (upload_result != DONG_ATLAS_OK) {
        DONG_LOG_ERROR("GpuGPUDriver: ensureImageInAtlas atlas upload failed for '%s', error=%d",
                       src.c_str(), (int)upload_result);
        return false;
    }

    ImageAtlasEntryGpu cached{};
    cached.entry = entry;
    cached.width = width;
    cached.height = height;
    image_atlas_entries_[src] = cached;
    out_entry = cached;
    return true;
}

bool GpuGPUDriverImpl::updateExternalImageRGBA(const std::string& key, const uint8_t* rgba, uint32_t width,
                                               uint32_t height, uint32_t stride_bytes) {
    if (!rgba || width == 0 || height == 0) {
        return false;
    }
    auto& slot = external_images_[key];
    if (!slot.texture || slot.width != width || slot.height != height) {
        if (slot.texture) {
            resources_.destroyTexture(slot.texture);
        }
        DongGPUTextureDesc desc{};
        desc.width = width;
        desc.height = height;
        desc.format = DONG_GPU_TEXTURE_FORMAT_RGBA8_UNORM;
        desc.usage = DONG_GPU_TEXTURE_USAGE_SAMPLER | DONG_GPU_TEXTURE_USAGE_TRANSFER_DST;
        desc.mip_levels = 1;
        desc.debug_name = "external_rgba";
        slot.texture = resources_.createTexture(&desc);
        slot.width = width;
        slot.height = height;
    }
    if (!slot.texture) {
        return false;
    }
    return resources_.uploadTextureSubrect(slot.texture, rgba, 0, 0, width, height, stride_bytes, nullptr) == 0;
}

bool GpuGPUDriverImpl::updateExternalImageYUV420P(const std::string& key, const uint8_t* plane_y, uint32_t stride_y,
                                                  const uint8_t* plane_u, uint32_t stride_u, const uint8_t* plane_v,
                                                  uint32_t stride_v, uint32_t width, uint32_t height) {
    if (!plane_y || !plane_u || !plane_v || width == 0 || height == 0) {
        return false;
    }

    // Convert YUV420P (3 separate planes) to interleaved RGBA8 on the CPU, then reuse the
    // RGBA external-image upload path. BT.601 full-range-ish conversion (U/V centered at 128).
    std::vector<uint8_t> rgba(static_cast<size_t>(width) * height * 4);
    for (uint32_t y = 0; y < height; ++y) {
        const uint32_t uv_row = y >> 1;
        for (uint32_t x = 0; x < width; ++x) {
            const uint32_t uv_col = x >> 1;
            const float yv = static_cast<float>(plane_y[y * stride_y + x]);
            const float u = static_cast<float>(plane_u[uv_row * stride_u + uv_col]) - 128.0f;
            const float v = static_cast<float>(plane_v[uv_row * stride_v + uv_col]) - 128.0f;

            float r = yv + 1.402f * v;
            float g = yv - 0.344f * u - 0.714f * v;
            float b = yv + 1.772f * u;

            auto clamp8 = [](float c) -> uint8_t {
                if (c < 0.0f) return 0;
                if (c > 255.0f) return 255;
                return static_cast<uint8_t>(c + 0.5f);
            };

            const size_t idx = (static_cast<size_t>(y) * width + x) * 4;
            rgba[idx + 0] = clamp8(r);
            rgba[idx + 1] = clamp8(g);
            rgba[idx + 2] = clamp8(b);
            rgba[idx + 3] = 255;
        }
    }

    return updateExternalImageRGBA(key, rgba.data(), width, height, width * 4);
}

bool GpuGPUDriverImpl::ensureIntermediateTexture(uint32_t width, uint32_t height) {
    if (width == 0) {
        width = 1;
    }
    if (height == 0) {
        height = 1;
    }
    if (intermediate_texture_ && intermediate_width_ == width && intermediate_height_ == height) {
        return true;
    }
    if (intermediate_texture_) {
        resources_.destroyTexture(intermediate_texture_);
        intermediate_texture_ = nullptr;
    }

    DongGPUTextureDesc desc{};
    desc.width = width;
    desc.height = height;
    desc.format = DONG_GPU_TEXTURE_FORMAT_RGBA8_UNORM;
    desc.usage = DONG_GPU_TEXTURE_USAGE_COLOR_TARGET | DONG_GPU_TEXTURE_USAGE_SAMPLER |
                 DONG_GPU_TEXTURE_USAGE_TRANSFER_SRC;
    desc.mip_levels = 1;
    desc.debug_name = "window_intermediate";
    intermediate_texture_ = resources_.createTexture(&desc);
    intermediate_width_ = intermediate_texture_ ? width : 0;
    intermediate_height_ = intermediate_texture_ ? height : 0;
    return intermediate_texture_ != nullptr;
}

void GpuGPUDriverImpl::executeSwapchainBlitPass(GpuGraphPassContext* ctx, DongGPUTexture intermediate,
                                                uint32_t width, uint32_t height) {
    if (!ctx || !ctx->renderPass || !intermediate || width == 0 || height == 0) {
        DONG_LOG_ERROR("GpuGPUDriver: SwapchainBlit skipped (ctx=%p renderPass=%p intermediate=%p w=%u h=%u)",
                       static_cast<void*>(ctx), ctx ? static_cast<void*>(ctx->renderPass) : nullptr,
                       static_cast<void*>(intermediate), width, height);
        return;
    }
    auto pipe = pipelines().presentImagePipeline();
    if (!pipe) {
        pipe = pipelines().pipeline(GpuPipelineKind::Image);
    }
    if (!pipe) {
        DONG_LOG_ERROR("GpuGPUDriver: SwapchainBlit has no pipeline (present or image) - backbuffer stays cleared");
        return;
    }

    struct ImageUniformData {
        float rect[4];
        float uv_rect[4];
        float viewport[4];
        float transform[8];
        float tint[4];
        float clip_rects[4][4];
        float clip_radii[4];
        float clip_meta[4];
    } u{};
    u.rect[0] = 0.0f;
    u.rect[1] = 0.0f;
    u.rect[2] = static_cast<float>(width);
    u.rect[3] = static_cast<float>(height);
    u.uv_rect[0] = 0.0f;
    u.uv_rect[1] = 0.0f;
    u.uv_rect[2] = 1.0f;
    u.uv_rect[3] = 1.0f;
    u.viewport[0] = static_cast<float>(width);
    u.viewport[1] = static_cast<float>(height);
    u.viewport[2] = 0.0f;
    u.viewport[3] = 0.0f;
    // uTransform is a 2x3 affine matrix packed as two float4 rows: row0 = (a, b, tx, _),
    // row1 = (c, d, ty, _), so identity requires transform[0]=a=1 AND transform[5]=d=1 (NOT
    // transform[4], which is c - the x-contributes-to-y shear term). Setting transform[4] instead
    // of transform[5] collapsed every quad vertex's y coordinate to its x coordinate, producing a
    // zero-area quad that rasterized nothing - the backbuffer stayed at its black clear color
    // while the intermediate texture (rendered by a different, correctly-configured pass)
    // remained pixel-perfect. This is the root cause of the "black window" symptom.
    u.transform[0] = 1.0f;
    u.transform[5] = 1.0f;
    u.tint[0] = 1.0f;
    u.tint[1] = 1.0f;
    u.tint[2] = 1.0f;
    u.tint[3] = 1.0f;

    gpuCmdSetViewport(ctx->renderPass, 0, 0, static_cast<float>(width), static_cast<float>(height));
    gpuCmdBindRenderPipeline(ctx->renderPass, pipe);
    gpuPassSetCbuffer(ctx->renderPass, "ImageUniforms", &u, sizeof(u));
    auto tex_handle = resources_.gpuTextureHandle(intermediate);
    gpuPassBindTexture(static_cast<GpuDevice>(gpu_device_), ctx->renderPass, "imageTexture", tex_handle);
    gpuPassBindSampler(static_cast<GpuDevice>(gpu_device_), ctx->renderPass, "imageSampler",
                       pipelines().linearSampler());
    gpuCmdDraw(ctx->renderPass, 4, 1, 0, 0);
}

namespace {
bool writeDebugBMP(const char* path, uint32_t width, uint32_t height, const uint8_t* rgba) {
    if (!path || !rgba || width == 0 || height == 0) {
        return false;
    }
    const uint32_t row_bytes = ((width * 3 + 3) / 4) * 4;
    const uint32_t pixel_bytes = row_bytes * height;
    const uint32_t file_size = 14 + 40 + pixel_bytes;
    FILE* f = std::fopen(path, "wb");
    if (!f) {
        return false;
    }
    uint8_t header[14] = {'B', 'M'};
    header[2] = static_cast<uint8_t>(file_size);
    header[3] = static_cast<uint8_t>(file_size >> 8);
    header[4] = static_cast<uint8_t>(file_size >> 16);
    header[5] = static_cast<uint8_t>(file_size >> 24);
    header[10] = 54;
    std::fwrite(header, 1, 14, f);

    uint8_t info[40] = {0};
    info[0] = 40;
    info[4] = static_cast<uint8_t>(width);
    info[5] = static_cast<uint8_t>(width >> 8);
    info[6] = static_cast<uint8_t>(width >> 16);
    info[7] = static_cast<uint8_t>(width >> 24);
    info[8] = static_cast<uint8_t>(height);
    info[9] = static_cast<uint8_t>(height >> 8);
    info[10] = static_cast<uint8_t>(height >> 16);
    info[11] = static_cast<uint8_t>(height >> 24);
    info[12] = 1;
    info[14] = 24;
    std::fwrite(info, 1, 40, f);

    std::vector<uint8_t> row(row_bytes, 0);
    for (int32_t y = static_cast<int32_t>(height) - 1; y >= 0; --y) {
        for (uint32_t x = 0; x < width; ++x) {
            const uint8_t* p = rgba + (static_cast<uint32_t>(y) * width + x) * 4;
            row[x * 3 + 0] = p[2];
            row[x * 3 + 1] = p[1];
            row[x * 3 + 2] = p[0];
        }
        std::fwrite(row.data(), 1, row_bytes, f);
    }
    std::fclose(f);
    return true;
}
} // namespace

bool GpuGPUDriverImpl::shouldDumpThisFrame(uint64_t* out_frame_index) {
    static const char* dir = std::getenv("DONG_GPU_DUMP_WINDOW_DIR");
    if (!dir) {
        return false;
    }
    static const char* every_env = std::getenv("DONG_GPU_DUMP_WINDOW_EVERY");
    const uint64_t every = every_env ? static_cast<uint64_t>(std::atoi(every_env)) : 30;
    const uint64_t current = dump_frame_counter_++;
    if (out_frame_index) {
        *out_frame_index = current;
    }
    return every != 0 && (current % every) == 0;
}

void GpuGPUDriverImpl::maybeDumpWindowFrame(uint32_t width, uint32_t height, uint64_t frame_index) {
    static const char* dir = std::getenv("DONG_GPU_DUMP_WINDOW_DIR");
    if (!dir || !intermediate_texture_) {
        return;
    }

    resources_.waitForGpu();
    std::vector<uint8_t> pixels(static_cast<size_t>(width) * height * 4);
    if (resources_.readbackTextureRGBA(intermediate_texture_, width, height, pixels.data(), pixels.size()) != 0) {
        DONG_LOG_WARN("GpuGPUDriver: window frame dump readback failed");
        return;
    }
    char path[512];
    std::snprintf(path, sizeof(path), "%s/intermediate_%06llu.bmp", dir, static_cast<unsigned long long>(frame_index));
    if (writeDebugBMP(path, width, height, pixels.data())) {
        std::fprintf(stderr, "[GpuGPUDriver] dumped intermediate frame -> %s\n", path);
    }
}

void GpuGPUDriverImpl::maybeDumpBackbuffer(GpuSurfaceTexture backbuffer, uint32_t width, uint32_t height,
                                           uint64_t frame_index) {
    static const char* dir = std::getenv("DONG_GPU_DUMP_WINDOW_DIR");
    if (!dir || !backbuffer || !gpu_device_ || width == 0 || height == 0) {
        return;
    }

    resources_.waitForGpu();
    std::vector<uint8_t> pixels(static_cast<size_t>(width) * height * 4);
    uint32_t actual_w = 0;
    uint32_t actual_h = 0;
    if (gpuSurfaceTextureReadbackRGBA(static_cast<GpuDevice>(gpu_device_), backbuffer, pixels.data(), pixels.size(),
                                      &actual_w, &actual_h) != GPU_SUCCESS) {
        DONG_LOG_WARN("GpuGPUDriver: backbuffer dump readback failed");
        return;
    }
    char path[512];
    std::snprintf(path, sizeof(path), "%s/backbuffer_%06llu.bmp", dir, static_cast<unsigned long long>(frame_index));
    if (writeDebugBMP(path, actual_w ? actual_w : width, actual_h ? actual_h : height, pixels.data())) {
        std::fprintf(stderr, "[GpuGPUDriver] dumped backbuffer (post-blit, pre-present) -> %s (%ux%u)\n", path,
                    actual_w, actual_h);
    }
}

void GpuGPUDriverImpl::flushUploadPass(GpuGraphPassContext* ctx) {
    (void)ctx;
    resources_.waitForGpu();
}

void GpuGPUDriverImpl::executeUiMainPass(GpuGraphPassContext* ctx, const DongUiPassBundle* bundle) {
    if (!ctx || !ctx->renderPass || !bundle || !bundle->command_list) {
        return;
    }
    const auto* list = static_cast<const render::GPUCommandList*>(bundle->command_list);
    if (!list) {
        return;
    }

    GpuExecuteContext exec{};
    exec.pass = ctx->renderPass;
    exec.device = static_cast<GpuDevice>(gpu_device_);
    queryFramebufferSize(exec.viewport_w, exec.viewport_h);
    gpuCmdSetViewport(exec.pass, 0, 0, static_cast<float>(exec.viewport_w), static_cast<float>(exec.viewport_h));
    execute_dispatcher_->dispatch(*this, exec, *list);
}

void GpuGPUDriverImpl::prepareResourcesImpl(const void* command_list) {
    if (!initialized_ || !command_list) {
        return;
    }
    const auto* list = static_cast<const render::GPUCommandList*>(command_list);
    if (!list) {
        return;
    }

    std::unordered_map<uint32_t, std::vector<render::GlyphAtlas::GlyphRequest>> tier_requests;
    std::unordered_set<std::string> seen;

    for (const auto& cmd : list->commands) {
        if (cmd.type != render::GPUCommandType::DrawText || cmd.glyphs.empty()) {
            continue;
        }
        std::string font_path = cmd.font_path;
        if (font_path.empty() && !cmd.font_paths.empty()) {
            font_path = cmd.font_paths[0];
        }
        if (font_path.empty()) {
            font_path = render::resolveFontPath(cmd.font_family, cmd.font_weight, cmd.font_style);
        }
        if (font_path.empty()) {
            continue;
        }
        render::GlyphAtlas* atlas = glyphAtlasForFontSize(cmd.font_size > 0 ? cmd.font_size : 16.0f);
        if (!atlas) {
            continue;
        }
        const uint32_t tier = atlas->getGlyphBitmapSize();
        for (const auto& glyph : cmd.glyphs) {
            if (glyph.glyph_id == 0) {
                continue;
            }
            std::string glyph_font = font_path;
            if (!cmd.font_paths.empty() && glyph.font_path_index < cmd.font_paths.size()) {
                glyph_font = cmd.font_paths[glyph.font_path_index];
            }
            const std::string key = glyph_font + "#" + std::to_string(glyph.glyph_id) + "#" + std::to_string(tier);
            if (!seen.insert(key).second) {
                continue;
            }
            render::GlyphAtlas::GlyphRequest req{};
            req.glyph_id = glyph.glyph_id;
            req.font_path = glyph_font;
            tier_requests[tier].push_back(std::move(req));
        }
    }

    for (auto& [tier, requests] : tier_requests) {
        if (requests.empty()) {
            continue;
        }
        for (auto& atlas : glyph_atlas_tiers_) {
            if (atlas->getGlyphBitmapSize() == tier) {
                atlas->addGlyphsBatched(requests);
                break;
            }
        }
    }

    for (const auto& cmd : list->commands) {
        if (cmd.type == render::GPUCommandType::DrawImageQuad && !cmd.image_src.empty() &&
            cmd.image_src.rfind("video://", 0) != 0) {
            ImageAtlasEntryGpu tmp{};
            ensureImageInAtlas(cmd.image_src, tmp);
        }
    }

    if (slug_font_cache_ && pipelines().pipeline(GpuPipelineKind::Slug)) {
        prepareSlugResources(*list);
    }
}
#endif

int GpuGPUDriverImpl::beginFrameOffscreen(DongGPUTexture target, uint32_t width, uint32_t height) {
#ifdef DONG_HAS_IN_DREAMING_GPU
    if (!initialized_ || !target) {
        return 0;
    }
    offscreen_.target = target;
    offscreen_.target_view = resources_.gpuTextureHandle(target);
    if (resources_.gpuTextureShaderView(target).index != 0) {
        offscreen_.target_view = resources_.gpuTextureShaderView(target);
    }
    offscreen_.width = width > 0 ? width : 1;
    offscreen_.height = height > 0 ? height : 1;
    offscreen_.active = true;
    return 1;
#else
    (void)target;
    (void)width;
    (void)height;
    return 0;
#endif
}

int GpuGPUDriverImpl::endFrameOffscreen() {
#ifdef DONG_HAS_IN_DREAMING_GPU
    offscreen_ = {};
#endif
    return 1;
}

int GpuGPUDriverImpl::execute(const void* command_list) {
    if (!initialized_) {
        return 0;
    }

    // Reset the per-frame Slug vertex bump allocator. Must happen before prepareResources()/the
    // pass callbacks below run, since drawTextSlug() hands out offsets into slug_vertex_buffer_.
    slug_vertex_write_offset_ = 0;

    dong_ui_graph_prepare(&ui_graph_, command_list);
    // Align with SDL bridge: glyph/atlas/Slug textures must be ready before draw passes.
    prepareResources(command_list);

    if (api_driver_.embedded_mode) {
        if (ui_graph_.host_graph) {
            dong_ui_graph_add_passes(&ui_graph_);
        }
        return 1;
    }

#ifdef DONG_HAS_IN_DREAMING_GPU
    if (!gpu_device_ || !gpu_queue_) {
        return 0;
    }

    syncSurfaceToWindow();

    const DongUiPassBundle* bundle = dong_ui_graph_get_pass_bundle(&ui_graph_);

    GpuGraph graph = nullptr;
    if (gpuGraphCreate(static_cast<GpuDevice>(gpu_device_), &graph) != GPU_SUCCESS) {
        return 0;
    }

    const bool window_mode = gpu_surface_ && !offscreen_.active;
    uint32_t fb_w = 1;
    uint32_t fb_h = 1;
    queryFramebufferSize(fb_w, fb_h);

    GpuGraphResource color_target = GPU_GRAPH_NULL_RESOURCE;
    GpuGraphResource backbuffer_res = GPU_GRAPH_NULL_RESOURCE;
    GpuSurfaceTexture backbuffer = nullptr;

    if (window_mode) {
        if (!ensureIntermediateTexture(fb_w, fb_h)) {
            gpuGraphDestroy(graph);
            return 0;
        }
        auto tex = resources_.gpuTextureHandle(intermediate_texture_);
        color_target = gpuGraphImportTexture(graph, tex, GPU_RESOURCE_STATE_RENDER_TARGET, "ui_intermediate");
    } else if (offscreen_.active && offscreen_.target) {
        auto tex = resources_.gpuTextureHandle(offscreen_.target);
        color_target = gpuGraphImportTexture(graph, tex, GPU_RESOURCE_STATE_RENDER_TARGET, "offscreen");
    } else if (gpu_surface_) {
        if (gpuSurfaceAcquireNextImage(static_cast<GpuSurface>(gpu_surface_), &backbuffer) == GPU_SUCCESS) {
            color_target = gpuGraphImportSurfaceTexture(graph, backbuffer, "backbuffer");
            backbuffer_res = color_target;
        }
    }

    gpu_register_ui_passes_on_graph(graph, bundle, color_target, this);

    if (window_mode && color_target != GPU_GRAPH_NULL_RESOURCE) {
        GpuResult acquire_result = gpuSurfaceAcquireNextImage(static_cast<GpuSurface>(gpu_surface_), &backbuffer);
        if (acquire_result == GPU_SUCCESS) {
            backbuffer_res = gpuGraphImportSurfaceTexture(graph, backbuffer, "backbuffer");
            if (backbuffer_res == GPU_GRAPH_NULL_RESOURCE) {
                DONG_LOG_ERROR("GpuGPUDriver: gpuGraphImportSurfaceTexture failed (backbuffer=%p)",
                               static_cast<void*>(backbuffer));
            }
        } else {
            static uint64_t s_acquire_fail_count = 0;
            if ((s_acquire_fail_count++ % 60) == 0) {
                DONG_LOG_ERROR("GpuGPUDriver: gpuSurfaceAcquireNextImage failed result=%d (frame will not present)",
                               (int)acquire_result);
            }
        }
        if (backbuffer_res != GPU_GRAPH_NULL_RESOURCE && backbuffer) {
            struct BlitPassPayload {
                GpuGPUDriverImpl* impl = nullptr;
                DongGPUTexture intermediate = nullptr;
                uint32_t width = 0;
                uint32_t height = 0;
            };
            static thread_local BlitPassPayload blit_payload;
            blit_payload.impl = this;
            blit_payload.intermediate = intermediate_texture_;
            blit_payload.width = fb_w;
            blit_payload.height = fb_h;

            GpuGraphPass blit = gpuGraphAddRenderPass(graph, "Dong.SwapchainBlit");
            GpuGraphColorAttachment blit_ca{};
            blit_ca.resource = backbuffer_res;
            blit_ca.loadOp = GPU_LOAD_OP_CLEAR;
            blit_ca.storeOp = GPU_STORE_OP_STORE;
            blit_ca.clearColor[0] = 0.0f;
            blit_ca.clearColor[1] = 0.0f;
            blit_ca.clearColor[2] = 0.0f;
            blit_ca.clearColor[3] = 1.0f;
            gpuGraphPassSetColorAttachments(blit, 1, &blit_ca);
            // The intermediate texture is sampled (not attached) in this pass; without an
            // explicit read declaration the graph's automatic barrier pass has no way to know
            // this pass depends on it, so it never transitions it out of RENDER_TARGET state.
            // Sampling a texture still in RENDER_TARGET state is undefined on most backends and
            // manifests as flicker / a hazy, semi-garbage overlay - exactly the symptom reported.
            gpuGraphPassRead(blit, color_target);
            gpuGraphPassSetCallback(
                blit,
                [](GpuGraphPassContext* ctx, void* user_data) {
                    const auto* payload = static_cast<const BlitPassPayload*>(user_data);
                    if (payload && payload->impl) {
                        payload->impl->executeSwapchainBlitPass(ctx, payload->intermediate, payload->width,
                                                                  payload->height);
                    }
                },
                &blit_payload);

            GpuGraphPass present = gpuGraphAddRenderPass(graph, "Dong.Present");
            gpuGraphPassPresent(present, backbuffer_res);
        }
    } else if (color_target != GPU_GRAPH_NULL_RESOURCE && gpu_surface_ && !offscreen_.active) {
        GpuGraphPass present = gpuGraphAddRenderPass(graph, "Dong.Present");
        gpuGraphPassPresent(present, color_target);
    } else if (offscreen_.active && color_target != GPU_GRAPH_NULL_RESOURCE) {
        // Offscreen render targets (used by e.g. scene3d's per-screen textures) are sampled
        // *after* this graph is destroyed by raw, non-GpuGraph code (scene3d_world_gpu_render's
        // gpuCmdBeginRenderPass + gpuPassBindTexture) with no barrier of its own. Without this
        // explicit read declaration the texture is left behind in RENDER_TARGET state from the
        // UI passes above, so sampling it as a shader resource reads an undefined/garbage GPU
        // layout - this manifests as an all-black quad (the "3d_screens_simple 黑屏" symptom).
        // Declaring the read here forces the graph's barrier system to transition it to
        // GPU_RESOURCE_STATE_SHADER_RESOURCE before gpuGraphExecute() returns.
        GpuGraphPass export_pass = gpuGraphAddRenderPass(graph, "Dong.OffscreenExport");
        gpuGraphPassRead(export_pass, color_target);
    }

    GpuResult compile_result = gpuGraphCompile(graph);
    if (compile_result == GPU_SUCCESS) {
        gpuGraphExecute(graph, static_cast<GpuCommandQueue>(gpu_queue_));
    } else {
        DONG_LOG_ERROR("GpuGPUDriver: gpuGraphCompile failed result=%d (window_mode=%d color_target=%d "
                       "backbuffer_res=%d)",
                       (int)compile_result, (int)window_mode, (int)color_target, (int)backbuffer_res);
    }
    gpuGraphDestroy(graph);

    if (window_mode) {
        uint64_t dump_frame_index = 0;
        if (shouldDumpThisFrame(&dump_frame_index)) {
            // Order matters: the backbuffer must be read back before gpuSurfacePresent()/
            // gpuSurfaceTextureRelease() below invalidate it. Dumping both the intermediate
            // (post-UI-pass) and backbuffer (post-blit, pre-present) textures for the same
            // frame_index lets us tell apart "UI rendering is wrong" from "blit/present to the
            // swapchain is wrong" - the two candidate causes of an all-black window.
            maybeDumpWindowFrame(fb_w, fb_h, dump_frame_index);
            maybeDumpBackbuffer(backbuffer, fb_w, fb_h, dump_frame_index);
        }
    }

    if (backbuffer) {
        if (gpu_surface_) {
            gpuSurfacePresent(static_cast<GpuSurface>(gpu_surface_));
        }
        gpuSurfaceTextureRelease(backbuffer);
    }
    return 1;
#else
    (void)command_list;
    return 1;
#endif
}

void GpuGPUDriverImpl::prepareResources(const void* command_list) {
#ifdef DONG_HAS_IN_DREAMING_GPU
    prepareResourcesImpl(command_list);
#else
    (void)command_list;
#endif
}

} // namespace dong::gpu_backend
