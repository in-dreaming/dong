// =============================================================================
// SDL GPU Driver - Execute Implementation (Phase 2B)
// =============================================================================
// Migrated from: src/render/sdl_render/gpu_driver_sdl_execute.cpp
// =============================================================================

#include "sdl_gpu_driver.hpp"
#include "../../src/render/gpu_ir.hpp"
#include "sdl_gpu_device.hpp"
#include "sdl_shader_manager.hpp"
#include "../../src/render/resource_manager.hpp"
#include "../../src/render/glyph_atlas.hpp"
#include "../../src/render/font_resolver.hpp"
#include "../../src/core/log.h"
#include "../../src/core/profiler.h"

#include <SDL3/SDL_video.h>

// ImageAtlas
#include "dong_image_atlas.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <utility>
#include <vector>

namespace dong {
namespace render {

namespace {

// 直接传递 sRGB 颜色，不做 gamma 转换
// 因为 CSS 颜色本身就是 sRGB 空间，且 GPU 混合也在 sRGB 空间进行
void writeLinearColor(const Color& color, float out_rgba[4]) {
    out_rgba[0] = color.r;
    out_rgba[1] = color.g;
    out_rgba[2] = color.b;
    out_rgba[3] = color.a;
}

void writeTransform(float (&out)[8], const float transform[6]) {
    // 填充 2D transform 矩阵到 uniform buffer
    // 格式: [m0,m1,m2,0] [m3,m4,m5,0]
    out[0] = transform[0];
    out[1] = transform[1];
    out[2] = transform[2];
    out[3] = 0.0f;
    out[4] = transform[3];
    out[5] = transform[4];
    out[6] = transform[5];
    out[7] = 0.0f;

    // DEBUG: 打印非单位矩阵
    bool is_identity = (transform[0] == 1.0f && transform[1] == 0.0f && transform[2] == 0.0f &&
                        transform[3] == 0.0f && transform[4] == 1.0f && transform[5] == 0.0f);
    if (!is_identity) {
        DONG_LOG_DEBUG("[GPU_TRANSFORM] matrix=[%.3f %.3f %.1f; %.3f %.3f %.1f]",
                       transform[0], transform[1], transform[2],
                       transform[3], transform[4], transform[5]);
    } else {
        DONG_LOG_DEBUG("[GPU_TRANSFORM] identity matrix");
    }
}

struct RenderTargetState {
    SDL_GPUTexture* texture = nullptr;
    Uint32 width = 0;
    Uint32 height = 0;
    bool is_swapchain = false;

    // 当前 render target 在"屏幕坐标系"中的原点（左上角）。
    // swapchain / intermediate: (0,0)
    // isolated layer texture:   (layer_bounds.x, layer_bounds.y)
    float origin_x = 0.0f;
    float origin_y = 0.0f;
};

struct IsolatedLayerState {
    SDL_GPUTexture* texture = nullptr;
    Uint32 width = 0;
    Uint32 height = 0;
    Rect bounds;
    float opacity = 1.0f;
    uint64_t id = 0;
    bool dirty = true;
    bool cache_composited = false; // 非脏图层复用缓存时，是否已在 BeginIsolatedLayer 提前合成

    // 处理"外层复用缓存 + 内层脏图层需要重栅格"的组合：
    // 外层会把 ctx.skip_draw_depth++，导致内层图层内部的绘制命令也被跳过。
    // 进入脏图层时暂存并清零，退出时恢复。
    int saved_skip_draw_depth = 0;
    bool restore_skip_draw_depth = false;

    float draw_transform[6] = {1.0f, 0.0f, 0.0f,
                               0.0f, 1.0f, 0.0f};
    float composite_transform[6] = {1.0f, 0.0f, 0.0f,
                                    0.0f, 1.0f, 0.0f};
    float scroll[2] = {0.0f, 0.0f};
};

constexpr int kMaxRoundedClipUniforms = 4;

struct ClipStackEntry {
    SDL_Rect scissor;
    bool has_rounded = false;
    Rect rounded_rect;
    float rounded_radius = 0.0f;
};

struct ClipUniformBlock {
    float clip_rects[kMaxRoundedClipUniforms][4];
    float clip_radii[kMaxRoundedClipUniforms];
    float clip_meta[4];
};

struct PipelineBindingState {
    enum class ActivePipeline : uint8_t {
        None,
        Rect,
        RoundRect,
        Shadow,
        Image,
        ImageYUV,
        Text,
    } active = ActivePipeline::None;

    bool image_sampler_bound = false;
    bool text_sampler_bound = false;

    void reset() {
        active = ActivePipeline::None;
        image_sampler_bound = false;
        text_sampler_bound = false;
    }
};

SDL_Rect to_sdl_rect(const Rect& rect) {
    const float x0 = rect.x;
    const float y0 = rect.y;
    const float x1 = rect.x + rect.width;
    const float y1 = rect.y + rect.height;
    SDL_Rect result{};
    result.x = static_cast<int>(std::floor(x0));
    result.y = static_cast<int>(std::floor(y0));
    const int right = static_cast<int>(std::ceil(x1));
    const int bottom = static_cast<int>(std::ceil(y1));
    result.w = std::max(0, right - result.x);
    result.h = std::max(0, bottom - result.y);
    return result;
}

SDL_Rect intersect_rect(const SDL_Rect& a, const SDL_Rect& b) {
    SDL_Rect out{};
    const int x = std::max(a.x, b.x);
    const int y = std::max(a.y, b.y);
    const int right = std::min(a.x + a.w, b.x + b.w);
    const int bottom = std::min(a.y + a.h, b.y + b.h);
    out.x = x;
    out.y = y;
    out.w = std::max(0, right - x);
    out.h = std::max(0, bottom - y);
    return out;
}

} // namespace

struct SDLGPUDriver::ExecuteContext {
    // 引用：允许在拆分 cmd buffer 时自动同步
    SDL_GPUCommandBuffer*& cmd_buf;

    SDL_GPUDevice* dev = nullptr;
    SDL_Window* window = nullptr;

    SDL_GPUTexture* real_swapchain_texture = nullptr;
    SDL_GPUTexture* swapchain_texture = nullptr;
    Uint32 w = 0;
    Uint32 h = 0;
    bool use_intermediate = false;

    float device_pixel_ratio = 1.0f;
    float inv_device_pixel_ratio = 1.0f;

    SDL_GPUColorTargetInfo color_target{};
    SDL_GPURenderPass* pass = nullptr;

    std::vector<RenderTargetState> render_target_stack;
    std::vector<IsolatedLayerState> isolated_layer_stack;
    int skip_draw_depth = 0;

    std::vector<ClipStackEntry> clip_stack;
    PipelineBindingState pipeline_state;

    uint32_t debug_layer_cache_rasterized = 0;
    uint32_t debug_layer_cache_reused = 0;

    bool debug_rt_enabled = false;
    bool debug_log_layer_cache = false;
    bool debug_log_draw_batches = false;
    bool layer_cache_enabled = false;
    bool split_cmd_buf_for_isolated_layers = true;

    bool offscreen = false;
    unsigned long long frame_index = 0;

    bool aborted = false;

    ExecuteContext(SDL_GPUCommandBuffer*& in_cmd_buf, SDL_GPUDevice* in_dev, SDL_Window* in_window)
        : cmd_buf(in_cmd_buf), dev(in_dev), window(in_window) {
    }

    std::pair<Uint32, Uint32> currentTargetDimensions() const {
        if (render_target_stack.empty()) {
            return {w, h};
        }
        return {render_target_stack.back().width, render_target_stack.back().height};
    }

    std::pair<float, float> currentViewport() const {
        auto [cw, ch] = currentTargetDimensions();
        if (cw == 0) cw = 1;
        if (ch == 0) ch = 1;
        return {static_cast<float>(cw), static_cast<float>(ch)};
    }

    void writeViewport(float (&out)[4]) const {
        auto [vw, vh] = currentViewport();
        out[0] = vw;
        out[1] = vh;
        out[2] = 0.0f;
        out[3] = 0.0f;
    }

    const float* getCurrentTransform() const {
        if (!isolated_layer_stack.empty()) {
            const float* transform = isolated_layer_stack.back().draw_transform;
            DONG_LOG_DEBUG("[GET_TRANSFORM] Using draw transform: [%.3f %.3f %.1f; %.3f %.3f %.1f]",
                           transform[0], transform[1], transform[2],
                           transform[3], transform[4], transform[5]);
            return transform;
        }
        DONG_LOG_DEBUG("[GET_TRANSFORM] Using identity matrix (no isolated layers)");
        static const float identity[6] = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
        return identity;
    }

    void applyScissor(SDL_GPURenderPass* target_pass) {
        if (!target_pass) {
            return;
        }

        auto [cw, ch] = currentTargetDimensions();
        const int target_w = static_cast<int>(cw);
        const int target_h = static_cast<int>(ch);

        // clip_stack 中的 scissor 统一使用"屏幕坐标系"。
        // 当渲染到 isolated layer 的紧凑纹理时，需要把 scissor 平移到 render target 的局部坐标系。
        float ox_f = 0.0f;
        float oy_f = 0.0f;
        if (!render_target_stack.empty()) {
            ox_f = render_target_stack.back().origin_x;
            oy_f = render_target_stack.back().origin_y;
        }
        const int ox = static_cast<int>(std::floor(ox_f));
        const int oy = static_cast<int>(std::floor(oy_f));

        SDL_Rect scissor{};
        if (clip_stack.empty()) {
            scissor.x = 0;
            scissor.y = 0;
            scissor.w = target_w;
            scissor.h = target_h;
        } else {
            scissor = clip_stack.back().scissor;
            scissor.x -= ox;
            scissor.y -= oy;
        }

        // Clamp to target bounds to avoid negative sizes.
        SDL_Rect target_bounds{};
        target_bounds.x = 0;
        target_bounds.y = 0;
        target_bounds.w = std::max(0, target_w);
        target_bounds.h = std::max(0, target_h);
        scissor = intersect_rect(scissor, target_bounds);

        if (debug_rt_enabled) {
            DONG_LOG_DEBUG("[apply_scissor] origin=(%.1f,%.1f) scissor: x=%d y=%d w=%d h=%d",
                    ox_f, oy_f, scissor.x, scissor.y, scissor.w, scissor.h);
        }

        SDL_SetGPUScissor(target_pass, &scissor);
    }

