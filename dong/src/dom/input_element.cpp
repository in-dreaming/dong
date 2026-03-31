#include "input_element.hpp"
#include <algorithm>
#include <unordered_map>
#include <regex>
#include <cmath>

namespace dong::dom {

// 全局 input 状态存储（简化实现，生产环境应绑定到 DOMNode）
static std::unordered_map<void*, std::unique_ptr<InputElementState>> g_input_states;

InputElementState::InputElementState() = default;

void InputElementState::setValue(const std::string& value) {
    value_ = value;
    // 确保光标位置有效
    size_t char_count = utf8CharCount();
    if (cursor_position_ > char_count) {
        cursor_position_ = char_count;
    }
    clearSelection();
}

void InputElementState::setCursorPosition(size_t pos) {
    size_t max_pos = utf8CharCount();
    cursor_position_ = std::min(pos, max_pos);
    clearSelection();
}

void InputElementState::moveCursor(int delta) {
    if (delta < 0) {
        size_t abs_delta = static_cast<size_t>(-delta);
        cursor_position_ = (cursor_position_ > abs_delta) ? cursor_position_ - abs_delta : 0;
    } else {
        cursor_position_ = std::min(cursor_position_ + static_cast<size_t>(delta), utf8CharCount());
    }
    clearSelection();
}

void InputElementState::insertText(const std::string& text, DOMNodePtr node) {
    // Check maxlength attribute if node is provided
    if (node && node->hasAttribute("maxlength")) {
        try {
            int maxlength = std::stoi(node->getAttribute("maxlength"));
            if (maxlength >= 0) {
                size_t current_length = utf8CharCount();
                // Calculate how many characters we can insert
                size_t text_chars = 0;
                for (size_t i = 0; i < text.size(); ) {
                    unsigned char c = text[i];
                    if ((c & 0x80) == 0) i += 1;
                    else if ((c & 0xE0) == 0xC0) i += 2;
                    else if ((c & 0xF0) == 0xE0) i += 3;
                    else i += 4;
                    text_chars++;
                }

                // If we have a selection, it will be deleted first
                size_t chars_after_deletion = current_length;
                if (hasSelection()) {
                    chars_after_deletion -= (selection_end_ - selection_start_);
                }

                // Check if we would exceed maxlength
                if (chars_after_deletion + text_chars > static_cast<size_t>(maxlength)) {
                    // Truncate text to fit maxlength
                    size_t available = static_cast<size_t>(maxlength) - chars_after_deletion;
                    if (available == 0) {
                        return; // Cannot insert any characters
                    }
                    // Truncate text to available characters
                    size_t byte_count = 0;
                    size_t char_count = 0;
                    for (size_t i = 0; i < text.size() && char_count < available; ) {
                        unsigned char c = text[i];
                        if ((c & 0x80) == 0) byte_count += 1, i += 1;
                        else if ((c & 0xE0) == 0xC0) byte_count += 2, i += 2;
                        else if ((c & 0xF0) == 0xE0) byte_count += 3, i += 3;
                        else byte_count += 4, i += 4;
                        char_count++;
                    }
                    std::string truncated_text = text.substr(0, byte_count);
                    // Continue with truncated text
                    if (hasSelection()) {
                        deleteSelection();
                    }
                    size_t byte_offset = utf8ByteOffset(cursor_position_);
                    value_.insert(byte_offset, truncated_text);
                    cursor_position_ += char_count;
                    return;
                }
            }
        } catch (...) {
            // Invalid maxlength value, ignore
        }
    }

    if (hasSelection()) {
        deleteSelection();
    }

    // 在光标位置插入文本
    size_t byte_offset = utf8ByteOffset(cursor_position_);
    value_.insert(byte_offset, text);

    // 移动光标到插入文本之后
    // 计算插入文本的字符数
    size_t inserted_chars = 0;
    for (size_t i = 0; i < text.size(); ) {
        unsigned char c = text[i];
        if ((c & 0x80) == 0) i += 1;
        else if ((c & 0xE0) == 0xC0) i += 2;
        else if ((c & 0xF0) == 0xE0) i += 3;
        else i += 4;
        inserted_chars++;
    }
    cursor_position_ += inserted_chars;
}

void InputElementState::deleteBackward() {
    if (hasSelection()) {
        deleteSelection();
        return;
    }
    
    if (cursor_position_ == 0) return;
    
    // 删除光标前一个字符
    size_t byte_end = utf8ByteOffset(cursor_position_);
    cursor_position_--;
    size_t byte_start = utf8ByteOffset(cursor_position_);
    
    value_.erase(byte_start, byte_end - byte_start);
}

void InputElementState::deleteForward() {
    if (hasSelection()) {
        deleteSelection();
        return;
    }
    
    size_t char_count = utf8CharCount();
    if (cursor_position_ >= char_count) return;
    
    // 删除光标后一个字符
    size_t byte_start = utf8ByteOffset(cursor_position_);
    size_t byte_end = utf8ByteOffset(cursor_position_ + 1);
    
    value_.erase(byte_start, byte_end - byte_start);
}

void InputElementState::clear() {
    value_.clear();
    cursor_position_ = 0;
    clearSelection();
}

void InputElementState::setSelection(size_t start, size_t end) {
    size_t max_pos = utf8CharCount();
    selection_start_ = std::min(start, max_pos);
    selection_end_ = std::min(end, max_pos);
    if (selection_start_ > selection_end_) {
        std::swap(selection_start_, selection_end_);
    }
    // Keep cursor at the end of the selection for keyboard navigation
    cursor_position_ = selection_end_;
}

void InputElementState::clearSelection() {
    selection_start_ = cursor_position_;
    selection_end_ = cursor_position_;
}

void InputElementState::selectAll() {
    selection_start_ = 0;
    selection_end_ = utf8CharCount();
    cursor_position_ = selection_end_;
}

std::string InputElementState::getSelectedText() const {
    if (!hasSelection()) return "";
    
    size_t byte_start = utf8ByteOffset(selection_start_);
    size_t byte_end = utf8ByteOffset(selection_end_);
    
    return value_.substr(byte_start, byte_end - byte_start);
}

void InputElementState::deleteSelection() {
    if (!hasSelection()) return;
    
    size_t byte_start = utf8ByteOffset(selection_start_);
    size_t byte_end = utf8ByteOffset(selection_end_);
    
    value_.erase(byte_start, byte_end - byte_start);
    cursor_position_ = selection_start_;
    clearSelection();
}

size_t InputElementState::utf8CharCount() const {
    size_t count = 0;
    for (size_t i = 0; i < value_.size(); ) {
        unsigned char c = value_[i];
        if ((c & 0x80) == 0) i += 1;
        else if ((c & 0xE0) == 0xC0) i += 2;
        else if ((c & 0xF0) == 0xE0) i += 3;
        else i += 4;
        count++;
    }
    return count;
}

size_t InputElementState::utf8ByteOffset(size_t char_index) const {
    size_t byte_offset = 0;
    size_t char_count = 0;
    
    while (byte_offset < value_.size() && char_count < char_index) {
        unsigned char c = value_[byte_offset];
        if ((c & 0x80) == 0) byte_offset += 1;
        else if ((c & 0xE0) == 0xC0) byte_offset += 2;
        else if ((c & 0xF0) == 0xE0) byte_offset += 3;
        else byte_offset += 4;
        char_count++;
    }
    
    return byte_offset;
}

size_t InputElementState::utf8CharAt(size_t byte_offset) const {
    size_t char_count = 0;
    size_t current_offset = 0;
    
    while (current_offset < byte_offset && current_offset < value_.size()) {
        unsigned char c = value_[current_offset];
        if ((c & 0x80) == 0) current_offset += 1;
        else if ((c & 0xE0) == 0xC0) current_offset += 2;
        else if ((c & 0xF0) == 0xE0) current_offset += 3;
        else current_offset += 4;
        char_count++;
    }
    
    return char_count;
}

// --- IME composition ---

static size_t utf8CharCountOf(const std::string& s) {
    size_t count = 0;
    for (size_t i = 0; i < s.size(); ) {
        unsigned char c = s[i];
        if ((c & 0x80) == 0) i += 1;
        else if ((c & 0xE0) == 0xC0) i += 2;
        else if ((c & 0xF0) == 0xE0) i += 3;
        else i += 4;
        count++;
    }
    return count;
}

size_t InputElementState::getCompositionLength() const {
    return utf8CharCountOf(composition_text_);
}

void InputElementState::startComposition(const std::string& text) {
    if (hasSelection()) {
        deleteSelection();
    }
    is_composing_ = true;
    composition_start_ = cursor_position_;
    composition_text_ = text;

    // 在光标位置插入组合文本
    size_t byte_offset = utf8ByteOffset(cursor_position_);
    value_.insert(byte_offset, text);
    cursor_position_ += utf8CharCountOf(text);
}

void InputElementState::updateComposition(const std::string& text) {
    if (!is_composing_) {
        startComposition(text);
        return;
    }

    // 删除旧的组合文本
    size_t old_chars = utf8CharCountOf(composition_text_);
    size_t byte_start = utf8ByteOffset(composition_start_);
    size_t byte_end = utf8ByteOffset(composition_start_ + old_chars);
    value_.erase(byte_start, byte_end - byte_start);

    // 插入新的组合文本
    composition_text_ = text;
    value_.insert(byte_start, text);
    cursor_position_ = composition_start_ + utf8CharCountOf(text);
}

void InputElementState::endComposition() {
    if (!is_composing_) return;

    // 删除组合文本（SDL 会紧接着发 TextInput 提交最终文本）
    size_t comp_chars = utf8CharCountOf(composition_text_);
    size_t byte_start = utf8ByteOffset(composition_start_);
    size_t byte_end = utf8ByteOffset(composition_start_ + comp_chars);
    value_.erase(byte_start, byte_end - byte_start);
    cursor_position_ = composition_start_;

    is_composing_ = false;
    composition_text_.clear();
    composition_start_ = 0;
}

void InputElementState::cancelComposition() {
    if (!is_composing_) return;

    // 撤销组合文本
    size_t comp_chars = utf8CharCountOf(composition_text_);
    size_t byte_start = utf8ByteOffset(composition_start_);
    size_t byte_end = utf8ByteOffset(composition_start_ + comp_chars);
    value_.erase(byte_start, byte_end - byte_start);
    cursor_position_ = composition_start_;

    is_composing_ = false;
    composition_text_.clear();
    composition_start_ = 0;
}

// 辅助函数实现
InputElementState* getInputState(DOMNodePtr node) {
    if (!node || !isInputElement(node)) return nullptr;
    
    void* key = node.get();
    auto it = g_input_states.find(key);
    if (it == g_input_states.end()) {
        auto state = std::make_unique<InputElementState>();
        // 从 DOM 属性初始化
        if (node->hasAttribute("value")) {
            std::string val = node->getAttribute("value");
            state->setValue(val);
            state->setDefaultValue(val);
        }
        if (node->hasAttribute("placeholder")) {
            state->setPlaceholder(node->getAttribute("placeholder"));
        }
        if (node->hasAttribute("type")) {
            state->setType(node->getAttribute("type"));
        }
        if (node->hasAttribute("pattern")) {
            state->setPattern(node->getAttribute("pattern"));
        }
        if (node->hasAttribute("min")) {
            state->setMin(node->getAttribute("min"));
        }
        if (node->hasAttribute("max")) {
            state->setMax(node->getAttribute("max"));
        }
        if (node->hasAttribute("step")) {
            state->setStep(node->getAttribute("step"));
        }
        auto* ptr = state.get();
        g_input_states[key] = std::move(state);
        return ptr;
    }
    return it->second.get();
}

bool isInputElement(const DOMNodePtr& node) {
    if (!node) return false;
    if (node->getType() != DOMNode::NodeType::ELEMENT) return false;
    const std::string& tag = node->getTagName();
    return tag == "input" || tag == "textarea";
}

bool isEditableElement(const DOMNodePtr& node) {
    if (!node) return false;
    if (isInputElement(node)) return true;
    if (node->hasAttribute("contenteditable")) {
        std::string value = node->getAttribute("contenteditable");
        return value != "false";
    }
    return false;
}

// Pattern validation implementation
bool InputElementState::matchesPattern(const std::string& value) const {
    if (pattern_.empty()) return true;
    if (value.empty()) return false;

    // CSS pattern attribute implicitly anchors: must match full string
    // Use ECMAScript regex (same as JS regex used in CSS pattern attribute)
    try {
        std::regex re(pattern_, std::regex::ECMAScript);
        return std::regex_match(value, re);
    } catch (...) {
        // Invalid regex pattern: treat value as invalid
        return false;
    }
}

// Range validation implementation
bool InputElementState::isInRange(const std::string& value) const {
    if (value.empty()) return true;

    // For numeric types
    if (type_ == "number" || type_ == "range") {
        try {
            double num_value = std::stod(value);

            // Check min constraint
            if (!min_.empty()) {
                double min_val = std::stod(min_);
                if (num_value < min_val) return false;
            }

            // Check max constraint
            if (!max_.empty()) {
                double max_val = std::stod(max_);
                if (num_value > max_val) return false;
            }

            // Check step constraint
            if (!step_.empty() && step_ != "any") {
                double step_val = std::stod(step_);
                if (step_val > 0) {
                    if (!min_.empty()) {
                        double min_val = std::stod(min_);
                        double remainder = std::fmod(num_value - min_val, step_val);
                        // Allow for floating point precision issues
                        if (std::abs(remainder) > 1e-10 && std::abs(remainder - step_val) > 1e-10) {
                            return false;
                        }
                    }
                }
            }

            return true;
        } catch (...) {
            // Invalid number format
            return false;
        }
    }

    // For date/time types (simplified implementation)
    if (type_ == "date" || type_ == "time" || type_ == "datetime-local") {
        // Simple string comparison for date/time ranges
        if (!min_.empty() && value < min_) return false;
        if (!max_.empty() && value > max_) return false;
        return true;
    }

    // For other input types, range validation doesn't apply
    return true;
}

} // namespace dong::dom
