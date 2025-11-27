#pragma once

#include <algorithm>
#include <vector>
#include <cstdint>
#include <string>
#include "display_list.hpp"
#include <SDL3/SDL_log.h>

namespace dong::render {

// GPU 级别的命令类型（与具体后端无关）
enum class GPUCommandType : uint8_t {
    BeginFrame,
    EndFrame,
    BeginPass,
    EndPass,
    SetViewport,
    SetPipeline,
    BindTexture,
    BindSampler,
    BindInstanceBuffer,
    DrawInstancedQuads,
    DrawImageQuad,
    DrawRoundedRectQuad,
    DrawText,  // 文字绘制命令
    PushClipRect,
    PopClip,
    BeginIsolatedLayer,
    EndIsolatedLayer,
};

struct GPUViewport {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

// 为了保持 IR 简单，这里用整型 id 代表各种资源（pipeline / texture / buffer）
using PipelineId = uint32_t;
using TextureId = uint32_t;
using BufferId = uint32_t;

struct GPUCommand {
    GPUCommandType type;

    GPUViewport viewport{};
    PipelineId pipeline = 0;
    TextureId texture = 0;
    BufferId instance_buffer = 0;
    uint32_t instance_count = 0;

    // 最小实现：为 DrawRect / DrawRoundedRect / DrawImage 携带一份立即模式的矩形和颜色数据
    Rect rect;      // 目标矩形（像素坐标）
    Color color;    // 颜色或调制色（用于纯色/圆角矩形、后续也可用于调制图片）
    float radius = 0.0f; // 圆角矩形半径（仅在 DrawRoundedRectQuad 时使用）

    // 图片绘制专用字段（仅在 DrawImageQuad 时使用）
    std::string image_src; // 原始图片资源标识（路径）
    float opacity = 1.0f;  // 图片整体透明度
    float layer_opacity = 1.0f; // 图层合成透明度（BeginIsolatedLayer）