    void fillClipUniform(ClipUniformBlock& block) const {
        for (int i = 0; i < kMaxRoundedClipUniforms; ++i) {
            block.clip_rects[i][0] = 0.0f;
            block.clip_rects[i][1] = 0.0f;
            block.clip_rects[i][2] = 0.0f;
            block.clip_rects[i][3] = 0.0f;
            block.clip_radii[i] = 0.0f;
        }
        block.clip_meta[0] = 0.0f;
        block.clip_meta[1] = 0.0f;
        block.clip_meta[2] = 0.0f;
        block.clip_meta[3] = 0.0f;

        float ox = 0.0f;
        float oy = 0.0f;
        if (!render_target_stack.empty()) {
            ox = render_target_stack.back().origin_x;
            oy = render_target_stack.back().origin_y;
        }

        int clip_index = 0;
        for (const auto& entry : clip_stack) {
            if (!entry.has_rounded) {
                continue;
            }

            // Rounded clip 需要和当前 render target 的局部坐标系对齐。
            const float x0 = entry.rounded_rect.x - ox;
            const float y0 = entry.rounded_rect.y - oy;
            const float x1 = entry.rounded_rect.x + entry.rounded_rect.width - ox;
            const float y1 = entry.rounded_rect.y + entry.rounded_rect.height - oy;

            block.clip_rects[clip_index][0] = x0;
            block.clip_rects[clip_index][1] = y0;
            block.clip_rects[clip_index][2] = x1;
            block.clip_rects[clip_index][3] = y1;
            block.clip_radii[clip_index] = entry.rounded_radius;
            ++clip_index;
            if (clip_index >= kMaxRoundedClipUniforms) {
                break;
            }
        }
        block.clip_meta[0] = static_cast<float>(clip_index);
    }

