#include "css_value.hpp"
#include <algorithm>

namespace dong::dom {

void CSSEnvironment::set(const std::string& name, float value, const std::string& unit) {
    CSSValue env_value;

    if (unit == "px") {
        env_value = CSSValue(value, CSSValue::Unit::PIXEL);
    } else if (unit == "%") {
        env_value = CSSValue(value, CSSValue::Unit::PERCENT);
    } else if (unit == "em") {
        env_value = CSSValue(value, CSSValue::Unit::EM);
    } else if (unit == "rem") {
        env_value = CSSValue(value, CSSValue::Unit::REM);
    } else if (unit == "vw") {
        env_value = CSSValue(value, CSSValue::Unit::VW);
    } else if (unit == "vh") {
        env_value = CSSValue(value, CSSValue::Unit::VH);
    } else {
        // Default to pixels
        env_value = CSSValue(value, CSSValue::Unit::PIXEL);
    }

    env_variables_[name] = env_value;
}

CSSValue CSSEnvironment::get(const std::string& name, const CSSValue& fallback) const {
    auto it = env_variables_.find(name);
    if (it != env_variables_.end()) {
        return it->second;
    }
    return fallback;
}

bool CSSEnvironment::has(const std::string& name) const {
    return env_variables_.find(name) != env_variables_.end();
}

void CSSEnvironment::remove(const std::string& name) {
    env_variables_.erase(name);
}

void CSSEnvironment::clear() {
    env_variables_.clear();
}

void CSSEnvironment::setSafeAreaInsets(float top, float right, float bottom, float left) {
    set("safe-area-inset-top", top, "px");
    set("safe-area-inset-right", right, "px");
    set("safe-area-inset-bottom", bottom, "px");
    set("safe-area-inset-left", left, "px");
}

} // namespace dong::dom