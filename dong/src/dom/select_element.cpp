#include "select_element.hpp"
#include <algorithm>
#include <cmath>
#include <memory>
#include <unordered_map>

namespace dong::dom {

// 全局 select 状态存储（简化实现；DOM 重载时必须 clearAllSelectStates()）
static std::unordered_map<void*, std::unique_ptr<SelectElementState>> g_select_states;

namespace {

bool pointInRect(float px, float py, const Rect& r) {
    return px >= r.x && px <= r.x + r.width && py >= r.y && py <= r.y + r.height;
}

std::optional<size_t> findOptionIndexByValue(const std::vector<OptionData>& options, const std::string& value) {
    if (value.empty()) return std::nullopt;
    for (size_t i = 0; i < options.size(); ++i) {
        if (options[i].value == value) return i;
    }
    return std::nullopt;
}

size_t clampIndex(size_t i, size_t n) {
    if (n == 0) return 0;
    return std::min(i, n - 1);
}

std::optional<size_t> findFirstEnabled(const std::vector<OptionData>& options) {
    for (size_t i = 0; i < options.size(); ++i) {
        if (!options[i].disabled) return i;
    }
    return std::nullopt;
}

} // namespace

SelectElementState::SelectElementState() = default;

std::string SelectElementState::getSelectedValue() const {
    if (selected_index_ < options_.size()) {
        return options_[selected_index_].value;
    }
    return "";
}

void SelectElementState::toggle() {
    if (is_open_) {
        close();
    } else {
        open();
    }
}

void SelectElementState::open() {
    is_open_ = true;
    if (options_.empty()) {
        hover_index_ = -1;
        scroll_offset_ = 0.0f;
        return;
    }

    const size_t sel = clampIndex(selected_index_, options_.size());
    hover_index_ = static_cast<int>(sel);

    // Ensure selected option is visible in the dropdown viewport.
    const float viewport_h = std::min(static_cast<float>(options_.size()) * kSelectOptionHeight, kSelectDropdownMaxHeight);
    const float top = static_cast<float>(sel) * kSelectOptionHeight;
    const float bottom = top + kSelectOptionHeight;

    if (top < scroll_offset_) {
        scroll_offset_ = top;
    } else if (bottom > scroll_offset_ + viewport_h) {
        scroll_offset_ = std::max(0.0f, bottom - viewport_h);
    }

    scrollBy(0.0f, viewport_h);
}


void SelectElementState::close() {
    is_open_ = false;
    hover_index_ = -1;
}

void SelectElementState::selectOption(size_t index) {
    if (index >= options_.size()) return;
    if (options_[index].disabled) return;
    selected_index_ = index;
    if (!is_open_) {
        hover_index_ = -1;
    }
}

void SelectElementState::selectByValue(const std::string& value) {
    auto idx = findOptionIndexByValue(options_, value);
    if (!idx) return;
    selectOption(*idx);
}

bool SelectElementState::isOptionDisabled(size_t index) const {
    if (index >= options_.size()) return true;
    return options_[index].disabled;
}

void SelectElementState::scrollBy(float dy_px, float viewport_height_px) {
    if (options_.empty()) {
        scroll_offset_ = 0.0f;
        return;
    }

    const float content_h = static_cast<float>(options_.size()) * kSelectOptionHeight;
    const float max_scroll = std::max(0.0f, content_h - std::max(0.0f, viewport_height_px));

    scroll_offset_ = std::clamp(scroll_offset_ + dy_px, 0.0f, max_scroll);
}

void SelectElementState::syncFromDOM(const DOMNodePtr& node) {
    if (!node) return;

    // 清空现有选项
    options_.clear();

    const std::string value_attr = node->getAttribute("value");
    std::optional<size_t> selected_by_value;
    std::optional<size_t> selected_by_attr;

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

        const size_t idx = options_.size();
        if (!selected_by_attr && child->hasAttribute("selected")) {
            selected_by_attr = idx;
        }
        if (!selected_by_value && !value_attr.empty() && opt.value == value_attr) {
            selected_by_value = idx;
        }

        options_.push_back(std::move(opt));
    }

