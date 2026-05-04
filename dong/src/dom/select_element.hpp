#pragma once

#include "dom/dom_node.hpp"
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace dong::dom {

inline constexpr float kSelectOptionHeight = 30.0f;
inline constexpr float kSelectOptgroupHeight = 35.0f;  // optgroup 标题略高
inline constexpr float kSelectDropdownMaxHeight = 240.0f; // 8 options * 30px

// Simple point structure for coordinates
struct Point {
    int32_t x = 0;
    int32_t y = 0;
};

// Simple rect structure for bounds checking
struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

// 选项类型标识符
enum class SelectItemType : uint8_t {
    Option,      // 普通 <option> 元素
    Optgroup,    // <optgroup> 分组标题
};

struct OptionData {
    std::string value;
    std::string display_text;
    bool disabled = false;
    SelectItemType type = SelectItemType::Option;
    std::string optgroup_label;  // 所属 optgroup 的 label，空表示不属于任何分组
};

/**
 * Select 元素状态管理
 *
 * 为 <select> 元素提供下拉列表功能，包括：
 * - 打开/关闭状态
 * - 选中项管理
 * - 选项数据存储
 * - 下拉滚动/悬停状态
 */
class SelectElementState {
public:
    SelectElementState();
    ~SelectElementState() = default;

    // 状态查询
    bool isOpen() const { return is_open_; }
    size_t getSelectedIndex() const { return selected_index_; }
    size_t getOptionCount() const { return options_.size(); }
    std::string getSelectedValue() const;
    int getHoverIndex() const { return hover_index_; }
    float getScrollOffset() const { return scroll_offset_; }

    // 状态变更
    void toggle();
    void open();
    void close();

    void selectOption(size_t index);
    void selectByValue(const std::string& value);

    bool isOptionDisabled(size_t index) const;

    void setHoverIndex(int index) { hover_index_ = index; }

    // dropdown scroll
    void scrollBy(float dy_px, float viewport_height_px);

    // DOM 同步
    void syncFromDOM(const DOMNodePtr& node);
    void applySelectionToDOM(const DOMNodePtr& select_node) const;

    // 事件处理（供上层使用；返回是否“消费”该事件）
    bool handleClick(const Point& pos, const Rect& dropdown_bounds);
    bool handleKeyDown(uint32_t key_code);

    // 访问选项
    const std::vector<OptionData>& getOptions() const { return options_; }

    // 计算指定类型项目的高度
    static float getItemHeight(SelectItemType type) {
        if (type == SelectItemType::Optgroup) return kSelectOptgroupHeight;
        return kSelectOptionHeight;
    }

private:
    bool is_open_ = false;
    size_t selected_index_ = 0;
    int hover_index_ = -1;
    float scroll_offset_ = 0.0f;
    std::vector<OptionData> options_;
};

/**
 * 辅助函数：从 DOMNode 获取或创建 SelectElementState
 */
SelectElementState* getSelectState(DOMNodePtr node);

/**
 * 辅助函数：检查节点是否为 select 元素
 */
bool isSelectElement(const DOMNodePtr& node);

/**
 * 辅助函数：移除 select 状态（节点销毁时调用）
 */
void removeSelectState(DOMNodePtr node);

/**
 * DOM 被重载/替换时，需要清空全局 select 状态，避免悬空指针 key 被复用。
 */
void clearAllSelectStates();

} // namespace dong::dom

