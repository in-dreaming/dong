// Painter backdrop rendering for <dialog> modal backdrop pseudo-element

#include "../painter.hpp"
#include "../../dom/dom/dom_node.hpp"
#include "painter_style_utils.hpp"

namespace dong::render {

// Render a ::backdrop pseudo-element as a full-viewport semi-transparent overlay.
// Called from the top-layer rendering pass in Painter.
void paintDialogBackdrop(DisplayListBuilder& builder, float viewport_width, float viewport_height,
                         const dong::dom::DOMNodePtr& backdrop_pseudo) {
    // Default backdrop style: semi-transparent black rgba(0,0,0,0.4)
    Color bg_color{0.0f, 0.0f, 0.0f, 0.4f};

    if (backdrop_pseudo) {
        const auto& style = backdrop_pseudo->getComputedStyle();
        if (!style.background_color.empty() && style.background_color != "transparent") {
            bg_color = painter_detail::makeColorFromCss(style.background_color);
        }
    }

    Rect backdrop_rect{0.0f, 0.0f, viewport_width, viewport_height};
    builder.addRect(backdrop_rect, bg_color);
}

} // namespace dong::render
