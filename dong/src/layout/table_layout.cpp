// table_layout.cpp - Post-layout pass for CSS table elements
// Implements basic table layout (border-collapse, border-spacing, table-layout: fixed/auto)

#include "layout_engine.hpp"
#include "../core/log.h"
#include "../core/string_utils.h"
#include "../render/text_shaper.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

namespace dong::layout {

namespace {

bool isTableDisplay(const std::string& display) {
    return display == "table" || display == "inline-table";
}

bool isTableRowDisplay(const std::string& display) {
    return display == "table-row";
}

bool isTableCellDisplay(const std::string& display) {
    return display == "table-cell";
}

bool isTableRowGroupDisplay(const std::string& display) {
    return display == "table-row-group" || display == "table-header-group" ||
           display == "table-footer-group";
}

static dom::DOMNodePtr findTableCaptionNode(const dom::DOMNodePtr& table_node) {
    if (!table_node) return nullptr;
    for (const auto& child : table_node->getChildren()) {
        if (!child || child->getType() != dom::DOMNode::NodeType::ELEMENT) continue;
        if (child->getComputedStyle().display == "table-caption") {
            return child;
        }
    }
    return nullptr;
}

static bool isCaptionSideBottom(const dom::DOMNodePtr& caption_node) {
    if (!caption_node) return false;
    std::string side = ::dong::toLower(caption_node->getComputedStyle().caption_side);
    return side == "bottom";
}

static float getNodeLayoutHeightPx(Engine* engine, const dom::DOMNodePtr& node) {
    if (!engine || !node) return 0.0f;
    const auto* ln = engine->getLayout(node);
    if (!ln) return 0.0f;
    const float h = ln->layout.dimensions[1];
    return (h > 0.0f && std::isfinite(h)) ? h : 0.0f;
}

void collectRows(const dom::DOMNodePtr& table_node,
                 std::vector<dom::DOMNodePtr>& rows) {
    for (const auto& child : table_node->getChildren()) {
        if (!child || child->getType() != dom::DOMNode::NodeType::ELEMENT) continue;
        const auto& display = child->getComputedStyle().display;
        if (isTableRowDisplay(display)) {
            rows.push_back(child);
        } else if (isTableRowGroupDisplay(display)) {
            for (const auto& row_child : child->getChildren()) {
                if (!row_child || row_child->getType() != dom::DOMNode::NodeType::ELEMENT) continue;
                if (isTableRowDisplay(row_child->getComputedStyle().display)) {
                    rows.push_back(row_child);
                }
            }
        }
    }
}

void collectCells(const dom::DOMNodePtr& row_node,
                  std::vector<dom::DOMNodePtr>& cells) {
    for (const auto& child : row_node->getChildren()) {
        if (!child || child->getType() != dom::DOMNode::NodeType::ELEMENT) continue;
        if (isTableCellDisplay(child->getComputedStyle().display)) {
            cells.push_back(child);
        }
    }
}

struct RowBlock {
    dom::DOMNodePtr group; // nullptr means rows are direct children of <table>
    std::vector<dom::DOMNodePtr> rows;
};

// Forward declaration (used by helpers below)
void updateLayoutRect(LayoutNode* ln, float x, float y, float w, float h);


std::vector<RowBlock> collectRowBlocks(const dom::DOMNodePtr& table_node) {
    std::vector<RowBlock> blocks;

    for (const auto& child : table_node->getChildren()) {
        if (!child || child->getType() != dom::DOMNode::NodeType::ELEMENT) continue;

        const auto& display = child->getComputedStyle().display;
        if (isTableRowDisplay(display)) {
            RowBlock b;
            b.rows.push_back(child);
            blocks.push_back(std::move(b));
            continue;
        }

        if (isTableRowGroupDisplay(display)) {
            RowBlock b;
            b.group = child;
            for (const auto& row_child : child->getChildren()) {
                if (!row_child || row_child->getType() != dom::DOMNode::NodeType::ELEMENT) continue;
                if (isTableRowDisplay(row_child->getComputedStyle().display)) {
                    b.rows.push_back(row_child);
                }
            }
            if (!b.rows.empty()) {
                blocks.push_back(std::move(b));
            }
        }
    }

    return blocks;
}

float computeRowHeightPx(Engine* engine, const std::vector<dom::DOMNodePtr>& cells) {
    float row_h = 0.0f;
    for (const auto& cell : cells) {
        const auto* cl = engine->getLayout(cell);
        if (!cl) continue;
        row_h = std::max(row_h, cl->layout.dimensions[1]);
    }
    return row_h;
}

float positionCellsInRowAbs(Engine* engine,
                            const std::vector<dom::DOMNodePtr>& cells,
                            const std::vector<float>& col_widths,
                            float row_x,
                            float row_y,
                            float spacing,
                            float row_h) {
    float cur_x = row_x;
    for (size_t c = 0; c < cells.size() && c < col_widths.size(); ++c) {
        auto* cell_ln = engine->getLayoutMutable(cells[c]);
        if (!cell_ln) continue;
        updateLayoutRect(cell_ln, cur_x, row_y, col_widths[c], row_h);
        cur_x += col_widths[c] + spacing;
    }

    if (!cells.empty() && spacing > 0.0f) {
        cur_x = std::max(row_x, cur_x - spacing);
    }

    return std::max(0.0f, cur_x - row_x);
}


size_t getMaxColumns(const std::vector<dom::DOMNodePtr>& rows) {
    size_t max_cols = 0;
    for (const auto& row : rows) {
        std::vector<dom::DOMNodePtr> cells;
        collectCells(row, cells);
        max_cols = std::max(max_cols, cells.size());
    }
    return max_cols;
}


// Update a LayoutNode's position and dimensions consistently
void updateLayoutRect(LayoutNode* ln, float x, float y, float w, float h) {
    ln->layout.position[0] = x;
    ln->layout.position[1] = y;
    ln->layout.dimensions[0] = w;
    ln->layout.dimensions[1] = h;
    ln->x = x;
    ln->y = y;
    ln->width = w;
    ln->height = h;
}

void appendTextRecursive(const dom::DOMNodePtr& node, std::string& out) {
    if (!node) return;
    if (node->getType() == dom::DOMNode::NodeType::TEXT) {
        out.append(node->getTextContent());
        return;
    }
    for (const auto& ch : node->getChildren()) {
        appendTextRecursive(ch, out);
    }
}

std::string collectCellText(const dom::DOMNodePtr& cell) {
    std::string raw;
    for (const auto& ch : cell->getChildren()) {
        appendTextRecursive(ch, raw);
    }
    return ::dong::collapseWhitespace(raw);
}

float measureTextWidthPx(const std::string& text, const dom::ComputedStyle& style) {
    using ::dong::render::TextMeasureCache;
    using ::dong::render::TextMeasureCacheKey;
    using ::dong::render::TextMeasureResult;
    using ::dong::render::TextShaper;
    using ::dong::render::TextShapeRequest;
    using ::dong::render::ShapedText;

    if (text.empty()) return 0.0f;

    const float font_size = (style.font_size > 0.0f && std::isfinite(style.font_size)) ? style.font_size : 16.0f;

    TextMeasureCacheKey key{
        text,
        style.font_family,
        style.font_weight,
        style.font_style,
        font_size,
        style.letter_spacing_em,
        style.word_spacing_px
    };

    TextMeasureResult cached;
    auto& cache = TextMeasureCache::instance();
    if (cache.lookup(key, cached) && cached.valid) {
        return cached.content_width_px;
    }

    TextShaper shaper;
    TextShapeRequest req{text, style.font_family, style.font_weight, style.font_style, font_size};
    ShapedText shaped;
    if (!shaper.shape(req, shaped) || shaped.glyphs.empty() || shaped.scale_to_pixels <= 0.0f) {
        return 0.0f;
    }

    float width_px = shaped.width_units * shaped.scale_to_pixels;
    if (width_px < 0.0f || !std::isfinite(width_px)) width_px = 0.0f;

    TextMeasureResult r;
    r.content_width_px = width_px;
    r.line_height_px = shaped.line_height_units * shaped.scale_to_pixels;
    r.scale_to_pixels = shaped.scale_to_pixels;
    r.ascent_units = shaped.ascent_units;
    r.descent_units = shaped.descent_units;
    r.line_height_units = shaped.line_height_units;
    r.glyph_count = shaped.glyphs.size();
    r.valid = true;
    cache.insert(key, r);

    return width_px;
}

float measureCellPreferredWidth(const dom::DOMNodePtr& cell) {
    if (!cell) return 0.0f;
    const auto& cs = cell->getComputedStyle();

    float pad_l = cs.padding_left.isPixel() ? cs.padding_left.value : 0.0f;
    float pad_r = cs.padding_right.isPixel() ? cs.padding_right.value : 0.0f;

    float border = 0.0f;
    if (cs.border_style != "none" && cs.border_style != "hidden") {
        border = std::max(0.0f, cs.border_width);
    }

    const std::string text = collectCellText(cell);
    const float text_w = measureTextWidthPx(text, cs);

    const float w = text_w + pad_l + pad_r + border * 2.0f;
    return (w > 0.0f && std::isfinite(w)) ? w : 0.0f;
}

} // anonymous namespace

void Engine::layoutTableElements(dom::DOMNodePtr root) {
    if (!root) return;

    const auto& style = root->getComputedStyle();
    if (isTableDisplay(style.display)) {
        layoutSingleTable(root);
    }

    for (const auto& child : root->getChildren()) {
        if (!child) continue;
        if (child->getType() == dom::DOMNode::NodeType::ELEMENT) {
            layoutTableElements(child);
        }
    }
}

void Engine::layoutSingleTable(const dom::DOMNodePtr& table_node) {
    auto* table_ln = getLayoutMutable(table_node);
    if (!table_ln) return;

    const auto& style = table_node->getComputedStyle();
    const bool is_collapse = (style.border_collapse == "collapse");
    const float spacing_x = is_collapse ? 0.0f : style.border_spacing_x;
    const float spacing_y = is_collapse ? 0.0f : style.border_spacing_y;
    const bool is_fixed = (style.table_layout == "fixed");

    std::vector<dom::DOMNodePtr> rows;
    collectRows(table_node, rows);
    if (rows.empty()) return;

    const size_t num_cols = getMaxColumns(rows);
    if (num_cols == 0) return;

    std::vector<float> col_widths(num_cols, 0.0f);
    computeColumnWidths(table_node, rows, num_cols, is_fixed, spacing_x, col_widths);
    positionTableCells(table_node, rows, col_widths, spacing_x, spacing_y, is_collapse);
}

void Engine::computeColumnWidths(
    const dom::DOMNodePtr& table_node,
    const std::vector<dom::DOMNodePtr>& rows,
    size_t num_cols, bool is_fixed, float spacing_x,
    std::vector<float>& col_widths) {

    auto* table_ln = getLayoutMutable(table_node);
    if (!table_ln) return;

    const auto& ts = table_node->getComputedStyle();
    const float table_width = table_ln->layout.dimensions[0];
    const float bw = ts.border_width;
    const float pt_l = ts.padding_left.isPixel() ? ts.padding_left.value : 0.0f;
    const float pt_r = ts.padding_right.isPixel() ? ts.padding_right.value : 0.0f;
    const float avail = table_width - bw * 2.0f - pt_l - pt_r;
    const float total_spacing = spacing_x * static_cast<float>(num_cols + 1);

    if (is_fixed && table_width > 0.0f) {
        float col_w = std::max(0.0f, (avail - total_spacing) / static_cast<float>(num_cols));
        for (size_t c = 0; c < num_cols; ++c) {
            col_widths[c] = col_w;
        }
        return;
    }

    // Auto: approximate column widths from cell preferred widths (text + padding + borders).
    // Yoga doesn't implement CSS tables; its computed widths for <td>/<th> are often unusable here.
    for (const auto& row : rows) {
        std::vector<dom::DOMNodePtr> cells;
        collectCells(row, cells);
        for (size_t c = 0; c < cells.size() && c < num_cols; ++c) {
            col_widths[c] = std::max(col_widths[c], measureCellPreferredWidth(cells[c]));
        }
    }

    float total_cell_w = 0.0f;
    for (size_t c = 0; c < num_cols; ++c) total_cell_w += col_widths[c];

    const float target = avail - total_spacing;
    if (target <= 0.0f || !std::isfinite(target)) {
        return;
    }

    if (total_cell_w <= 0.0f || !std::isfinite(total_cell_w)) {
        const float col_w = target / static_cast<float>(num_cols);
        for (size_t c = 0; c < num_cols; ++c) col_widths[c] = std::max(0.0f, col_w);
        return;
    }

    if (total_cell_w < target) {
        const float remaining = target - total_cell_w;
        for (size_t c = 0; c < num_cols; ++c) {
            col_widths[c] += remaining * (col_widths[c] / total_cell_w);
        }
        return;
    }

    // Shrink-to-fit: scale down proportionally.
    const float scale = target / total_cell_w;
    for (size_t c = 0; c < num_cols; ++c) {
        col_widths[c] = std::max(0.0f, col_widths[c] * scale);
    }
}

void Engine::positionTableCells(
    const dom::DOMNodePtr& table_node,
    const std::vector<dom::DOMNodePtr>& /*rows*/,
    const std::vector<float>& col_widths,
    float spacing_x, float spacing_y, bool /*is_collapse*/) {

    auto* table_ln = getLayoutMutable(table_node);
    if (!table_ln) return;

    const auto& ts = table_node->getComputedStyle();
    const float table_x = table_ln->layout.position[0];
    const float table_y = table_ln->layout.position[1];

    const float bw = std::max(0.0f, ts.border_width);
    const float pt_l = ts.padding_left.isPixel() ? ts.padding_left.value : 0.0f;
    const float pt_r = ts.padding_right.isPixel() ? ts.padding_right.value : 0.0f;
    const float pt_t = ts.padding_top.isPixel() ? ts.padding_top.value : 0.0f;
    const float pt_b = ts.padding_bottom.isPixel() ? ts.padding_bottom.value : 0.0f;


    // Our LayoutNode coordinates are absolute (see layout extraction in layout_engine.cpp).
    // Table post-pass must therefore write absolute positions for rows/cells as well.

    const auto blocks = collectRowBlocks(table_node);

    // Caption participates in the table wrapper box, but is *outside* the table border box.
    // Browsers draw the table border only around the grid box, not including the caption.
    const dom::DOMNodePtr caption_node = findTableCaptionNode(table_node);
    const float caption_h = getNodeLayoutHeightPx(this, caption_node);
    const bool caption_bottom = caption_node ? isCaptionSideBottom(caption_node) : false;
    const float caption_top_offset = (caption_node && !caption_bottom) ? caption_h : 0.0f;

    const float content_x = table_x + bw + pt_l + spacing_x;
    float cur_y = table_y + caption_top_offset + bw + pt_t + spacing_y;

    for (const auto& block : blocks) {
        LayoutNode* group_ln = nullptr;
        const float group_start_y = cur_y;
        float group_w = 0.0f;

        if (block.group) {
            group_ln = getLayoutMutable(block.group);
        }

        for (const auto& row : block.rows) {
            auto* row_ln = getLayoutMutable(row);
            if (!row_ln) continue;

            std::vector<dom::DOMNodePtr> cells;
            collectCells(row, cells);

            const float row_h = computeRowHeightPx(this, cells);
            const float row_w = positionCellsInRowAbs(this, cells, col_widths, content_x, cur_y, spacing_x, row_h);

            updateLayoutRect(row_ln, content_x, cur_y, row_w, row_h);

            group_w = std::max(group_w, row_w);
            cur_y += row_h + spacing_y;
        }

        if (block.group && group_ln) {
            // Exclude the trailing spacing after the last row from row-group's own height.
            const float end_y = (cur_y > group_start_y) ? (cur_y - spacing_y) : group_start_y;
            const float group_h = std::max(0.0f, end_y - group_start_y);
            updateLayoutRect(group_ln, content_x, group_start_y, group_w, group_h);
        }
    }

    // Compute the table border-box bottom (grid box only, excluding bottom caption).
    const float border_box_bottom_y = cur_y + bw + pt_b;

    // Place caption in wrapper (outside the border box).
    if (caption_node) {
        auto* caption_ln = getLayoutMutable(caption_node);
        if (caption_ln) {
            const float table_w = table_ln->layout.dimensions[0];
            const float cap_w = std::max(0.0f, table_w);
            const float cap_x = table_x;
            const float cap_y = caption_bottom ? border_box_bottom_y : table_y;
            updateLayoutRect(caption_ln, cap_x, cap_y, cap_w, caption_h);
        }
    }

    // Update table *wrapper* height: grid border-box + optional top/bottom caption.
    float wrapper_h = std::max(0.0f, border_box_bottom_y - table_y);
    if (caption_node && caption_bottom) {
        wrapper_h += caption_h;
    }

    // Yoga may over-allocate height for tables in certain flex/block contexts.
    // For `height: auto` (default), shrink-to-fit content so borders/backgrounds don't
    // paint a huge empty box.
    const bool height_auto = ts.height.isAuto() || ts.height.isUnset();
    const float prev_h = table_ln->layout.dimensions[1];
    if (height_auto) {
        table_ln->layout.dimensions[1] = wrapper_h;
        table_ln->height = wrapper_h;
    } else if (wrapper_h > prev_h) {
        // Respect author-specified height/min-height by only expanding.
        table_ln->layout.dimensions[1] = wrapper_h;
        table_ln->height = wrapper_h;
    }

    if (std::isfinite(prev_h) && std::isfinite(table_ln->height) && std::fabs(prev_h - table_ln->height) > 0.1f) {
        dirty_rect_.expand(table_x, table_y, table_ln->layout.dimensions[0], table_ln->height);
    }

}




// Declared but not in the header (file-local helper called from positionTableCells)
void Engine::updateRowGroupLayouts(const dom::DOMNodePtr& table_node) {
    for (const auto& child : table_node->getChildren()) {
        if (!child || child->getType() != dom::DOMNode::NodeType::ELEMENT) continue;
        if (!isTableRowGroupDisplay(child->getComputedStyle().display)) continue;

        auto* group_ln = getLayoutMutable(child);
        if (!group_ln) continue;

        float min_y = 1e9f, max_y = 0.0f;
        for (const auto& row_child : child->getChildren()) {
            if (!row_child || row_child->getType() != dom::DOMNode::NodeType::ELEMENT) continue;
            const auto* rl = getLayout(row_child);
            if (!rl) continue;
            min_y = std::min(min_y, rl->layout.position[1]);
            max_y = std::max(max_y, rl->layout.position[1] + rl->layout.dimensions[1]);
        }
        if (min_y < max_y) {
            group_ln->layout.position[1] = min_y;
            group_ln->layout.dimensions[1] = max_y - min_y;
            group_ln->y = min_y;
            group_ln->height = max_y - min_y;
        }
    }
}

void Engine::compactNormalFlowSiblingsAfter(const dom::DOMNodePtr& start_node) {
    if (!start_node) return;
    auto parent = start_node->getParent();
    if (!parent || parent->getType() != dom::DOMNode::NodeType::ELEMENT) return;

    auto it_prev = layout_cache.find(start_node.get());
    if (it_prev == layout_cache.end() || !it_prev->second) return;

    LayoutNode* prev_layout = it_prev->second.get();
    float prev_mb = 0.0f;
    {
        const auto& cs = start_node->getComputedStyle();
        prev_mb = cs.margin_bottom.isPixel() ? cs.margin_bottom.value : 0.0f;
    }

    bool after = false;
    for (const auto& child : parent->getChildren()) {
        if (!child) continue;
        if (child.get() == start_node.get()) {
            after = true;
            continue;
        }
        if (!after) continue;

        if (child->getType() != dom::DOMNode::NodeType::ELEMENT) {
            continue;
        }

        const auto& cs = child->getComputedStyle();
        if (cs.display == "none" || cs.position == "absolute" || cs.position == "fixed") {
            continue;
        }

        auto it = layout_cache.find(child.get());
        if (it == layout_cache.end() || !it->second) {
            continue;
        }

        LayoutNode* ln = it->second.get();
        const float mt = cs.margin_top.isPixel() ? cs.margin_top.value : 0.0f;
        const float expected_y = prev_layout->y + prev_layout->height + prev_mb + mt;
        const float shift = expected_y - ln->y;

        if (std::abs(shift) > 0.1f) {
            shiftSubtreeY(child, shift);
        }

        // Use updated values after potential shift.
        prev_layout = it->second.get();
        prev_mb = cs.margin_bottom.isPixel() ? cs.margin_bottom.value : 0.0f;
    }
}

} // namespace dong::layout

