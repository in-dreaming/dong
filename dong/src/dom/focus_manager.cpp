#include "focus_manager.hpp"
#include <algorithm>

namespace dong::dom {

FocusManager::FocusManager() = default;

FocusManager::~FocusManager() = default;

void FocusManager::setEventDispatcher(EventDispatcher* dispatcher) {
    event_dispatcher_ = dispatcher;
}

namespace {
    void markFocusChainDirty(const dong::dom::DOMNodePtr& node) {
        auto current = node;
        while (current) {
            current->markStyleDirty();
            current = current->getParent();
        }
    }
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

    if (old_focus) {
        old_focus->setFocused(false);
        old_focus->setFocusVisible(false);
        markFocusChainDirty(old_focus);
    }

    // 触发旧元素的 blur 事件，relatedTarget 是新获得焦点的元素
    if (old_focus) {
        dispatchBlurEvent(old_focus, element);
    }

    // 更新焦点
    focused_element_ = element;

    if (element) {
        element->setFocused(true);
        // 只有键盘聚焦时才设置 focus-visible
        element->setFocusVisible(keyboard_focus_);
        markFocusChainDirty(element);
    }

    // 触发新元素的 focus 事件，relatedTarget 是失去焦点的元素
    if (element) {
        dispatchFocusEvent(element, old_focus);
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

namespace {
    // 获取元素的 tabindex 值
    // 返回值：正数 = 显式 tabindex > 0, 0 = tabindex="0" 或默认可聚焦, -1 = 不可 tab 聚焦
    int getTabIndexValue(const dong::dom::DOMNodePtr& element) {
        if (!element) return -1;
        
        if (element->hasAttribute("tabindex")) {
            std::string tabindex = element->getAttribute("tabindex");
            try {
                int value = std::stoi(tabindex);
                return value;
            } catch (...) {
                return 0; // 解析失败，视为 0
            }
        }
        
        // 默认可聚焦元素（无 tabindex 属性）
        const std::string& tag = element->getTagName();
        if (tag == "input" || tag == "textarea" || tag == "select" || 
            tag == "button" || tag == "a") {
            return 0;
        }
        
        // contenteditable 元素
        if (element->hasAttribute("contenteditable")) {
            std::string value = element->getAttribute("contenteditable");
            if (value != "false") {
                return 0;
            }
        }
        
        return -1;
    }
    
    // 按 tabindex 排序的比较函数
    // 规则：正 tabindex 按值升序 -> tabindex=0 和默认可聚焦按文档顺序
    bool compareTabOrder(const dong::dom::DOMNodePtr& a, const dong::dom::DOMNodePtr& b) {
        int tabA = getTabIndexValue(a);
        int tabB = getTabIndexValue(b);
        
        // 都是正数 tabindex，按值排序
        if (tabA > 0 && tabB > 0) {
            return tabA < tabB;
        }
        
        // 一个是正数，一个不是，正数的在前
        if (tabA > 0) return true;
        if (tabB > 0) return false;
        
        // 都是 0 或都是 -1，保持文档顺序（通过收集顺序保证）
        return false; // 保持原有顺序
    }
}

void FocusManager::moveFocus(DOMNodePtr root, bool reverse) {
    if (!root) return;

    // 通过 Tab 键移动焦点，设置为键盘聚焦
    keyboard_focus_ = true;

    // 收集所有可聚焦元素
    std::vector<DOMNodePtr> focusable;
    collectFocusableElements(root, focusable);

    if (focusable.empty()) {
        blur();
        return;
    }

    // 按 tabindex 排序（正数 tabindex 在前，按值升序；0 和默认可聚焦在后，保持文档顺序）
    std::stable_sort(focusable.begin(), focusable.end(), compareTabOrder);
    
    // 过滤掉 tabindex="-1" 的元素（它们不在 tab 序列中）
    focusable.erase(
        std::remove_if(focusable.begin(), focusable.end(),
            [](const DOMNodePtr& elem) { return getTabIndexValue(elem) < 0; }),
        focusable.end()
    );
    
    if (focusable.empty()) {
        blur();
        return;
    }
    
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

    // 鼠标点击聚焦，不是键盘聚焦
    keyboard_focus_ = false;

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

void FocusManager::dispatchFocusEvent(DOMNodePtr element, DOMNodePtr related) {
    if (!event_dispatcher_ || !element) return;

    Event event = event_dispatcher_->createEvent(EventType::FOCUS);
    event.target = element;
    event.current_target = element;
    event.related_target = related;  // The element that lost focus
    event_dispatcher_->dispatch(event);
}

void FocusManager::dispatchBlurEvent(DOMNodePtr element, DOMNodePtr related) {
    if (!event_dispatcher_ || !element) return;

    Event event = event_dispatcher_->createEvent(EventType::BLUR);
    event.target = element;
    event.current_target = element;
    event.related_target = related;  // The element that will gain focus
    event_dispatcher_->dispatch(event);
}

} // namespace dong::dom
