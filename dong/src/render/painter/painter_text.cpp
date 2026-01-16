#include "../painter.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

#include "../../core/log.h"
#include "painter_style_utils.hpp"

namespace dong::render {

void Painter::paintTextAndInput(const dom::DOMNodePtr& node,
                               const layout::LayoutNode* layout_node,
                               const std::string& tag,
                               const dom::ComputedStyle& style,
                               bool is_hidden,
                               DisplayListBuilder& builder) {
    using painter_detail::collapseWhitespace;
    using painter_detail::fillTextShadow;
    using painter_detail::makeColorFromCss;

    if (!node || !layout_node || is_hidden) {
        return;
    }

    // 3. 文本内容
    if (tag != "script" && tag != "style" && tag != "head" && tag != "img" && tag != "video") {
        std::string debug_class = node->getAttribute("class");
        if (debug_class.find("overlay-row") != std::string::npos) {
            DONG_LOG_INFO("[Painter] overlay-row: checking children, count=%zu", node->getChildren().size());
            for (const auto& child : node->getChildren()) {
                if (child) {
                    DONG_LOG_INFO("[Painter] overlay-row child type=%d text='%s'",
                            static_cast<int>(child->getType()), child->getTextContent().c_str());
                }
            }
        }
        bool has_text_child = false;
        bool has_inline_element_child = false;
        std::string raw_text;

        for (const auto& child : node->getChildren()) {
            if (!child) continue;
            if (child->getType() == dom::DOMNode::NodeType::TEXT) {
                raw_text += child->getTextContent();
                has_text_child = true;
            } else if (child->getType() == dom::DOMNode::NodeType::ELEMENT) {
                const auto& child_style = child->getComputedStyle();
                // 检查是否为 inline/inline-block 元素
                if (child_style.display == "inline" || child_style.display == "inline-block") {
                    has_inline_element_child = true;
                }
            }
        }

        const bool tag_prefers_text =
            tag == "h1" || tag == "h2" || tag == "h3" || tag == "h4" ||
            tag == "h5" || tag == "h6" || tag == "p" || tag == "span" ||
            tag == "button" || tag == "code" || tag == "div" || tag == "footer";

        if (debug_class.find("overlay-row") != std::string::npos) {
            DONG_LOG_INFO("[Painter] overlay-row: has_text=%d has_inline=%d tag_prefers=%d raw='%s'",
                    has_text_child, has_inline_element_child, tag_prefers_text, raw_text.c_str());
        }

        // 当有 inline 子元素时，需要按顺序处理每个子节点，避免文本重叠
        // 使用容器层完全接管混合内容的布局和绘制
        // 注意：如果容器是 flex 布局，不应该使用混合内容路径，因为 flex 会正确计算子元素位置
        const bool is_flex_container = (style.display == "flex" || style.display == "inline-flex");
        if (has_text_child && has_inline_element_child && tag_prefers_text && !is_flex_container) {
            if (debug_class.find("overlay-row") != std::string::npos) {
                DONG_LOG_INFO("[Painter] overlay-row MIXED PATH raw_text='%s'", raw_text.c_str());
            }
            // 混合内容：有 TEXT 节点 + inline 元素
            // 按子节点顺序分别绘制每个内容，计算正确的起始 X 位置
            float x = layout_node->layout.position[0];
            float y = layout_node->layout.position[1];
            float width = layout_node->layout.dimensions[0];

            float pad_left = style.padding_left.isPixel() ? style.padding_left.value : 0.0f;
            float pad_right = style.padding_right.isPixel() ? style.padding_right.value : 0.0f;
            float pad_top = style.padding_top.isPixel() ? style.padding_top.value : 0.0f;

            float container_font_size = style.font_size > 0.0f ? style.font_size : 16.0f;
            Color container_text_color = makeColorFromCss(style.color);

            float inner_width = width - pad_left - pad_right;
            if (inner_width <= 0.0f) inner_width = width > 0.0f ? width : 0.0f;

            // 计算容器的 baseline 度量
            float container_baseline_offset = 0.0f;
            float container_ascent_px = 0.0f;
            float container_line_height_px = 0.0f;
            {
                TextShapeRequest req{};
                req.text = "X";  // 使用 X 作为基准字符
                req.font_family = style.font_family;
                req.font_weight = style.font_weight;
                req.font_style = style.font_style;
                req.font_size = container_font_size;

                ShapedText shaped{};
                if (text_shaper_.shape(req, shaped)) {
                    float scale = shaped.scale_to_pixels;
                    float ascent_units = shaped.ascent_units > 0.0f ? shaped.ascent_units : container_font_size / scale;
                    float descent_units = shaped.descent_units;
                    float line_height_units = shaped.line_height_units;

                    if (style.line_height > 0.0f) {
                        if (style.line_height_is_unitless) {
                            line_height_units = (style.line_height * container_font_size) / std::max(scale, 1e-3f);
                        } else {
                            line_height_units = style.line_height / std::max(scale, 1e-3f);
                        }
                    }
                    if (line_height_units <= 0.0f) line_height_units = container_font_size / scale;

                    float descent_abs_units = descent_units < 0.0f ? -descent_units : 0.0f;
                    float metrics_height_units = ascent_units + descent_abs_units;
                    float extra_leading_units = std::max(line_height_units - metrics_height_units, 0.0f);
                    float top_leading_units = extra_leading_units * 0.5f;

                    container_baseline_offset = (top_leading_units + ascent_units) * scale;
                    container_ascent_px = ascent_units * scale;
                    container_line_height_px = line_height_units * scale;
                }
            }

            float cumulative_x_offset = 0.0f;
            float baseline_y = y + pad_top + container_baseline_offset;

            // 按子节点顺序处理所有内容（TEXT + inline ELEMENT）
            for (const auto& child : node->getChildren()) {
                if (!child) continue;

                if (child->getType() == dom::DOMNode::NodeType::ELEMENT) {
                    const auto& child_style = child->getComputedStyle();

                    // inline-block 元素应该通过正常递归渲染，使用布局引擎计算的位置和尺寸
                    // 只有纯 inline 元素才在这里处理文本
                    if (child_style.display == "inline") {
                        // 获取 inline 元素的文本内容并直接在此绘制
                        std::string child_text;
                        for (const auto& grandchild : child->getChildren()) {
                            if (grandchild && grandchild->getType() == dom::DOMNode::NodeType::TEXT) {
                                child_text += grandchild->getTextContent();
                            }
                        }
                        child_text = collapseWhitespace(child_text);

                        if (!child_text.empty()) {
                            // 使用 inline 元素的样式
                            float child_font_size = child_style.font_size > 0.0f ? child_style.font_size : container_font_size;
                            std::string child_font_family = !child_style.font_family.empty() ? child_style.font_family : style.font_family;
                            std::string child_font_weight = !child_style.font_weight.empty() ? child_style.font_weight : style.font_weight;
                            std::string child_font_style = !child_style.font_style.empty() ? child_style.font_style : style.font_style;
                            Color child_color = makeColorFromCss(!child_style.color.empty() ? child_style.color : style.color);

                            TextShapeRequest req{};
                            req.text = child_text;
                            req.font_family = child_font_family;
                            req.font_weight = child_font_weight;
                            req.font_style = child_font_style;
                            req.font_size = child_font_size;

                            ShapedText shaped{};
                            if (text_shaper_.shape(req, shaped) && !shaped.glyphs.empty()) {
                                float scale = shaped.scale_to_pixels;
                                float text_width_px = shaped.width_units * scale;
                                float ascent_units = shaped.ascent_units > 0.0f ? shaped.ascent_units : child_font_size / scale;
                                float ascent_px = ascent_units * scale;

                                // 计算 inline 元素的 padding
                                float child_pad_left = child_style.padding_left.isPixel() ? child_style.padding_left.value : 0.0f;
                                float child_pad_right = child_style.padding_right.isPixel() ? child_style.padding_right.value : 0.0f;

                                float text_x = x + pad_left + cumulative_x_offset + child_pad_left;

                                // 绘制 inline 元素的背景（如果有）
                                if (!child_style.background_color.empty() && child_style.background_color != "transparent") {
                                    Color bg_color = makeColorFromCss(child_style.background_color);
                                    float bg_radius = child_style.border_radius > 0.0f ? child_style.border_radius : 0.0f;
                                    Rect bg_rect{};
                                    bg_rect.x = x + pad_left + cumulative_x_offset;
                                    bg_rect.y = baseline_y - container_ascent_px;
                                    bg_rect.width = text_width_px + child_pad_left + child_pad_right;
                                    bg_rect.height = container_line_height_px;
                                    if (bg_radius > 0.0f) {
                                        builder.addRoundedRect(bg_rect, bg_color, bg_radius);
                                    } else {
                                        builder.addRect(bg_rect, bg_color);
                                    }
                                }

                                DrawGlyphRunData glyph_run{};
                                glyph_run.rect.x = text_x;
                                glyph_run.rect.y = baseline_y - ascent_px;
                                glyph_run.rect.width = text_width_px;
                                glyph_run.rect.height = container_line_height_px;
                                glyph_run.color = child_color;
                                glyph_run.font_size = child_font_size;
                                glyph_run.font_family = child_font_family;
                                glyph_run.font_weight = child_font_weight;
                                glyph_run.font_style = child_font_style;
                                glyph_run.font_path = shaped.font_path;

                                glyph_run.baseline_x = text_x;
                                glyph_run.baseline_y = baseline_y;
                                glyph_run.units_per_em = shaped.units_per_em;
                                glyph_run.scale_to_pixels = shaped.scale_to_pixels;
                                fillTextShadow(glyph_run, child_style);

                                for (const auto& sg : shaped.glyphs) {
                                    GlyphInstance inst{};
                                    inst.glyph_id = sg.glyph_id;
                                    inst.pen_x_units = sg.pen_x_units;
                                    inst.pen_y_units = sg.pen_y_units;
                                    inst.font_path = sg.font_path;
                                    inst.units_per_em = sg.units_per_em;
                                    glyph_run.glyphs.push_back(inst);
                                }

                                builder.addGlyphRun(std::move(glyph_run));
                                cumulative_x_offset += text_width_px + child_pad_left + child_pad_right;
                            }

                            // 标记该 inline 元素已经在容器层绘制
                            child->setAttribute("__inline_rendered__", "1");
                        }
                    }
                    // inline-block 元素不在这里处理，让它们通过正常递归渲染
                } else if (child->getType() == dom::DOMNode::NodeType::TEXT) {
                    std::string text_content = child->getTextContent();
                    std::string text = collapseWhitespace(text_content);
                    if (text.empty()) continue;

                    TextShapeRequest req{};
                    req.text = text;
                    req.font_family = style.font_family;
                    req.font_weight = style.font_weight;
                    req.font_style = style.font_style;
                    req.font_size = container_font_size;

                    ShapedText shaped{};
                    if (!text_shaper_.shape(req, shaped) || shaped.glyphs.empty()) {
                        continue;
                    }

                    float scale = shaped.scale_to_pixels;
                    float text_width_px = shaped.width_units * scale;
                    float ascent_units = shaped.ascent_units > 0.0f ? shaped.ascent_units : container_font_size / scale;
                    float ascent_px = ascent_units * scale;

                    float text_x = x + pad_left + cumulative_x_offset;

                    DrawGlyphRunData glyph_run{};
                    glyph_run.rect.x = text_x;
                    glyph_run.rect.y = baseline_y - ascent_px;
                    glyph_run.rect.width = text_width_px;
                    glyph_run.rect.height = container_line_height_px;
                    glyph_run.color = container_text_color;
                    glyph_run.font_size = container_font_size;
                    glyph_run.font_family = style.font_family;
                    glyph_run.font_weight = style.font_weight;
                    glyph_run.font_style = style.font_style;
                    glyph_run.font_path = shaped.font_path;

                    glyph_run.baseline_x = text_x;
                    glyph_run.baseline_y = baseline_y;
                    glyph_run.units_per_em = shaped.units_per_em;
                    glyph_run.scale_to_pixels = shaped.scale_to_pixels;
                    fillTextShadow(glyph_run, style);

                    for (const auto& sg : shaped.glyphs) {
                        GlyphInstance inst{};
                        inst.glyph_id = sg.glyph_id;
                        inst.pen_x_units = sg.pen_x_units;
                        inst.pen_y_units = sg.pen_y_units;
                        inst.font_path = sg.font_path;
                        inst.units_per_em = sg.units_per_em;
                        glyph_run.glyphs.push_back(inst);
                    }

                    builder.addGlyphRun(std::move(glyph_run));
                    cumulative_x_offset += text_width_px;
                }
            }
        } else if (has_text_child && (tag_prefers_text || !has_inline_element_child)) {
            std::string debug_class2 = node->getAttribute("class");

            if (debug_class2.find("abs-badge") != std::string::npos) {
                DONG_LOG_INFO("[Painter] ABS badge text raw='%s'", raw_text.c_str());
            }

            if (debug_class2.find("overlay-row") != std::string::npos) {
                DONG_LOG_INFO("[Painter] overlay-row raw_text='%s' (len=%zu)", raw_text.c_str(), raw_text.size());
            }
            std::string text = collapseWhitespace(raw_text);
            if (debug_class2.find("overlay-row") != std::string::npos) {
                DONG_LOG_INFO("[Painter] overlay-row after collapse='%s' (len=%zu)", text.c_str(), text.size());
            }

            // 应用 text-transform
            if (!text.empty() && !style.text_transform.empty() && style.text_transform != "none") {
                if (style.text_transform == "uppercase") {
                    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
                        return static_cast<char>(std::toupper(c));
                    });
                } else if (style.text_transform == "lowercase") {
                    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
                        return static_cast<char>(std::tolower(c));
                    });
                }
                // capitalize 暂不支持（需要更复杂的 Unicode 处理）
            }

            if (!text.empty()) {
                float x = layout_node->layout.position[0];
                float y = layout_node->layout.position[1];
                float width = layout_node->layout.dimensions[0];
                float height = layout_node->layout.dimensions[1];

                float pad_left = style.padding_left.isPixel() ? style.padding_left.value : 0.0f;
                float pad_right = style.padding_right.isPixel() ? style.padding_right.value : 0.0f;
                float pad_top = style.padding_top.isPixel() ? style.padding_top.value : 0.0f;
                float pad_bottom = style.padding_bottom.isPixel() ? style.padding_bottom.value : 0.0f;

                const bool debug_button = (std::getenv("DONG_DEBUG_BUTTON") != nullptr);

                float font_size = style.font_size > 0.0f ? style.font_size : 16.0f;
                if (debug_button && tag == "button") {
                    DONG_LOG_INFO("[BTN] rect=(%.1f,%.1f %.1fx%.1f) pad=(%.1f %.1f %.1f %.1f) border_w=%.1f bg='%s' font=%.1f text='%s'",
                        x, y, width, height,
                        pad_top, pad_right, pad_bottom, pad_left,
                        style.border_width,
                        style.background_color.c_str(),
                        font_size,
                        text.c_str());
                }

                Color text_color = makeColorFromCss(style.color);

                float inner_width = width - pad_left - pad_right;
                if (inner_width <= 0.0f) inner_width = width > 0.0f ? width : 0.0f;

                // 使用 HarfBuzz 做一次完整 shaping，并基于 glyph 宽度进行换行
                TextShapeRequest full_req{};
                full_req.text = text;
                full_req.font_family = style.font_family;
                full_req.font_weight = style.font_weight;
                full_req.font_style = style.font_style;
                full_req.font_size = font_size;

                ShapedText shaped_full{};
                if (!text_shaper_.shape(full_req, shaped_full) || shaped_full.glyphs.empty()) {
                    return;
                }

                const float scale = shaped_full.scale_to_pixels;

                float letter_spacing_em = style.letter_spacing_em;
                float letter_spacing_px = 0.0f;
                float letter_spacing_units = 0.0f;
                if (letter_spacing_em != 0.0f) {
                    letter_spacing_px = letter_spacing_em * font_size;
                    if (scale > 0.0f) {
                        letter_spacing_units = letter_spacing_px / scale;
                    }
                }

                // word-spacing: 额外的单词间距（应用于空格字符）
                float word_spacing_px = style.word_spacing_px;
                float word_spacing_units = 0.0f;
                if (word_spacing_px != 0.0f && scale > 0.0f) {
                    word_spacing_units = word_spacing_px / scale;
                }

                // 行高/基线度量（设计单位）
                // 优先使用 CSS line-height，如果未设置则使用字体度量
                float line_height_units = shaped_full.line_height_units;
                float ascent_units = shaped_full.ascent_units;
                float descent_units = shaped_full.descent_units;

                // 应用 CSS line-height 属性
                if (style.line_height > 0.0f) {
                    if (style.line_height_is_unitless) {
                        float css_line_height_px = style.line_height * font_size;
                        line_height_units = css_line_height_px / std::max(scale, 1e-3f);
                    } else {
                        line_height_units = style.line_height / std::max(scale, 1e-3f);
                    }
                }

                if (line_height_units <= 0.0f) {
                    line_height_units = font_size / std::max(scale, 1e-3f);
                }
                if (ascent_units <= 0.0f) {
                    ascent_units = font_size / std::max(scale, 1e-3f);
                }
                float descent_abs_units = descent_units < 0.0f ? -descent_units : 0.0f;
                float metrics_height_units = ascent_units + descent_abs_units;
                if (metrics_height_units <= 0.0f) {
                    metrics_height_units = line_height_units;
                }
                float extra_leading_units = line_height_units - metrics_height_units;
                if (extra_leading_units < 0.0f) {
                    extra_leading_units = 0.0f;
                }
                float top_leading_units = extra_leading_units * 0.5f;

                float effective_line_height = line_height_units * scale;
                float ascent_px = ascent_units * scale;
                float baseline_offset = (top_leading_units + ascent_units) * scale;

                // 基于 UTF-8 字节偏移构建潜在换行点（空格 + CJK 字符后）
                std::vector<size_t> break_points;
                break_points.reserve(text.size() + 1);

                const auto push_break = [&](size_t byte_index) {
                    if (!break_points.empty() && break_points.back() == byte_index) {
                        return;
                    }
                    break_points.push_back(byte_index);
                };

                size_t i_byte = 0;
                while (i_byte < text.size()) {
                    unsigned char c = static_cast<unsigned char>(text[i_byte]);
                    size_t char_len = 1;
                    if ((c & 0b10000000) == 0) {
                        i_byte += 1;
                        if (c == ' ') {
                            push_break(i_byte);
                        }
                        continue;
                    } else if ((c & 0b11100000) == 0b11000000) {
                        char_len = 2;
                    } else if ((c & 0b11110000) == 0b11100000) {
                        char_len = 3;
                    } else if ((c & 0b11111000) == 0b11110000) {
                        char_len = 4;
                    }
                    i_byte += char_len;
                    // 对于非 ASCII（如 CJK），在字符后允许换行
                    push_break(i_byte);
                }

                if (break_points.empty() || break_points.back() != text.size()) {
                    break_points.push_back(text.size());
                }

                const auto measure_range_units = [&](size_t byte_start, size_t byte_end) -> float {
                    const auto& glyphs = shaped_full.glyphs;
                    int first = -1;
                    int last = -1;
                    int glyph_count = 0;
                    int space_count = 0;
                    for (size_t gi = 0; gi < glyphs.size(); ++gi) {
                        uint32_t cluster = glyphs[gi].cluster;
                        if (cluster < byte_start) {
                            continue;
                        }
                        if (cluster >= byte_end) {
                            break;
                        }
                        if (first == -1) {
                            first = static_cast<int>(gi);
                        }
                        last = static_cast<int>(gi);
                        ++glyph_count;
                        if (cluster < text.size() && text[cluster] == ' ') {
                            ++space_count;
                        }
                    }
                    if (first == -1 || last == -1 || glyph_count == 0) {
                        return 0.0f;
                    }
                    const ShapedGlyph& g_first = shaped_full.glyphs[static_cast<size_t>(first)];
                    const ShapedGlyph& g_last = shaped_full.glyphs[static_cast<size_t>(last)];
                    float left = g_first.pen_x_units;
                    float right = g_last.pen_x_units + g_last.advance_x_units;
                    float w = right - left;
                    if (glyph_count > 1 && letter_spacing_units != 0.0f) {
                        w += letter_spacing_units * static_cast<float>(glyph_count - 1);
                    }
                    if (space_count > 0 && word_spacing_units != 0.0f) {
                        int effective_spaces = space_count;
                        if (g_last.cluster < text.size() && text[g_last.cluster] == ' ') {
                            effective_spaces = std::max(0, effective_spaces - 1);
                        }
                        w += word_spacing_units * static_cast<float>(effective_spaces);
                    }

                    return w > 0.0f ? w : 0.0f;
                };

                const auto glyph_range_for_bytes = [&](size_t byte_start, size_t byte_end,
                                                       int& out_first, int& out_last) {
                    out_first = -1;
                    out_last = -1;
                    const auto& glyphs = shaped_full.glyphs;
                    for (size_t gi = 0; gi < glyphs.size(); ++gi) {
                        uint32_t cluster = glyphs[gi].cluster;
                        if (cluster < byte_start) {
                            continue;
                        }
                        if (cluster >= byte_end) {
                            break;
                        }
                        if (out_first == -1) {
                            out_first = static_cast<int>(gi);
                        }
                        out_last = static_cast<int>(gi);
                    }
                };

                const float max_line_width_px = inner_width;

                // Flex 容器的纯文本子节点：用 justify-content/align-items 做一次简化对齐。
                // 说明：我们当前不会把 TEXT 节点作为 Yoga flex item，因此需要在绘制阶段补齐常见对齐语义。
                const bool is_flex_container2 = (style.display == "flex" || style.display == "inline-flex");
                const bool flex_dir_is_row = (style.flex_direction.empty() || style.flex_direction == "row" || style.flex_direction == "row-reverse");

                std::string effective_text_align = style.text_align;
                float flex_baseline_adjust = 0.0f;

                if (is_flex_container2) {
                    const std::string& main_align = style.justify_content;
                    const std::string& cross_align = style.align_items;

                    if (flex_dir_is_row) {
                        if (main_align == "center") {
                            effective_text_align = "center";
                        } else if (main_align == "flex-end" || main_align == "end") {
                            effective_text_align = "right";
                        } else if (main_align == "flex-start" || main_align == "start") {
                            effective_text_align = "left";
                        }
                    } else {
                        if (cross_align == "center") {
                            effective_text_align = "center";
                        } else if (cross_align == "flex-end" || cross_align == "end") {
                            effective_text_align = "right";
                        } else if (cross_align == "flex-start" || cross_align == "start") {
                            effective_text_align = "left";
                        }
                    }

                    const float full_line_w_px = measure_range_units(0, text.size()) * scale;
                    const bool is_single_line = full_line_w_px <= max_line_width_px + 0.1f;
                    if (height > 0.0f && is_single_line) {
                        float inner_h = height - pad_top - pad_bottom;
                        if (inner_h < 0.0f) inner_h = 0.0f;

                        const float top_leading_px = top_leading_units * scale;
                        const float current_top = y + pad_top + top_leading_px;

                        float desired_top = y + pad_top;

                        const std::string& v_align = flex_dir_is_row ? cross_align : main_align;
                        if (v_align == "center") {
                            desired_top += std::max(0.0f, (inner_h - effective_line_height) * 0.5f);
                        } else if (v_align == "flex-end" || v_align == "end") {
                            desired_top += std::max(0.0f, (inner_h - effective_line_height));
                        }

                        flex_baseline_adjust = desired_top - current_top;
                    }
                }

                size_t text_len = text.size();
                size_t line_start = 0;
                int line_index = 0;

                while (line_start < text_len) {
                    // 跳过行首空格
                    while (line_start < text_len && text[line_start] == ' ') {
                        ++line_start;
                    }
                    if (line_start >= text_len) {
                        break;
                    }

                    float best_width_units = 0.0f;
                    size_t best_break = text_len;
                    bool found_any = false;

                    for (size_t bp : break_points) {
                        if (bp <= line_start) {
                            continue;
                        }
                        float w_units = measure_range_units(line_start, bp);
                        float w_px = w_units * scale;
                        if (w_px <= max_line_width_px + 0.1f) {
                            found_any = true;
                            best_break = bp;
                            best_width_units = w_units;
                        } else {
                            break;
                        }
                    }

                    if (!found_any) {
                        size_t fallback_break = text_len;
                        for (size_t bp : break_points) {
                            if (bp > line_start) {
                                fallback_break = bp;
                                break;
                            }
                        }
                        best_break = fallback_break;
                        best_width_units = measure_range_units(line_start, best_break);
                    }

                    int first_glyph = -1;
                    int last_glyph = -1;
                    glyph_range_for_bytes(line_start, best_break, first_glyph, last_glyph);
                    if (first_glyph == -1 || last_glyph == -1) {
                        line_start = best_break;
                        continue;
                    }

                    float line_width_px = best_width_units * scale;

                    float text_x = x + pad_left;
                    if (effective_text_align == "center") {
                        text_x = x + pad_left + std::max(0.0f, (inner_width - line_width_px) * 0.5f);
                    } else if (effective_text_align == "right") {
                        text_x = x + pad_left + std::max(0.0f, inner_width - line_width_px);
                    }

                    float base_baseline = y + pad_top + baseline_offset + flex_baseline_adjust;
                    float baseline_y = base_baseline + static_cast<float>(line_index) * effective_line_height;

                    DrawGlyphRunData glyph_run{};

                    glyph_run.rect.x = text_x;
                    glyph_run.rect.y = baseline_y - ascent_px;
                    glyph_run.rect.width = line_width_px;
                    glyph_run.rect.height = effective_line_height;
                    glyph_run.color = text_color;
                    glyph_run.font_size = font_size;
                    glyph_run.font_family = style.font_family;
                    glyph_run.font_weight = style.font_weight;
                    glyph_run.font_style = style.font_style;
                    glyph_run.font_path = shaped_full.font_path;

                    glyph_run.baseline_x = text_x;
                    glyph_run.baseline_y = baseline_y;
                    glyph_run.units_per_em = shaped_full.units_per_em;
                    glyph_run.scale_to_pixels = shaped_full.scale_to_pixels;
                    fillTextShadow(glyph_run, style);

                    const auto& glyphs = shaped_full.glyphs;
                    glyph_run.glyphs.reserve(static_cast<size_t>(last_glyph - first_glyph + 1));

                    float first_pen_x_units = glyphs[static_cast<size_t>(first_glyph)].pen_x_units;

                    int glyph_index_in_run = 0;
                    int accumulated_spaces = 0;
                    for (int gi = first_glyph; gi <= last_glyph; ++gi) {
                        const ShapedGlyph& sg = glyphs[static_cast<size_t>(gi)];
                        GlyphInstance inst{};
                        inst.glyph_id = sg.glyph_id;
                        float base_x_units = sg.pen_x_units - first_pen_x_units;
                        if (letter_spacing_units != 0.0f && glyph_index_in_run > 0) {
                            base_x_units += letter_spacing_units * static_cast<float>(glyph_index_in_run);
                        }
                        if (word_spacing_units != 0.0f && accumulated_spaces > 0) {
                            base_x_units += word_spacing_units * static_cast<float>(accumulated_spaces);
                        }
                        inst.pen_x_units = base_x_units;
                        inst.pen_y_units = sg.pen_y_units;
                        inst.font_path = sg.font_path;
                        inst.units_per_em = sg.units_per_em;
                        glyph_run.glyphs.push_back(inst);
                        ++glyph_index_in_run;
                        if (sg.cluster < text.size() && text[sg.cluster] == ' ') {
                            ++accumulated_spaces;
                        }
                    }

                    builder.addGlyphRun(std::move(glyph_run));

                    // 绘制 text-decoration（下划线/删除线/上划线）
                    if (!style.text_decoration.empty() && style.text_decoration != "none") {
                        Color deco_color = style.text_decoration_color.empty()
                            ? text_color
                            : makeColorFromCss(style.text_decoration_color);
                        float deco_thickness = std::max(1.0f, font_size * 0.05f);

                        if (style.text_decoration == "underline") {
                            Rect underline{text_x, baseline_y + font_size * 0.1f, line_width_px, deco_thickness};
                            builder.addRect(underline, deco_color);
                        } else if (style.text_decoration == "line-through") {
                            Rect strikethrough{text_x, baseline_y - ascent_px * 0.35f, line_width_px, deco_thickness};
                            builder.addRect(strikethrough, deco_color);
                        } else if (style.text_decoration == "overline") {
                            Rect overline{text_x, baseline_y - ascent_px, line_width_px, deco_thickness};
                            builder.addRect(overline, deco_color);
                        }
                    }

                    line_start = best_break;
                    ++line_index;
                }
            }
        }
    }
    // 3.5 Input 元素特殊渲染
    if (tag == "input") {
        float x = layout_node->layout.position[0];
        float y = layout_node->layout.position[1];
        float width = layout_node->layout.dimensions[0];
        float height = layout_node->layout.dimensions[1];

        float pad_left = style.padding_left.isPixel() ? style.padding_left.value : 0.0f;

        float font_size = style.font_size > 0.0f ? style.font_size : 16.0f;

        // 获取 value / placeholder
        std::string display_text = node->getAttribute("value");
        Color text_color = makeColorFromCss(style.color);

        if (display_text.empty()) {
            display_text = node->getAttribute("placeholder");
            // placeholder 使用半透明颜色
            text_color.a *= 0.5f;
        }

        if (!display_text.empty() && width > 0.0f && height > 0.0f) {
            TextShapeRequest req{};
            req.text = display_text;
            req.font_family = style.font_family;
            req.font_weight = style.font_weight;
            req.font_style = style.font_style;
            req.font_size = font_size;

            ShapedText shaped{};
            if (text_shaper_.shape(req, shaped) && !shaped.glyphs.empty()) {
                float scale = shaped.scale_to_pixels;
                float ascent_units = shaped.ascent_units > 0.0f ? shaped.ascent_units : font_size / scale;
                float descent_units = shaped.descent_units;
                float line_height_units = shaped.line_height_units;

                if (line_height_units <= 0.0f) line_height_units = font_size / scale;

                float descent_abs_units = descent_units < 0.0f ? -descent_units : 0.0f;
                float metrics_height_units = ascent_units + descent_abs_units;
                float extra_leading_units = std::max(line_height_units - metrics_height_units, 0.0f);
                float top_leading_units = extra_leading_units * 0.5f;

                float baseline_offset = (top_leading_units + ascent_units) * scale;
                float text_height_px = line_height_units * scale;

                // 垂直居中
                float text_y = y + (height - text_height_px) * 0.5f;
                float baseline_y = text_y + baseline_offset;
                float text_x = x + pad_left;

                DrawGlyphRunData glyph_run{};
                glyph_run.rect.x = text_x;
                glyph_run.rect.y = text_y;
                glyph_run.rect.width = shaped.width_units * scale;
                glyph_run.rect.height = text_height_px;
                glyph_run.color = text_color;
                glyph_run.font_size = font_size;
                glyph_run.font_family = style.font_family;
                glyph_run.font_weight = style.font_weight;
                glyph_run.font_style = style.font_style;
                glyph_run.font_path = shaped.font_path;

                glyph_run.baseline_x = text_x;
                glyph_run.baseline_y = baseline_y;
                glyph_run.units_per_em = shaped.units_per_em;
                glyph_run.scale_to_pixels = shaped.scale_to_pixels;
                fillTextShadow(glyph_run, style);

                for (const auto& sg : shaped.glyphs) {
                    GlyphInstance inst{};
                    inst.glyph_id = sg.glyph_id;
                    inst.pen_x_units = sg.pen_x_units;
                    inst.pen_y_units = sg.pen_y_units;
                    inst.font_path = sg.font_path;
                    inst.units_per_em = sg.units_per_em;
                    glyph_run.glyphs.push_back(inst);
                }

                builder.addGlyphRun(std::move(glyph_run));
            }
        }
    }
}

} // namespace dong::render
