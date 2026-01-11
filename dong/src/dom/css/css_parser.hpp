#pragma once

#include "computed_style.hpp"
#include <string>
#include <vector>
#include <unordered_map>

namespace dong::dom {

// CSS rule representation with specificity
struct CSSRule {
    std::string selector;
    ComputedStyle style;
    int specificity = 0;
    int source_order = 0;
};

// @keyframes rule
struct KeyframesRule {
    std::string name;
    std::vector<CSSKeyframe> keyframes;
};

// @media rule
struct MediaRule {
    std::string query;
    std::vector<CSSRule> rules;
};

// @font-face rule
struct FontFaceRule {
    std::string family;
    std::string src;
    std::string style = "normal";
    std::string weight = "normal";
};

// CSS Parser - parses CSS text into rules
class CSSParser {
public:
    CSSParser() = default;
    
    // Parse CSS text and return rules
    std::vector<CSSRule> parse(const std::string& css);
    
    // Parse @keyframes
    std::vector<KeyframesRule> parseKeyframes(const std::string& css);
    
    // Parse @media rules
    std::vector<MediaRule> parseMediaRules(const std::string& css);
    
    // Parse @font-face rules
    std::vector<FontFaceRule> parseFontFaceRules(const std::string& css);
    
    // Parse inline style string
    static void parseInlineStyle(const std::string& style_str, ComputedStyle& style);
    
    // Apply a single property-value pair to style
    static void applyProperty(const std::string& property, const std::string& value, 
                              ComputedStyle& style);
    
    // Parse CSS value with unit (supports calc(), min(), max(), clamp())
    static CSSValue parseValue(const std::string& value);
    
    // Parse calc() expression
    static std::shared_ptr<CSSCalcExpression> parseCalc(const std::string& expr);
    
    // Parse min()/max()/clamp() expressions
    static std::shared_ptr<CSSCalcExpression> parseMinMaxClamp(const std::string& func, const std::string& args);
    
    // Parse color value
    static std::string parseColor(const std::string& value);
    
    // Parse gradient
    static CSSGradient parseGradient(const std::string& value);
    
    // Parse filter
    static std::vector<CSSFilter> parseFilter(const std::string& value);
    
    // Parse transition
    static std::vector<CSSTransition> parseTransition(const std::string& value);
    
    // Parse animation
    static CSSAnimation parseAnimation(const std::string& value);
    
    // Calculate specificity of a selector
    static int calculateSpecificity(const std::string& selector);
    
    // Parse helpers (public for dispatch table access)
    static float parseFloat(const std::string& s);
    static void parseBoxShadow(const std::string& value, ComputedStyle& style);
    static void parseTextShadow(const std::string& value, ComputedStyle& style);
    static void parseTransform(const std::string& value, ComputedStyle& style);

private:
    int rule_order_counter_ = 0;
    std::unordered_map<std::string, KeyframesRule> keyframes_map_;
    
    // Helper methods
    std::string removeComments(const std::string& css);
    std::string trimWhitespace(const std::string& str);
    std::vector<std::string> splitDeclarations(const std::string& css);
    
    // Parse individual property types (private - used internally or via helpers)
    static void parseMarginShorthand(const std::string& value, ComputedStyle& style);
    static void parsePaddingShorthand(const std::string& value, ComputedStyle& style);
    static void parseBorderShorthand(const std::string& value, ComputedStyle& style);
    static void parseBorderRadiusShorthand(const std::string& value, ComputedStyle& style);
    static void parseFlexShorthand(const std::string& value, ComputedStyle& style);
    static void parseBackgroundShorthand(const std::string& value, ComputedStyle& style);
    static void parseFontShorthand(const std::string& value, ComputedStyle& style);
    
    // calc() parsing helpers
    static std::shared_ptr<CSSCalcExpression> parseCalcExpression(const std::string& expr, size_t& pos);
    static std::shared_ptr<CSSCalcExpression> parseCalcTerm(const std::string& expr, size_t& pos);
    static std::shared_ptr<CSSCalcExpression> parseCalcFactor(const std::string& expr, size_t& pos);
};

} // namespace dong::dom
