#pragma once

#include "dom/dom_node.hpp"
#include "event_system.hpp"
#include <functional>

namespace dong::dom {

/**
 * 焦点管理器
 * 
 * 负责跟踪当前焦点元素，处理焦点切换，
 * 并在焦点变化时触发 focus/blur 事件。
 */
class FocusManager {
public:
    FocusManager();
    ~FocusManager();

    /**
     * 设置事件分发器（用于触发 focus/blur 事件）
     */
    void setEventDispatcher(EventDispatcher* dispatcher);

    /**
     * 设置焦点到指定元素
     * 会触发旧元素的 blur 事件和新元素的 focus 事件
     * @param element 要聚焦的元素，nullptr 表示清除焦点
     */
    void setFocus(DOMNodePtr element);

    /**
     * 清除当前焦点
     * 等同于 setFocus(nullptr)
     */
    void blur();

    /**
     * 获取当前焦点元素
     * @return 当前焦点元素，如果没有焦点则返回 nullptr
     */
    DOMNodePtr getFocusedElement() const;

    /**
     * 检查指定元素是否可聚焦
     * 可聚焦的元素包括：
     * - <input>, <textarea>, <select>, <button>, <a>
     * - 带有 tabindex 属性的元素
     * - contenteditable 元素
     */
    static bool isFocusable(const DOMNodePtr& element);

    /**
     * 检查指定元素是否为当前焦点
     */
    bool hasFocus(const DOMNodePtr& element) const;

    /**
     * 移动焦点到下一个可聚焦元素
     * @param root DOM 根节点，用于遍历
     * @param reverse 是否反向（Shift+Tab）
     */
    void moveFocus(DOMNodePtr root, bool reverse = false);

    /**
     * 尝试聚焦点击位置的元素
     * @param element 被点击的元素
     * @return true 如果焦点发生了变化
     */
    bool focusOnClick(DOMNodePtr element);

    /**
     * 设置焦点变化回调
     * 在焦点变化时调用，参数为 (旧元素, 新元素)
     */
    using FocusChangeCallback = std::function<void(DOMNodePtr old_focus, DOMNodePtr new_focus)>;
    void setFocusChangeCallback(FocusChangeCallback callback);

    /**
     * 设置焦点是否通过键盘（如 Tab 键）获得
     * 影响 :focus-visible 伪类的匹配
     */
    void setKeyboardFocus(bool is_keyboard) { keyboard_focus_ = is_keyboard; }
    bool isKeyboardFocus() const { return keyboard_focus_; }

    /**
     * Modal dialog focus trap: restrict Tab cycling to within a dialog subtree.
     * Pass nullptr to clear the trap.
     */
    void setModalDialogRoot(DOMNodePtr dialog) { modal_dialog_root_ = dialog; }
    DOMNodePtr getModalDialogRoot() const { return modal_dialog_root_; }

private:
    DOMNodePtr focused_element_;
    EventDispatcher* event_dispatcher_ = nullptr;
    FocusChangeCallback focus_change_callback_;
    bool keyboard_focus_ = false; // 焦点是否通过键盘获得
    DOMNodePtr modal_dialog_root_; // Focus trap root for modal dialog

    // 收集所有可聚焦元素（按 tab 顺序）
    void collectFocusableElements(DOMNodePtr node, std::vector<DOMNodePtr>& out);

    // 触发 focus 事件
    void dispatchFocusEvent(DOMNodePtr element, DOMNodePtr related_target = nullptr);

    // 触发 blur 事件
    void dispatchBlurEvent(DOMNodePtr element, DOMNodePtr related_target = nullptr);
};

} // namespace dong::dom
