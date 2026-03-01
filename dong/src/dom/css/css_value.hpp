#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <variant>

namespace dong::dom {

// Forward declaration
struct CSSCalcExpression;

// CSS style value - can be pixel, percent, auto, or calc expression
struct CSSValue {
    enum class Unit {
        UNSET,  // Property not explicitly set (default)
        PIXEL,
        PERCENT,
        AUTO,
        INHERIT,
        EM,
        REM,
        VW,
        VH,
        VMIN,
        VMAX,
        CH,
        CALC,  // Value is a calc() expression
        ENV,   // Value is an env() function call
        CONTENT  // Value is the 'content' keyword (e.g., for flex-basis)
    };

    float value = 0.0f;
    Unit unit = Unit::UNSET;
    std::shared_ptr<CSSCalcExpression> calc_expr;  // For calc() values
    std::string env_name;                          // For ENV values: environment variable name
    std::shared_ptr<CSSValue> env_fallback;         // For ENV values: fallback value if env not found

    CSSValue() = default;
    CSSValue(float v, Unit u) : value(v), unit(u) {}

    bool isUnset() const { return unit == Unit::UNSET; }
    bool isAuto() const { return unit == Unit::AUTO; }
    bool isPercent() const { return unit == Unit::PERCENT; }
    bool isPixel() const { return unit == Unit::PIXEL; }
    bool isCalc() const { return unit == Unit::CALC && calc_expr != nullptr; }
    bool isSet() const { return unit != Unit::UNSET; }
    bool isContent() const { return unit == Unit::CONTENT; }

    // Resolve to pixels given context
    float resolvePixels(float parent_size, float root_font_size, float viewport_width, float viewport_height) const;
};

// CSS calc() expression node
struct CSSCalcExpression {
    enum class Op {
        VALUE,      // Leaf node with a value
        ADD,        // +
        SUBTRACT,   // -
        MULTIPLY,   // *
        DIVIDE,     // /
        MIN,        // min()
        MAX,        // max()
        CLAMP       // clamp()
    };
    
    Op op = Op::VALUE;
    CSSValue value;  // For VALUE nodes
    std::vector<std::shared_ptr<CSSCalcExpression>> children;
    
    // Evaluate the expression given context
    float evaluate(float parent_size, float root_font_size, 
                   float viewport_width, float viewport_height) const;
};

// CSS custom properties (variables) storage
class CSSVariables {
public:
    void set(const std::string& name, const std::string& value);
    std::string get(const std::string& name, const std::string& fallback = "") const;
    bool has(const std::string& name) const;
    void remove(const std::string& name);
    void clear();

    // Resolve var() references in a value
    std::string resolveVarReferences(const std::string& value) const;

private:
    std::unordered_map<std::string, std::string> variables_;
};

// CSS environment variables storage (for env() function)
class CSSEnvironment {
public:
    // Set environment variable value
    void set(const std::string& name, float value, const std::string& unit = "px");

    // Get environment variable value as CSSValue
    CSSValue get(const std::string& name, const CSSValue& fallback) const;

    // Check if environment variable exists
    bool has(const std::string& name) const;

    // Remove environment variable
    void remove(const std::string& name);

    // Clear all environment variables
    void clear();

    // Set safe area inset values
    void setSafeAreaInsets(float top, float right, float bottom, float left);

private:
    std::unordered_map<std::string, CSSValue> env_variables_;
};

// box-shadow description
struct BoxShadow {
    float offset_x = 0.0f;
    float offset_y = 0.0f;
    float blur_radius = 0.0f;
    float spread_radius = 0.0f;
    std::string color;
    bool inset = false;
};

// CSS transition definition
struct CSSTransition {
    std::string property = "all";
    float duration = 0.0f;      // seconds
    float delay = 0.0f;         // seconds
    std::string timing_function = "ease";
};

// CSS animation keyframe
struct CSSKeyframe {
    float offset = 0.0f;  // 0.0 to 1.0 (0% to 100%)
    std::unordered_map<std::string, std::string> properties;
};

// CSS animation definition
struct CSSAnimation {
    std::string name;
    float duration = 0.0f;      // seconds
    float delay = 0.0f;         // seconds
    std::string timing_function = "ease";
    int iteration_count = 1;    // -1 for infinite
    std::string direction = "normal";  // normal, reverse, alternate, alternate-reverse
    std::string fill_mode = "none";    // none, forwards, backwards, both
    std::string play_state = "running"; // running, paused
    std::vector<CSSKeyframe> keyframes;
};

// Gradient stop
struct GradientStop {
    std::string color;
    float position = 0.0f;  // 0.0 to 1.0
};

// CSS gradient
struct CSSGradient {
    enum class Type {
        LINEAR,
        RADIAL,
        CONIC,
        REPEATING_LINEAR,
        REPEATING_RADIAL
    };
    
    Type type = Type::LINEAR;
    float angle = 180.0f;  // for linear gradient (degrees)
    std::string shape;     // for radial: circle, ellipse
    std::string size;      // for radial: closest-side, farthest-corner, etc.
    float center_x = 50.0f;  // percentage
    float center_y = 50.0f;  // percentage
    std::vector<GradientStop> stops;
};

// CSS filter function
struct CSSFilter {
    enum class Type {
        BLUR,
        BRIGHTNESS,
        CONTRAST,
        GRAYSCALE,
        HUE_ROTATE,
        INVERT,
        OPACITY,
        SATURATE,
        SEPIA,
        DROP_SHADOW
    };
    
    Type type;
    float value = 0.0f;
    // For drop-shadow
    float offset_x = 0.0f;
    float offset_y = 0.0f;
    float blur = 0.0f;
    std::string color;
};

} // namespace dong::dom
