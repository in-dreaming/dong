#pragma once

#include "dom/dom_node.hpp"
#include <cstdint>

namespace dong::layout {
class Engine;
}

namespace dong::render {
class TextShaper;
}

namespace dong::dom {

// Result of a text hit test: which text node and character offset was clicked
struct TextHitResult {
    DOMNodePtr text_node;
    uint32_t char_offset = 0;
    bool found = false;
};

// Hit tests a pixel coordinate to find the text node and character offset.
// Uses layout positions and (optionally) shaped glyph data for precise results.
class TextHitTester {
public:
    // Perform a hit test at (x, y) within the given DOM subtree.
    // Returns the text node and character offset at that position.
    // If a TextShaper is provided, uses font shaping for accurate offset;
    // otherwise falls back to proportional estimation.
    static TextHitResult hitTestAt(const DOMNodePtr& root,
                                    dong::layout::Engine* layout_engine,
                                    int32_t x, int32_t y,
                                    dong::render::TextShaper* shaper = nullptr);

private:
    // Recursively find text nodes near the click position
    static TextHitResult hitTestRecursive(const DOMNodePtr& node,
                                           dong::layout::Engine* layout_engine,
                                           int32_t x, int32_t y,
                                           dong::render::TextShaper* shaper);

    // Estimate character offset within a text node based on x position
    static uint32_t estimateCharOffset(const DOMNodePtr& text_node,
                                        dong::layout::Engine* layout_engine,
                                        int32_t x,
                                        dong::render::TextShaper* shaper);
};

} // namespace dong::dom
