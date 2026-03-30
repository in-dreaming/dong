#pragma once

#include "display_list.hpp"
#include <vector>
#include <mutex>

namespace dong::render {

// Global overlay: display items injected by JS (bypasses DOM/layout pipeline).
// Thread-safe via mutex since JS runs on the main thread and paint runs right after.
class OverlayDraw {
public:
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

    bool isDirty() const { return dirty_; }
    void clearDirty() { dirty_ = false; }

    const std::vector<DisplayItem>& items() const { return items_; }
    bool empty() const { return items_.empty(); }

private:
    OverlayDraw() = default;
    std::vector<DisplayItem> items_;
    bool dirty_ = false;
};

} // namespace dong::render
