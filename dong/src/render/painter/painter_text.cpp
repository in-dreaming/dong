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

    auto normalizeBorderStyle = [&](const std::string& s) -> std::string {
        return painter_detail::toLowerCopy(collapseWhitespace(s));
    };
    auto effectiveBorderStyle = [&](const std::string& side_style) -> std::string {
        return normalizeBorderStyle(!side_style.empty() ? side_style : style.border_style);
    };
    auto effectiveBorderWidth = [&](float side_width, const std::string& side_style) -> float {
        float w = (side_width >= 0.0f) ? side_width : style.border_width;
        if (w < 0.0f) w = 0.0f;
        const std::string st = effectiveBorderStyle(side_style);
        if (st == "none" || st == "hidden") return 0.0f;
        return w;
    };

    const float bt = effectiveBorderWidth(style.border_top_width, style.border_top_style);
    const float br = effectiveBorderWidth(style.border_right_width, style.border_right_style);
    const float bb = effectiveBorderWidth(style.border_bottom_width, style.border_bottom_style);
    const float bl = effectiveBorderWidth(style.border_left_width, style.border_left_style);

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
        const auto& children = node->getChildren();

        bool has_text_child = false;
        bool has_inline_element_child = false;
        size_t raw_text_len = 0;

        // Pass 1: 统计 + 判定（避免在这里拼接字符串导致反复扩容/拷贝）
        for (const auto& child : children) {
            if (!child) continue;
            if (child->getType() == dom::DOMNode::NodeType::TEXT) {
                raw_text_len += child->getRawTextContent().size();
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

        // 注意：混合内容路径会对 TEXT 节点逐个 shape，不需要提前拼出整段 raw_text。
        // 我们只在“整段文本路径”或调试日志需要时才构造 raw_text。
        const bool is_flex_container = (style.display == "flex" || style.display == "inline-flex");
        const bool use_mixed_path = (has_text_child && has_inline_element_child && tag_prefers_text && !is_flex_container);
        const bool need_full_text = (!use_mixed_path) && has_text_child && (tag_prefers_text || !has_inline_element_child);

        const bool debug_overlay_row = (debug_class.find("overlay-row") != std::string::npos);
        const bool debug_abs_badge = (debug_class.find("abs-badge") != std::string::npos);

        std::string raw_text;
        if ((need_full_text || debug_overlay_row || debug_abs_badge) && raw_text_len > 0) {
            raw_text.reserve(raw_text_len);
            for (const auto& child : children) {
                if (!child) continue;
                if (child->getType() == dom::DOMNode::NodeType::TEXT) {
                    raw_text.append(child->getRawTextContent());
                }
            }
        }

        if (debug_overlay_row) {
            DONG_LOG_INFO("[Painter] overlay-row: has_text=%d has_inline=%d tag_prefers=%d raw='%s'",
                    has_text_child, has_inline_element_child, tag_prefers_text, raw_text.c_str());
        }


        // 当有 inline 子元素时，需要按顺序处理每个子节点，避免文本重叠
        // 使用容器层完全接管混合内容的布局和绘制
        // 注意：如果容器是 flex 布局，不应该使用混合内容路径，因为 flex 会正确计算子元素位置
        if (use_mixed_path) {
            if (debug_overlay_row) {
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

            // Text painting coordinates should start at the padding box (border excluded).
            x += bl;
            y += bt;
            width = std::max(0.0f, width - bl - br);

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
                        size_t child_text_len = 0;
                        for (const auto& grandchild : child->getChildren()) {
                            if (grandchild && grandchild->getType() == dom::DOMNode::NodeType::TEXT) {
                                child_text_len += grandchild->getRawTextContent().size();
                            }
                        }
                        if (child_text_len > 0) {
                            child_text.reserve(child_text_len);
                        }
                        for (const auto& grandchild : child->getChildren()) {
                            if (grandchild && grandchild->getType() == dom::DOMNode::NodeType::TEXT) {
                                child_text.append(grandchild->getRawTextContent());
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
                                glyph_run.font_paths = shaped.font_paths;
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
                                    inst.font_path_index = sg.font_path_index;
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
                    const std::string& text_content = child->getRawTextContent();
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
                    glyph_run.font_paths = shaped.font_paths;
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
                        inst.font_path_index = sg.font_path_index;
                        inst.units_per_em = sg.units_per_em;
                        glyph_run.glyphs.push_back(inst);
                    }

                    builder.addGlyphRun(std::move(glyph_run));
                    cumulative_x_offset += text_width_px;
                }
            }
        } else if (need_full_text) {
            if (debug_abs_badge) {
                DONG_LOG_INFO("[Painter] ABS badge text raw='%s'", raw_text.c_str());
            }

            if (debug_overlay_row) {
                DONG_LOG_INFO("[Painter] overlay-row raw_text='%s' (len=%zu)", raw_text.c_str(), raw_text.size());
            }


            // Respect `white-space`:
            // - normal/nowrap: collapse whitespace into single spaces (current behavior)
            // - pre/pre-wrap: preserve whitespace/newlines
            // - pre-line: collapse spaces/tabs, preserve newlines
            std::string text;
            text.swap(raw_text);
            const std::string white_space = painter_detail::toLowerCopy(collapseWhitespace(style.white_space));
            if (white_space == "normal" || white_space == "nowrap") {
                text = collapseWhitespace(text);
            } else if (white_space == "pre-line") {
                text = painter_detail::collapseSpacesPreserveNewlines(text);
            }

            if (debug_overlay_row) {
                DONG_LOG_INFO("[Painter] overlay-row after ws('%s') len=%zu", white_space.c_str(), text.size());
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

                // Text painting coordinates should start at the padding box (border excluded).
                x += bl;
                y += bt;
                width = std::max(0.0f, width - bl - br);
                height = std::max(0.0f, height - bt - bb);

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

                const std::string white_space = painter_detail::toLowerCopy(collapseWhitespace(style.white_space));
                const bool preserve_newlines = (white_space == "pre" || white_space == "pre-wrap" || white_space == "pre-line");

                // Fast/correct path for multi-line text (e.g. `white-space: pre-wrap` logs):
                // Split by '\n' and shape/wrap per line to avoid O(n^2) behavior on a single mega-paragraph.
                if (preserve_newlines && text.find('\n') != std::string::npos) {
                    // Metrics (scale/ascent/line-height) from a small shaping request.
                    TextShapeRequest metrics_req{};
                    metrics_req.text = "X";
                    metrics_req.font_family = style.font_family;
                    metrics_req.font_weight = style.font_weight;
                    metrics_req.font_style = style.font_style;
                    metrics_req.font_size = font_size;

                    ShapedText metrics{};
                    if (!text_shaper_.shape(metrics_req, metrics)) {
                        return;
                    }

                    const float scale = metrics.scale_to_pixels;

                    float letter_spacing_em = style.letter_spacing_em;
                    float letter_spacing_px = 0.0f;
                    float letter_spacing_units = 0.0f;
                    if (letter_spacing_em != 0.0f) {
                        letter_spacing_px = letter_spacing_em * font_size;
                        if (scale > 0.0f) {
                            letter_spacing_units = letter_spacing_px / scale;
                        }
                    }

                    float word_spacing_px = style.word_spacing_px;
                    float word_spacing_units = 0.0f;
                    if (word_spacing_px != 0.0f && scale > 0.0f) {
                        word_spacing_units = word_spacing_px / scale;
                    }

                    float line_height_units = metrics.line_height_units;
                    float ascent_units = metrics.ascent_units;
                    float descent_units = metrics.descent_units;

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

                    const float max_line_width_px = inner_width;
                    const std::string effective_text_align = style.text_align;

                    auto emitRunForRange = [&](const std::string& line_text,
                                               const ShapedText& shaped_line,
                                               int first_glyph,
                                               int last_glyph,
                                               float baseline_y,
                                               float text_x) {
                        if (first_glyph < 0 || last_glyph < 0) return;
                        const auto& glyphs = shaped_line.glyphs;

                        DrawGlyphRunData glyph_run{};
                        glyph_run.color = text_color;
                        glyph_run.font_size = font_size;
                        glyph_run.font_family = style.font_family;
                        glyph_run.font_weight = style.font_weight;
                        glyph_run.font_style = style.font_style;
                        glyph_run.font_paths = shaped_line.font_paths;
                        glyph_run.font_path = shaped_line.font_path;
                        glyph_run.baseline_x = text_x;
                        glyph_run.baseline_y = baseline_y;
                        glyph_run.units_per_em = shaped_line.units_per_em;
                        glyph_run.scale_to_pixels = shaped_line.scale_to_pixels;
                        fillTextShadow(glyph_run, style);


                        glyph_run.rect.x = text_x;
                        glyph_run.rect.y = baseline_y - ascent_px;
                        glyph_run.rect.height = effective_line_height;

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
                            inst.font_path_index = sg.font_path_index;
                            inst.units_per_em = sg.units_per_em;


                            glyph_run.glyphs.push_back(inst);

                            if (word_spacing_units != 0.0f && sg.cluster < line_text.size() && line_text[sg.cluster] == ' ') {
                                accumulated_spaces += 1;
                            }
                            glyph_index_in_run += 1;
                        }

                        // Compute width in px from glyphs we actually emitted.
                        const ShapedGlyph& g_first = glyphs[static_cast<size_t>(first_glyph)];
                        const ShapedGlyph& g_last = glyphs[static_cast<size_t>(last_glyph)];
                        float left_units = g_first.pen_x_units;
                        float right_units = g_last.pen_x_units + g_last.advance_x_units;
                        float w_units = right_units - left_units;
                        if (last_glyph > first_glyph && letter_spacing_units != 0.0f) {
                            w_units += letter_spacing_units * static_cast<float>(last_glyph - first_glyph);
                        }
                        glyph_run.rect.width = std::max(0.0f, w_units * scale);

                        builder.addGlyphRun(std::move(glyph_run));
                    };

                    auto shapeAndWrapLine = [&](std::string line_text, int& line_index) {
                        if (line_text.empty()) {
                            line_index += 1;
                            return;
                        }

                        // Drop trailing CR if the HTML used CRLF.
                        if (!line_text.empty() && line_text.back() == '\r') {
                            line_text.pop_back();
                        }

                        TextShapeRequest req{};
                        req.text = line_text;
                        req.font_family = style.font_family;
                        req.font_weight = style.font_weight;
                        req.font_style = style.font_style;
                        req.font_size = font_size;

                        ShapedText shaped_line{};
                        if (!text_shaper_.shape(req, shaped_line)) {
                            line_index += 1;
                            return;
                        }

                        // `pre`: no wrapping, emit the whole line.
                        if (white_space == "pre" || max_line_width_px <= 0.0f) {
                            float line_width_px = shaped_line.width_units * scale;
                            float text_x = x + pad_left;
                            if (effective_text_align == "center") {
                                text_x = x + pad_left + std::max(0.0f, (inner_width - line_width_px) * 0.5f);
                            } else if (effective_text_align == "right") {
                                text_x = x + pad_left + std::max(0.0f, inner_width - line_width_px);
                            }
                            float baseline_y = y + pad_top + baseline_offset + static_cast<float>(line_index) * effective_line_height;
                            if (!shaped_line.glyphs.empty()) {
                                emitRunForRange(line_text, shaped_line, 0, static_cast<int>(shaped_line.glyphs.size() - 1), baseline_y, text_x);
                            }
                            line_index += 1;
                            return;
                        }

                        if (shaped_line.glyphs.empty()) {
                            line_index += 1;
                            return;
                        }

                        // Fast path: most log lines fit without wrapping.
                        {
                            float full_w_px = shaped_line.width_units * scale;
                            if (full_w_px <= max_line_width_px + 0.1f) {
                                float text_x = x + pad_left;
                                if (effective_text_align == "center") {
                                    text_x = x + pad_left + std::max(0.0f, (inner_width - full_w_px) * 0.5f);
                                } else if (effective_text_align == "right") {
                                    text_x = x + pad_left + std::max(0.0f, inner_width - full_w_px);
                                }
                                float baseline_y = y + pad_top + baseline_offset + static_cast<float>(line_index) * effective_line_height;
                                emitRunForRange(line_text, shaped_line, 0, static_cast<int>(shaped_line.glyphs.size() - 1), baseline_y, text_x);
                                line_index += 1;
                                return;
                            }
                        }

                        const auto& glyphs = shaped_line.glyphs;


                        // Pre-build clusters for binary searching glyph ranges.
                        std::vector<uint32_t> glyph_clusters;
                        glyph_clusters.reserve(glyphs.size());
                        std::vector<int> space_prefix(glyphs.size() + 1, 0);
                        for (size_t gi = 0; gi < glyphs.size(); ++gi) {
                            glyph_clusters.push_back(glyphs[gi].cluster);
                            const bool is_space = (glyphs[gi].cluster < line_text.size() && line_text[glyphs[gi].cluster] == ' ');
                            space_prefix[gi + 1] = space_prefix[gi] + (is_space ? 1 : 0);
                        }

                        const auto glyph_range_for_bytes = [&](size_t byte_start, size_t byte_end,
                                                               int& out_first, int& out_last) {
                            out_first = -1;
                            out_last = -1;
                            if (glyphs.empty() || byte_end <= byte_start) {
                                return;
                            }

                            auto it_first = std::lower_bound(glyph_clusters.begin(), glyph_clusters.end(), static_cast<uint32_t>(byte_start));
                            auto it_end = std::lower_bound(glyph_clusters.begin(), glyph_clusters.end(), static_cast<uint32_t>(byte_end));

                            const size_t first_idx = static_cast<size_t>(it_first - glyph_clusters.begin());
                            const size_t end_idx = static_cast<size_t>(it_end - glyph_clusters.begin());
                            if (first_idx >= end_idx) {
                                return;
                            }
                            out_first = static_cast<int>(first_idx);
                            out_last = static_cast<int>(end_idx - 1);
                        };

                        const auto measure_range_units = [&](size_t byte_start, size_t byte_end) -> float {
                            int first = -1;
                            int last = -1;
                            glyph_range_for_bytes(byte_start, byte_end, first, last);
                            if (first < 0 || last < 0) {
                                return 0.0f;
                            }

                            const ShapedGlyph& g_first = glyphs[static_cast<size_t>(first)];
                            const ShapedGlyph& g_last = glyphs[static_cast<size_t>(last)];

                            float left = g_first.pen_x_units;
                            float right = g_last.pen_x_units + g_last.advance_x_units;
                            float w = right - left;

                            const int glyph_count = last - first + 1;
                            if (glyph_count > 1 && letter_spacing_units != 0.0f) {
                                w += letter_spacing_units * static_cast<float>(glyph_count - 1);
                            }

                            if (word_spacing_units != 0.0f) {
                                const int space_count = space_prefix[static_cast<size_t>(last) + 1] - space_prefix[static_cast<size_t>(first)];
                                if (space_count > 0) {
                                    w += word_spacing_units * static_cast<float>(space_count);
                                }
                            }

                            return w > 0.0f ? w : 0.0f;
                        };

                        // Break points (spaces + potential CJK char boundaries).
                        std::vector<size_t> break_points;
                        break_points.reserve(line_text.size() + 1);
                        const auto push_break = [&](size_t byte_index) {
                            if (!break_points.empty() && break_points.back() == byte_index) {
                                return;
                            }
                            break_points.push_back(byte_index);
                        };

                        size_t i_byte = 0;
                        while (i_byte < line_text.size()) {
                            unsigned char c = static_cast<unsigned char>(line_text[i_byte]);
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
                            push_break(i_byte);
                        }
                        if (break_points.empty() || break_points.back() != line_text.size()) {
                            break_points.push_back(line_text.size());
                        }

                        size_t text_len = line_text.size();
                        size_t line_start = 0;

                        while (line_start < text_len) {
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
                                line_index += 1;
                                continue;
                            }

                            float line_width_px = best_width_units * scale;

                            float text_x = x + pad_left;
                            if (effective_text_align == "center") {
                                text_x = x + pad_left + std::max(0.0f, (inner_width - line_width_px) * 0.5f);
                            } else if (effective_text_align == "right") {
                                text_x = x + pad_left + std::max(0.0f, inner_width - line_width_px);
                            }

                            float baseline_y = y + pad_top + baseline_offset + static_cast<float>(line_index) * effective_line_height;

                            emitRunForRange(line_text, shaped_line, first_glyph, last_glyph, baseline_y, text_x);

                            line_start = best_break;
                            // Trim leading spaces for wrapped continuation lines only for normal/pre-line.
                            if (white_space != "pre-wrap") {
                                while (line_start < text_len && line_text[line_start] == ' ') {
                                    ++line_start;
                                }
                            }
                            line_index += 1;
                        }
                    };

                    int line_index = 0;
                    size_t start = 0;
                    while (start <= text.size()) {
                        size_t end = text.find('\n', start);
                        if (end == std::string::npos) {
                            end = text.size();
                        }
                        std::string line_text = (end > start) ? text.substr(start, end - start) : std::string();
                        shapeAndWrapLine(std::move(line_text), line_index);
                        if (end >= text.size()) {
                            break;
                        }
                        start = end + 1;
                    }

                    return;
                }

                // Single-paragraph path (no explicit newlines): shape once, then wrap.
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

                // 预构建 cluster->glyph 索引，避免每次测量都 O(n) 扫整段 glyph。
                // 这在长文本（例如 log）+ 多次换行试探时能显著降低 buildDisplayList 时间。
                const auto& glyphs = shaped_full.glyphs;
                std::vector<uint32_t> glyph_clusters;
                glyph_clusters.reserve(glyphs.size());
                std::vector<int> space_prefix(glyphs.size() + 1, 0);
                for (size_t gi = 0; gi < glyphs.size(); ++gi) {
                    glyph_clusters.push_back(glyphs[gi].cluster);
                    const bool is_space = (glyphs[gi].cluster < text.size() && text[glyphs[gi].cluster] == ' ');
                    space_prefix[gi + 1] = space_prefix[gi] + (is_space ? 1 : 0);
                }

                const auto glyph_range_for_bytes = [&](size_t byte_start, size_t byte_end,
                                                       int& out_first, int& out_last) {
                    out_first = -1;
                    out_last = -1;
                    if (glyphs.empty() || byte_end <= byte_start) {
                        return;
                    }

                    // clusters 是非递减序列（ligature 可能导致重复 cluster），lower_bound 仍然可用。
                    auto it_first = std::lower_bound(glyph_clusters.begin(), glyph_clusters.end(), static_cast<uint32_t>(byte_start));
                    auto it_end = std::lower_bound(glyph_clusters.begin(), glyph_clusters.end(), static_cast<uint32_t>(byte_end));

                    const size_t first_idx = static_cast<size_t>(it_first - glyph_clusters.begin());
                    const size_t end_idx = static_cast<size_t>(it_end - glyph_clusters.begin());
                    if (first_idx >= end_idx) {
                        return;
                    }
                    out_first = static_cast<int>(first_idx);
                    out_last = static_cast<int>(end_idx - 1);
                };

                const auto measure_range_units = [&](size_t byte_start, size_t byte_end) -> float {
                    int first = -1;
                    int last = -1;
                    glyph_range_for_bytes(byte_start, byte_end, first, last);
                    if (first < 0 || last < 0) {
                        return 0.0f;
                    }

                    const size_t first_u = static_cast<size_t>(first);
                    const size_t last_u = static_cast<size_t>(last);

                    const ShapedGlyph& g_first = glyphs[first_u];
                    const ShapedGlyph& g_last = glyphs[last_u];

                    float left = g_first.pen_x_units;
                    float right = g_last.pen_x_units + g_last.advance_x_units;
                    float w = right - left;

                    const int glyph_count = static_cast<int>(last_u - first_u + 1);
                    if (glyph_count > 1 && letter_spacing_units != 0.0f) {
                        w += letter_spacing_units * static_cast<float>(glyph_count - 1);
                    }

                    if (word_spacing_units != 0.0f) {
                        const int space_count = space_prefix[last_u + 1] - space_prefix[first_u];
                        if (space_count > 0) {
                            int effective_spaces = space_count;
                            if (g_last.cluster < text.size() && text[g_last.cluster] == ' ') {
                                effective_spaces = std::max(0, effective_spaces - 1);
                            }
                            w += word_spacing_units * static_cast<float>(effective_spaces);
                        }
                    }

                    return w > 0.0f ? w : 0.0f;
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
                    glyph_run.font_paths = shaped_full.font_paths;
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
                        inst.font_path_index = sg.font_path_index;
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

        // Text painting coordinates should start at the padding box (border excluded).
        x += bl;
        y += bt;
        width = std::max(0.0f, width - bl - br);
        height = std::max(0.0f, height - bt - bb);

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
                glyph_run.font_paths = shaped.font_paths;
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
                    inst.font_path_index = sg.font_path_index;
                    inst.units_per_em = sg.units_per_em;
                    glyph_run.glyphs.push_back(inst);
                }


                builder.addGlyphRun(std::move(glyph_run));
            }
        }
    }
}

} // namespace dong::render
