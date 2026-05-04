#pragma once

#include "text_renderer_mode.hpp"
#include "../render/display_list.hpp"

namespace dong::render {

// Resolves the actual text renderer to use for a given glyph run.
class TextRendererSelector {
public:
    // Set whether Slug runtime is available (initialized and ready).
    void setSlugAvailable(bool available) { slug_available_ = available; }
    bool isSlugAvailable() const { return slug_available_; }

    // Resolve the renderer for a glyph run given the requested mode.
    TextRendererSelection resolve(TextRendererMode requested) const;

private:
    bool slug_available_ = false;
};

} // namespace dong::render
