#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct dong_porf_module;

namespace dong::script {

class JSBindings;
class PorfforScriptRegistry;

class PorfforHost {
public:
    static constexpr size_t kMaxImportStringBytes = 4 * 1024 * 1024;

    PorfforHost();

    void setBindings(JSBindings* bindings);
    JSBindings* bindings() const;

    void setActiveMemory(char* memory, unsigned int* memory_pages);
    char* activeMemory() const { return memory_; }
    unsigned int* activeMemoryPages() const { return memory_pages_; }

    void processTimers(double now_ms);

    void printString(double str_ptr);
    void benchLog(double str_ptr);
    double timeNow() const;
    double domGetElementById(double id_ptr);
    void domSetTextContent(double node_id, double text_ptr);
    void domPrepareTextContent(double node_id);
    void domAddEventListener(double node_id, double type_ptr, double handler_ptr);
    double timerSetTimeout(double handler_ptr, double delay_ms);

    double strLen() const;
    double strRead(double dest_ptr, double max_len);
    double strByteAt(double index) const;

    void stage0(double v);
    void stage1(double v);
    void stage2(double v);
    void commitSetTextContent();
    void commitAddEventListener();
    double commitSetTimeout();

    void getValue(double node_id);
    void setValue(double node_id, double value_ptr);
    double getChecked(double node_id) const;
    void setChecked(double node_id, double checked);
    double getDisabled(double node_id) const;
    void setDisabled(double node_id, double disabled);
    void getAttribute(double node_id, double name_ptr);
    void setAttribute(double node_id, double name_ptr, double value_ptr);
    void removeAttribute(double node_id, double name_ptr);
    void setInnerHTML(double node_id, double html_ptr);

    double querySelector(double root_id, double selector_ptr) const;
    void querySelectorAll(double root_id, double selector_ptr);
    void getElementsByTagName(double root_id, double tag_ptr);

    void classAdd(double node_id, double cls_ptr);
    void classRemove(double node_id, double cls_ptr);
    double classToggle(double node_id, double cls_ptr) const;
    double classContains(double node_id, double cls_ptr) const;

    void styleSet(double node_id, double prop_ptr, double value_ptr);
    void styleGet(double node_id, double prop_ptr);
    void computedStyleGet(double node_id, double prop_ptr);

    void getRect(double node_id);
    double getMetric(double node_id, double metric_id) const;
    double getScrollTop(double node_id) const;
    void setScrollTop(double node_id, double value);
    double getScrollLeft(double node_id) const;
    void setScrollLeft(double node_id, double value);

    void focusNode(double node_id);
    void blurNode(double node_id);
    void clickNode(double node_id);
    double matchesSelector(double node_id, double selector_ptr) const;
    double closestSelector(double node_id, double selector_ptr) const;

    double createElement(double tag_ptr);
    double createTextNode(double text_ptr);
    void appendChild(double parent_id, double child_id);
    void insertBefore(double parent_id, double new_id, double ref_id);
    void removeNode(double node_id);
    void replaceChild(double parent_id, double new_id, double old_id);
    double getParentId(double node_id) const;
    double getFirstChildId(double node_id) const;
    double getNextSiblingId(double node_id) const;
    double cloneNodeId(double node_id, double deep) const;

private:
    JSBindings* bindings_ = nullptr;
    char* memory_ = nullptr;
    unsigned int* memory_pages_ = nullptr;

    double stage_0_ = 0;
    double stage_1_ = 0;
    double stage_2_ = 0;

    std::string result_slot_;
    std::vector<std::string> result_slot_stack_;

    size_t memoryCapacityBytes() const;
    std::string readByteString(double ptr) const;
    void setResultString(std::string s);
    void pushResultSlot();
    void popResultSlot();

    struct TimerTask {
        double fire_at_ms = 0;
        std::string module_name;
        std::string export_name;
    };
    std::vector<TimerTask> timers_;
    double start_ms_ = 0;
};

void PorfforHost_setGlobals(PorfforHost* host, PorfforScriptRegistry* registry);
void PorfforHost_setActiveModule(const struct dong_porf_module* mod);
const struct dong_porf_module* PorfforHost_activeModule();

} // namespace dong::script