    void logRenderTargetStack(const char* prefix) const {
        if (!debug_rt_enabled) {
            return;
        }
        if (render_target_stack.empty()) {
            DONG_LOG_DEBUG("%s frame=%llu rt_depth=0 (swapchain=%p)", prefix, frame_index, (void*)swapchain_texture);
            return;
        }
        const RenderTargetState& top = render_target_stack.back();
        DONG_LOG_DEBUG("%s frame=%llu rt_depth=%zu top_rt=%p size=%ux%u is_swapchain=%d",
                prefix,
                frame_index,
                render_target_stack.size(),
                (void*)top.texture,
                top.width,
                top.height,
                top.is_swapchain ? 1 : 0);
    }
};

bool SDLGPUDriver::executeSetupMainTarget(ExecuteContext& ctx) {
    if (!ctx.dev) {
        DONG_LOG_ERROR("SDLGPUDriver::execute: no GPU device");
        return false;
    }

    ctx.real_swapchain_texture = nullptr;
    ctx.swapchain_texture = nullptr;
    ctx.w = 0;
    ctx.h = 0;
    ctx.use_intermediate = false;

    if (offscreen_target_) {
        ctx.swapchain_texture = offscreen_target_;
        ctx.w = offscreen_width_;
        ctx.h = offscreen_height_;
        if (ctx.debug_rt_enabled) {
            DONG_LOG_DEBUG("[SDLGPUDriver::execute] frame=%llu mode=offscreen viewport=%ux%u", ctx.frame_index, ctx.w, ctx.h);
        }
        return true;
    }

    if (!window_) {
        DONG_LOG_ERROR("SDLGPUDriver::execute: no window for swapchain rendering");
        return false;
    }

    // 关键：不要在这里 AcquireGPUSwapchainTexture。
    // 因为隔离层切换可能会 split/submit command buffer，而 swapchain texture 只能在 acquire 它的 command buffer 上使用。
    // 我们会在 EndPass（blit 到 swapchain 之前）用 *当前* command buffer 再 acquire 一次。
    int win_w = 0;
    int win_h = 0;
    if (!SDL_GetWindowSizeInPixels(window_, &win_w, &win_h) || win_w <= 0 || win_h <= 0) {
        if (!SDL_GetWindowSize(window_, &win_w, &win_h) || win_w <= 0 || win_h <= 0) {
            DONG_LOG_ERROR("SDLGPUDriver::execute: failed to query window size");
            return false;
        }
    }

    ctx.w = static_cast<Uint32>(win_w);
    ctx.h = static_cast<Uint32>(win_h);

    // 创建或复用中间纹理
    if (!intermediate_texture_ || intermediate_width_ != ctx.w || intermediate_height_ != ctx.h) {
        if (intermediate_texture_) {
            SDL_ReleaseGPUTexture(ctx.dev, intermediate_texture_);
            intermediate_texture_ = nullptr;
        }
        intermediate_valid_ = false;

        SDL_GPUTextureCreateInfo tex_info{};
        tex_info.type = SDL_GPU_TEXTURETYPE_2D;
        tex_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
        tex_info.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
        tex_info.width = ctx.w;
        tex_info.height = ctx.h;
        tex_info.layer_count_or_depth = 1;

        tex_info.num_levels = 1;
        tex_info.sample_count = SDL_GPU_SAMPLECOUNT_1;

        intermediate_texture_ = SDL_CreateGPUTexture(ctx.dev, &tex_info);
        if (!intermediate_texture_) {
            DONG_LOG_ERROR("SDLGPUDriver::execute: failed to create intermediate texture: %s", SDL_GetError());
            // 回退：直接渲染到 swapchain（此路径下不允许 split cmd buf）
            Uint32 w = ctx.w;
            Uint32 h = ctx.h;
            if (!SDL_AcquireGPUSwapchainTexture(ctx.cmd_buf, window_, &ctx.real_swapchain_texture, &w, &h)) {
                DONG_LOG_ERROR("SDLGPUDriver::execute: failed to acquire swapchain texture (fallback)");
                return false;
            }
            if (!ctx.real_swapchain_texture) {
                DONG_LOG_DEBUG("[SDLGPUDriver::execute] swapchain texture is null (fallback), skipping frame");
                return false;
            }
            ctx.swapchain_texture = ctx.real_swapchain_texture;
        } else {
            intermediate_width_ = ctx.w;
            intermediate_height_ = ctx.h;
            ctx.swapchain_texture = intermediate_texture_;
            ctx.use_intermediate = true;
            if (ctx.debug_rt_enabled) {
                DONG_LOG_DEBUG("[SDLGPUDriver::execute] frame=%llu created intermediate texture %p size=%ux%u",
                        ctx.frame_index, (void*)intermediate_texture_, ctx.w, ctx.h);
            }
        }
    } else {
        ctx.swapchain_texture = intermediate_texture_;
        ctx.use_intermediate = true;
    }

    if (ctx.debug_rt_enabled) {
        DONG_LOG_DEBUG("[SDLGPUDriver::execute] frame=%llu mode=window viewport=%ux%u swapchain=%p intermediate=%p use_intermediate=%d",
                ctx.frame_index, ctx.w, ctx.h, (void*)ctx.real_swapchain_texture, (void*)intermediate_texture_, ctx.use_intermediate ? 1 : 0);
    }

    return true;
}

void SDLGPUDriver::executePreuploadImages(const GPUCommandList& commands) {
    // Pre-upload images into atlas before any render pass begins.
    // SDL_gpu forbids beginning a copy pass while a render pass is active.
    // If we lazily upload inside DrawImageQuad (which runs inside a render pass), SDL will assert.
    for (const auto& cmd : commands.commands) {
        if (cmd.type != GPUCommandType::DrawImageQuad) {
            continue;
        }
        if (cmd.image_src.empty()) {
            continue;
        }

        // Dynamic textures (e.g. video://...) are updated externally and should not enter the atlas.
        if (cmd.image_src.rfind("video://", 0) == 0) {
            continue;
        }

        ImageAtlasEntry tmp{};
        (void)ensureImageInAtlas(cmd.image_src, tmp);
    }
}

void SDLGPUDriver::executeBeginPass(ExecuteContext& ctx) {
    if (ctx.pass) {
        SDL_EndGPURenderPass(ctx.pass);
        ctx.pass = nullptr;
    }

    ctx.render_target_stack.clear();
    ctx.isolated_layer_stack.clear();
    ctx.clip_stack.clear();

    ctx.render_target_stack.push_back(RenderTargetState{ctx.swapchain_texture, ctx.w, ctx.h, offscreen_target_ == nullptr, 0.0f, 0.0f});

    if (ctx.debug_rt_enabled) {
        DONG_LOG_DEBUG("[SDLGPUDriver::execute] BeginPass frame=%llu swapchain_texture=%p offscreen_target_=%p is_offscreen=%d viewport=%ux%u",
                ctx.frame_index, (void*)ctx.swapchain_texture, (void*)offscreen_target_, offscreen_target_ != nullptr ? 1 : 0, ctx.w, ctx.h);
    }

    ctx.color_target = {};
    ctx.color_target.texture = ctx.swapchain_texture;
    ctx.color_target.mip_level = 0;
    ctx.color_target.layer_or_depth_plane = 0;

    if (offscreen_target_) {
        ctx.color_target.clear_color = SDL_FColor{0.0f, 0.0f, 0.0f, 0.0f};
    } else {
        ctx.color_target.clear_color = SDL_FColor{0.0f, 0.0f, 0.0f, 1.0f};
    }

    ctx.color_target.load_op = SDL_GPU_LOADOP_CLEAR;
    ctx.color_target.store_op = SDL_GPU_STOREOP_STORE;
    ctx.color_target.resolve_texture = nullptr;
    ctx.color_target.resolve_mip_level = 0;
    ctx.color_target.resolve_layer = 0;
    ctx.color_target.cycle = true;
    ctx.color_target.cycle_resolve_texture = false;

    ctx.pass = SDL_BeginGPURenderPass(ctx.cmd_buf, &ctx.color_target, 1, nullptr);
    if (!ctx.pass) {
        DONG_LOG_ERROR("SDLGPUDriver::execute: failed to begin render pass: %s", SDL_GetError());
        ctx.aborted = true;
        return;
    }

    SDL_GPUViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.w = static_cast<float>(ctx.w);
    viewport.h = static_cast<float>(ctx.h);
    viewport.min_depth = 0.0f;
    viewport.max_depth = 1.0f;
    SDL_SetGPUViewport(ctx.pass, &viewport);

    ctx.applyScissor(ctx.pass);
    ctx.pipeline_state.reset();
}

void SDLGPUDriver::executeEndPass(ExecuteContext& ctx) {
    if (ctx.debug_rt_enabled) {
        DONG_LOG_DEBUG("[SDLGPUDriver::execute] frame=%llu EndPass: pass=%p offscreen=%d use_intermediate=%d",
                ctx.frame_index, (void*)ctx.pass, offscreen_target_ != nullptr ? 1 : 0, ctx.use_intermediate ? 1 : 0);
    }

    if (ctx.pass) {
        SDL_EndGPURenderPass(ctx.pass);
        ctx.pass = nullptr;
    }

    // 如果使用了中间纹理，将其 blit 到真正的 swapchain
    if (ctx.use_intermediate && intermediate_texture_) {
        Uint32 sw = ctx.w;
        Uint32 sh = ctx.h;

        if (!offscreen_target_) {
            DONG_LOG_DEBUG("[SDLGPUDriver] EndPass acquiring swapchain (window mode) frame=%llu cmd_buf=%p win_size=%ux%u",
                           ctx.frame_index, (void*)ctx.cmd_buf, ctx.w, ctx.h);

            // swapchain acquire：默认阻塞（确定性更强），但可通过环境变量切到非阻塞以减少卡顿：
            // - DONG_GPU_SWAPCHAIN_NOWAIT=1  => 使用 SDL_AcquireGPUSwapchainTexture；texture==NULL 时跳帧
            // - 未设置                 => 使用 SDL_WaitAndAcquireGPUSwapchainTexture
            static const bool kSwapchainNowait = (std::getenv("DONG_GPU_SWAPCHAIN_NOWAIT") != nullptr);
            if (!kSwapchainNowait) {
                DONG_PROFILE_SCOPE_CAT("SDL_WaitAndAcquireGPUSwapchainTexture", "gpu");
                if (!SDL_WaitAndAcquireGPUSwapchainTexture(ctx.cmd_buf, window_, &ctx.real_swapchain_texture, &sw, &sh)) {
                    DONG_LOG_ERROR("SDLGPUDriver::execute: failed to wait+acquire swapchain texture at EndPass");
                    return;
                }
            } else {
                DONG_PROFILE_SCOPE_CAT("SDL_AcquireGPUSwapchainTexture", "gpu");
                if (!SDL_AcquireGPUSwapchainTexture(ctx.cmd_buf, window_, &ctx.real_swapchain_texture, &sw, &sh)) {
                    DONG_LOG_ERROR("SDLGPUDriver::execute: failed to acquire swapchain texture at EndPass");
                    return;
                }
            }

            if (!ctx.real_swapchain_texture) {
                // Swapchain 不可用（窗口最小化或未准备好），静默跳过此帧
                return;
            }

            DONG_LOG_DEBUG("[SDLGPUDriver] EndPass acquired swapchain (window mode) frame=%llu swapchain_tex=%p swapchain_size=%ux%u",
                           ctx.frame_index, (void*)ctx.real_swapchain_texture, sw, sh);
        }

        if (!ctx.real_swapchain_texture) {
            return;
        }

        if (ctx.debug_rt_enabled) {
            DONG_LOG_DEBUG("[SDLGPUDriver::execute] frame=%llu EndPass: blit intermediate=%p to swapchain=%p",
                    ctx.frame_index, (void*)intermediate_texture_, (void*)ctx.real_swapchain_texture);
        }

        // 只要成功走到 EndPass 并准备 blit，就认为 intermediate 有了可复用的有效内容。
        intermediate_valid_ = true;

        const Uint32 src_w = ctx.w;
        const Uint32 src_h = ctx.h;
        const Uint32 dst_w = sw;
        const Uint32 dst_h = sh;
        if (src_w == 0 || src_h == 0 || dst_w == 0 || dst_h == 0) {
            return;
        }

        if (ctx.debug_rt_enabled && (src_w != dst_w || src_h != dst_h)) {
            DONG_LOG_DEBUG("[SDLGPUDriver::execute] EndPass: blit scale src=%ux%u dst=%ux%u", src_w, src_h, dst_w, dst_h);
        }

        SDL_GPUBlitInfo blit_info{};
        blit_info.source.texture = intermediate_texture_;
        blit_info.source.mip_level = 0;
        blit_info.source.layer_or_depth_plane = 0;
        blit_info.source.x = 0;
        blit_info.source.y = 0;
        blit_info.source.w = src_w;
        blit_info.source.h = src_h;

        blit_info.destination.texture = ctx.real_swapchain_texture;
        blit_info.destination.mip_level = 0;
        blit_info.destination.layer_or_depth_plane = 0;
        blit_info.destination.x = 0;
        blit_info.destination.y = 0;
        blit_info.destination.w = dst_w;
        blit_info.destination.h = dst_h;

        blit_info.load_op = SDL_GPU_LOADOP_DONT_CARE;
        blit_info.filter = (src_w != dst_w || src_h != dst_h) ? SDL_GPU_FILTER_LINEAR : SDL_GPU_FILTER_NEAREST;
        blit_info.cycle = true;

        SDL_BlitGPUTexture(ctx.cmd_buf, &blit_info);
    }
}

void SDLGPUDriver::executePushClipRect(ExecuteContext& ctx, const GPUCommand& cmd) {
    DONG_LOG_DEBUG("[GPU Execute] ENTER PushClipRect case");
    SDL_Rect clip = to_sdl_rect(cmd.rect);
    DONG_LOG_DEBUG("[GPU] PushClipRect: x=%.1f y=%.1f w=%.1f h=%.1f -> SDL x=%d y=%d w=%d h=%d",
                   cmd.rect.x, cmd.rect.y, cmd.rect.width, cmd.rect.height,
                   clip.x, clip.y, clip.w, clip.h);
    if (!ctx.clip_stack.empty()) {
        clip = intersect_rect(ctx.clip_stack.back().scissor, clip);
    }

    ClipStackEntry entry{};
    entry.scissor = clip;
    if (cmd.radius > 0.0f) {
        entry.has_rounded = true;
        entry.rounded_rect = cmd.rect;
        entry.rounded_radius = cmd.radius;
    }

    ctx.clip_stack.push_back(entry);
    ctx.applyScissor(ctx.pass);
}

void SDLGPUDriver::executePopClip(ExecuteContext& ctx) {
    if (!ctx.clip_stack.empty()) {
        ctx.clip_stack.pop_back();
    }
    ctx.applyScissor(ctx.pass);
}

void SDLGPUDriver::executeBeginIsolatedLayer(ExecuteContext& ctx, const GPUCommand& cmd) {
    if (ctx.render_target_stack.empty()) {
        DONG_LOG_ERROR("SDLGPUDriver::execute: BeginIsolatedLayer without active render target");
        return;
    }

    if (ctx.debug_rt_enabled) {
        DONG_LOG_DEBUG("[SDLGPUDriver::execute] BeginIsolatedLayer frame=%llu layer_id=%llu layer_dirty=%d skip_depth=%d",
                ctx.frame_index,
                static_cast<unsigned long long>(cmd.layer_id),
                cmd.layer_dirty ? 1 : 0,
                ctx.skip_draw_depth);
        ctx.logRenderTargetStack("  RT before BeginIsolatedLayer");
    }

    if (!ctx.dev) {
        DONG_LOG_ERROR("SDLGPUDriver::execute: no GPU device for isolated layer");
        return;
    }

    const uint64_t layer_id = cmd.layer_id;
    const bool layer_dirty = cmd.layer_dirty;

    // 重要：隔离图层纹理尺寸应当尽可能紧贴 layer bounds。
    // 旧实现使用父 render target 尺寸（通常是整屏）会导致每个 transform 元素都创建/渲染整屏离屏纹理，切到 Transform 页面会瞬间爆卡。
    Uint32 target_w = static_cast<Uint32>(std::max(1.0f, std::ceil(cmd.rect.width)));
    Uint32 target_h = static_cast<Uint32>(std::max(1.0f, std::ceil(cmd.rect.height)));

    // 优先检查是否有有效的缓存可以复用（无论 layer_dirty 标志如何）
    LayerRenderTarget* cache_entry = nullptr;
    if (layer_id != 0) {
        for (auto& entry : layer_render_targets_) {
            if (entry.layer_id == layer_id && entry.texture &&
                entry.valid_for_cache && entry.width == target_w && entry.height == target_h) {
                cache_entry = &entry;
                break;
            }
        }
    }

    IsolatedLayerState layer_state{};
    layer_state.bounds = cmd.rect;
    layer_state.opacity = cmd.layer_opacity;
    layer_state.id = layer_id;
    layer_state.dirty = layer_dirty;
    for (int i = 0; i < 6; ++i) {
        layer_state.composite_transform[i] = cmd.layer_transform[i];
        layer_state.draw_transform[i] = 0.0f;
    }
    layer_state.draw_transform[0] = 1.0f;
    layer_state.draw_transform[4] = 1.0f;

    // 把"屏幕坐标系"的绘制命令平移到离屏纹理的局部坐标系（0,0）。
    // 这样可以让 layer 纹理只覆盖自身 bounds，而不是整屏。
    layer_state.draw_transform[2] = -layer_state.bounds.x;
    layer_state.draw_transform[5] = -layer_state.bounds.y;

    layer_state.scroll[0] = cmd.layer_scroll[0];
    layer_state.scroll[1] = cmd.layer_scroll[1];

    // 只有当图层非脏且有有效缓存时才复用缓存
    if (ctx.layer_cache_enabled && !layer_dirty && cache_entry && cache_entry->texture) {
        if (ctx.debug_log_layer_cache) {
            ++ctx.debug_layer_cache_reused;
            DONG_LOG_DEBUG("[SDLGPUDriver layer-cache] frame=%llu reuse layer id=%llu size=%ux%u bounds=(%.1f,%.1f,%.1f,%.1f)",
                    ctx.frame_index,
                    static_cast<unsigned long long>(layer_id),
                    static_cast<unsigned int>(cache_entry->width),
                    static_cast<unsigned int>(cache_entry->height),
                    layer_state.bounds.x,
                    layer_state.bounds.y,
                    layer_state.bounds.width,
                    layer_state.bounds.height);
        }
        layer_state.texture = cache_entry->texture;
        layer_state.width = cache_entry->width;
        layer_state.height = cache_entry->height;
        layer_state.dirty = false;

        // 关键修复：非脏图层复用缓存时，把缓存内容提前在 Begin 处合成到当前 render target。
        // 否则若该图层内部存在"子隔离图层"（例如 video layer 每帧重栅格），
        // 原本在 EndIsolatedLayer 再合成缓存会把子层的新内容覆盖掉，表现为视频冻结。
        if (ctx.pass && image_pipeline_ && image_sampler_ && layer_state.bounds.width > 0.0f && layer_state.bounds.height > 0.0f) {
            struct LayerCompositeUniforms {
                float rect[4];
                float uv_rect[4];
                float viewport[4];
                float transform[8];
                float tint[4];
                ClipUniformBlock clip;
            };

            LayerCompositeUniforms u{};
            u.rect[0] = layer_state.bounds.x;
            u.rect[1] = layer_state.bounds.y;
            u.rect[2] = layer_state.bounds.width;
            u.rect[3] = layer_state.bounds.height;

            u.uv_rect[0] = 0.0f;
            u.uv_rect[1] = 0.0f;
            u.uv_rect[2] = 1.0f;
            u.uv_rect[3] = 1.0f;

            ctx.writeViewport(u.viewport);
            writeTransform(u.transform, layer_state.composite_transform);

            u.tint[0] = 1.0f;
            u.tint[1] = 1.0f;
            u.tint[2] = 1.0f;
            u.tint[3] = layer_state.opacity;
            ctx.fillClipUniform(u.clip);

            SDL_PushGPUVertexUniformData(ctx.cmd_buf, 0, &u, sizeof(u));
            SDL_PushGPUFragmentUniformData(ctx.cmd_buf, 0, &u, sizeof(u));

            SDL_BindGPUGraphicsPipeline(ctx.pass, image_pipeline_);

            SDL_GPUTextureSamplerBinding binding{};
            binding.texture = layer_state.texture;
            binding.sampler = image_sampler_;
            SDL_BindGPUFragmentSamplers(ctx.pass, 0, &binding, 1);

            SDL_DrawGPUPrimitives(ctx.pass, 4, 1, 0, 0);
            layer_state.cache_composited = true;
        }

        ctx.isolated_layer_stack.push_back(layer_state);
        ++ctx.skip_draw_depth;
        return;
    }

    // 脏图层：需要重新栅格，切换到离屏纹理
    // 注意：外层若是"非脏复用缓存"的隔离图层，会把 skip_draw_depth>0。
    // 但我们仍然需要在该脏图层内部执行绘制（尤其是嵌套的 video layer）。
    // 因此进入脏图层时暂存并清零 skip_draw_depth，等离开该脏图层时再恢复。
    if (ctx.skip_draw_depth > 0) {
        layer_state.saved_skip_draw_depth = ctx.skip_draw_depth;
        layer_state.restore_skip_draw_depth = true;
        ctx.skip_draw_depth = 0;
    }

    if (ctx.pass) {
        SDL_EndGPURenderPass(ctx.pass);
        ctx.pass = nullptr;
    }

    if (ctx.split_cmd_buf_for_isolated_layers) {
        DONG_LOG_DEBUG("[SDLGPUDriver] Split command buffer for isolated layer frame=%llu where=BeginIsolatedLayer(before_layer_pass) offscreen=%d cmd_buf_old=%p",
                       ctx.frame_index, offscreen_target_ ? 1 : 0, (void*)ctx.cmd_buf);

        if ((offscreen_target_ || ctx.use_intermediate) && gpu_device_ && ctx.cmd_buf) {
            gpu_device_->submitCommandBuffer(ctx.cmd_buf);
            ctx.cmd_buf = gpu_device_->acquireCommandBuffer();
            if (!ctx.cmd_buf) {
                DONG_LOG_ERROR("SDLGPUDriver::execute: failed to acquire command buffer after split");
                ctx.aborted = true;
                return;
            }
        }

        DONG_LOG_DEBUG("[SDLGPUDriver] Split command buffer done frame=%llu where=BeginIsolatedLayer(before_layer_pass) cmd_buf_new=%p",
                       ctx.frame_index, (void*)ctx.cmd_buf);
    }

    SDL_GPUTexture* layer_texture = nullptr;

    // 优先复用同一 layer_id 的缓存纹理
    if (layer_id != 0) {
        for (auto& entry : layer_render_targets_) {
            if (entry.layer_id == layer_id && entry.texture && !entry.in_use) {
                if (entry.width != target_w || entry.height != target_h) {
                    SDL_ReleaseGPUTexture(ctx.dev, entry.texture);
                    entry.texture = nullptr;
                    entry.valid_for_cache = false;
                    entry.width = target_w;
                    entry.height = target_h;
                }
                if (!entry.texture) {
                    SDL_GPUTextureCreateInfo tex_info{};
                    tex_info.type = SDL_GPU_TEXTURETYPE_2D;
                    tex_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
                    tex_info.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
                    tex_info.width = target_w;
                    tex_info.height = target_h;
                    tex_info.layer_count_or_depth = 1;

                    tex_info.num_levels = 1;
                    tex_info.sample_count = SDL_GPU_SAMPLECOUNT_1;
                    entry.texture = SDL_CreateGPUTexture(ctx.dev, &tex_info);
                    if (!entry.texture) {
                        DONG_LOG_ERROR("SDLGPUDriver::execute: failed to recreate layer texture: %s", SDL_GetError());
                        break;
                    }
                }
                entry.in_use = true;
                entry.valid_for_cache = false;
                layer_texture = entry.texture;
                cache_entry = &entry;
                break;
            }
        }
    }

    // 其次复用未绑定 layer_id 的空闲纹理
    if (!layer_texture) {
        for (auto& entry : layer_render_targets_) {
            if (entry.layer_id == 0 && !entry.in_use && entry.texture &&
                entry.width == target_w && entry.height == target_h) {
                entry.in_use = true;
                entry.valid_for_cache = false;
                entry.layer_id = layer_id;
                layer_texture = entry.texture;
                cache_entry = &entry;
                break;
            }
        }
    }

    // 都没有的话，新建一个并加入池
    if (!layer_texture) {
        SDL_GPUTextureCreateInfo tex_info{};
        tex_info.type = SDL_GPU_TEXTURETYPE_2D;
        tex_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
        tex_info.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
        tex_info.width = target_w;
        tex_info.height = target_h;
        tex_info.layer_count_or_depth = 1;

        tex_info.num_levels = 1;
        tex_info.sample_count = SDL_GPU_SAMPLECOUNT_1;

        layer_texture = SDL_CreateGPUTexture(ctx.dev, &tex_info);
        if (!layer_texture) {
            DONG_LOG_ERROR("SDLGPUDriver::execute: failed to create layer texture: %s", SDL_GetError());
            return;
        }

        LayerRenderTarget entry{};
        entry.texture = layer_texture;
        entry.width = target_w;
        entry.height = target_h;
        entry.layer_id = layer_id;
        entry.in_use = true;
        entry.valid_for_cache = false;
        layer_render_targets_.push_back(entry);
        cache_entry = &layer_render_targets_.back();
    }

    if (ctx.debug_log_layer_cache) {
        ++ctx.debug_layer_cache_rasterized;
        DONG_LOG_DEBUG("[SDLGPUDriver layer-cache] frame=%llu rasterize layer id=%llu size=%ux%u bounds=(%.1f,%.1f,%.1f,%.1f)",
                ctx.frame_index,
                static_cast<unsigned long long>(layer_id),
                static_cast<unsigned int>(target_w),
                static_cast<unsigned int>(target_h),
                layer_state.bounds.x,
                layer_state.bounds.y,
                layer_state.bounds.width,
                layer_state.bounds.height);
    }

    ctx.render_target_stack.push_back(RenderTargetState{layer_texture, target_w, target_h, false, cmd.rect.x, cmd.rect.y});

    SDL_GPUColorTargetInfo layer_target{};
    layer_target.texture = layer_texture;
    layer_target.mip_level = 0;
    layer_target.layer_or_depth_plane = 0;
    layer_target.clear_color = SDL_FColor{0.0f, 0.0f, 0.0f, 0.0f};
    layer_target.load_op = SDL_GPU_LOADOP_CLEAR;
    layer_target.store_op = SDL_GPU_STOREOP_STORE;

    ctx.pass = SDL_BeginGPURenderPass(ctx.cmd_buf, &layer_target, 1, nullptr);
    if (!ctx.pass) {
        DONG_LOG_ERROR("SDLGPUDriver::execute: failed to begin layer pass: %s", SDL_GetError());
        SDL_ReleaseGPUTexture(ctx.dev, layer_texture);
        ctx.render_target_stack.pop_back();
        if (cache_entry) {
            cache_entry->texture = nullptr;
            cache_entry->in_use = false;
            cache_entry->valid_for_cache = false;
            cache_entry->layer_id = 0;
        }
        return;
    }

    SDL_GPUViewport layer_viewport{};
    layer_viewport.x = 0.0f;
    layer_viewport.y = 0.0f;
    layer_viewport.w = static_cast<float>(target_w);
    layer_viewport.h = static_cast<float>(target_h);
    layer_viewport.min_depth = 0.0f;
    layer_viewport.max_depth = 1.0f;
    SDL_SetGPUViewport(ctx.pass, &layer_viewport);

    ctx.applyScissor(ctx.pass);
    ctx.pipeline_state.reset();

    layer_state.dirty = true;
    layer_state.texture = layer_texture;
    layer_state.width = target_w;
    layer_state.height = target_h;
    ctx.isolated_layer_stack.push_back(layer_state);
}

void SDLGPUDriver::executeEndIsolatedLayer(ExecuteContext& ctx, const GPUCommand&) {
    if (ctx.isolated_layer_stack.empty()) {
        DONG_LOG_ERROR("SDLGPUDriver::execute: EndIsolatedLayer without matching layer");
        return;
    }

    IsolatedLayerState layer_info = ctx.isolated_layer_stack.back();
    ctx.isolated_layer_stack.pop_back();

    if (layer_info.restore_skip_draw_depth) {
        ctx.skip_draw_depth = layer_info.saved_skip_draw_depth;
    }

    // 非脏图层：复用缓存，之前没有切换 render target
    if (!layer_info.dirty) {
        if (ctx.skip_draw_depth > 0) {
            --ctx.skip_draw_depth;
        }

        // 如果在 BeginIsolatedLayer 已经提前把缓存内容合成过，这里无需再合成。
        // 这样可以保证该图层内部的子隔离图层（例如视频）后绘制的内容不会被父缓存覆盖。
        if (layer_info.cache_composited) {
            return;
        }

        SDL_GPUTexture* layer_texture = layer_info.texture;
        if (!ctx.pass || !image_pipeline_ || !image_sampler_ || !layer_texture) {
            return;
        }
        if (layer_info.bounds.width <= 0.0f || layer_info.bounds.height <= 0.0f) {
            return;
        }

        struct LayerCompositeUniforms {
            float rect[4];
            float uv_rect[4];
            float viewport[4];
            float transform[8];
            float tint[4];
            ClipUniformBlock clip;
        };

        LayerCompositeUniforms u{};
        u.rect[0] = layer_info.bounds.x;
        u.rect[1] = layer_info.bounds.y;
        u.rect[2] = layer_info.bounds.width;
        u.rect[3] = layer_info.bounds.height;

        // 紧凑纹理：整张 texture 就是 layer 的内容，因此直接采样全域。
        u.uv_rect[0] = 0.0f;
        u.uv_rect[1] = 0.0f;
        u.uv_rect[2] = 1.0f;
        u.uv_rect[3] = 1.0f;

        ctx.writeViewport(u.viewport);
        writeTransform(u.transform, layer_info.composite_transform);

        u.tint[0] = 1.0f;
        u.tint[1] = 1.0f;
        u.tint[2] = 1.0f;
        u.tint[3] = layer_info.opacity;
        ctx.fillClipUniform(u.clip);

        SDL_PushGPUVertexUniformData(ctx.cmd_buf, 0, &u, sizeof(u));
        SDL_PushGPUFragmentUniformData(ctx.cmd_buf, 0, &u, sizeof(u));

        SDL_BindGPUGraphicsPipeline(ctx.pass, image_pipeline_);

        SDL_GPUTextureSamplerBinding binding{};
        binding.texture = layer_texture;
        binding.sampler = image_sampler_;
        SDL_BindGPUFragmentSamplers(ctx.pass, 0, &binding, 1);

        SDL_DrawGPUPrimitives(ctx.pass, 4, 1, 0, 0);
        return;
    }

    // 脏图层：结束子 pass 并合成到父 render target
    if (ctx.render_target_stack.size() <= 1) {
        DONG_LOG_ERROR("SDLGPUDriver::execute: EndIsolatedLayer without matching layer render target");
        return;
    }

    if (ctx.pass) {
        SDL_EndGPURenderPass(ctx.pass);
        ctx.pass = nullptr;
    }

    if (ctx.split_cmd_buf_for_isolated_layers) {
        DONG_LOG_DEBUG("[SDLGPUDriver] Split command buffer for isolated layer frame=%llu where=EndIsolatedLayer(before_parent_pass) offscreen=%d cmd_buf_old=%p",
                       ctx.frame_index, offscreen_target_ ? 1 : 0, (void*)ctx.cmd_buf);

        if ((offscreen_target_ || ctx.use_intermediate) && gpu_device_ && ctx.cmd_buf) {
            gpu_device_->submitCommandBuffer(ctx.cmd_buf);
            ctx.cmd_buf = gpu_device_->acquireCommandBuffer();
            if (!ctx.cmd_buf) {
                DONG_LOG_ERROR("SDLGPUDriver::execute: failed to acquire command buffer after split");
                ctx.aborted = true;
                return;
            }
        }

        DONG_LOG_DEBUG("[SDLGPUDriver] Split command buffer done frame=%llu where=EndIsolatedLayer(before_parent_pass) cmd_buf_new=%p",
                       ctx.frame_index, (void*)ctx.cmd_buf);
    }

    RenderTargetState child_target = ctx.render_target_stack.back();
    ctx.render_target_stack.pop_back();
    RenderTargetState& parent_target = ctx.render_target_stack.back();

    if (ctx.debug_rt_enabled) {
        DONG_LOG_DEBUG("[SDLGPUDriver::execute] EndIsolatedLayer frame=%llu layer_id=%llu dirty=%d compositing child_rt=%p -> parent_rt=%p",
                ctx.frame_index,
                static_cast<unsigned long long>(layer_info.id),
                layer_info.dirty ? 1 : 0,
                (void*)child_target.texture,
                (void*)parent_target.texture);
        ctx.logRenderTargetStack("  RT at EndIsolatedLayer");
    }

    SDL_GPUColorTargetInfo color_target{};
    color_target.texture = parent_target.texture;
    color_target.mip_level = 0;
    color_target.layer_or_depth_plane = 0;
    color_target.load_op = SDL_GPU_LOADOP_LOAD;
    color_target.store_op = SDL_GPU_STOREOP_STORE;

    ctx.pass = SDL_BeginGPURenderPass(ctx.cmd_buf, &color_target, 1, nullptr);
    if (!ctx.pass) {
        DONG_LOG_ERROR("SDLGPUDriver::execute: failed to begin parent render pass for EndIsolatedLayer");
        return;
    }

    SDL_GPUViewport parent_viewport{};
    parent_viewport.x = 0.0f;
    parent_viewport.y = 0.0f;
    parent_viewport.w = static_cast<float>(parent_target.width);
    parent_viewport.h = static_cast<float>(parent_target.height);
    parent_viewport.min_depth = 0.0f;
    parent_viewport.max_depth = 1.0f;
    SDL_SetGPUViewport(ctx.pass, &parent_viewport);

    ctx.applyScissor(ctx.pass);
    ctx.pipeline_state.reset();

    if (!image_pipeline_ || !image_sampler_ || !child_target.texture) {
        return;
    }

    if (layer_info.bounds.width <= 0.0f || layer_info.bounds.height <= 0.0f) {
        return;
    }

    struct LayerCompositeUniforms {
        float rect[4];
        float uv_rect[4];
        float viewport[4];
        float transform[8];
        float tint[4];
        ClipUniformBlock clip;
    };

    LayerCompositeUniforms u{};
    u.rect[0] = layer_info.bounds.x;
    u.rect[1] = layer_info.bounds.y;
    u.rect[2] = layer_info.bounds.width;
    u.rect[3] = layer_info.bounds.height;

    // 紧凑纹理：整张 texture 就是 layer 的内容，因此直接采样全域。
    u.uv_rect[0] = 0.0f;
    u.uv_rect[1] = 0.0f;
    u.uv_rect[2] = 1.0f;
    u.uv_rect[3] = 1.0f;

    ctx.writeViewport(u.viewport);
    writeTransform(u.transform, layer_info.composite_transform);

    u.tint[0] = 1.0f;
    u.tint[1] = 1.0f;
    u.tint[2] = 1.0f;
    u.tint[3] = layer_info.opacity;
    ctx.fillClipUniform(u.clip);

    SDL_PushGPUVertexUniformData(ctx.cmd_buf, 0, &u, sizeof(u));
    SDL_PushGPUFragmentUniformData(ctx.cmd_buf, 0, &u, sizeof(u));

    SDL_BindGPUGraphicsPipeline(ctx.pass, image_pipeline_);

    SDL_GPUTextureSamplerBinding binding{};
    binding.texture = child_target.texture;
    binding.sampler = image_sampler_;
    SDL_BindGPUFragmentSamplers(ctx.pass, 0, &binding, 1);

    SDL_DrawGPUPrimitives(ctx.pass, 4, 1, 0, 0);

    // 合成完成后，标记缓存可复用
    if (ctx.layer_cache_enabled && layer_info.id != 0 && child_target.texture) {
        for (auto& entry : layer_render_targets_) {
            if (entry.layer_id == layer_info.id && entry.texture == child_target.texture) {
                entry.valid_for_cache = true;
                break;
            }
        }
    }
}

void SDLGPUDriver::executeDrawRect(ExecuteContext& ctx, const GPUCommand& cmd) {
    if (!ctx.pass || !rect_pipeline_) {
        DONG_LOG_DEBUG("[RECT] SKIP: pass=%p rect_pipeline_=%p", (void*)ctx.pass, (void*)rect_pipeline_);
        return;
    }

    struct RectUniformData {
        float rect[4];
        float color[4];
        float viewport[4];
        float transform[8];
        ClipUniformBlock clip;
    };

    RectUniformData u{};
    u.rect[0] = cmd.rect.x;
    u.rect[1] = cmd.rect.y;
    u.rect[2] = cmd.rect.width;
    u.rect[3] = cmd.rect.height;

    writeLinearColor(cmd.color, u.color);

    ctx.writeViewport(u.viewport);
    writeTransform(u.transform, ctx.getCurrentTransform());
    ctx.fillClipUniform(u.clip);

    SDL_PushGPUVertexUniformData(ctx.cmd_buf, 0, &u, sizeof(u));
    SDL_PushGPUFragmentUniformData(ctx.cmd_buf, 0, &u, sizeof(u));

    if (ctx.pipeline_state.active != PipelineBindingState::ActivePipeline::Rect) {
        SDL_BindGPUGraphicsPipeline(ctx.pass, rect_pipeline_);
        ctx.pipeline_state.active = PipelineBindingState::ActivePipeline::Rect;
    }

    SDL_DrawGPUPrimitives(ctx.pass, 4, 1, 0, 0);
}

void SDLGPUDriver::executeDrawRoundedRect(ExecuteContext& ctx, const GPUCommand& cmd) {
    DONG_LOG_DEBUG("[GPU_CMD] DrawRoundedRectQuad: rect=(%.1f,%.1f,%.1f,%.1f)",
                   cmd.rect.x, cmd.rect.y, cmd.rect.width, cmd.rect.height);

    if (!ctx.pass || !round_rect_pipeline_) {
        DONG_LOG_DEBUG("[ROUND_RECT] SKIP: pass=%p round_rect_pipeline_=%p", (void*)ctx.pass, (void*)round_rect_pipeline_);
        return;
    }

    DONG_LOG_DEBUG("[ROUND_RECT] DRAW: rect=(%.1f,%.1f,%.1f,%.1f) radius=%.1f color=(%.3f,%.3f,%.3f,%.3f)",
                   cmd.rect.x, cmd.rect.y, cmd.rect.width, cmd.rect.height, cmd.radius,
                   cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a);

    struct RoundRectUniformData {
        float rect[4];
        float radius[4];
        float viewport[4];
        float transform[8];
        float color[4];
        ClipUniformBlock clip;
    };

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
    writeTransform(u.transform, ctx.getCurrentTransform());
    writeLinearColor(cmd.color, u.color);
    ctx.fillClipUniform(u.clip);

    SDL_PushGPUVertexUniformData(ctx.cmd_buf, 0, &u, sizeof(u));
    SDL_PushGPUFragmentUniformData(ctx.cmd_buf, 0, &u, sizeof(u));

    if (ctx.pipeline_state.active != PipelineBindingState::ActivePipeline::RoundRect) {
        SDL_BindGPUGraphicsPipeline(ctx.pass, round_rect_pipeline_);
        ctx.pipeline_state.active = PipelineBindingState::ActivePipeline::RoundRect;
    }

    SDL_DrawGPUPrimitives(ctx.pass, 4, 1, 0, 0);
}

void SDLGPUDriver::executeDrawShadow(ExecuteContext& ctx, const GPUCommand& cmd) {
    if (!ctx.pass || !shadow_pipeline_) {
        return;
    }

    struct ShadowUniformData {
        float rect[4];
        float radius[4];
        float viewport[4];
        float transform[8];
        float color[4];
        ClipUniformBlock clip;
    };

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
    writeTransform(u.transform, ctx.getCurrentTransform());
    writeLinearColor(cmd.color, u.color);
    ctx.fillClipUniform(u.clip);

    SDL_PushGPUVertexUniformData(ctx.cmd_buf, 0, &u, sizeof(u));
    SDL_PushGPUFragmentUniformData(ctx.cmd_buf, 0, &u, sizeof(u));

    if (ctx.pipeline_state.active != PipelineBindingState::ActivePipeline::Shadow) {
        SDL_BindGPUGraphicsPipeline(ctx.pass, shadow_pipeline_);
        ctx.pipeline_state.active = PipelineBindingState::ActivePipeline::Shadow;
    }

    SDL_DrawGPUPrimitives(ctx.pass, 4, 1, 0, 0);
}

void SDLGPUDriver::executeDrawImage(ExecuteContext& ctx, const GPUCommand& cmd) {
    if (!ctx.pass || !image_pipeline_ || !image_sampler_) {
        return;
    }
    if (cmd.image_src.empty()) {
        return;
    }

    // Resolve texture source:
    // - "video://..." : external dynamic texture uploaded by View
    // - otherwise      : static image in atlas
    SDL_GPUTexture* src_texture = nullptr;
    SDL_GPUTexture* video_tex_u = nullptr;
    SDL_GPUTexture* video_tex_v = nullptr;
    bool is_video_yuv = false;
    ImageAtlasEntry entry{};

    if (cmd.image_src.rfind("video://", 0) == 0) {
        auto it = external_images_.find(cmd.image_src);
        if (it == external_images_.end()) {
            return;
        }
        const ExternalImage& ex = it->second;
        if (ex.format == ExternalImageFormat::YUV420P) {
            if (!ex.texture || !ex.texture_u || !ex.texture_v || !video_yuv_pipeline_) {
                return;
            }
            // For YUV, src_texture holds Y for convenience; U/V stored separately.
            src_texture = ex.texture;
            video_tex_u = ex.texture_u;
            video_tex_v = ex.texture_v;
            is_video_yuv = true;
        } else {
            if (!ex.texture) {
                return;
            }
            src_texture = ex.texture;
        }

        entry.u0 = 0.0f;
        entry.v0 = 0.0f;
        entry.u1 = 1.0f;
        entry.v1 = 1.0f;
        entry.width = ex.width;
        entry.height = ex.height;
    } else {
        // Get texture from ImageAtlas
        SDL_GPUTexture* atlas_texture = static_cast<SDL_GPUTexture*>(dong_atlas_get_texture(image_atlas_, 0));
        if (!atlas_texture) {
            return;
        }
        if (!ensureImageInAtlas(cmd.image_src, entry)) {
            return;
        }
        src_texture = atlas_texture;
    }

    struct ImageUniformData {
        float rect[4];
        float uv_rect[4];
        float viewport[4];
        float transform[8];
        float tint[4];
        ClipUniformBlock clip;
    };

    ImageUniformData u{};

    const float img_w = static_cast<float>(entry.width);
    const float img_h = static_cast<float>(entry.height);
    const float dst_w = cmd.rect.width;
    const float dst_h = cmd.rect.height;

    float draw_x = cmd.rect.x;
    float draw_y = cmd.rect.y;
    float draw_w = dst_w;
    float draw_h = dst_h;

    if (img_w > 0.0f && img_h > 0.0f && dst_w > 0.0f && dst_h > 0.0f) {
        const float scale_x = dst_w / img_w;
        const float scale_y = dst_h / img_h;

        if (cmd.image_fit == ImageFitMode::Contain) {
            const float scale = (scale_x < scale_y) ? scale_x : scale_y;
            draw_w = img_w * scale;
            draw_h = img_h * scale;
            const float offset_x = (dst_w - draw_w) * 0.5f;
            const float offset_y = (dst_h - draw_h) * 0.5f;
            draw_x = cmd.rect.x + offset_x;
            draw_y = cmd.rect.y + offset_y;
        } else if (cmd.image_fit == ImageFitMode::Cover) {
            const float scale = (scale_x > scale_y) ? scale_x : scale_y;
            draw_w = img_w * scale;
            draw_h = img_h * scale;
            const float offset_x = (dst_w - draw_w) * 0.5f;
            const float offset_y = (dst_h - draw_h) * 0.5f;
            draw_x = cmd.rect.x + offset_x;
            draw_y = cmd.rect.y + offset_y;
        }
        // Fill: keep cmd.rect as-is.
    }

    u.rect[0] = draw_x;
    u.rect[1] = draw_y;
    u.rect[2] = draw_w;
    u.rect[3] = draw_h;

    u.uv_rect[0] = entry.u0;
    u.uv_rect[1] = entry.v0;
    u.uv_rect[2] = entry.u1;
    u.uv_rect[3] = entry.v1;

    ctx.writeViewport(u.viewport);
    writeTransform(u.transform, ctx.getCurrentTransform());

    u.tint[0] = 1.0f;
    u.tint[1] = 1.0f;
    u.tint[2] = 1.0f;
    u.tint[3] = cmd.opacity;
    ctx.fillClipUniform(u.clip);

    SDL_PushGPUVertexUniformData(ctx.cmd_buf, 0, &u, sizeof(u));
    SDL_PushGPUFragmentUniformData(ctx.cmd_buf, 0, &u, sizeof(u));

    SDL_GPUGraphicsPipeline* pipeline = image_pipeline_;
    PipelineBindingState::ActivePipeline desired = PipelineBindingState::ActivePipeline::Image;
    if (is_video_yuv) {
        pipeline = video_yuv_pipeline_;
        desired = PipelineBindingState::ActivePipeline::ImageYUV;
    }

    if (ctx.pipeline_state.active != desired) {
        SDL_BindGPUGraphicsPipeline(ctx.pass, pipeline);
        ctx.pipeline_state.active = desired;
    }

    if (is_video_yuv) {
        SDL_GPUTextureSamplerBinding bindings[3] = {};
        bindings[0].texture = src_texture; // Y
        bindings[0].sampler = image_sampler_;
        bindings[1].texture = video_tex_u;
        bindings[1].sampler = image_sampler_;
        bindings[2].texture = video_tex_v;
        bindings[2].sampler = image_sampler_;
        SDL_BindGPUFragmentSamplers(ctx.pass, 0, bindings, 3);
    } else {
        SDL_GPUTextureSamplerBinding binding{};
        binding.texture = src_texture;
        binding.sampler = image_sampler_;
        SDL_BindGPUFragmentSamplers(ctx.pass, 0, &binding, 1);
    }

    ctx.pipeline_state.image_sampler_bound = true;
    ctx.pipeline_state.text_sampler_bound = false;

    SDL_DrawGPUPrimitives(ctx.pass, 4, 1, 0, 0);
}

void SDLGPUDriver::executeDrawText(ExecuteContext& ctx, const GPUCommand& cmd) {
    if (!ctx.pass || !text_pipeline_ || glyph_atlas_tiers_.empty() || cmd.glyphs.empty()) {
        DONG_LOG_WARN("[DrawText] EARLY EXIT: pass=%p text_pipeline=%p tiers_empty=%d glyphs_empty=%d",
                      (void*)ctx.pass,
                      (void*)text_pipeline_,
                      glyph_atlas_tiers_.empty() ? 1 : 0,
                      cmd.glyphs.empty() ? 1 : 0);
        return;
    }

    DONG_LOG_DEBUG("[DrawText] frame=%llu glyphs_count=%zu baseline=(%.2f,%.2f) font_size=%.1f has_shadow=%d",
                   ctx.frame_index,
                   cmd.glyphs.size(),
                   cmd.baseline_x,
                   cmd.baseline_y,
                   cmd.font_size,
                   cmd.has_text_shadow ? 1 : 0);

    std::string resolved_primary_font_path;
    const std::string* default_font_path = nullptr;

    if (!cmd.font_paths.empty()) {
        default_font_path = &cmd.font_paths[0];
    } else if (!cmd.font_path.empty()) {
        default_font_path = &cmd.font_path;
    } else {
        resolved_primary_font_path = resolveFontPath(cmd.font_family, cmd.font_weight, cmd.font_style);
        default_font_path = &resolved_primary_font_path;
    }

    if (!default_font_path || default_font_path->empty()) {
        DONG_LOG_WARN("SDLGPUDriver: no valid font found for family '%s'", cmd.font_family.c_str());
        return;
    }

    float font_size = cmd.font_size > 0.0f ? cmd.font_size : 16.0f;

    GlyphAtlas* glyph_atlas = getGlyphAtlasForFontSize(font_size);
    if (!glyph_atlas || !text_sampler_) {
        DONG_LOG_WARN("SDLGPUDriver: no glyph atlas available for font_size=%.1f", font_size);
        return;
    }

    DONG_LOG_DEBUG("[DrawText] font_size=%.1f atlas=%p font=%s",
                   font_size,
                   (void*)glyph_atlas,
                   default_font_path->c_str());

    // Get atlas properties from the glyph atlas
    const float atlas_range = glyph_atlas->getGlyphDistanceRange();
    const float gamma_correction = -2.2f;
    const float pixel_scale = cmd.scale_to_pixels;

    if (ctx.pipeline_state.active != PipelineBindingState::ActivePipeline::Text) {
        SDL_BindGPUGraphicsPipeline(ctx.pass, text_pipeline_);
        ctx.pipeline_state.active = PipelineBindingState::ActivePipeline::Text;
    }

    constexpr int kMaxGlyphsPerBatch = 61;

    struct GlyphInstanceUniform {
        float rect[4];
        float uv_rect[4];
        float color[4];
        float params[4];
    };

    struct TextBatchUniformData {
        float viewport[4];
        float transform[8];
        ClipUniformBlock clip;
        GlyphInstanceUniform glyphs[kMaxGlyphsPerBatch];
    };

    struct PreparedGlyph {
        GlyphInstanceUniform instance;
        uint32_t atlas_page = 0;
    };

    TextBatchUniformData batch_uniform{};
    ctx.writeViewport(batch_uniform.viewport);
    writeTransform(batch_uniform.transform, ctx.getCurrentTransform());
    ctx.fillClipUniform(batch_uniform.clip);

    std::vector<PreparedGlyph> prepared_shadow;
    std::vector<PreparedGlyph> prepared_main;

    if (cmd.has_text_shadow) {
        prepared_shadow.reserve(cmd.glyphs.size());
    }
    prepared_main.reserve(cmd.glyphs.size());

    for (size_t glyph_idx = 0; glyph_idx < cmd.glyphs.size(); ++glyph_idx) {
        const auto& glyph = cmd.glyphs[glyph_idx];
        if (glyph.glyph_id == 0) {
            continue;
        }

        const std::string* glyph_font_path_ptr = default_font_path;
        if (!cmd.font_paths.empty()) {
            const uint16_t idx = glyph.font_path_index;
            if (idx < cmd.font_paths.size()) {
                glyph_font_path_ptr = &cmd.font_paths[idx];
            }
        }
        const std::string& glyph_font_path = glyph_font_path_ptr ? *glyph_font_path_ptr : *default_font_path;

        float glyph_pixel_scale = pixel_scale;
        if (glyph.units_per_em > 0 && glyph.units_per_em != cmd.units_per_em) {
            glyph_pixel_scale = font_size / static_cast<float>(glyph.units_per_em);
        }

        const AtlasEntry* entry = glyph_atlas->addGlyph(glyph.glyph_id, glyph_font_path);
        if (!entry) {
            DONG_LOG_VERBOSE("[DrawText] SKIP glyph[%zu]: glyph_id=%u no_entry (font=%s)",
                             glyph_idx,
                             glyph.glyph_id,
                             glyph_font_path.c_str());
            continue;
        }

        if (entry->metrics.width_units <= 0.0f || entry->metrics.height_units <= 0.0f ||
            entry->u1 <= entry->u0 || entry->v1 <= entry->v0) {
            DONG_LOG_VERBOSE("[DrawText] SKIP glyph[%zu]: glyph_id=%u invalid_metrics (w=%.1f h=%.1f u0=%.4f u1=%.4f v0=%.4f v1=%.4f)",
                             glyph_idx,
                             glyph.glyph_id,
                             entry->metrics.width_units,
                             entry->metrics.height_units,
                             entry->u0,
                             entry->u1,
                             entry->v0,
                             entry->v1);
            continue;
        }

        const float msdf_scale = (entry->metrics.msdf_scale > 0.0f) ? entry->metrics.msdf_scale : 1.0f;
        const float msdf_size = static_cast<float>(glyph_atlas->getGlyphBitmapSize());

        const float render_scale = glyph_pixel_scale / msdf_scale;
        const float glyph_w = msdf_size * render_scale;
        const float glyph_h = msdf_size * render_scale;

        if (glyph_w <= 0.0f || glyph_h <= 0.0f) {
            DONG_LOG_VERBOSE("[DrawText] SKIP glyph[%zu]: glyph_id=%u zero_size (w=%.1f h=%.1f render_scale=%.4f msdf_size=%.0f)",
                             glyph_idx,
                             glyph.glyph_id,
                             glyph_w,
                             glyph_h,
                             render_scale,
                             msdf_size);
            continue;
        }

        const float pen_x_px = glyph.pen_x_units * cmd.scale_to_pixels + cmd.baseline_x;
        const float pen_y_px = glyph.pen_y_units * cmd.scale_to_pixels + cmd.baseline_y;

        const float tile_x = pen_x_px - entry->metrics.msdf_translate_x * glyph_pixel_scale;
        const float tile_y = pen_y_px - msdf_size * render_scale + entry->metrics.msdf_translate_y * glyph_pixel_scale;

        const float glyph_x = tile_x;
        const float glyph_y = tile_y;

        const float px_range_screen = atlas_range * (glyph_pixel_scale / msdf_scale);
        const float unit_range = atlas_range / msdf_size;

        if (cmd.has_text_shadow) {
            const float blur = cmd.text_shadow_blur;
            const float base_off_x = cmd.text_shadow_offset_x;
            const float base_off_y = cmd.text_shadow_offset_y;

            auto push_shadow = [&](float off_x, float off_y, float alpha_scale, float precomputed_px_range) {
                GlyphInstanceUniform shadow_inst{};
                shadow_inst.rect[0] = glyph_x + off_x;
                shadow_inst.rect[1] = glyph_y + off_y;
                shadow_inst.rect[2] = glyph_w;
                shadow_inst.rect[3] = glyph_h;

                shadow_inst.uv_rect[0] = entry->u0;
                shadow_inst.uv_rect[1] = entry->v0;
                shadow_inst.uv_rect[2] = entry->u1;
                shadow_inst.uv_rect[3] = entry->v1;

                shadow_inst.color[0] = cmd.text_shadow_color.r;
                shadow_inst.color[1] = cmd.text_shadow_color.g;
                shadow_inst.color[2] = cmd.text_shadow_color.b;
                shadow_inst.color[3] = cmd.text_shadow_color.a * alpha_scale;

                shadow_inst.params[0] = precomputed_px_range;
                shadow_inst.params[1] = unit_range;
                shadow_inst.params[2] = msdf_subpixel_enabled_ ? 1.0f : 0.0f;
                shadow_inst.params[3] = gamma_correction;

                PreparedGlyph pg_shadow{};
                pg_shadow.instance = shadow_inst;
                pg_shadow.atlas_page = entry->atlas_page;
                prepared_shadow.push_back(pg_shadow);
            };

            if (blur <= 0.0f) {
                push_shadow(base_off_x, base_off_y, 1.0f, px_range_screen);
            } else {
                const int num_layers = std::min(4, std::max(2, static_cast<int>(std::ceil(blur / 3.0f))));
                const bool use_8_dirs = (blur >= 3.0f);

                static constexpr float kDirs8[8][2] = {
                    {1.0f, 0.0f}, {-1.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, -1.0f},
                    {0.7071f, 0.7071f}, {-0.7071f, 0.7071f}, {0.7071f, -0.7071f}, {-0.7071f, -0.7071f},
                };
                static constexpr float kDirs4[4][2] = {
                    {1.0f, 0.0f}, {-1.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, -1.0f},
                };

                const int dir_count = use_8_dirs ? 8 : 4;
                const float (*dirs)[2] = use_8_dirs ? kDirs8 : kDirs4;

                const float soft_px_range = std::min(1.5f, std::max(0.5f, px_range_screen / (1.0f + blur * 0.7f)));

                for (int layer = 0; layer < num_layers; ++layer) {
                    const float t = static_cast<float>(layer + 1) / static_cast<float>(num_layers);
                    const float radius = blur * t;

                    const float layer_alpha = (1.0f - t) * 0.55f + 0.10f;
                    const float per_sample_alpha = layer_alpha / static_cast<float>(dir_count);

                    for (int i = 0; i < dir_count; ++i) {
                        const float dx = dirs[i][0] * radius;
                        const float dy = dirs[i][1] * radius;
                        push_shadow(base_off_x + dx, base_off_y + dy, per_sample_alpha, soft_px_range);
                    }
                }

                push_shadow(base_off_x, base_off_y, 0.20f, soft_px_range);
            }
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

        writeLinearColor(cmd.color, inst.color);

        inst.params[0] = px_range_screen;
        inst.params[1] = unit_range;
        inst.params[2] = msdf_subpixel_enabled_ ? 1.0f : 0.0f;
        inst.params[3] = gamma_correction;

        PreparedGlyph pg{};
        pg.instance = inst;
        pg.atlas_page = entry->atlas_page;
        prepared_main.push_back(pg);
    }

    if (prepared_main.empty()) {
        DONG_LOG_WARN("[DrawText] frame=%llu ABORT: no glyphs prepared!", ctx.frame_index);
        return;
    }

    auto render_glyphs = [&](const std::vector<PreparedGlyph>& prepared) {
        uint32_t page_count = glyph_atlas->getPageCount();

        for (uint32_t page_index = 0; page_index < page_count; ++page_index) {
            SDL_GPUTexture* atlas_texture = static_cast<SDL_GPUTexture*>(glyph_atlas->getAtlasTextureForPage(page_index));
            if (!atlas_texture) {
                continue;
            }

            SDL_GPUTextureSamplerBinding binding{};
            binding.texture = atlas_texture;
            binding.sampler = text_sampler_;
            SDL_BindGPUFragmentSamplers(ctx.pass, 0, &binding, 1);
            ctx.pipeline_state.text_sampler_bound = true;

            int glyphs_in_batch = 0;

            auto flush_batch = [&]() {
                if (glyphs_in_batch <= 0) {
                    return;
                }

                SDL_PushGPUVertexUniformData(ctx.cmd_buf, 0, &batch_uniform, sizeof(batch_uniform));
                SDL_PushGPUFragmentUniformData(ctx.cmd_buf, 0, &batch_uniform, sizeof(batch_uniform));
                SDL_DrawGPUPrimitives(ctx.pass, 4, static_cast<Uint32>(glyphs_in_batch), 0, 0);
                glyphs_in_batch = 0;
            };

            for (const auto& pg : prepared) {
                if (pg.atlas_page != page_index) {
                    continue;
                }

                batch_uniform.glyphs[glyphs_in_batch] = pg.instance;
                ++glyphs_in_batch;

                if (glyphs_in_batch == kMaxGlyphsPerBatch) {
                    flush_batch();
                }
            }

            flush_batch();
        }
    };

    if (!prepared_shadow.empty()) {
        DONG_LOG_DEBUG("[DrawText] rendering shadow glyphs");
        render_glyphs(prepared_shadow);
    }

    DONG_LOG_DEBUG("[DrawText] rendering main glyphs");
    render_glyphs(prepared_main);
}

void SDLGPUDriver::executeDispatchCommand(const GPUCommand& cmd, ExecuteContext& ctx) {
    if (ctx.aborted) {
        return;
    }

    // 当在非脏隔离图层内部时，跳过除 Begin/EndIsolatedLayer 和 BeginPass/EndPass 以外的命令
    if (ctx.skip_draw_depth > 0 && cmd.type != GPUCommandType::BeginIsolatedLayer &&
        cmd.type != GPUCommandType::EndIsolatedLayer &&
        cmd.type != GPUCommandType::BeginPass &&
        cmd.type != GPUCommandType::EndPass) {
        if (ctx.debug_rt_enabled) {
            DONG_LOG_DEBUG("[SDLGPUDriver::execute] frame=%llu skip cmd type=%d due_to_non_dirty_layer depth=%d",
                    ctx.frame_index,
                    static_cast<int>(cmd.type),
                    ctx.skip_draw_depth);
        }
        return;
    }

    switch (cmd.type) {
    case GPUCommandType::BeginFrame:
    case GPUCommandType::EndFrame:
        // beginFrame/endFrame 由调用方负责
        break;
    case GPUCommandType::BeginPass:
        executeBeginPass(ctx);
        break;
    case GPUCommandType::EndPass:
        executeEndPass(ctx);
        break;
    case GPUCommandType::PushClipRect:
        executePushClipRect(ctx, cmd);
        break;
    case GPUCommandType::PopClip:
        executePopClip(ctx);
        break;
    case GPUCommandType::BeginIsolatedLayer:
        executeBeginIsolatedLayer(ctx, cmd);
        break;
    case GPUCommandType::EndIsolatedLayer:
        executeEndIsolatedLayer(ctx, cmd);
        break;
    case GPUCommandType::DrawInstancedQuads:
        executeDrawRect(ctx, cmd);
        break;
    case GPUCommandType::DrawRoundedRectQuad:
        executeDrawRoundedRect(ctx, cmd);
        break;
    case GPUCommandType::DrawShadowQuad:
        executeDrawShadow(ctx, cmd);
        break;
    case GPUCommandType::DrawImageQuad:
        executeDrawImage(ctx, cmd);
        break;
    case GPUCommandType::DrawText:
        executeDrawText(ctx, cmd);
        break;
    default:
        break;
    }
}

void SDLGPUDriver::execute(const GPUCommandList& commands) {
    DONG_PROFILE_SCOPE_CAT("GPU::execute", "gpu");

    if (!in_frame_ || !current_cmd_buf_ || !gpu_device_) {
        DONG_LOG_ERROR("SDLGPUDriver::execute: invalid state");
        return;
    }

    SDL_GPUDevice* dev = gpu_device_->getHandle();

    ExecuteContext ctx(current_cmd_buf_, dev, window_);
    ctx.debug_rt_enabled = debug_rt_enabled_;
    ctx.debug_log_draw_batches = debug_log_draw_batches_;
    ctx.debug_log_layer_cache = debug_log_layer_cache_;
    ctx.layer_cache_enabled = layer_cache_enabled_;
    ctx.split_cmd_buf_for_isolated_layers = split_cmd_buf_for_isolated_layers_;
    ctx.offscreen = (offscreen_target_ != nullptr);
    ctx.frame_index = frame_index_;

    if (!executeSetupMainTarget(ctx)) {
        return;
    }

    // 计算 device pixel ratio（用于未来需要像素对齐/clip/采样时）
    ctx.device_pixel_ratio = 1.0f;
    if (window_) {
        int logical_w = 0;
        int logical_h = 0;
        int drawable_w = 0;
        int drawable_h = 0;
        SDL_GetWindowSize(window_, &logical_w, &logical_h);
        SDL_GetWindowSizeInPixels(window_, &drawable_w, &drawable_h);
        if (logical_w > 0 && drawable_w > 0) {
            ctx.device_pixel_ratio = static_cast<float>(drawable_w) / static_cast<float>(logical_w);
        }
    }
    ctx.inv_device_pixel_ratio = ctx.device_pixel_ratio > 0.0f ? (1.0f / ctx.device_pixel_ratio) : 1.0f;

    const bool present_only = (!offscreen_target_ && ctx.use_intermediate && intermediate_texture_ && commands.commands.empty());

    // Present-only 快路径：当上层判断"内容没有变化"时，可以传入空 command list。
    // 我们直接把上一帧的 intermediate blit 到 swapchain，避免每帧重复遍历巨大的 command list。
    if (present_only) {
        DONG_PROFILE_SCOPE_CAT("GPU::presentOnly", "gpu");
        if (!intermediate_valid_) {
            // 首帧/尺寸变化后 intermediate 还没内容：至少 clear 一次，保证 deterministic。
            executeBeginPass(ctx);
            if (ctx.aborted) {
                return;
            }
        }
        executeEndPass(ctx);
        return;
    }

    // 注意：glyph 预处理已移到 prepareResources()，必须在 beginFrame() 之前调用
    executePreuploadImages(commands);

    {
        DONG_PROFILE_SCOPE_CAT("GPU::executeCommands", "gpu");

        int cmd_index = 0;
        for (const auto& cmd : commands.commands) {
            if (cmd.type == GPUCommandType::PushClipRect) {
                DONG_LOG_DEBUG("[GPU Execute] cmd[%d] PushClipRect: y=%.1f h=%.1f skip_depth=%d",
                               cmd_index,
                               cmd.rect.y,
                               cmd.rect.height,
                               ctx.skip_draw_depth);
            } else if (cmd.type == GPUCommandType::PopClip) {
                DONG_LOG_DEBUG("[GPU Execute] cmd[%d] PopClip skip_depth=%d", cmd_index, ctx.skip_draw_depth);
            }
            cmd_index++;

            if (ctx.debug_rt_enabled && offscreen_target_) {
                DONG_LOG_DEBUG("[SDLGPUDriver::execute] frame=%llu processing cmd type=%d skip_depth=%d",
                        ctx.frame_index,
                        static_cast<int>(cmd.type),
                        ctx.skip_draw_depth);
            }

            executeDispatchCommand(cmd, ctx);
            if (ctx.aborted) {
                break;
            }
        }
    }

    if (ctx.debug_log_layer_cache && (ctx.debug_layer_cache_rasterized > 0 || ctx.debug_layer_cache_reused > 0)) {
        DONG_LOG_DEBUG("[SDLGPUDriver layer-cache] frame=%llu summary: rasterize=%u, reuse=%u",
                ctx.frame_index,
                static_cast<unsigned int>(ctx.debug_layer_cache_rasterized),
                static_cast<unsigned int>(ctx.debug_layer_cache_reused));
    }

    if (ctx.debug_log_draw_batches && !commands.draw_batches.empty()) {
        for (size_t i = 0; i < commands.draw_batches.size(); ++i) {
            const DrawBatchRange& batch = commands.draw_batches[i];
            DONG_LOG_DEBUG("[SDLGPUDriver debug] draw batch %zu: type=%d, sort_key=0x%llx, count=%u, start=%u",
                    i,
                    static_cast<int>(batch.type),
                    static_cast<unsigned long long>(batch.sort_key),
                    batch.count,
                    batch.start);
        }
    }

    if (ctx.pass) {
        SDL_EndGPURenderPass(ctx.pass);
        ctx.pass = nullptr;
    }
}

} // namespace render
} // namespace dong
