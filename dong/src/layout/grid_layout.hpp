#pragma once

#include "../dom/dom/dom_node.hpp"
#include <vector>
#include <string>

namespace dong::layout {

struct GridTrack {
    float size = 0.0f;     // Resolved size in pixels
    float min_size = 0.0f;
    float max_size = 0.0f;
    float fr_value = 0.0f; // fr unit value (0 = not fr)
    bool is_fr = false;
};

struct GridPlacement {
    int row_start = 0;     // 0-based
    int row_end = 0;
    int col_start = 0;
    int col_end = 0;
};

// Parse "grid-template-columns: 1fr 2fr 100px 50%" into tracks
std::vector<GridTrack> parseGridTemplate(const std::string& tmpl, float container_size);

// Resolve fr units: distribute remaining space after fixed tracks
void resolveGridFrUnits(std::vector<GridTrack>& tracks, float container_size, float gap);

} // namespace dong::layout
