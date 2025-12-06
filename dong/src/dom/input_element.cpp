#include "input_element.hpp"
#include <algorithm>
#include <unordered_map>

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

void InputElementState::insertText(const std::string& text) {
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

// 辅助函数实现
InputElementState* getInputState(DOMNodePtr node) {
    if (!node || !isInputElement(node)) return nullptr;
    
    void* key = node.get();
    auto it = g_input_states.find(key);
    if (it == g_input_states.end()) {
        auto state = std::make_unique<InputElementState>();
        // 从 DOM 属性初始化
        if (node->hasAttribute("value")) {
            state->setValue(node->getAttribute("value"));
        }
        if (node->hasAttribute("placeholder")) {
            state->setPlaceholder(node->getAttribute("placeholder"));
        }
        if (node->hasAttribute("type")) {
            state->setType(node->getAttribute("type"));
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

} // namespace dong::dom
