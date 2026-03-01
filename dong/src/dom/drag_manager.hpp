#pragma once

#include "../dom/dom/dom_node.hpp"
#include <cstdint>
#include <string>

namespace dong::drag {

// Drag state for Phase 1: Local UI Drag
enum class DragState {
    Idle,           // No active drag
    Beginning,      // Mouse down waiting for threshold
    Dragging,       // Active drag in progress
};

// Simplified drop effect for Phase 1
enum class DropEffect {
    None,
    Copy,
    Move,
};

// Drag manager for tracking drag & drop state
class DragManager {
public:
    DragManager();
    ~DragManager();

    // Start drag operation (called on mousedown with draggable element)
    void beginDrag(const dom::DOMNodePtr& source, int32_t start_x, int32_t start_y);

    // Update drag operation (called on mousemove)
    void updateDrag(int32_t current_x, int32_t current_y);

    // End drag operation (called on mouseup)
    void endDrag(int32_t drop_x, int32_t drop_y);

    // Check if currently dragging
    bool isDragging() const { return state_ == DragState::Dragging; }
    bool hasPotentialDrag() const { return state_ == DragState::Beginning; }

    // Get current drag source
    dom::DOMNodePtr getDragSource() const { return drag_source_; }

    // Get/set drag data (simplified text data for Phase 1)
    void setDragData(const std::string& data) { drag_data_ = data; }
    const std::string& getDragData() const { return drag_data_; }

    // Get current position (relative to drag start)
    int32_t getOffsetX() const { return current_x_ - start_x_; }
    int32_t getOffsetY() const { return current_y_ - start_y_; }

    // Get drag ghost position
    int32_t getGhostX() const { return current_x_; }
    int32_t getGhostY() const { return current_y_; }

    // Drag threshold (pixels to move before drag starts)
    static constexpr int32_t DRAG_THRESHOLD = 5;

private:
    DragState state_;
    dom::DOMNodePtr drag_source_;
    dom::DOMNodePtr drag_target_;  // Current hover target

    // Position tracking
    int32_t start_x_;
    int32_t start_y_;
    int32_t current_x_;
    int32_t current_y_;

    // Drag data (simplified for Phase 1)
    std::string drag_data_;

    // Check if threshold exceeded
    bool exceededThreshold() const;
};

} // namespace dong::drag
