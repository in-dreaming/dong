#pragma once

#include "dom/dom_node.hpp"
#include <string>
#include <vector>

namespace dong::dom {

// Dialog element state management
// Provides show()/showModal()/close() functionality for <dialog> elements.
class DialogElementState {
public:
    DialogElementState();
    ~DialogElementState() = default;

    bool isOpen() const { return is_open_; }
    bool isModal() const { return is_modal_; }
    const std::string& getReturnValue() const { return return_value_; }

    // Show the dialog (non-modal)
    void show();

    // Show the dialog as modal (blocks interaction outside)
    void showModal();

    // Close the dialog with optional return value
    void close(const std::string& return_value = "");

private:
    bool is_open_ = false;
    bool is_modal_ = false;
    std::string return_value_;
};

// Get or create DialogElementState for a <dialog> node
DialogElementState* getDialogState(DOMNodePtr node);

// Remove dialog state
void removeDialogState(DOMNodePtr node);

// Clear all dialog states (on DOM reload)
void clearAllDialogStates();

// Top-layer management (modal dialogs are rendered above everything)
// These are managed per-view via EngineView

} // namespace dong::dom
