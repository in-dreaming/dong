// Dialog element state implementation

#include "dialog_element.hpp"
#include <unordered_map>

namespace dong::dom {

// Global state storage (same pattern as SelectElementState)
static std::unordered_map<void*, DialogElementState> g_dialog_states;

DialogElementState::DialogElementState() = default;

void DialogElementState::show() {
    if (is_open_) return;
    is_open_ = true;
    is_modal_ = false;
}

void DialogElementState::showModal() {
    if (is_open_) return;
    is_open_ = true;
    is_modal_ = true;
}

void DialogElementState::close(const std::string& return_value) {
    if (!is_open_) return;
    is_open_ = false;
    is_modal_ = false;
    return_value_ = return_value;
}

DialogElementState* getDialogState(DOMNodePtr node) {
    if (!node) return nullptr;
    void* key = node.get();
    auto it = g_dialog_states.find(key);
    if (it != g_dialog_states.end()) {
        return &it->second;
    }
    auto [inserted, _] = g_dialog_states.emplace(key, DialogElementState{});
    return &inserted->second;
}

void removeDialogState(DOMNodePtr node) {
    if (!node) return;
    g_dialog_states.erase(node.get());
}

void clearAllDialogStates() {
    g_dialog_states.clear();
}

} // namespace dong::dom
