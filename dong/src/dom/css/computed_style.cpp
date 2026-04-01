#include "computed_style.hpp"

namespace dong::dom {

void ComputedStyle::updateBFCFlag() {
    creates_block_formatting_context =
        (display == CSSDisplay::FlowRoot) ||
        (overflow != CSSOverflow::Visible) ||
        (position == CSSPosition::Absolute || position == CSSPosition::Fixed) ||
        (float_value != CSSFloat::None);
}

} // namespace dong::dom
