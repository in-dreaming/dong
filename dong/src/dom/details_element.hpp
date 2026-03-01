#pragma once

#include "dom/dom_node.hpp"
#include <string>

namespace dong {
namespace dom {

/**
 * Details 元素状态管理
 *
 * 为 <details> 元素提供展开/折叠功能，包括：
 * - open 属性状态管理
 * - toggle 事件触发
 * - 子内容显示/隐藏控制
 * - disclosure triangle 指示器
 */
class DetailsElementState {
public:
    DetailsElementState();
    ~DetailsElementState() = default;

    // 状态查询
    bool isOpen() const { return is_open_; }
    bool hasSummary() const { return has_summary_; }

    // 状态变更
    void toggle();
    void open();
    void close();
    void setOpen(bool should_open);

    // DOM 同步
    void syncFromDOM(const DOMNodePtr& node);
    void applyOpenStateToDOM(const DOMNodePtr& details_node) const;

    // 事件处理（供上层使用；返回是否"消费"该事件）
    bool handleClick(const DOMNodePtr& target_node);

private:
    bool is_open_ = false;
    bool has_summary_ = false;
    std::string summary_text_;
};

/**
 * 辅助函数：从 DOMNode 获取或创建 DetailsElementState
 */
DetailsElementState* getDetailsState(DOMNodePtr node);

/**
 * 辅助函数：检查节点是否为 details 元素
 */
bool isDetailsElement(const DOMNodePtr& node);

/**
 * 辅助函数：检查节点是否为 summary 元素
 */
bool isSummaryElement(const DOMNodePtr& node);

/**
 * 辅助函数：移除 details 状态（节点销毁时调用）
 */
void removeDetailsState(DOMNodePtr node);

/**
 * DOM 被重载/替换时，需要清空全局 details 状态，避免悬空指针 key 被复用。
 */
void clearAllDetailsStates();

} // namespace dom
} // namespace dong