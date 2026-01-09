#include "focus_manager.hpp"
#include <algorithm>

namespace dong::dom {

FocusManager::FocusManager() = default;

FocusManager::~FocusManager() = default;

void FocusManager::setEventDispatcher(EventDispatcher* dispatcher) {
    event_dispatcher_ = dispatcher;
}

void FocusManager::setFocus(DOMNodePtr element) {
    // 如果元素相同，不做任何事
    if (focused_element_ == element) {
        return;
    }

    // 如果新元素不可聚焦，不做任何事
    if (element && !isFocusable(element)) {
        return;
    }

    DOMNodePtr old_focus = focused_element_;
    
    // 触发旧元素的 blur 事件
    if (old_focus) {
        dispatchBlurEvent(old_focus);
    }

    // 更新焦点
    focused_element_ = element;

    // 触发新元素的 focus 事件
    if (element) {
        dispatchFocusEvent(element);
    }

    // 调用回调
    if (focus_change_callback_) {
        focus_change_callback_(old_focus, element);
    }
}

void FocusManager::blur() {
    setFocus(nullptr);
}

DOMNodePtr FocusManager::getFocusedElement() const {
    return focused_element_;
}

bool FocusManager::isFocusable(const DOMNodePtr& element) {
    if (!element) return false;
    if (element->getType() != DOMNode::NodeType::ELEMENT) return false;

    const std::string& tag = element->getTagName();
    
    // 原生可聚焦元素
    if (tag == "input" || tag == "textarea" || tag == "select" || 
        tag == "button" || tag == "a") {
        // 检查是否被禁用
        if (element->hasAttribute("disabled")) {
            return false;
        }
        return true;
    }

    // tabindex 属性
    if (element->hasAttribute("tabindex")) {
        std::string tabindex = element->getAttribute("tabindex");
        // tabindex="-1" 表示可编程聚焦但不在 tab 序列中
        // 这里我们认为它仍然是可聚焦的
        return true;
    }

    // contenteditable 元素
    if (element->hasAttribute("contenteditable")) {
        std::string value = element->getAttribute("contenteditable");
        return value != "false";
    }

    return false;
}

bool FocusManager::hasFocus(const DOMNodePtr& element) const {
    return focused_element_ && focused_element_ == element;
}

void FocusManager::moveFocus(DOMNodePtr root, bool reverse) {
    if (!root) return;

    // 收集所有可聚焦元素
    std::vector<DOMNodePtr> focusable;
    collectFocusableElements(root, focusable);

    if (focusable.empty()) {
        blur();
        return;
    }

    // 按 tabindex 排序（简化版：只区分正数和其他）
    // TODO: 完整的 tabindex 排序
    
    // 找到当前焦点的位置
    auto it = std::find(focusable.begin(), focusable.end(), focused_element_);
    
    if (it == focusable.end()) {
        // 当前没有焦点，聚焦第一个（或最后一个）
        setFocus(reverse ? focusable.back() : focusable.front());
    } else {
        // 移动到下一个（或上一个）
        if (reverse) {
            if (it == focusable.begin()) {
                setFocus(focusable.back());
            } else {
                setFocus(*(it - 1));
            }
        } else {
            ++it;
            if (it == focusable.end()) {
                setFocus(focusable.front());
            } else {
                setFocus(*it);
            }
        }
    }
}

bool FocusManager::focusOnClick(DOMNodePtr element) {
    if (!element) return false;

    // 向上查找可聚焦的祖先
    DOMNodePtr current = element;
    while (current) {
        if (isFocusable(current)) {
            if (current != focused_element_) {
                setFocus(current);
                return true;
            }
            return false;
        }
        current = current->getParent();
    }

    // 点击了不可聚焦的区域，清除焦点
    if (focused_element_) {
        blur();
        return true;
    }

    return false;
}

void FocusManager::setFocusChangeCallback(FocusChangeCallback callback) {
    focus_change_callback_ = std::move(callback);
}

void FocusManager::collectFocusableElements(DOMNodePtr node, std::vector<DOMNodePtr>& out) {
    if (!node) return;

    if (isFocusable(node)) {
        out.push_back(node);
    }

    for (const auto& child : node->getChildren()) {
        collectFocusableElements(child, out);
    }
}

void FocusManager::dispatchFocusEvent(DOMNodePtr element) {
    if (!event_dispatcher_ || !element) return;

    Event event = event_dispatcher_->createEvent(EventType::FOCUS);
    event.target = element;
    event.current_target = element;
    event_dispatcher_->dispatch(event);
}

void FocusManager::dispatchBlurEvent(DOMNodePtr element) {
    if (!event_dispatcher_ || !element) return;

    Event event = event_dispatcher_->createEvent(EventType::BLUR);
    event.target = element;
    event.current_target = element;
    event_dispatcher_->dispatch(event);
}

} // namespace dong::dom
