#pragma once

#include "../dom/css/css_value.hpp"
#include "../dom/css/computed_style.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <chrono>

namespace dong::dom {
class DOMNode;
using DOMNodePtr = std::shared_ptr<DOMNode>;
}

namespace dong::animation {

// Easing functions
enum class EasingType {
    LINEAR,
    EASE,
    EASE_IN,
    EASE_OUT,
    EASE_IN_OUT,
    EASE_IN_QUAD,
    EASE_OUT_QUAD,
    EASE_IN_OUT_QUAD,
    EASE_IN_CUBIC,
    EASE_OUT_CUBIC,
    EASE_IN_OUT_CUBIC,
    CUBIC_BEZIER
};

// Cubic bezier parameters
struct CubicBezier {
    float x1 = 0.0f, y1 = 0.0f;
    float x2 = 1.0f, y2 = 1.0f;
};

// Parse timing function string to easing type
EasingType parseTimingFunction(const std::string& timing, CubicBezier* bezier = nullptr);

// Apply easing function
float applyEasing(float t, EasingType type, const CubicBezier* bezier = nullptr);

// Animation state for a single property
struct PropertyAnimation {
    std::string property;
    std::string from_value;
    std::string to_value;
    float current_value = 0.0f;  // For numeric properties
};

// Active animation instance
struct ActiveAnimation {
    uint64_t id = 0;
    std::weak_ptr<dong::dom::DOMNode> target;
    dong::dom::CSSAnimation definition;
    
    // Runtime state
    double start_time = 0.0;
    double current_time = 0.0;
    float progress = 0.0f;  // 0.0 to 1.0
    int current_iteration = 0;
    bool is_playing = true;
    bool is_finished = false;
    bool is_reversed = false;  // For alternate direction
    
    // Cached keyframe data
    std::vector<PropertyAnimation> properties;
};

// Active transition instance
struct ActiveTransition {
    uint64_t id = 0;
    std::weak_ptr<dong::dom::DOMNode> target;
    std::string property;
    
    // Values
    std::string from_value;
    std::string to_value;
    float from_numeric = 0.0f;
    float to_numeric = 0.0f;
    bool is_numeric = false;
    
    // Timing
    float duration = 0.0f;  // seconds
    float delay = 0.0f;     // seconds
    EasingType easing = EasingType::EASE;
    CubicBezier bezier;
    
    // Runtime state
    double start_time = 0.0;
    float progress = 0.0f;
    bool is_finished = false;
};

// Animation controller - manages all active animations and transitions
class AnimationController {
public:
    AnimationController();
    ~AnimationController();
    
    // Update all animations (called each frame)
    // delta_time in seconds
    void update(double current_time);
    
    // Start animation on a node
    uint64_t startAnimation(dong::dom::DOMNodePtr node, const dong::dom::CSSAnimation& animation);
    
    // Start transition on a node
    uint64_t startTransition(dong::dom::DOMNodePtr node, const std::string& property,
                             const std::string& from_value, const std::string& to_value,
                             const dong::dom::CSSTransition& transition);
    
    // Stop animation/transition
    void stopAnimation(uint64_t id);
    void stopTransition(uint64_t id);
    void stopAllAnimations(dong::dom::DOMNodePtr node);
    void stopAllTransitions(dong::dom::DOMNodePtr node);
    
    // Pause/resume
    void pauseAnimation(uint64_t id);
    void resumeAnimation(uint64_t id);
    
    // Query state
    bool hasActiveAnimations() const { return !active_animations_.empty() || !active_transitions_.empty(); }
    size_t getAnimationCount() const { return active_animations_.size(); }
    size_t getTransitionCount() const { return active_transitions_.size(); }
    
    // Callbacks
    using AnimationCallback = std::function<void(uint64_t id, const std::string& event)>;
    void setAnimationCallback(AnimationCallback callback) { animation_callback_ = callback; }
    
private:
    std::vector<ActiveAnimation> active_animations_;
    std::vector<ActiveTransition> active_transitions_;
    uint64_t next_id_ = 1;
    AnimationCallback animation_callback_;
    
    // Internal helpers
    void updateAnimation(ActiveAnimation& anim, double current_time);
    void updateTransition(ActiveTransition& trans, double current_time);
    void applyAnimationFrame(ActiveAnimation& anim);
    void applyTransitionFrame(ActiveTransition& trans);
    
    // Interpolation helpers
    float interpolateNumeric(float from, float to, float t);
    std::string interpolateColor(const std::string& from, const std::string& to, float t);
    
    // Parse property value to numeric
    bool parseNumericValue(const std::string& value, float& out);
    
    // Find keyframes for current progress
    std::pair<const dong::dom::CSSKeyframe*, const dong::dom::CSSKeyframe*> 
        findKeyframes(const ActiveAnimation& anim, float progress);
};

// Global animation controller instance
AnimationController& getAnimationController();

} // namespace dong::animation
