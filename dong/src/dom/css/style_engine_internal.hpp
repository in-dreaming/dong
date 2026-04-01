#pragma once

#include "computed_style.hpp"
#include <memory>

namespace dong::dom {
class DOMNode;
using DOMNodePtr = std::shared_ptr<DOMNode>;
} // namespace dong::dom

namespace dong::dom::style_engine_internal {

void applyLogicalProperties(ComputedStyle& style);
void resolveTextAlignForDirection(ComputedStyle& style);
void applyRuleProperties(const ComputedStyle& rs, ComputedStyle& computed);
void applyImportantPropertiesOnly(const ComputedStyle& rs, ComputedStyle& computed);

void applyInlineStyleAttributeIfAny(DOMNodePtr node);
void applyPresentationalAttributesIfAny(DOMNodePtr node);
void applyDirAttributeIfAny(DOMNodePtr node);

} // namespace dong::dom::style_engine_internal
