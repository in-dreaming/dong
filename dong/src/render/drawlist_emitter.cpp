/**
 * @file drawlist_emitter.cpp
 * @brief Translates internal GPUCommandList into the public dong_draw_cmd_t array.
 *
 * UberQuadBatch commands are expanded into individual draw commands (RECT,
 * ROUNDED_RECT, SHADOW) based on the material ID stored in each instance's
 * params[0]. This ensures consumers of the public API see a flat, simple
 * command stream without needing to understand batching internals.
 */

#include "drawlist_emitter.hpp"
#include "gpu_ir.hpp"

#include <vector>
#include <cstring>

namespace dong::render {

// Persistent storage for the emitted draw list (valid until next emitDrawList call).
static std::vector<dong_draw_cmd_t> s_emitted_commands;

// Persistent storage for gradient stop data (referenced by stop_data pointers).
struct EmittedGradientStop {
    float position;
    dong_color_t color;
};
static std::vector<std::vector<EmittedGradientStop>> s_gradient_stop_storage;

void emitDrawList(const GPUCommandList& gpu_cmds) {
    s_emitted_commands.clear();
    s_gradient_stop_storage.clear();

    // Reserve generous estimate: commands + expanded uber instances
    size_t estimate = gpu_cmds.commands.size() + gpu_cmds.uber_instance_pool.size();
    s_emitted_commands.reserve(estimate);

    for (const auto& cmd : gpu_cmds.commands) {
        dong_draw_cmd_t out{};
        memset(&out, 0, sizeof(out));

        switch (cmd.type) {
            case GPUCommandType::DrawInstancedQuads:
                out.type = DONG_DRAW_CMD_RECT;
                out.rect = {cmd.rect.x, cmd.rect.y, cmd.rect.width, cmd.rect.height};
                out.color = {cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a};
                s_emitted_commands.push_back(out);
                break;

            case GPUCommandType::DrawRoundedRectQuad:
                out.type = DONG_DRAW_CMD_ROUNDED_RECT;
                out.rect = {cmd.rect.x, cmd.rect.y, cmd.rect.width, cmd.rect.height};
                out.color = {cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a};
                out.radius = cmd.radius;
                out.stroke_width = cmd.stroke_width;
                s_emitted_commands.push_back(out);
                break;

            case GPUCommandType::DrawImageQuad:
                out.type = DONG_DRAW_CMD_IMAGE;
                out.rect = {cmd.rect.x, cmd.rect.y, cmd.rect.width, cmd.rect.height};
                out.opacity = cmd.opacity;
                // image_id and UV will be filled when image handle system is ready (P1-2)
                s_emitted_commands.push_back(out);
                break;

            case GPUCommandType::DrawShadowQuad:
                out.type = DONG_DRAW_CMD_SHADOW;
                out.rect = {cmd.rect.x, cmd.rect.y, cmd.rect.width, cmd.rect.height};
                out.color = {cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a};
                out.radius = cmd.radius;
                out.blur = cmd.blur;
                s_emitted_commands.push_back(out);
                break;

            case GPUCommandType::DrawText:
                out.type = DONG_DRAW_CMD_TEXT;
                out.rect = {cmd.rect.x, cmd.rect.y, cmd.rect.width, cmd.rect.height};
                out.color = {cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a};
                out.glyph_count = static_cast<uint32_t>(cmd.glyphs.size());
                out.glyph_data = cmd.glyphs.empty() ? nullptr : cmd.glyphs.data();
                s_emitted_commands.push_back(out);
                break;

            case GPUCommandType::DrawGradientQuad: {
                out.type = DONG_DRAW_CMD_GRADIENT;
                out.rect = {cmd.rect.x, cmd.rect.y, cmd.rect.width, cmd.rect.height};
                out.radius = cmd.radius;
                out.gradient_angle = cmd.gradient_angle_deg;
                out.stop_count = static_cast<uint32_t>(cmd.gradient_stop_count);
                // Store gradient stops in persistent storage
                if (cmd.gradient_stop_count > 0) {
                    std::vector<EmittedGradientStop> stops;
                    stops.reserve(cmd.gradient_stop_count);
                    for (int i = 0; i < cmd.gradient_stop_count; ++i) {
                        EmittedGradientStop s;
                        s.position = cmd.gradient_stops[i].position;
                        s.color = {cmd.gradient_stops[i].color.r,
                                   cmd.gradient_stops[i].color.g,
                                   cmd.gradient_stops[i].color.b,
                                   cmd.gradient_stops[i].color.a};
                        stops.push_back(s);
                    }
                    s_gradient_stop_storage.push_back(std::move(stops));
                    out.stop_data = s_gradient_stop_storage.back().data();
                }
                s_emitted_commands.push_back(out);
                break;
            }

            case GPUCommandType::DrawConicGradientQuad: {
                out.type = DONG_DRAW_CMD_CONIC_GRADIENT;
                out.rect = {cmd.rect.x, cmd.rect.y, cmd.rect.width, cmd.rect.height};
                out.radius = cmd.radius;
                out.gradient_angle = cmd.conic_from_angle_deg;
                out.stop_count = static_cast<uint32_t>(cmd.gradient_stop_count);
                if (cmd.gradient_stop_count > 0) {
                    std::vector<EmittedGradientStop> stops;
                    stops.reserve(cmd.gradient_stop_count);
                    for (int i = 0; i < cmd.gradient_stop_count; ++i) {
                        EmittedGradientStop s;
                        s.position = cmd.gradient_stops[i].position;
                        s.color = {cmd.gradient_stops[i].color.r,
                                   cmd.gradient_stops[i].color.g,
                                   cmd.gradient_stops[i].color.b,
                                   cmd.gradient_stops[i].color.a};
                        stops.push_back(s);
                    }
                    s_gradient_stop_storage.push_back(std::move(stops));
                    out.stop_data = s_gradient_stop_storage.back().data();
                }
                s_emitted_commands.push_back(out);
                break;
            }

            case GPUCommandType::DrawHostView:
                out.type = DONG_DRAW_CMD_HOST_VIEW;
                out.rect = {cmd.rect.x, cmd.rect.y, cmd.rect.width, cmd.rect.height};
                out.host_view_id = cmd.host_view_id;
                out.opacity = cmd.opacity;
                s_emitted_commands.push_back(out);
                break;

            case GPUCommandType::BeginIsolatedLayer:
                out.type = DONG_DRAW_CMD_LAYER_PUSH;
                out.rect = {cmd.rect.x, cmd.rect.y, cmd.rect.width, cmd.rect.height};
                out.opacity = cmd.layer_opacity;
                s_emitted_commands.push_back(out);
                break;

            case GPUCommandType::EndIsolatedLayer:
                out.type = DONG_DRAW_CMD_LAYER_POP;
                s_emitted_commands.push_back(out);
                break;

            case GPUCommandType::PushClipRect:
                out.type = DONG_DRAW_CMD_CLIP_PUSH;
                out.rect = {cmd.rect.x, cmd.rect.y, cmd.rect.width, cmd.rect.height};
                out.radius = cmd.radius;
                s_emitted_commands.push_back(out);
                break;

            case GPUCommandType::PopClip:
                out.type = DONG_DRAW_CMD_CLIP_POP;
                s_emitted_commands.push_back(out);
                break;

            case GPUCommandType::UberQuadBatch: {
                // Expand uber batch into individual draw commands based on material type.
                const uint32_t offset = cmd.uber_instance_offset;
                const uint32_t count = cmd.uber_instance_count;
                for (uint32_t i = 0; i < count; ++i) {
                    const uint32_t idx = offset + i;
                    if (idx >= gpu_cmds.uber_instance_pool.size()) break;

                    const UberQuadInstance& inst = gpu_cmds.uber_instance_pool[idx];

                    dong_draw_cmd_t uber_out{};
                    memset(&uber_out, 0, sizeof(uber_out));

                    uber_out.rect = {inst.rect[0], inst.rect[1], inst.rect[2], inst.rect[3]};
                    uber_out.color = {inst.color[0], inst.color[1], inst.color[2], inst.color[3]};

                    const float material = inst.params[0];
                    const float radius = inst.params[1];
                    const float stroke = inst.params[2];
                    const float blur_val = inst.params[3];

                    if (material == UberQuadMaterial::kSolid) {
                        uber_out.type = DONG_DRAW_CMD_RECT;
                    } else if (material == UberQuadMaterial::kRounded) {
                        uber_out.type = DONG_DRAW_CMD_ROUNDED_RECT;
                        uber_out.radius = radius;
                        uber_out.stroke_width = stroke;
                    } else if (material == UberQuadMaterial::kShadow) {
                        uber_out.type = DONG_DRAW_CMD_SHADOW;
                        uber_out.radius = radius;
                        uber_out.blur = blur_val;
                    } else {
                        // Unknown material, emit as generic rect
                        uber_out.type = DONG_DRAW_CMD_RECT;
                    }

                    s_emitted_commands.push_back(uber_out);
                }
                break;
            }

            // Skip internal frame/pass management commands
            case GPUCommandType::BeginFrame:
            case GPUCommandType::EndFrame:
            case GPUCommandType::BeginPass:
            case GPUCommandType::EndPass:
            default:
                continue;
        }
    }
}

const dong_draw_cmd_t* getEmittedCommands(uint32_t* out_count) {
    if (out_count) {
        *out_count = static_cast<uint32_t>(s_emitted_commands.size());
    }
    return s_emitted_commands.empty() ? nullptr : s_emitted_commands.data();
}

} // namespace dong::render
