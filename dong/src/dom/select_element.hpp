#pragma once

#include "dom/dom_node.hpp"
#include <string>
#include <vector>

namespace dong::dom {

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

struct OptionData {
    std::string value;
    std::string display_text;
    bool disabled = false;
};

/**
 * Select 元素状态管理
 *
 * 为 <select> 元素提供下拉列表功能，包括：
 * - 打开/关闭状态
 * - 选中项管理
 * - 选项数据存储
 * - 事件处理（将在后续 chunk 实现）
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

    // 状态变更
    void toggle();
    void open();
    void close();
    void selectOption(size_t index);
    void selectByValue(const std::string& value);
    void setHoverIndex(int index) { hover_index_ = index; }

    // 事件处理（将在后续 chunk 实现）
    bool handleClick(const Point& pos, const Rect& dropdown_bounds);
    bool handleKeyDown(uint32_t key_code);

    // DOM 同步
    void syncFromDOM(const DOMNodePtr& node);

    // 访问选项
    const std::vector<OptionData>& getOptions() const { return options_; }

private:
    bool is_open_ = false;
    size_t selected_index_ = 0;
    int hover_index_ = -1;
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

} // namespace dong::dom
