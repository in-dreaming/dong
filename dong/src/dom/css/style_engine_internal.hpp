#pragma once

#include "computed_style.hpp"

namespace dong::dom::style_engine_internal {

void applyLogicalProperties(ComputedStyle& style);
void resolveTextAlignForDirection(ComputedStyle& style);
void applyRuleProperties(const ComputedStyle& rs, ComputedStyle& computed);
void applyImportantPropertiesOnly(const ComputedStyle& rs, ComputedStyle& computed);

} // namespace dong::dom::style_engine_internal
