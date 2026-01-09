#pragma once

#include <algorithm>
#include <vector>
#include <cstdint>
#include <string>
#include <functional>
#include "display_list.hpp"
#include "layer_tree.hpp"
#include "../core/log.h"

namespace dong::render {

// GPU 级别的命令类型（与具体后端无关）
// 目前仅包含 DisplayList → GPUDriverSDL 实际会用到的子集。
// 后续如果增加新的 UI 原语，应在这里显式扩展并在 GPUDriver 中实现。
enum class GPUCommandType : uint8_t {
    BeginFrame,
    EndFrame,
    BeginPass,
    EndPass,

    // 绘制命令：一条命令对应一个"逻辑图元批次"
    // 后续可以通过 instance buffer 将其中的 rect/image/text 扩展为真正的 GPU instancing。
    DrawInstancedQuads,      // 纯色矩形（当前每条命令只画 1 个 quad）
    DrawImageQuad,           // 图片 quad
    DrawRoundedRectQuad,     // 圆角矩形 quad（analytic SDF）
    DrawShadowQuad,          // 带模糊的阴影 quad（SDF + blur）
    DrawText,                // 文本 glyph run

    // 剪裁与图层命令
    PushClipRect,
    PopClip,
    BeginIsolatedLayer,
    EndIsolatedLayer,
};

// GPU 级别的通用命令。这里刻意只保留当前真实使用的字段，
// 避免为了"未来也许会用"的场景引入多余的抽象。
struct GPUCommand {
    GPUCommandType type;

    // 当前所有绘制命令的 instance_count 都为 1；
    // 引入 instance buffer 后可以将多个实例合并到同一命令。
    uint32_t instance_count = 1;

    // 基于 pipeline / 资源的排序 key，供后续批处理/排序使用。
    // 当前编译阶段会填充该字段，但执行阶段仍按原始顺序渲染。
    uint64_t sort_key = 0;

    // 通用几何/颜色参数：不同命令按需读取
    Rect rect;      // 目标矩形（像素坐标）
    Color color;    // 颜色或调制色（用于纯色/圆角矩形、后续也可用于调制图片）
    float radius = 0.0f; // 圆角矩形半径（仅在 DrawRoundedRectQuad/DrawShadowQuad 时使用）
    float blur = 0.0f;   // 模糊半径（仅在 DrawShadowQuad 时使用）

    // 图片绘制专用字段（仅在 DrawImageQuad 时使用）
    std::string image_src; // 原始图片资源标识（路径）
    float opacity = 1.0f;  // 图片整体透明度
    float layer_opacity = 1.0f; // 图层合成透明度（BeginIsolatedLayer）
    uint64_t layer_id = 0;      // 图层的稳定 ID，用于跨帧缓存
    bool layer_dirty = true;    // 本帧该图层是否需要重新栅格

    // LayerTree 相关属性：目前主要由 Begin/EndIsolatedLayer 使用
    float layer_transform[6] = {1.0f, 0.0f, 0.0f,
                                0.0f, 1.0f, 0.0f};
    float layer_scroll[2] = {0.0f, 0.0f};

    // 文字绘制专用字段（仅在 DrawText 时使用）
    float font_size = 16.0f;
    std::string font_family;
    std::string font_weight; // CSS font-weight（"normal"/"bold"/数值）
    std::string font_path;
    float baseline_x = 0.0f;
    float baseline_y = 0.0f;
    std::vector<GlyphInstance> glyphs;
    
    // design units 元数据（文本布局与渲染共享）
    uint32_t units_per_em = 0;
    float scale_to_pixels = 1.0f;
};

struct DrawBatchRange {
    uint32_t start = 0;      // 在 sorted_draw_indices 中的起始位置
    uint32_t count = 0;      // 连续归为同一批次的绘制命令数量
    uint64_t sort_key = 0;   // 该批次共享的 sort_key（pipeline+资源）
    GPUCommandType type = GPUCommandType::BeginFrame; // 该批次的命令类型
};

struct GPUCommandList {
    std::vector<GPUCommand> commands;

    // 基于 sort_key 的排序视图（不改变 commands 的存储顺序）
    // 元素为 commands 的索引，按 sort_key 升序稳定排序
    std::vector<uint32_t> sorted_draw_indices;

