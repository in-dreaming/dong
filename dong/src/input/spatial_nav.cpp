#include "spatial_nav.hpp"

#include <cmath>
#include <limits>

namespace dong::input {

namespace {

struct Rect {
    float x, y, width, height;

    float centerX() const { return x + width * 0.5f; }
    float centerY() const { return y + height * 0.5f; }
    float left() const { return x; }
    float right() const { return x + width; }
    float top() const { return y; }
    float bottom() const { return y + height; }
};

// Check if the candidate is in the direction's half-plane relative to current.
bool isInDirection(const Rect& current, const Rect& candidate, NavDirection dir) {
    switch (dir) {
        case NavDirection::Up:
            return candidate.centerY() < current.top();
        case NavDirection::Down:
            return candidate.centerY() > current.bottom();
        case NavDirection::Left:
            return candidate.centerX() < current.left();
        case NavDirection::Right:
            return candidate.centerX() > current.right();
    }
    return false;
}

// Score a candidate for spatial navigation.
// Lower score = better candidate.
// score = alpha * orthogonal_displacement + beta * directional_distance
// alpha=2 (penalize off-axis candidates more), beta=1
float scoreCandidate(const Rect& current, const Rect& candidate, NavDirection dir) {
    constexpr float alpha = 2.0f;
    constexpr float beta = 1.0f;

    float orthogonal = 0.0f;
    float directional = 0.0f;

    switch (dir) {
        case NavDirection::Up:
            directional = current.top() - candidate.centerY();
            orthogonal = std::abs(candidate.centerX() - current.centerX());
            break;
        case NavDirection::Down:
            directional = candidate.centerY() - current.bottom();
            orthogonal = std::abs(candidate.centerX() - current.centerX());
            break;
        case NavDirection::Left:
            directional = current.left() - candidate.centerX();
            orthogonal = std::abs(candidate.centerY() - current.centerY());
            break;
        case NavDirection::Right:
            directional = candidate.centerX() - current.right();
            orthogonal = std::abs(candidate.centerY() - current.centerY());
            break;
    }

    return alpha * orthogonal + beta * directional;
}

} // anonymous namespace

dom::DOMNodePtr findSpatialNavTarget(
    const dom::DOMNodePtr& current,
    NavDirection dir,
    const std::vector<dom::DOMNodePtr>& candidates,
    layout::Engine* layout_engine) {

    if (!current || !layout_engine || candidates.empty()) {
        return nullptr;
    }

    // Check for explicit nav-* CSS override (P0-4 S3)
    {
        const auto& style = current->getComputedStyle();
        std::string explicit_target;
        switch (dir) {
            case NavDirection::Up: explicit_target = style.nav_up; break;
            case NavDirection::Down: explicit_target = style.nav_down; break;
            case NavDirection::Left: explicit_target = style.nav_left; break;
            case NavDirection::Right: explicit_target = style.nav_right; break;
        }
        if (!explicit_target.empty() && explicit_target != "auto") {
            if (explicit_target == "none") {
                return nullptr;  // Navigation disabled in this direction
            }
            // Target is a selector (e.g., "#my-button") - find it in candidates
            for (const auto& candidate : candidates) {
                if (!candidate) continue;
                // Check if candidate matches the selector (simple #id check)
                if (explicit_target[0] == '#' && candidate->hasAttribute("id")) {
                    if (candidate->getAttribute("id") == explicit_target.substr(1)) {
                        return candidate;
                    }
                }
            }
        }
    }

    // Get current element's layout rect
    const auto* current_layout = layout_engine->getLayout(current);
    if (!current_layout) {
        return nullptr;
    }

    Rect current_rect{
        current_layout->x,
        current_layout->y,
        current_layout->width,
        current_layout->height
    };

    dom::DOMNodePtr best_candidate = nullptr;
    float best_score = std::numeric_limits<float>::max();

    for (const auto& candidate : candidates) {
        if (!candidate || candidate.get() == current.get()) {
            continue;
        }

        const auto* candidate_layout = layout_engine->getLayout(candidate);
        if (!candidate_layout) {
            continue;
        }

        // Skip zero-size elements (hidden or not laid out)
        if (candidate_layout->width <= 0.0f || candidate_layout->height <= 0.0f) {
            continue;
        }

        Rect candidate_rect{
            candidate_layout->x,
            candidate_layout->y,
            candidate_layout->width,
            candidate_layout->height
        };

        // Filter: only consider candidates in the direction's half-plane
        if (!isInDirection(current_rect, candidate_rect, dir)) {
            continue;
        }

        float score = scoreCandidate(current_rect, candidate_rect, dir);
        if (score < best_score) {
            best_score = score;
            best_candidate = candidate;
        }
    }

    return best_candidate;
}

} // namespace dong::input
