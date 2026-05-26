#pragma once

#include "display_list.hpp"
#include <vector>
#include <cmath>

namespace dong::render {

// Global overlay: display items injected by JS (bypasses DOM/layout pipeline).
// Supports the full Immediate Mode API:
//   dong.clearOverlay()
//   dong.renderText({lines, font, color})
//   dong.drawRect({x, y, w, h, color, radius?, strokeWidth?, strokeColor?})
//   dong.drawCircle({cx, cy, r, color, strokeWidth?, strokeColor?})
class OverlayDraw {
public:
    OverlayDraw() = default;

    static OverlayDraw& instance() {
        static OverlayDraw inst;
        return inst;
    }

    void clear() {
        items_.clear();
        dirty_ = true;
    }

    void addGlyphRun(DrawGlyphRunData&& data) {
        DisplayItem item{};
        item.type = DisplayItemType::DrawGlyphRun;
        item.glyph_run = std::move(data);
        items_.push_back(std::move(item));
        dirty_ = true;
    }

    void addRect(const Rect& rect, const Color& color) {
        DisplayItem item{};
        item.type = DisplayItemType::DrawRect;
        item.rect = {rect, color};
        items_.push_back(std::move(item));
        dirty_ = true;
    }

    void addRoundedRect(const Rect& rect, const Color& color, float radius,
                        float stroke_width = 0.0f) {
        DisplayItem item{};
        item.type = DisplayItemType::DrawRoundedRect;
        item.rounded_rect = {rect, color, radius, stroke_width};
        items_.push_back(std::move(item));
        dirty_ = true;
    }

    void addCircle(float cx, float cy, float r, const Color& color,
                   float stroke_width = 0.0f) {
        Rect rect{cx - r, cy - r, r * 2.0f, r * 2.0f};
        addRoundedRect(rect, color, r, stroke_width);
    }

    bool isDirty() const { return dirty_; }
    void clearDirty() { dirty_ = false; }

    const std::vector<DisplayItem>& items() const { return items_; }
    bool empty() const { return items_.empty(); }

private:
    std::vector<DisplayItem> items_;
    bool dirty_ = false;
};

} // namespace dong::render