    // 将 sorted_draw_indices 中连续、且 sort_key/type 相同的绘制命令
    // 聚合成批次，方便后端按批次遍历或做进一步优化。
    std::vector<DrawBatchRange> draw_batches;
};

// DisplayList → GPUCommandList 的编译器骨架
class GPUCompiler {
public:
    // 支持可选的 LayerTree，用于在图层命令上携带 transform/scroll 等属性
    void compile(const DisplayList& dl, GPUCommandList& out, const LayerTree* layer_tree = nullptr) {
        out.commands.clear();

        auto find_layer_by_id = [layer_tree](uint64_t id) -> const LayerNode* {
            if (!layer_tree || !id) {
                return nullptr;
            }
            return layer_tree->findById(id);
        };

        // 统计各种类型
        int rect_count = 0;
        int round_rect_count = 0;
        int image_count = 0;
        int text_count = 0;

        std::vector<float> draw_opacity_stack;
        draw_opacity_stack.push_back(1.0f);
        std::vector<bool> layer_isolate_stack;
        std::vector<uint64_t> layer_id_stack;
        std::vector<bool> layer_dirty_stack;

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

        auto make_sort_key = [](GPUCommandType type, const GPUCommand& cmd) {
            uint64_t pipeline_id = 0;
            switch (type) {
            case GPUCommandType::DrawInstancedQuads:
                pipeline_id = 1; // 纯色矩形管线
                break;
            case GPUCommandType::DrawRoundedRectQuad:
                pipeline_id = 2; // 圆角矩形管线
                break;
            case GPUCommandType::DrawShadowQuad:
                pipeline_id = 5; // 阴影管线
                break;
            case GPUCommandType::DrawImageQuad:
                pipeline_id = 3; // 图片管线
                break;
            case GPUCommandType::DrawText:
                pipeline_id = 4; // 文本管线
                break;
            default:
                pipeline_id = 0; // 非绘制类命令
                break;
            }

            uint64_t resource_hash = 0;
            if (type == GPUCommandType::DrawImageQuad) {
                if (!cmd.image_src.empty()) {
                    resource_hash = static_cast<uint64_t>(std::hash<std::string>{}(cmd.image_src));
                }
            } else if (type == GPUCommandType::DrawText) {
                const std::string& key = !cmd.font_path.empty() ? cmd.font_path : cmd.font_family;
                if (!key.empty()) {
                    resource_hash = static_cast<uint64_t>(std::hash<std::string>{}(key));
                }
            }

            // sort_key 的低 8 位用于标记 pipeline，
            // 高位用于资源 hash，便于后续按 pipeline/纹理分组和排序。
            return (resource_hash << 8) | (pipeline_id & 0xffu);
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
                uint64_t layer_id = item.layer.id;
                bool is_dirty = item.layer.is_dirty;

                const LayerNode* layer_node = find_layer_by_id(layer_id);

                if (isolate) {
                    GPUCommand cmd{};
                    cmd.type = GPUCommandType::BeginIsolatedLayer;
                    cmd.rect = item.layer.bounds;
                    cmd.layer_opacity = clamped;
                    cmd.layer_id = layer_id;
                    cmd.layer_dirty = is_dirty;

                    if (layer_node) {
                        for (int i = 0; i < 6; ++i) {
                            cmd.layer_transform[i] = layer_node->transform.m[i];
                        }
                        cmd.layer_scroll[0] = layer_node->scroll_x;
                        cmd.layer_scroll[1] = layer_node->scroll_y;
                    }

                    out.commands.push_back(cmd);
                }
                float next_opacity = isolate ? parent : parent * clamped;
                draw_opacity_stack.push_back(next_opacity);
                layer_isolate_stack.push_back(isolate);
                layer_id_stack.push_back(layer_id);
                layer_dirty_stack.push_back(is_dirty);
                break;
            }
            case DisplayItemType::PopLayer: {
                if (!layer_isolate_stack.empty()) {
                    bool isolate = layer_isolate_stack.back();
                    layer_isolate_stack.pop_back();
                    uint64_t layer_id = 0;
                    bool is_dirty = true;
                    if (!layer_id_stack.empty()) {
                        layer_id = layer_id_stack.back();
                        layer_id_stack.pop_back();
                    }
                    if (!layer_dirty_stack.empty()) {
                        is_dirty = layer_dirty_stack.back();
                        layer_dirty_stack.pop_back();
                    }
                    if (draw_opacity_stack.size() > 1) {
                        draw_opacity_stack.pop_back();
                    }
                    if (isolate) {
                        GPUCommand cmd{};
                        cmd.type = GPUCommandType::EndIsolatedLayer;
                        cmd.layer_id = layer_id;
                        cmd.layer_dirty = is_dirty;

                        const LayerNode* layer_node = find_layer_by_id(layer_id);
                        if (layer_node) {
                            for (int i = 0; i < 6; ++i) {
                                cmd.layer_transform[i] = layer_node->transform.m[i];
                            }
                            cmd.layer_scroll[0] = layer_node->scroll_x;
                            cmd.layer_scroll[1] = layer_node->scroll_y;
                        }

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
                cmd.sort_key = make_sort_key(cmd.type, cmd);
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
                cmd.sort_key = make_sort_key(cmd.type, cmd);
                out.commands.push_back(cmd);
                round_rect_count++;
                break;
            }
            case DisplayItemType::DrawShadow: {
                GPUCommand cmd{};
                cmd.type = GPUCommandType::DrawShadowQuad;
                cmd.instance_count = 1;
                cmd.rect = item.shadow.rect;
                cmd.color = apply_opacity(item.shadow.color, current_opacity());
                cmd.radius = item.shadow.radius;
                cmd.blur = item.shadow.blur;
                cmd.sort_key = make_sort_key(cmd.type, cmd);
                out.commands.push_back(cmd);
                round_rect_count++;  // 统计为圆角矩形类
                break;
            }
            case DisplayItemType::DrawImage: {
                GPUCommand cmd{};
                cmd.type = GPUCommandType::DrawImageQuad;
                cmd.instance_count = 1;
                cmd.rect = item.image.rect;
                cmd.opacity = item.image.opacity * current_opacity();
                cmd.image_src = item.image.src;
                cmd.sort_key = make_sort_key(cmd.type, cmd);
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
                cmd.font_weight = item.glyph_run.font_weight;
                cmd.font_path = item.glyph_run.font_path;
                cmd.baseline_x = item.glyph_run.baseline_x;
                cmd.baseline_y = item.glyph_run.baseline_y;
                cmd.glyphs = item.glyph_run.glyphs;
                cmd.units_per_em = item.glyph_run.units_per_em;
                cmd.scale_to_pixels = item.glyph_run.scale_to_pixels;
                cmd.sort_key = make_sort_key(cmd.type, cmd);
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

        // 基于 sort_key 构建绘制命令的排序视图和批次描述。
        out.sorted_draw_indices.clear();
        out.draw_batches.clear();

        // 收集所有绘制类命令的索引。
        for (uint32_t i = 0; i < out.commands.size(); ++i) {
            const GPUCommandType t = out.commands[i].type;
            switch (t) {
            case GPUCommandType::DrawInstancedQuads:
            case GPUCommandType::DrawRoundedRectQuad:
            case GPUCommandType::DrawShadowQuad:
            case GPUCommandType::DrawImageQuad:
            case GPUCommandType::DrawText:
                out.sorted_draw_indices.push_back(i);
                break;
            default:
                break;
            }
        }

        // 按 sort_key 升序稳定排序（同 key 时保留原始顺序，用索引比较保证稳定性）。
        std::sort(out.sorted_draw_indices.begin(), out.sorted_draw_indices.end(),
                  [&out](uint32_t a, uint32_t b) {
                      const GPUCommand& ca = out.commands[a];
                      const GPUCommand& cb = out.commands[b];
                      if (ca.sort_key < cb.sort_key) return true;
                      if (ca.sort_key > cb.sort_key) return false;
                      return a < b;
                  });

        // 将排序后的绘制命令按 sort_key/type 聚合成批次。
        if (!out.sorted_draw_indices.empty()) {
            DrawBatchRange current{};
            const uint32_t first_index = out.sorted_draw_indices[0];
            current.start = 0;
            current.count = 1;
            current.sort_key = out.commands[first_index].sort_key;
            current.type = out.commands[first_index].type;

            for (uint32_t i = 1; i < out.sorted_draw_indices.size(); ++i) {
                const uint32_t cmd_index = out.sorted_draw_indices[i];
                const GPUCommand& cmd = out.commands[cmd_index];
                if (cmd.sort_key == current.sort_key && cmd.type == current.type) {
                    ++current.count;
                } else {
                    out.draw_batches.push_back(current);
                    current.start = i;
                    current.count = 1;
                    current.sort_key = cmd.sort_key;
                    current.type = cmd.type;
                }
            }
            out.draw_batches.push_back(current);
        }

        // 输出统计信息
        DONG_LOG_DEBUG("GPU Compiler: %d rects, %d round_rects, %d images, %d texts -> %zu GPU commands, %zu draw batches",
                rect_count,
                round_rect_count,
                image_count,
                text_count,
                out.commands.size(),
                out.draw_batches.size());
    }
};

} // namespace dong::render
