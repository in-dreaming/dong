#include "animation_controller.hpp"
#include "../dom/dom/dom_node.hpp"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <regex>

namespace dong::animation {

// Global instance
static std::unique_ptr<AnimationController> g_animation_controller;

AnimationController& getAnimationController() {
    if (!g_animation_controller) {
        g_animation_controller = std::make_unique<AnimationController>();
    }
    return *g_animation_controller;
}

// Easing function implementations
EasingType parseTimingFunction(const std::string& timing, CubicBezier* bezier) {
    if (timing.empty() || timing == "linear") return EasingType::LINEAR;
    if (timing == "ease") return EasingType::EASE;
    if (timing == "ease-in") return EasingType::EASE_IN;
    if (timing == "ease-out") return EasingType::EASE_OUT;
    if (timing == "ease-in-out") return EasingType::EASE_IN_OUT;
    
    // Parse cubic-bezier(x1, y1, x2, y2)
    if (timing.find("cubic-bezier") == 0 && bezier) {
        size_t start = timing.find('(');
        size_t end = timing.find(')');
        if (start != std::string::npos && end != std::string::npos) {
            std::string params = timing.substr(start + 1, end - start - 1);
            std::stringstream ss(params);
            char comma;
            ss >> bezier->x1 >> comma >> bezier->y1 >> comma >> bezier->x2 >> comma >> bezier->y2;
            return EasingType::CUBIC_BEZIER;
        }
    }
    
    return EasingType::EASE;
}

// Cubic bezier helper
static float cubicBezier(float t, float p1, float p2) {
    // Approximate cubic bezier using De Casteljau's algorithm
    float t2 = t * t;
    float t3 = t2 * t;
    float mt = 1.0f - t;
    float mt2 = mt * mt;
    float mt3 = mt2 * mt;
    
    return 3.0f * mt2 * t * p1 + 3.0f * mt * t2 * p2 + t3;
}

static float solveCubicBezierX(float x, float x1, float x2, float epsilon = 0.0001f) {
    // Newton-Raphson iteration to find t for given x
    float t = x;
    for (int i = 0; i < 8; ++i) {
        float currentX = cubicBezier(t, x1, x2);
        float diff = currentX - x;
        if (std::abs(diff) < epsilon) break;
        
        // Derivative
        float derivative = 3.0f * (1.0f - t) * (1.0f - t) * x1 + 
                          6.0f * (1.0f - t) * t * (x2 - x1) + 
                          3.0f * t * t * (1.0f - x2);
        if (std::abs(derivative) < epsilon) break;
        t -= diff / derivative;
        t = std::clamp(t, 0.0f, 1.0f);
    }
    return t;
}

float applyEasing(float t, EasingType type, const CubicBezier* bezier) {
    t = std::clamp(t, 0.0f, 1.0f);
    
    switch (type) {
        case EasingType::LINEAR:
            return t;
            
        case EasingType::EASE:
            // cubic-bezier(0.25, 0.1, 0.25, 1.0)
            {
                float solvedT = solveCubicBezierX(t, 0.25f, 0.25f);
                return cubicBezier(solvedT, 0.1f, 1.0f);
            }
            
        case EasingType::EASE_IN:
            // cubic-bezier(0.42, 0, 1.0, 1.0)
            {
                float solvedT = solveCubicBezierX(t, 0.42f, 1.0f);
                return cubicBezier(solvedT, 0.0f, 1.0f);
            }
            
        case EasingType::EASE_OUT:
            // cubic-bezier(0, 0, 0.58, 1.0)
            {
                float solvedT = solveCubicBezierX(t, 0.0f, 0.58f);
                return cubicBezier(solvedT, 0.0f, 1.0f);
            }
            
        case EasingType::EASE_IN_OUT:
            // cubic-bezier(0.42, 0, 0.58, 1.0)
            {
                float solvedT = solveCubicBezierX(t, 0.42f, 0.58f);
                return cubicBezier(solvedT, 0.0f, 1.0f);
            }
            
        case EasingType::EASE_IN_QUAD:
            return t * t;
            
        case EasingType::EASE_OUT_QUAD:
            return t * (2.0f - t);
            
        case EasingType::EASE_IN_OUT_QUAD:
            return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
            
        case EasingType::EASE_IN_CUBIC:
            return t * t * t;
            
        case EasingType::EASE_OUT_CUBIC:
            {
                float f = t - 1.0f;
                return f * f * f + 1.0f;
            }
            
        case EasingType::EASE_IN_OUT_CUBIC:
            return t < 0.5f ? 4.0f * t * t * t : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f;
            
        case EasingType::CUBIC_BEZIER:
            if (bezier) {
                float solvedT = solveCubicBezierX(t, bezier->x1, bezier->x2);
                return cubicBezier(solvedT, bezier->y1, bezier->y2);
            }
            return t;
            
        default:
            return t;
    }
}

AnimationController::AnimationController() = default;
AnimationController::~AnimationController() = default;

void AnimationController::update(double current_time) {
    // Update animations
    for (auto& anim : active_animations_) {
        if (!anim.is_finished && anim.is_playing) {
            updateAnimation(anim, current_time);
        }
    }
    
    // Update transitions
    for (auto& trans : active_transitions_) {
        if (!trans.is_finished) {
            updateTransition(trans, current_time);
        }
    }
    
    // Remove finished animations
    active_animations_.erase(
        std::remove_if(active_animations_.begin(), active_animations_.end(),
                       [](const ActiveAnimation& a) { return a.is_finished; }),
        active_animations_.end());
    
    // Remove finished transitions
    active_transitions_.erase(
        std::remove_if(active_transitions_.begin(), active_transitions_.end(),
                       [](const ActiveTransition& t) { return t.is_finished; }),
        active_transitions_.end());
}

uint64_t AnimationController::startAnimation(dong::dom::DOMNodePtr node, 
                                              const dong::dom::CSSAnimation& animation) {
    if (!node || animation.name.empty() || animation.duration <= 0.0f) {
        return 0;
    }
    
    ActiveAnimation anim;
    anim.id = next_id_++;
    anim.target = node;
    anim.definition = animation;
    anim.start_time = -1.0;  // Will be set on first update
    anim.is_playing = (animation.play_state == "running");
    
    active_animations_.push_back(std::move(anim));
    
    if (animation_callback_) {
        animation_callback_(anim.id, "animationstart");
    }
    
    return anim.id;
}

uint64_t AnimationController::startTransition(dong::dom::DOMNodePtr node,
                                               const std::string& property,
                                               const std::string& from_value,
                                               const std::string& to_value,
                                               const dong::dom::CSSTransition& transition) {
    if (!node || property.empty() || transition.duration <= 0.0f) {
        return 0;
    }
    
    // Check if there's already a transition for this property
    for (auto& existing : active_transitions_) {
        auto target = existing.target.lock();
        if (target.get() == node.get() && existing.property == property) {
            // Update existing transition
            existing.from_value = from_value;
            existing.to_value = to_value;
            existing.start_time = -1.0;
            existing.is_finished = false;
            return existing.id;
        }
    }
    
    ActiveTransition trans;
    trans.id = next_id_++;
    trans.target = node;
    trans.property = property;
    trans.from_value = from_value;
    trans.to_value = to_value;
    trans.duration = transition.duration;
    trans.delay = transition.delay;
    trans.easing = parseTimingFunction(transition.timing_function, &trans.bezier);
    trans.start_time = -1.0;
    
    // Try to parse as numeric
    trans.is_numeric = parseNumericValue(from_value, trans.from_numeric) &&
                       parseNumericValue(to_value, trans.to_numeric);
    
    active_transitions_.push_back(std::move(trans));
    
    return trans.id;
}

void AnimationController::stopAnimation(uint64_t id) {
    for (auto& anim : active_animations_) {
        if (anim.id == id) {
            anim.is_finished = true;
            if (animation_callback_) {
                animation_callback_(id, "animationcancel");
            }
            break;
        }
    }
}

void AnimationController::stopTransition(uint64_t id) {
    for (auto& trans : active_transitions_) {
        if (trans.id == id) {
            trans.is_finished = true;
            break;
        }
    }
}

void AnimationController::stopAllAnimations(dong::dom::DOMNodePtr node) {
    for (auto& anim : active_animations_) {
        auto target = anim.target.lock();
        if (target.get() == node.get()) {
            anim.is_finished = true;
        }
    }
}

void AnimationController::stopAllTransitions(dong::dom::DOMNodePtr node) {
    for (auto& trans : active_transitions_) {
        auto target = trans.target.lock();
        if (target.get() == node.get()) {
            trans.is_finished = true;
        }
    }
}

void AnimationController::pauseAnimation(uint64_t id) {
    for (auto& anim : active_animations_) {
        if (anim.id == id) {
            anim.is_playing = false;
            break;
        }
    }
}

void AnimationController::resumeAnimation(uint64_t id) {
    for (auto& anim : active_animations_) {
        if (anim.id == id) {
            anim.is_playing = true;
            break;
        }
    }
}

void AnimationController::updateAnimation(ActiveAnimation& anim, double current_time) {
    auto node = anim.target.lock();
    if (!node) {
        anim.is_finished = true;
        return;
    }
    
    // Initialize start time on first update
    if (anim.start_time < 0.0) {
        anim.start_time = current_time + anim.definition.delay;
    }
    
    // Check if still in delay
    if (current_time < anim.start_time) {
        return;
    }
    
    anim.current_time = current_time;
    
    // Calculate progress within current iteration
    double elapsed = current_time - anim.start_time;
    double iteration_duration = anim.definition.duration;
    
    if (iteration_duration <= 0.0) {
        anim.is_finished = true;
        return;
    }
    
    // Calculate iteration and progress
    int iteration = static_cast<int>(elapsed / iteration_duration);
    float raw_progress = static_cast<float>(std::fmod(elapsed, iteration_duration) / iteration_duration);
    
    // Check if animation should end
    if (anim.definition.iteration_count >= 0 && iteration >= anim.definition.iteration_count) {
        anim.is_finished = true;
        anim.progress = 1.0f;
        
        // Apply final state based on fill-mode
        if (anim.definition.fill_mode == "forwards" || anim.definition.fill_mode == "both") {
            applyAnimationFrame(anim);
        }
        
        if (animation_callback_) {
            animation_callback_(anim.id, "animationend");
        }
        return;
    }
    
    // Handle direction
    bool should_reverse = false;
    if (anim.definition.direction == "reverse") {
        should_reverse = true;
    } else if (anim.definition.direction == "alternate") {
        should_reverse = (iteration % 2 == 1);
    } else if (anim.definition.direction == "alternate-reverse") {
        should_reverse = (iteration % 2 == 0);
    }
    
    if (should_reverse) {
        raw_progress = 1.0f - raw_progress;
    }
    
    // Apply easing
    CubicBezier bezier;
    EasingType easing = parseTimingFunction(anim.definition.timing_function, &bezier);
    anim.progress = applyEasing(raw_progress, easing, &bezier);
    anim.current_iteration = iteration;
    anim.is_reversed = should_reverse;
    
    // Fire iteration event if iteration changed
    static std::unordered_map<uint64_t, int> last_iterations;
    if (last_iterations[anim.id] != iteration) {
        last_iterations[anim.id] = iteration;
        if (iteration > 0 && animation_callback_) {
            animation_callback_(anim.id, "animationiteration");
        }
    }
    
    // Apply animation frame
    applyAnimationFrame(anim);
}

void AnimationController::updateTransition(ActiveTransition& trans, double current_time) {
    auto node = trans.target.lock();
    if (!node) {
        trans.is_finished = true;
        return;
    }
    
    // Initialize start time on first update
    if (trans.start_time < 0.0) {
        trans.start_time = current_time + trans.delay;
    }
    
    // Check if still in delay
    if (current_time < trans.start_time) {
        return;
    }
    
    // Calculate progress
    double elapsed = current_time - trans.start_time;
    float raw_progress = static_cast<float>(elapsed / trans.duration);
    
    if (raw_progress >= 1.0f) {
        trans.progress = 1.0f;
        trans.is_finished = true;
    } else {
        trans.progress = applyEasing(raw_progress, trans.easing, &trans.bezier);
    }
    
    // Apply transition frame
    applyTransitionFrame(trans);
}

void AnimationController::applyAnimationFrame(ActiveAnimation& anim) {
    auto node = anim.target.lock();
    if (!node) return;
    
    const auto& keyframes = anim.definition.keyframes;
    if (keyframes.empty()) return;
    
    // Find surrounding keyframes
    auto [prev_kf, next_kf] = findKeyframes(anim, anim.progress);
    if (!prev_kf || !next_kf) return;
    
    // Calculate local progress between keyframes
    float local_progress = 0.0f;
    if (next_kf->offset > prev_kf->offset) {
        local_progress = (anim.progress - prev_kf->offset) / (next_kf->offset - prev_kf->offset);
    }
    
    // Interpolate each property
    for (const auto& [prop, to_value] : next_kf->properties) {
        auto it = prev_kf->properties.find(prop);
        std::string from_value = it != prev_kf->properties.end() ? it->second : to_value;
        
        // Interpolate and apply
        float from_num, to_num;
        if (parseNumericValue(from_value, from_num) && parseNumericValue(to_value, to_num)) {
            float interpolated = interpolateNumeric(from_num, to_num, local_progress);
            std::string unit;
            if (from_value.find("px") != std::string::npos) unit = "px";
            else if (from_value.find("%") != std::string::npos) unit = "%";
            else if (from_value.find("deg") != std::string::npos) unit = "deg";
            
            node->setInlineStyleProperty(prop, std::to_string(interpolated) + unit);
        } else if (from_value.find('#') == 0 || from_value.find("rgb") == 0) {
            // Color interpolation
            std::string interpolated = interpolateColor(from_value, to_value, local_progress);
            node->setInlineStyleProperty(prop, interpolated);
        } else {
            // Discrete - use target value at 50%
            node->setInlineStyleProperty(prop, local_progress >= 0.5f ? to_value : from_value);
        }
    }
    
    node->markLayoutDirty();
}

void AnimationController::applyTransitionFrame(ActiveTransition& trans) {
    auto node = trans.target.lock();
    if (!node) return;
    
    if (trans.is_numeric) {
        float interpolated = interpolateNumeric(trans.from_numeric, trans.to_numeric, trans.progress);
        std::string unit;
        if (trans.from_value.find("px") != std::string::npos) unit = "px";
        else if (trans.from_value.find("%") != std::string::npos) unit = "%";
        
        node->setInlineStyleProperty(trans.property, std::to_string(interpolated) + unit);
    } else if (trans.from_value.find('#') == 0 || trans.from_value.find("rgb") == 0) {
        std::string interpolated = interpolateColor(trans.from_value, trans.to_value, trans.progress);
        node->setInlineStyleProperty(trans.property, interpolated);
    } else {
        // Discrete
        node->setInlineStyleProperty(trans.property, trans.progress >= 0.5f ? trans.to_value : trans.from_value);
    }
    
    node->markLayoutDirty();
}

float AnimationController::interpolateNumeric(float from, float to, float t) {
    return from + (to - from) * t;
}

std::string AnimationController::interpolateColor(const std::string& from, const std::string& to, float t) {
    // Parse hex colors
    auto parseHex = [](const std::string& color, int& r, int& g, int& b, int& a) {
        a = 255;
        if (color.empty() || color[0] != '#') {
            r = g = b = 0;
            return;
        }
        
        if (color.size() == 4) {  // #rgb
            r = std::stoi(color.substr(1, 1), nullptr, 16) * 17;
            g = std::stoi(color.substr(2, 1), nullptr, 16) * 17;
            b = std::stoi(color.substr(3, 1), nullptr, 16) * 17;
        } else if (color.size() == 7) {  // #rrggbb
            r = std::stoi(color.substr(1, 2), nullptr, 16);
            g = std::stoi(color.substr(3, 2), nullptr, 16);
            b = std::stoi(color.substr(5, 2), nullptr, 16);
        } else if (color.size() == 9) {  // #rrggbbaa
            r = std::stoi(color.substr(1, 2), nullptr, 16);
            g = std::stoi(color.substr(3, 2), nullptr, 16);
            b = std::stoi(color.substr(5, 2), nullptr, 16);
            a = std::stoi(color.substr(7, 2), nullptr, 16);
        } else {
            r = g = b = 0;
        }
    };
    
    int r1, g1, b1, a1;
    int r2, g2, b2, a2;
    parseHex(from, r1, g1, b1, a1);
    parseHex(to, r2, g2, b2, a2);
    
    int r = static_cast<int>(r1 + (r2 - r1) * t);
    int g = static_cast<int>(g1 + (g2 - g1) * t);
    int b = static_cast<int>(b1 + (b2 - b1) * t);
    int a = static_cast<int>(a1 + (a2 - a1) * t);
    
    char buf[10];
    if (a == 255) {
        snprintf(buf, sizeof(buf), "#%02x%02x%02x", r, g, b);
    } else {
        snprintf(buf, sizeof(buf), "#%02x%02x%02x%02x", r, g, b, a);
    }
    return buf;
}

bool AnimationController::parseNumericValue(const std::string& value, float& out) {
    if (value.empty()) return false;
    
    try {
        size_t pos;
        out = std::stof(value, &pos);
        return true;
    } catch (...) {
        return false;
    }
}

std::pair<const dong::dom::CSSKeyframe*, const dong::dom::CSSKeyframe*>
AnimationController::findKeyframes(const ActiveAnimation& anim, float progress) {
    const auto& keyframes = anim.definition.keyframes;
    if (keyframes.empty()) return {nullptr, nullptr};
    
    // Find surrounding keyframes
    const dong::dom::CSSKeyframe* prev = &keyframes.front();
    const dong::dom::CSSKeyframe* next = &keyframes.back();
    
    for (size_t i = 0; i < keyframes.size(); ++i) {
        if (keyframes[i].offset <= progress) {
            prev = &keyframes[i];
        }
        if (keyframes[i].offset >= progress && next->offset > keyframes[i].offset) {
            next = &keyframes[i];
            break;
        }
    }
    
    // If progress is past all keyframes, use last one for both
    if (progress >= keyframes.back().offset) {
        prev = next = &keyframes.back();
    }
    
    return {prev, next};
}

} // namespace dong::animation
