// table_layout.cpp - Post-layout pass for CSS table elements
// Implements basic table layout (border-collapse, border-spacing, table-layout: fixed/auto)

#include "layout_engine.hpp"
#include "../core/log.h"

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
    const float spacing = is_collapse ? 0.0f : style.border_spacing;
    const bool is_fixed = (style.table_layout == "fixed");

    std::vector<dom::DOMNodePtr> rows;
    collectRows(table_node, rows);
    if (rows.empty()) return;

    const size_t num_cols = getMaxColumns(rows);
    if (num_cols == 0) return;

    std::vector<float> col_widths(num_cols, 0.0f);
    computeColumnWidths(table_node, rows, num_cols, is_fixed, spacing, col_widths);
    positionTableCells(table_node, rows, col_widths, spacing, is_collapse);
}

void Engine::computeColumnWidths(
    const dom::DOMNodePtr& table_node,
    const std::vector<dom::DOMNodePtr>& rows,
    size_t num_cols, bool is_fixed, float spacing,
    std::vector<float>& col_widths) {

    auto* table_ln = getLayoutMutable(table_node);
    if (!table_ln) return;

    const auto& ts = table_node->getComputedStyle();
    const float table_width = table_ln->layout.dimensions[0];
    const float bw = ts.border_width;
    const float pt_l = ts.padding_left.isPixel() ? ts.padding_left.value : 0.0f;
    const float pt_r = ts.padding_right.isPixel() ? ts.padding_right.value : 0.0f;
    const float avail = table_width - bw * 2.0f - pt_l - pt_r;
    const float total_spacing = spacing * static_cast<float>(num_cols + 1);

    if (is_fixed && table_width > 0.0f) {
        float col_w = std::max(0.0f, (avail - total_spacing) / static_cast<float>(num_cols));
        for (size_t c = 0; c < num_cols; ++c) {
            col_widths[c] = col_w;
        }
        return;
    }

    // Auto: use Yoga cell widths as basis, then distribute remaining
    for (const auto& row : rows) {
        std::vector<dom::DOMNodePtr> cells;
        collectCells(row, cells);
        for (size_t c = 0; c < cells.size() && c < num_cols; ++c) {
            const auto* cell_ln = getLayout(cells[c]);
            if (cell_ln) {
                col_widths[c] = std::max(col_widths[c], cell_ln->layout.dimensions[0]);
            }
        }
    }

    float total_cell_w = 0.0f;
    for (size_t c = 0; c < num_cols; ++c) total_cell_w += col_widths[c];
    float remaining = avail - total_spacing - total_cell_w;

    if (remaining > 0.0f && total_cell_w > 0.0f) {
        for (size_t c = 0; c < num_cols; ++c) {
            col_widths[c] += remaining * (col_widths[c] / total_cell_w);
        }
    }
}

void Engine::positionTableCells(
    const dom::DOMNodePtr& table_node,
    const std::vector<dom::DOMNodePtr>& rows,
    const std::vector<float>& col_widths,
    float spacing, bool /*is_collapse*/) {

    auto* table_ln = getLayoutMutable(table_node);
    if (!table_ln) return;

    const auto& ts = table_node->getComputedStyle();
    const float table_x = table_ln->layout.position[0];
    const float table_y = table_ln->layout.position[1];
    const float bw = ts.border_width;
    const float pt_l = ts.padding_left.isPixel() ? ts.padding_left.value : 0.0f;
    const float pt_t = ts.padding_top.isPixel() ? ts.padding_top.value : 0.0f;

    float cur_y = table_y + bw + pt_t + spacing;

    for (const auto& row : rows) {
        auto* row_ln = getLayoutMutable(row);
        if (!row_ln) continue;

        std::vector<dom::DOMNodePtr> cells;
        collectCells(row, cells);

        // Determine row height from tallest cell
        float row_h = 0.0f;
        for (const auto& cell : cells) {
            const auto* cl = getLayout(cell);
            if (cl) row_h = std::max(row_h, cl->layout.dimensions[1]);
        }

        // Position each cell
        float cur_x = table_x + bw + pt_l + spacing;
        for (size_t c = 0; c < cells.size() && c < col_widths.size(); ++c) {
            auto* cell_ln = getLayoutMutable(cells[c]);
            if (!cell_ln) continue;
            updateLayoutRect(cell_ln, cur_x, cur_y, col_widths[c], row_h);
            cur_x += col_widths[c] + spacing;
        }

        // Update row
        float row_w = cur_x - (table_x + bw + pt_l);
        updateLayoutRect(row_ln, table_x + bw + pt_l, cur_y, row_w, row_h);
        cur_y += row_h + spacing;
    }

    // Update row-group layouts (thead/tbody/tfoot)
    updateRowGroupLayouts(table_node);

    // Update table height to fit rows
    const float pt_b = ts.padding_bottom.isPixel() ? ts.padding_bottom.value : 0.0f;
    float new_h = cur_y - table_y + bw + pt_b;
    if (new_h > table_ln->layout.dimensions[1]) {
        table_ln->layout.dimensions[1] = new_h;
        table_ln->height = new_h;
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

} // namespace dong::layout
