// grid_layout.cpp - Post-layout pass for CSS Grid containers
// Implements basic grid layout (fr/px/%/auto track sizes, gap, auto-flow placement)

#include "grid_layout.hpp"
#include "layout_engine.hpp"
#include "../core/log.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <sstream>
#include <vector>

namespace dong::layout {

namespace {

// Trim leading/trailing whitespace
std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Check if string ends with a suffix (case-insensitive for units)
bool endsWith(const std::string& s, const char* suffix) {
    size_t len = std::strlen(suffix);
    if (s.size() < len) return false;
    return s.compare(s.size() - len, len, suffix) == 0;
}

// Parse a single track token into a GridTrack
GridTrack parseTrackToken(const std::string& token, float container_size) {
    GridTrack track;
    std::string t = trim(token);
    if (t.empty()) return track;

    // fr unit: e.g. "1fr", "2.5fr"
    if (endsWith(t, "fr")) {
        std::string num_str = t.substr(0, t.size() - 2);
        float val = 1.0f;
        if (!num_str.empty()) {
            try { val = std::stof(num_str); } catch (...) { val = 1.0f; }
        }
        track.fr_value = val;
        track.is_fr = true;
        return track;
    }

    // px unit: e.g. "100px"
    if (endsWith(t, "px")) {
        std::string num_str = t.substr(0, t.size() - 2);
        float val = 0.0f;
        try { val = std::stof(num_str); } catch (...) { val = 0.0f; }
        track.size = val;
        return track;
    }

    // percentage: e.g. "50%"
    if (endsWith(t, "%")) {
        std::string num_str = t.substr(0, t.size() - 1);
        float pct = 0.0f;
        try { pct = std::stof(num_str); } catch (...) { pct = 0.0f; }
        track.size = container_size * pct / 100.0f;
        return track;
    }

    // "auto", "min-content", "max-content" → treat as auto (0 size, resolved later)
    if (t == "auto" || t == "min-content" || t == "max-content") {
        track.size = 0.0f;
        track.min_size = 0.0f;
        track.max_size = std::numeric_limits<float>::max();
        return track;
    }

    // Plain number (no unit) → treat as px
    try {
        float val = std::stof(t);
        track.size = val;
    } catch (...) {
        // Unknown token, default to 0
        track.size = 0.0f;
    }
    return track;
}

// Expand repeat(N, pattern) - simplified: only supports repeat(N, single_token)
// Returns true if expansion happened
bool tryExpandRepeat(const std::string& tmpl, std::vector<std::string>& tokens, float container_size) {
    // Look for "repeat("
    size_t pos = tmpl.find("repeat(");
    if (pos == std::string::npos) return false;

    // Parse everything before repeat
    std::string before = tmpl.substr(0, pos);

    // Find matching closing paren
    size_t paren_start = pos + 7; // after "repeat("
    int depth = 1;
    size_t paren_end = paren_start;
    while (paren_end < tmpl.size() && depth > 0) {
        if (tmpl[paren_end] == '(') depth++;
        else if (tmpl[paren_end] == ')') depth--;
        paren_end++;
    }
    // paren_end is now one past the closing ')'

    std::string repeat_content = tmpl.substr(paren_start, paren_end - paren_start - 1);
    std::string after = (paren_end < tmpl.size()) ? tmpl.substr(paren_end) : "";

    // Parse repeat_content: "N, pattern"
    size_t comma = repeat_content.find(',');
    if (comma == std::string::npos) return false;

    std::string count_str = trim(repeat_content.substr(0, comma));
    std::string pattern = trim(repeat_content.substr(comma + 1));

    int count = 1;
    try { count = std::stoi(count_str); } catch (...) { count = 1; }
    if (count <= 0) count = 1;
    if (count > 100) count = 100; // Sanity limit

    // Tokenize "before" part
    std::istringstream before_ss(before);
    std::string tok;
    while (before_ss >> tok) {
        tokens.push_back(tok);
    }

    // Add repeated tokens
    // The pattern itself may have multiple space-separated tokens
    for (int i = 0; i < count; i++) {
        std::istringstream pattern_ss(pattern);
        while (pattern_ss >> tok) {
            tokens.push_back(tok);
        }
    }

    // Tokenize "after" part (may contain more repeats - recurse)
    if (!after.empty()) {
        std::string after_trimmed = trim(after);
        if (!after_trimmed.empty()) {
            // Check for nested repeat
            std::vector<std::string> after_tokens;
            if (tryExpandRepeat(after_trimmed, after_tokens, container_size)) {
                for (const auto& at : after_tokens) {
                    tokens.push_back(at);
                }
            } else {
                std::istringstream after_ss(after_trimmed);
                while (after_ss >> tok) {
                    tokens.push_back(tok);
                }
            }
        }
    }

    return true;
}

// Parse grid-column or grid-row placement string: "start / end" or "start" or "span N"
GridPlacement parsePlacementProperty(const std::string& start_str, const std::string& end_str,
                                      const std::string& shorthand, int max_tracks) {
    GridPlacement p;
    p.row_start = 0;
    p.row_end = 1;
    p.col_start = 0;
    p.col_end = 1;

    // Use shorthand if start/end not set
    std::string start_val = start_str;
    std::string end_val = end_str;

    if (!shorthand.empty() && start_val.empty() && end_val.empty()) {
        // shorthand format: "start / end" or just "start"
        size_t slash = shorthand.find('/');
        if (slash != std::string::npos) {
            start_val = trim(shorthand.substr(0, slash));
            end_val = trim(shorthand.substr(slash + 1));
        } else {
            start_val = trim(shorthand);
        }
    }

    // Parse start
    if (!start_val.empty() && start_val != "auto") {
        if (start_val.find("span") == 0) {
            // "span N" → we'll handle after determining start position
            std::string num = trim(start_val.substr(4));
            int span = 1;
            try { span = std::stoi(num); } catch (...) { span = 1; }
            p.row_start = -1; // Flag for auto-placed with span
            p.row_end = span;
            return p;
        }
        try {
            int line = std::stoi(start_val);
            // CSS grid lines are 1-based; convert to 0-based track index
            if (line > 0) p.row_start = line - 1;
            else if (line < 0) p.row_start = std::max(0, max_tracks + line);
        } catch (...) {}
    }

    // Parse end
    if (!end_val.empty() && end_val != "auto") {
        if (end_val.find("span") == 0) {
            std::string num = trim(end_val.substr(4));
            int span = 1;
            try { span = std::stoi(num); } catch (...) { span = 1; }
            p.row_end = p.row_start + span;
        } else {
            try {
                int line = std::stoi(end_val);
                if (line > 0) p.row_end = line - 1;
                else if (line < 0) p.row_end = std::max(0, max_tracks + line);
            } catch (...) {
                p.row_end = p.row_start + 1;
            }
        }
    } else {
        p.row_end = p.row_start + 1;
    }

    // Ensure start < end
    if (p.row_end <= p.row_start) {
        p.row_end = p.row_start + 1;
    }

    return p;
}

// Update a LayoutNode's position and dimensions
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

// Shift all descendant LayoutNodes by (dx, dy)
void shiftDescendants(Engine* engine, const dom::DOMNodePtr& node, float dx, float dy) {
    if (!node || (dx == 0.0f && dy == 0.0f)) return;
    for (const auto& child : node->getChildren()) {
        if (!child) continue;
        auto* ln = engine->getLayoutMutable(child);
        if (ln) {
            ln->layout.position[0] += dx;
            ln->layout.position[1] += dy;
            ln->x += dx;
            ln->y += dy;
        }
        shiftDescendants(engine, child, dx, dy);
    }
}

} // anonymous namespace


std::vector<GridTrack> parseGridTemplate(const std::string& tmpl, float container_size) {
    std::vector<GridTrack> tracks;
    if (tmpl.empty()) return tracks;

    // Try to expand repeat() first
    std::vector<std::string> tokens;
    if (!tryExpandRepeat(tmpl, tokens, container_size)) {
        // Simple space-separated tokenization
        std::istringstream ss(tmpl);
        std::string tok;
        while (ss >> tok) {
            tokens.push_back(tok);
        }
    }

    for (const auto& tok : tokens) {
        tracks.push_back(parseTrackToken(tok, container_size));
    }

    return tracks;
}

void resolveGridFrUnits(std::vector<GridTrack>& tracks, float container_size, float gap) {
    if (tracks.empty()) return;

    // Calculate total gap space
    float total_gap = (tracks.size() > 1) ? gap * (float)(tracks.size() - 1) : 0.0f;

    // Sum fixed track sizes
    float fixed_sum = 0.0f;
    float total_fr = 0.0f;
    for (const auto& track : tracks) {
        if (track.is_fr) {
            total_fr += track.fr_value;
        } else {
            fixed_sum += track.size;
        }
    }

    // Distribute remaining space among fr tracks
    float remaining = container_size - fixed_sum - total_gap;
    if (remaining < 0.0f) remaining = 0.0f;

    if (total_fr > 0.0f) {
        for (auto& track : tracks) {
            if (track.is_fr) {
                track.size = (track.fr_value / total_fr) * remaining;
            }
        }
    }

    // Auto tracks: if size is still 0 and it's not fr, give them equal share of leftover
    // (only if there's no fr to consume the space)
    if (total_fr == 0.0f) {
        int auto_count = 0;
        for (const auto& track : tracks) {
            if (track.size == 0.0f && !track.is_fr) {
                auto_count++;
            }
        }
        if (auto_count > 0) {
            float auto_size = remaining / (float)auto_count;
            for (auto& track : tracks) {
                if (track.size == 0.0f && !track.is_fr) {
                    track.size = auto_size;
                }
            }
        }
    }
}


void Engine::layoutGridElements(dom::DOMNodePtr root) {
    // Recursively find all grid containers and lay them out
    if (!root) return;

    const auto& style = root->getComputedStyle();
    if (style.display == dom::CSSDisplay::Grid || style.display == dom::CSSDisplay::InlineGrid) {
        layoutSingleGrid(root);
    }

    for (const auto& child : root->getChildren()) {
        if (!child) continue;
        if (child->getType() != dom::DOMNode::NodeType::ELEMENT) continue;
        layoutGridElements(child);
    }
}

void Engine::layoutSingleGrid(const dom::DOMNodePtr& grid_container) {
    auto* container_ln = getLayoutMutable(grid_container);
    if (!container_ln) return;

    const auto& style = grid_container->getComputedStyle();
    float container_width = container_ln->width;
    float container_height = container_ln->height;

    // Account for padding on the container
    float pad_left = style.padding_left.isPixel() ? style.padding_left.value : 0.0f;
    float pad_top = style.padding_top.isPixel() ? style.padding_top.value : 0.0f;
    float pad_right = style.padding_right.isPixel() ? style.padding_right.value : 0.0f;
    float pad_bottom = style.padding_bottom.isPixel() ? style.padding_bottom.value : 0.0f;

    float content_width = container_width - pad_left - pad_right;
    float content_height = container_height - pad_top - pad_bottom;
    if (content_width < 0.0f) content_width = 0.0f;
    if (content_height < 0.0f) content_height = 0.0f;

    float column_gap = style.column_gap;
    float row_gap = style.row_gap;

    // Parse and resolve column tracks
    std::vector<GridTrack> col_tracks = parseGridTemplate(style.grid_template_columns, content_width);
    resolveGridFrUnits(col_tracks, content_width, column_gap);

    // Collect grid children (elements only, skip text nodes)
    std::vector<dom::DOMNodePtr> children;
    for (const auto& child : grid_container->getChildren()) {
        if (!child) continue;
        if (child->getType() != dom::DOMNode::NodeType::ELEMENT) continue;
        const auto& child_style = child->getComputedStyle();
        if (child_style.display == dom::CSSDisplay::None) continue;
        children.push_back(child);
    }

    if (children.empty()) return;

    // If no columns defined, create a single column spanning full width
    if (col_tracks.empty()) {
        GridTrack single;
        single.size = content_width;
        col_tracks.push_back(single);
    }

    int num_cols = (int)col_tracks.size();

    // Determine placements for all children
    struct ChildPlacement {
        int col_start = 0;
        int col_end = 1;
        int row_start = 0;
        int row_end = 1;
    };
    std::vector<ChildPlacement> placements(children.size());

    // Auto-flow cursor
    int cursor_row = 0;
    int cursor_col = 0;

    for (size_t i = 0; i < children.size(); i++) {
        const auto& child_style = children[i]->getComputedStyle();

        // Parse column placement
        GridPlacement col_place = parsePlacementProperty(
            child_style.grid_column_start, child_style.grid_column_end,
            child_style.grid_column, num_cols);

        // Parse row placement (for explicit placement)
        int max_rows = (int)children.size(); // Upper bound
        GridPlacement row_place = parsePlacementProperty(
            child_style.grid_row_start, child_style.grid_row_end,
            child_style.grid_row, max_rows);

        bool has_explicit_col = !child_style.grid_column_start.empty() ||
                                !child_style.grid_column.empty();
        bool has_explicit_row = !child_style.grid_row_start.empty() ||
                                !child_style.grid_row.empty();

        if (has_explicit_col && col_place.row_start >= 0) {
            placements[i].col_start = std::min(col_place.row_start, num_cols - 1);
            placements[i].col_end = std::min(col_place.row_end, num_cols);
        } else {
            // Auto-flow column
            placements[i].col_start = cursor_col;
            int span = 1;
            if (has_explicit_col && col_place.row_start == -1) {
                span = col_place.row_end; // span value stored in row_end
            }
            placements[i].col_end = cursor_col + span;
            if (placements[i].col_end > num_cols) {
                // Wrap to next row
                cursor_row++;
                cursor_col = 0;
                placements[i].col_start = 0;
                placements[i].col_end = span;
            }
        }

        if (has_explicit_row && row_place.row_start >= 0) {
            placements[i].row_start = row_place.row_start;
            placements[i].row_end = row_place.row_end;
        } else {
            // Auto-flow row
            placements[i].row_start = cursor_row;
            int row_span = 1;
            if (has_explicit_row && row_place.row_start == -1) {
                row_span = row_place.row_end;
            }
            placements[i].row_end = cursor_row + row_span;
        }

        // Advance auto-flow cursor
        if (!has_explicit_col || col_place.row_start < 0) {
            cursor_col = placements[i].col_end;
            if (cursor_col >= num_cols) {
                cursor_col = 0;
                cursor_row++;
            }
        }
    }

    // Determine number of rows needed
    int num_rows = 0;
    for (const auto& p : placements) {
        num_rows = std::max(num_rows, p.row_end);
    }
    if (num_rows <= 0) num_rows = 1;

    // Parse and resolve row tracks
    std::vector<GridTrack> row_tracks = parseGridTemplate(style.grid_template_rows, content_height);

    // Ensure we have enough row tracks (auto-generate if needed)
    while ((int)row_tracks.size() < num_rows) {
        GridTrack auto_track;
        auto_track.size = 0.0f; // Will be resolved based on content or equal distribution
        row_tracks.push_back(auto_track);
    }

    // For row tracks that are still 0 (auto), compute based on child content heights
    // First pass: measure children to determine auto row heights
    std::vector<float> row_heights(num_rows, 0.0f);
    for (size_t i = 0; i < children.size(); i++) {
        auto* child_ln = getLayoutMutable(children[i]);
        if (!child_ln) continue;
        const auto& p = placements[i];
        float child_height = child_ln->height;
        int span = p.row_end - p.row_start;
        if (span <= 0) span = 1;
        // Distribute child height among spanned rows (only for auto rows)
        float per_row = child_height / (float)span;
        for (int r = p.row_start; r < p.row_end && r < num_rows; r++) {
            row_heights[r] = std::max(row_heights[r], per_row);
        }
    }

    // Apply measured heights to auto row tracks, then resolve fr
    for (int r = 0; r < (int)row_tracks.size() && r < num_rows; r++) {
        if (!row_tracks[r].is_fr && row_tracks[r].size == 0.0f) {
            row_tracks[r].size = row_heights[r];
        }
    }
    resolveGridFrUnits(row_tracks, content_height, row_gap);

    // Position each child
    float container_abs_x = container_ln->x;
    float container_abs_y = container_ln->y;

    for (size_t i = 0; i < children.size(); i++) {
        auto* child_ln = getLayoutMutable(children[i]);
        if (!child_ln) continue;

        const auto& p = placements[i];

        // Compute X position: sum of column tracks before col_start + gaps
        float x = pad_left;
        for (int c = 0; c < p.col_start && c < (int)col_tracks.size(); c++) {
            x += col_tracks[c].size + column_gap;
        }

        // Compute Y position: sum of row tracks before row_start + gaps
        float y = pad_top;
        for (int r = 0; r < p.row_start && r < (int)row_tracks.size(); r++) {
            y += row_tracks[r].size + row_gap;
        }

        // Compute width: sum of spanned column tracks + internal gaps
        float w = 0.0f;
        for (int c = p.col_start; c < p.col_end && c < (int)col_tracks.size(); c++) {
            w += col_tracks[c].size;
            if (c > p.col_start) w += column_gap;
        }

        // Compute height: sum of spanned row tracks + internal gaps
        float h = 0.0f;
        for (int r = p.row_start; r < p.row_end && r < (int)row_tracks.size(); r++) {
            h += row_tracks[r].size;
            if (r > p.row_start) h += row_gap;
        }

        // Convert to absolute coordinates
        float abs_x = container_abs_x + x;
        float abs_y = container_abs_y + y;

        // Compute delta from current position to shift descendants
        float dx = abs_x - child_ln->x;
        float dy = abs_y - child_ln->y;

        // Update child layout
        updateLayoutRect(child_ln, abs_x, abs_y, w, h);

        // Shift all descendants of this child by the same delta
        shiftDescendants(this, children[i], dx, dy);
    }
}

} // namespace dong::layout
