#include "details_element.hpp"
#include <memory>
#include <unordered_map>

namespace dong {
namespace dom {

// 全局 details 状态存储（简化实现；DOM 重载时必须 clearAllDetailsStates()）
static std::unordered_map<void*, std::unique_ptr<DetailsElementState> > g_details_states;

DetailsElementState::DetailsElementState() {
    // 默认构造函数
}

void DetailsElementState::toggle() {
    if (is_open_) {
        close();
    } else {
        open();
    }
}

void DetailsElementState::open() {
    if (is_open_) return;
    is_open_ = true;
    // toggle 事件将在上层触发
}

void DetailsElementState::close() {
    if (!is_open_) return;
    is_open_ = false;
    // toggle 事件将在上层触发
}

void DetailsElementState::setOpen(bool should_open) {
    if (should_open) {
        open();
    } else {
        close();
    }
}

void DetailsElementState::syncFromDOM(const DOMNodePtr& node) {
    if (!node) return;

    // 同步 open 属性
    is_open_ = node->hasAttribute("open");

    // 检查是否有 summary 子元素
    has_summary_ = false;
    summary_text_.clear();

    for (const auto& child : node->getChildren()) {
        if (child && child->getType() == DOMNode::NodeType::ELEMENT &&
            child->getTagName() == "summary") {
            has_summary_ = true;
            summary_text_ = child->getTextContent();
            break;
        }
    }
}

void DetailsElementState::applyOpenStateToDOM(const DOMNodePtr& details_node) const {
    if (!details_node) return;

    if (is_open_) {
        details_node->setAttribute("open", "");
    } else {
        details_node->removeAttribute("open");
    }
}

bool DetailsElementState::handleClick(const DOMNodePtr& target_node) {
    if (!target_node) return false;

    // 如果点击的是 summary 元素或其子元素，则触发 toggle
    DOMNodePtr current = target_node;
    while (current.get()) {
        if (current->getTagName() == "summary") {
            toggle();
            return true;
        }
        current = current->getParent();
    }

    return false;
}

// 辅助函数实现

DetailsElementState* getDetailsState(DOMNodePtr node) {
    if (!node || !isDetailsElement(node)) return nullptr;

    void* key = node.get();
    std::unordered_map<void*, std::unique_ptr<DetailsElementState> >::iterator it = g_details_states.find(key);
    if (it == g_details_states.end()) {
        std::unique_ptr<DetailsElementState> state(new DetailsElementState());
        state->syncFromDOM(node);
        DetailsElementState* ptr = state.get();
        g_details_states[key] = std::move(state);
        return ptr;
    }

    return it->second.get();
}

bool isDetailsElement(const DOMNodePtr& node) {
    if (!node) return false;
    if (node->getType() != DOMNode::NodeType::ELEMENT) return false;
    return node->getTagName() == "details";
}

bool isSummaryElement(const DOMNodePtr& node) {
    if (!node) return false;
    if (node->getType() != DOMNode::NodeType::ELEMENT) return false;
    return node->getTagName() == "summary";
}

void removeDetailsState(DOMNodePtr node) {
    if (!node) return;
    void* key = node.get();
    g_details_states.erase(key);
}

void clearAllDetailsStates() {
    g_details_states.clear();
}

} // namespace dom
} // namespace dong