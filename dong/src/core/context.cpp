#include "context.hpp"

namespace dong {

Context::Context() : initialized_(false) {
    initialized_ = true;
}

Context::~Context() {
    initialized_ = false;
}

} // namespace dong