    // 文字绘制专用字段（仅在 DrawText 时使用）
    float font_size = 16.0f;
    std::string font_family;
    std::string font_path;
    std::vector<GlyphInstance> glyphs;
};

struct GPUCommandList {
    std::vector<GPUCommand> commands;
};

// DisplayList → GPUCommandList 的编译器骨架
class GPUCompiler {
public:
    // 简化版编译接口：只处理最基础的 rect / image / text 类型
    void compile(const DisplayList& dl, GPUCommandList& out) {
        out.commands.clear();

        // 统计各种类型
        int rect_count = 0;
        int round_rect_count = 0;
        int image_count = 0;
        int text_count = 0;

        std::vector<float> draw_opacity_stack;
        draw_opacity_stack.push_back(1.0f);
        std::vector<bool> layer_isolate_stack;

        auto apply_opacity = [](const Color& base_color, float opacity) {
            Color result = base_color;
            float clamped = std::clamp(opacity, 0.0f, 1.0f);
            result.r *= clamped;
            result.g *= clamped;
            result.b *= clamped;
            result.a *= clamped;
            return result;
        };

        auto current_opacity = [&draw_opacity_stack]() {
            if (draw_opacity_stack.empty()) {
                return 1.0f;
            }
            return draw_opacity_stack.back();
        };

        // 一个最小的实现：
        // - 为整个 DisplayList 生成一对 BeginFrame/EndFrame
        // - 在其中包一对 BeginPass/EndPass
        GPUCommand begin_frame{};
        begin_frame.type = GPUCommandType::BeginFrame;
        out.commands.push_back(begin_frame);

        GPUCommand begin_pass{};
        begin_pass.type = GPUCommandType::BeginPass;
        out.commands.push_back(begin_pass);

        for (const auto& item : dl.items) {
            switch (item.type) {
            case DisplayItemType::PushLayer: {
                float clamped = std::clamp(item.layer.opacity, 0.0f, 1.0f);
                float parent = current_opacity();
                bool isolate = item.layer.isolate;
                if (isolate) {
                    GPUCommand cmd{};
                    cmd.type = GPUCommandType::BeginIsolatedLayer;
                    cmd.rect = item.layer.bounds;
                    cmd.layer_opacity = clamped;
                    out.commands.push_back(cmd);
                }
                float next_opacity = isolate ? parent : parent * clamped;
                draw_opacity_stack.push_back(next_opacity);
                layer_isolate_stack.push_back(isolate);
                break;
            }
            case DisplayItemType::PopLayer: {
                if (!layer_isolate_stack.empty()) {
                    bool isolate = layer_isolate_stack.back();
                    layer_isolate_stack.pop_back();
                    if (draw_opacity_stack.size() > 1) {
                        draw_opacity_stack.pop_back();
                    }
                    if (isolate) {
                        GPUCommand cmd{};
                        cmd.type = GPUCommandType::EndIsolatedLayer;
                        out.commands.push_back(cmd);
                    }
                }
                break;
            }
            case DisplayItemType::PushClipRect:
            case DisplayItemType::PushClipRoundedRect: {
                GPUCommand cmd{};
                cmd.type = GPUCommandType::PushClipRect;
                cmd.rect = item.clip.rect;
                cmd.radius = item.clip.is_rounded ? item.clip.radius : 0.0f;
                out.commands.push_back(cmd);
                break;
            }
            case DisplayItemType::PopClip: {
                GPUCommand cmd{};
                cmd.type = GPUCommandType::PopClip;
                out.commands.push_back(cmd);
                break;
            }
            case DisplayItemType::DrawRect: {
                GPUCommand cmd{};
                cmd.type = GPUCommandType::DrawInstancedQuads;
                cmd.instance_count = 1;
                cmd.rect = item.rect.rect;
                cmd.color = apply_opacity(item.rect.color, current_opacity());
                out.commands.push_back(cmd);
                rect_count++;
                break;
            }
            case DisplayItemType::DrawRoundedRect: {
                GPUCommand cmd{};
                cmd.type = GPUCommandType::DrawRoundedRectQuad;
                cmd.instance_count = 1;
                cmd.rect = item.rounded_rect.rect;
                cmd.color = apply_opacity(item.rounded_rect.color, current_opacity());
                cmd.radius = item.rounded_rect.radius;
                out.commands.push_back(cmd);
                round_rect_count++;
                break;
            }
            case DisplayItemType::DrawImage: {
                GPUCommand cmd{};
                cmd.type = GPUCommandType::DrawImageQuad;
                cmd.instance_count = 1;
                cmd.rect = item.image.rect;
                cmd.opacity = item.image.opacity * current_opacity();
                cmd.image_src = item.image.src;
                out.commands.push_back(cmd);
                image_count++;
                break;
            }
            case DisplayItemType::DrawGlyphRun: {
                GPUCommand cmd{};
                cmd.type = GPUCommandType::DrawText;
                cmd.instance_count = 1;
                cmd.rect = item.glyph_run.rect;
                cmd.color = apply_opacity(item.glyph_run.color, current_opacity());
                cmd.font_size = item.glyph_run.font_size;
                cmd.font_family = item.glyph_run.font_family;
                cmd.font_path = item.glyph_run.font_path;
                cmd.glyphs = item.glyph_run.glyphs;
                out.commands.push_back(cmd);
                text_count++;
                break;
            }
            default:
                // 其他类型先忽略，后续再逐步接入
                break;
            }
        }

        GPUCommand end_pass{};
        end_pass.type = GPUCommandType::EndPass;
        out.commands.push_back(end_pass);

        GPUCommand end_frame{};
        end_frame.type = GPUCommandType::EndFrame;
        out.commands.push_back(end_frame);

        // 输出统计信息
        SDL_Log("GPU Compiler: %d rects, %d round_rects, %d images, %d texts -> %zu GPU commands",
                rect_count, round_rect_count, image_count, text_count, out.commands.size());
    }
};

} // namespace dong::render