    if (options_.empty()) {
        selected_index_ = 0;
        hover_index_ = -1;
        scroll_offset_ = 0.0f;
        return;
    }

    // 优先级：value 属性 -> option[selected] -> 维持旧 selected_index_ -> 默认 0
    size_t resolved = selected_index_;
    if (selected_by_value) {
        resolved = *selected_by_value;
    } else if (selected_by_attr) {
        resolved = *selected_by_attr;
    }
    resolved = clampIndex(resolved, options_.size());

    // 如果选中的 option 是 disabled，回退到第一个可用项
    if (options_[resolved].disabled) {
        if (auto first = findFirstEnabled(options_)) {
            resolved = *first;
        }
    }

    selected_index_ = resolved;

    // 保持滚动范围合法
    scrollBy(0.0f, kSelectDropdownMaxHeight);
}

void SelectElementState::applySelectionToDOM(const DOMNodePtr& select_node) const {
    if (!select_node) return;

    // 同步 select.value
    if (selected_index_ < options_.size()) {
        select_node->setAttribute("value", options_[selected_index_].value);
    } else {
        select_node->removeAttribute("value");
    }

    // 同步 option[selected]
    size_t opt_index = 0;
    for (const auto& child : select_node->getChildren()) {
        if (!child) continue;
        if (child->getType() != DOMNode::NodeType::ELEMENT) continue;
        if (child->getTagName() != "option") continue;

        if (opt_index == selected_index_) {
            child->setAttribute("selected", "");
        } else {
            child->removeAttribute("selected");
        }
        ++opt_index;
    }
}

bool SelectElementState::handleClick(const Point& pos, const Rect& dropdown_bounds) {
    if (!is_open_) return false;

    const float px = static_cast<float>(pos.x);
    const float py = static_cast<float>(pos.y);

    if (!pointInRect(px, py, dropdown_bounds)) {
        close();
        return false;
    }

    const float local_y = py - dropdown_bounds.y;
    const float y_in_content = local_y + scroll_offset_;
    if (y_in_content < 0.0f) return true;

    const size_t idx = static_cast<size_t>(std::floor(y_in_content / kSelectOptionHeight));
    if (idx >= options_.size()) return true;

    if (!options_[idx].disabled) {
        selectOption(idx);
        close();
        return true;
    }

    return true;
}

bool SelectElementState::handleKeyDown(uint32_t key_code) {
    constexpr uint32_t SDLK_RETURN = 13;
    constexpr uint32_t SDLK_ESCAPE = 27;
    constexpr uint32_t SDLK_UP = 0x40000052;
    constexpr uint32_t SDLK_DOWN = 0x40000051;

    if (options_.empty()) return false;

    if (key_code == SDLK_ESCAPE) {
        if (is_open_) {
            close();
            return true;
        }
        return false;
    }

    if (key_code == SDLK_RETURN) {
        toggle();
        return true;
    }

    if (key_code != SDLK_UP && key_code != SDLK_DOWN) return false;

    // closed: move selection; open: move hover
    const bool move_hover = is_open_;

    size_t idx = move_hover && hover_index_ >= 0 ? static_cast<size_t>(hover_index_) : selected_index_;
    const int dir = (key_code == SDLK_DOWN) ? 1 : -1;

    for (size_t step = 0; step < options_.size(); ++step) {
        const int next_i = static_cast<int>(idx) + dir;
        if (next_i < 0 || next_i >= static_cast<int>(options_.size())) break;
        idx = static_cast<size_t>(next_i);
        if (!options_[idx].disabled) {
            if (move_hover) {
                hover_index_ = static_cast<int>(idx);
            } else {
                selected_index_ = idx;
            }
            return true;
        }
    }

    return true;
}

// 辅助函数实现

SelectElementState* getSelectState(DOMNodePtr node) {
    if (!node || !isSelectElement(node)) return nullptr;

    void* key = node.get();
    auto it = g_select_states.find(key);
    if (it == g_select_states.end()) {
        auto state = std::make_unique<SelectElementState>();
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

void clearAllSelectStates() {
    g_select_states.clear();
}

} // namespace dong::dom

