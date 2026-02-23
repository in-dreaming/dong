#include "computed_style.hpp"

namespace dong::dom {

void ComputedStyle::updateBFCFlag() {
    creates_block_formatting_context =
        (display == "flow-root") ||
        (overflow != "visible") ||
        (position == "absolute" || position == "fixed") ||
        (float_value != "none");
}

} // namespace dong::dom
