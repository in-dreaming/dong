#include "drag_manager.hpp"
#include <cmath>

namespace dong::drag {

DragManager::DragManager()
    : state_(DragState::Idle)
    , start_x_(0)
    , start_y_(0)
    , current_x_(0)
    , current_y_(0) {
}

DragManager::~DragManager() = default;

void DragManager::beginDrag(const dom::DOMNodePtr& source, int32_t start_x, int32_t start_y) {
    drag_source_ = source;
    start_x_ = start_x;
    start_y_ = start_y;
    current_x_ = start_x;
    current_y_ = start_y;
    drag_target_.reset();
    drag_data_.clear();
    state_ = DragState::Beginning;
}

void DragManager::updateDrag(int32_t current_x, int32_t current_y) {
    current_x_ = current_x;
    current_y_ = current_y;

    if (state_ == DragState::Beginning && exceededThreshold()) {
        // Threshold exceeded, start actual drag
        state_ = DragState::Dragging;
    }
}

void DragManager::endDrag(int32_t drop_x, int32_t drop_y) {
    current_x_ = drop_x;
    current_y_ = drop_y;

    drag_source_.reset();
    drag_target_.reset();
    drag_data_.clear();
    state_ = DragState::Idle;
}

bool DragManager::exceededThreshold() const {
    int32_t dx = current_x_ - start_x_;
    int32_t dy = current_y_ - start_y_;
    return ((dx * dx + dy * dy) >= (DRAG_THRESHOLD * DRAG_THRESHOLD));
}

} // namespace dong::drag
