#include "text_renderer_selector.hpp"

namespace dong::render {

TextRendererSelection TextRendererSelector::resolve(TextRendererMode requested) const {
    TextRendererSelection sel;
    sel.requested = requested;

    switch (requested) {
        case TextRendererMode::Msdf:
            // Force MSDF — always available.
            sel.resolved = TextRendererMode::Msdf;
            sel.fallback_used = false;
            sel.reason = nullptr;
            break;

        case TextRendererMode::Slug:
            if (slug_available_) {
                sel.resolved = TextRendererMode::Slug;
                sel.fallback_used = false;
                sel.reason = nullptr;
            } else {
                sel.resolved = TextRendererMode::Msdf;
                sel.fallback_used = true;
                sel.reason = "Slug runtime not available";
            }
            break;

        case TextRendererMode::Auto:
        default:
            // Auto mode: prefer Slug when available, fallback to MSDF.
            if (slug_available_) {
                sel.resolved = TextRendererMode::Slug;
                sel.fallback_used = false;
                sel.reason = nullptr;
            } else {
                sel.resolved = TextRendererMode::Msdf;
                sel.fallback_used = false;
                sel.reason = nullptr;
            }
            break;
    }

    return sel;
}

} // namespace dong::render
