#include "painter.hpp"

namespace dong::render {

// Defined in painter_select.cpp
void renderSelectDropdownOverlay(
    const dom::DOMNodePtr& node,
    const layout::LayoutNode* layout,
    const dom::ComputedStyle& style,
    float bl, float bt, float br, float bb,
    TextShaper& shaper,
    DisplayListBuilder& builder
);

void Painter::paintSelectDropdownOverlays(DisplayListBuilder& builder) {
    if (open_select_overlays_.empty()) {
        return;
    }

    for (const auto& it : open_select_overlays_) {
        if (!it.node || !it.layout) {
            continue;
        }
        const float dx = it.tx - builder.getTranslateX();
        const float dy = it.ty - builder.getTranslateY();
        builder.pushTranslate(dx, dy);

        renderSelectDropdownOverlay(
            it.node,
            it.layout,
            it.node->getComputedStyle(),
            it.bl, it.bt, it.br, it.bb,
            text_shaper_,
            builder
        );

        builder.popTranslate();
    }
}

} // namespace dong::render
