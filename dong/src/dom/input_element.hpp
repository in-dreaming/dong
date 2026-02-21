#pragma once

#include "dom/dom_node.hpp"
#include <string>

namespace dong::dom {

/**
 * Input 元素状态管理
 * 
 * 为 <input> 元素提供文本编辑功能，包括：
 * - 文本内容存储
 * - 光标位置管理
 * - 文本选择（可选）
 * - 基础编辑操作
 */
class InputElementState {
public:
    InputElementState();
    ~InputElementState() = default;

    // 获取/设置文本值
    const std::string& getValue() const { return value_; }
    void setValue(const std::string& value);

    // 光标位置（UTF-8 字符索引）
    size_t getCursorPosition() const { return cursor_position_; }
    void setCursorPosition(size_t pos);
    void moveCursor(int delta);  // 正数向右，负数向左

    // 文本编辑操作
    void insertText(const std::string& text, DOMNodePtr node = nullptr);
    void deleteBackward();  // Backspace
    void deleteForward();   // Delete
    void clear();

    // 选择（简化版）
    bool hasSelection() const { return selection_start_ != selection_end_; }
    size_t getSelectionStart() const { return selection_start_; }
    size_t getSelectionEnd() const { return selection_end_; }
    void setSelection(size_t start, size_t end);
    void clearSelection();
    void selectAll();
    std::string getSelectedText() const;
    void deleteSelection();

    // placeholder
    const std::string& getPlaceholder() const { return placeholder_; }
    void setPlaceholder(const std::string& placeholder) { placeholder_ = placeholder; }

    // input type
    const std::string& getType() const { return type_; }
    void setType(const std::string& type) { type_ = type; }

private:
    std::string value_;
    std::string placeholder_;
    std::string type_ = "text";
    size_t cursor_position_ = 0;
    size_t selection_start_ = 0;
    size_t selection_end_ = 0;

    // UTF-8 辅助函数
    size_t utf8CharCount() const;
    size_t utf8ByteOffset(size_t char_index) const;
    size_t utf8CharAt(size_t byte_offset) const;
};

/**
 * 辅助函数：从 DOMNode 获取或创建 InputElementState
 */
InputElementState* getInputState(DOMNodePtr node);

/**
 * 辅助函数：检查节点是否为 input 元素
 */
bool isInputElement(const DOMNodePtr& node);

/**
 * 辅助函数：检查节点是否为可编辑元素（input, textarea, contenteditable）
 */
bool isEditableElement(const DOMNodePtr& node);

} // namespace dong::dom
