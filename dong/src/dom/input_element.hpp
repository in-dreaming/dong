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

    // 默认值（HTML defaultValue，用于 form.reset()）
    const std::string& getDefaultValue() const { return default_value_; }
    void setDefaultValue(const std::string& value) { default_value_ = value; }

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

    // Constraint validation (minimal)
    void setCustomValidity(const std::string& message) { custom_validity_ = message; }
    const std::string& getCustomValidity() const { return custom_validity_; }
    bool hasCustomError() const { return !custom_validity_.empty(); }

    // Pattern validation
    void setPattern(const std::string& pattern) { pattern_ = pattern; }
    const std::string& getPattern() const { return pattern_; }
    bool hasPattern() const { return !pattern_.empty(); }
    bool matchesPattern(const std::string& value) const;

    // Range validation
    void setMin(const std::string& min) { min_ = min; }
    void setMax(const std::string& max) { max_ = max; }
    void setStep(const std::string& step) { step_ = step; }
    const std::string& getMin() const { return min_; }
    const std::string& getMax() const { return max_; }
    const std::string& getStep() const { return step_; }
    bool hasMin() const { return !min_.empty(); }
    bool hasMax() const { return !max_.empty(); }
    bool hasStep() const { return !step_.empty(); }
    bool isInRange(const std::string& value) const;


    // IME composition 状态
    bool isComposing() const { return is_composing_; }
    const std::string& getCompositionText() const { return composition_text_; }
    size_t getCompositionStart() const { return composition_start_; }
    size_t getCompositionLength() const;

    // 开始 IME 组合：记录起始位置，插入初始文本
    void startComposition(const std::string& text);
    // 更新 IME 组合：替换组合区间内的文本
    void updateComposition(const std::string& text);
    // 结束 IME 组合：清除组合状态（不改文本，SDL 会再发 TextInput）
    void endComposition();
    // 取消 IME 组合：撤销组合区间文本，恢复光标
    void cancelComposition();

    // Public UTF-8 helpers for rendering/hit-testing
    size_t utf8ByteOffsetForCursor() const { return utf8ByteOffset(cursor_position_); }
    size_t utf8ByteOffsetAt(size_t char_index) const { return utf8ByteOffset(char_index); }
    size_t charCount() const { return utf8CharCount(); }

private:
    std::string value_;
    std::string default_value_;  // original HTML value for form reset
    std::string placeholder_;
    std::string type_ = "text";

    // Constraint validation
    std::string custom_validity_;
    std::string pattern_;
    std::string min_;
    std::string max_;
    std::string step_;

    size_t cursor_position_ = 0;
    size_t selection_start_ = 0;
    size_t selection_end_ = 0;

    // IME composition 状态
    bool is_composing_ = false;
    std::string composition_text_;
    size_t composition_start_ = 0;  // 组合文本起始字符位置

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
