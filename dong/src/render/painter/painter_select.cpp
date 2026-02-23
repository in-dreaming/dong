// painter_select.cpp - Select element rendering
#include "painter_style_utils.hpp"
#include "../../dom/select_element.hpp"
#include "../text_shaper.hpp"
#include "../display_list.hpp"

namespace dong::render {
namespace {

using painter_detail::makeColorFromCss;

void renderSelectClosed(
    const dom::DOMNodePtr& node,
    const dom::SelectElementState* state,
    const layout::LayoutNode* layout,
    const dom::ComputedStyle& style,
    float bl, float bt, float br, float bb,
    TextShaper& shaper,
    DisplayListBuilder& builder
) {
    // Compute select box bounds
    float x = layout->layout.position[0] + bl;
    float y = layout->layout.position[1] + bt;
    float w = std::max(0.0f, layout->layout.dimensions[0] - bl - br);
    float h = std::max(0.0f, layout->layout.dimensions[1] - bt - bb);

    if (w <= 0.0f || h <= 0.0f) return;

    // Get padding
    float pad_l = style.padding_left.isPixel() ? style.padding_left.value : 4.0f;
    float pad_r = style.padding_right.isPixel() ? style.padding_right.value : 4.0f;
    float pad_t = style.padding_top.isPixel() ? style.padding_top.value : 4.0f;

    // Draw background (use style background-color or default white)
    Color bg_color = {1.0f, 1.0f, 1.0f, 1.0f}; // default white
    if (!style.background_color.empty() && style.background_color != "transparent") {
        bg_color = makeColorFromCss(style.background_color);
    }

    float border_radius = style.border_radius;
    if (border_radius > 0.0f) {
        builder.addRoundedRect(Rect{x, y, w, h}, bg_color, border_radius);
    } else {
        builder.addRect(Rect{x, y, w, h}, bg_color);
    }

    // Draw border if specified
    float border_width = style.border_width >= 0.0f ? style.border_width : 0.0f;
    if (border_width > 0.0f) {
        Color border_color = {0.7f, 0.7f, 0.7f, 1.0f}; // default gray
        if (!style.border_color.empty() && style.border_color != "transparent") {
            border_color = makeColorFromCss(style.border_color);
        }

        if (border_radius > 0.0f) {
            builder.addRoundedRect(Rect{x, y, w, h}, border_color, border_radius, border_width);
        } else {
            // Draw border as 4 rects (top, right, bottom, left)
            builder.addRect(Rect{x, y, w, border_width}, border_color);                    // top
            builder.addRect(Rect{x + w - border_width, y, border_width, h}, border_color); // right
            builder.addRect(Rect{x, y + h - border_width, w, border_width}, border_color); // bottom
            builder.addRect(Rect{x, y, border_width, h}, border_color);                    // left
        }
    }

    // Get selected option text
    std::string display_text;
    if (state->getOptionCount() > 0 && state->getSelectedIndex() < state->getOptionCount()) {
        display_text = state->getOptions()[state->getSelectedIndex()].display_text;
    }

    float font_size = style.font_size > 0.0f ? style.font_size : 16.0f;

    // Shape and draw text (left-aligned with padding)
    if (!display_text.empty()) {
        TextShapeRequest req{display_text, style.font_family, style.font_weight, style.font_style, font_size};
        ShapedText shaped;
        if (shaper.shape(req, shaped) && !shaped.glyphs.empty()) {
            float scale = shaped.scale_to_pixels;
            float ascent = shaped.ascent_units > 0.0f ? shaped.ascent_units : font_size / scale;
            float line_h = shaped.line_height_units;
            if (line_h <= 0.0f) line_h = font_size / scale;
            float descent_abs = shaped.descent_units < 0.0f ? -shaped.descent_units : 0.0f;
            float extra = std::max(line_h - (ascent + descent_abs), 0.0f);
            float baseline_off = (extra * 0.5f + ascent) * scale;
            float text_h_px = line_h * scale;

            float text_y = y + (h - text_h_px) * 0.5f;
            float baseline_y = text_y + baseline_off;
            float text_x = x + pad_l;

            DrawGlyphRunData run;
            run.rect = {text_x, text_y, shaped.width_units * scale, text_h_px};
            run.color = makeColorFromCss(style.color);
            run.font_size = font_size;
            run.font_family = style.font_family;
            run.font_weight = style.font_weight;
            run.font_style = style.font_style;
            run.font_paths = shaped.font_paths;
            run.font_path = shaped.font_path;
            run.baseline_x = text_x;
            run.baseline_y = baseline_y;
            run.units_per_em = shaped.units_per_em;
            run.scale_to_pixels = shaped.scale_to_pixels;
            painter_detail::fillTextShadow(run, style);

            for (const auto& sg : shaped.glyphs) {
                GlyphInstance inst{sg.glyph_id, sg.pen_x_units, sg.pen_y_units,
                                  sg.font_path_index, sg.units_per_em};
                run.glyphs.push_back(inst);
            }
            builder.addGlyphRun(std::move(run));
        }
    }

    // Draw dropdown arrow (right side) - using Unicode ▼ (U+25BC)
    const std::string arrow_text = "\xE2\x96\xBC"; // UTF-8 encoding of ▼
    const float arrow_width = 20.0f;
    float arrow_x = x + w - arrow_width;

    TextShapeRequest arrow_req{arrow_text, style.font_family, style.font_weight, style.font_style, font_size};
    ShapedText arrow_shaped;
    if (shaper.shape(arrow_req, arrow_shaped) && !arrow_shaped.glyphs.empty()) {
        float scale = arrow_shaped.scale_to_pixels;
        float ascent = arrow_shaped.ascent_units > 0.0f ? arrow_shaped.ascent_units : font_size / scale;
        float line_h = arrow_shaped.line_height_units;
        if (line_h <= 0.0f) line_h = font_size / scale;
        float descent_abs = arrow_shaped.descent_units < 0.0f ? -arrow_shaped.descent_units : 0.0f;
        float extra = std::max(line_h - (ascent + descent_abs), 0.0f);
        float baseline_off = (extra * 0.5f + ascent) * scale;
        float text_h_px = line_h * scale;

        float arrow_y = y + (h - text_h_px) * 0.5f;
        float arrow_baseline_y = arrow_y + baseline_off;

        DrawGlyphRunData arrow_run;
        arrow_run.rect = {arrow_x, arrow_y, arrow_shaped.width_units * scale, text_h_px};
        arrow_run.color = makeColorFromCss(style.color);
        arrow_run.font_size = font_size;
        arrow_run.font_family = style.font_family;
        arrow_run.font_weight = style.font_weight;
        arrow_run.font_style = style.font_style;
        arrow_run.font_paths = arrow_shaped.font_paths;
        arrow_run.font_path = arrow_shaped.font_path;
        arrow_run.baseline_x = arrow_x;
        arrow_run.baseline_y = arrow_baseline_y;
        arrow_run.units_per_em = arrow_shaped.units_per_em;
        arrow_run.scale_to_pixels = arrow_shaped.scale_to_pixels;
        painter_detail::fillTextShadow(arrow_run, style);

        for (const auto& sg : arrow_shaped.glyphs) {
            GlyphInstance inst{sg.glyph_id, sg.pen_x_units, sg.pen_y_units,
                              sg.font_path_index, sg.units_per_em};
            arrow_run.glyphs.push_back(inst);
        }
        builder.addGlyphRun(std::move(arrow_run));
    }
}

} // anonymous namespace

void renderSelect(
    const dom::DOMNodePtr& node,
    const layout::LayoutNode* layout,
    const dom::ComputedStyle& style,
    float bl, float bt, float br, float bb,
    TextShaper& shaper,
    DisplayListBuilder& builder
) {
    // Get SelectElementState
    auto* state = dong::dom::getSelectState(node);
    if (!state) return;

    // For Chunk 2: only render closed state
    // Dropdown rendering will be added in Chunk 4
    if (!state->isOpen()) {
        renderSelectClosed(node, state, layout, style, bl, bt, br, bb, shaper, builder);
    }
}

} // namespace dong::render
