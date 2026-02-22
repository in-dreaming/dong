#include "select_element.hpp"
#include <algorithm>
#include <memory>
#include <unordered_map>

namespace dong::dom {

// 全局 select 状态存储（简化实现，生产环境应绑定到 DOMNode）
static std::unordered_map<void*, std::unique_ptr<SelectElementState>> g_select_states;

SelectElementState::SelectElementState() = default;

std::string SelectElementState::getSelectedValue() const {
    if (selected_index_ < options_.size()) {
        return options_[selected_index_].value;
    }
    return "";
}

void SelectElementState::toggle() {
    is_open_ = !is_open_;
}

void SelectElementState::open() {
    is_open_ = true;
}

void SelectElementState::close() {
    is_open_ = false;
}

void SelectElementState::selectOption(size_t index) {
    if (index < options_.size()) {
        selected_index_ = index;
        // NOTE: DOM sync (updating node's value attribute) happens in engine_view.cpp
    }
}

void SelectElementState::selectByValue(const std::string& value) {
    for (size_t i = 0; i < options_.size(); ++i) {
        if (options_[i].value == value) {
            selectOption(i);
            return;
        }
    }
}

void SelectElementState::syncFromDOM(const DOMNodePtr& node) {
    if (!node) return;

    // 清空现有选项
    options_.clear();

    // 遍历子节点，提取 <option> 元素
    const auto& children = node->getChildren();
    for (const auto& child : children) {
        if (!child) continue;
        if (child->getType() != DOMNode::NodeType::ELEMENT) continue;
        if (child->getTagName() != "option") continue;

        OptionData opt;

        // 提取 value 属性，如果没有则使用显示文本
        if (child->hasAttribute("value")) {
            opt.value = child->getAttribute("value");
        } else {
            opt.value = child->getTextContent();
        }

        // 提取显示文本
        opt.display_text = child->getTextContent();

        // 提取 disabled 属性
        opt.disabled = child->hasAttribute("disabled");

        options_.push_back(std::move(opt));
    }

    // 确保 selected_index_ 在有效范围内
    if (selected_index_ >= options_.size() && !options_.empty()) {
        selected_index_ = 0;
    }
}

bool SelectElementState::handleClick(const Point& pos, const Rect& dropdown_bounds) {
    // TODO (Chunk 3): 实现点击事件处理
    // - 检查点击位置是否在下拉框内
    // - 如果在下拉框内，选择对应选项并关闭下拉框
    // - 如果在下拉框外，关闭下拉框
    (void)pos;
    (void)dropdown_bounds;
    return false;
}

bool SelectElementState::handleKeyDown(uint32_t key_code) {
    // TODO (Chunk 3): 实现键盘事件处理
    // - ArrowDown/ArrowUp: 移动高亮选项
    // - Enter: 选择当前高亮选项并关闭下拉框
    // - Escape: 关闭下拉框
    (void)key_code;
    return false;
}

// 辅助函数实现

SelectElementState* getSelectState(DOMNodePtr node) {
    if (!node || !isSelectElement(node)) return nullptr;

    void* key = node.get();
    auto it = g_select_states.find(key);
    if (it == g_select_states.end()) {
        auto state = std::make_unique<SelectElementState>();
        // 从 DOM 同步初始状态
        state->syncFromDOM(node);
        auto* ptr = state.get();
        g_select_states[key] = std::move(state);
        return ptr;
    }
    return it->second.get();
}

bool isSelectElement(const DOMNodePtr& node) {
    if (!node) return false;
    if (node->getType() != DOMNode::NodeType::ELEMENT) return false;
    return node->getTagName() == "select";
}

void removeSelectState(DOMNodePtr node) {
    if (!node) return;
    void* key = node.get();
    g_select_states.erase(key);
}

} // namespace dong::dom
