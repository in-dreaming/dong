#include "vector_animation.hpp"
#include "../render/display_list.hpp"
#include "../core/log.h"
#include <fstream>
#include <cstring>
#include <algorithm>

namespace dong::vector {

// ============================================================================
// LottieAnimation stub
// ============================================================================

bool LottieAnimation::load(const void* json_data, size_t len) {
    if (!json_data || len == 0) return false;
    // TODO: Integrate rlottie to parse JSON and extract animation data.
    // For now, create a minimal placeholder frame.
    DONG_LOG_INFO("[Vector] Lottie loaded (%zu bytes) - stub mode", len);

    // Try to extract basic metadata from JSON (w, h, fr, op)
    std::string json(static_cast<const char*>(json_data), len);
    // Minimal JSON parsing for "w":N, "h":N, "fr":N
    auto extractNumber = [&](const char* key) -> float {
        std::string search = std::string("\"") + key + "\"";
        auto pos = json.find(search);
        if (pos == std::string::npos) return 0.0f;
        pos = json.find(':', pos);
        if (pos == std::string::npos) return 0.0f;
        pos++;
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
        return std::stof(json.substr(pos, 20));
    };

    width_ = extractNumber("w");
    height_ = extractNumber("h");
    frame_rate_ = extractNumber("fr");
    float op = extractNumber("op");
    float ip = extractNumber("ip");
    if (width_ <= 0) width_ = 100.0f;
    if (height_ <= 0) height_ = 100.0f;
    if (frame_rate_ <= 0) frame_rate_ = 30.0f;
    if (op > ip) duration_ = (op - ip) / frame_rate_;
    else duration_ = 1.0;

    // Create a placeholder frame with a colored rect
    frame_.width = width_;
    frame_.height = height_;
    VectorShape placeholder;
    placeholder.has_fill = true;
    placeholder.fill.color[0] = 0.4f;
    placeholder.fill.color[1] = 0.6f;
    placeholder.fill.color[2] = 1.0f;
    placeholder.fill.color[3] = 0.8f;
    PathCommand move{PathVerb::MoveTo, {{0, 0}}};
    PathCommand line1{PathVerb::LineTo, {{width_, 0}}};
    PathCommand line2{PathVerb::LineTo, {{width_, height_}}};
    PathCommand line3{PathVerb::LineTo, {{0, height_}}};
    PathCommand close{PathVerb::Close, {}};
    placeholder.commands = {move, line1, line2, line3, close};
    frame_.shapes.push_back(std::move(placeholder));

    return true;
}

bool LottieAnimation::loadFile(const std::string& path) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f.is_open()) return false;
    size_t sz = static_cast<size_t>(f.tellg());
    f.seekg(0);
    std::vector<char> buf(sz);
    f.read(buf.data(), sz);
    return load(buf.data(), sz);
}

void LottieAnimation::tick(float dt) {
    if (state_ != PlayState::Playing) return;
    current_time_ += dt * speed_;
    if (current_time_ >= duration_) {
        if (loop_count_ == -1 || (loop_count_ > 0 && loops_done_ < loop_count_)) {
            current_time_ = std::fmod(current_time_, duration_);
            loops_done_++;
        } else {
            current_time_ = duration_;
            state_ = PlayState::Stopped;
        }
    }
}

// ============================================================================
// RiveAnimation stub
// ============================================================================

bool RiveAnimation::load(const void* data, size_t len) {
    if (!data || len == 0) return false;
    DONG_LOG_INFO("[Vector] Rive loaded (%zu bytes) - stub mode", len);
    // Rive is binary format; actual parsing requires rive-cpp
    width_ = 400.0f;
    height_ = 400.0f;
    duration_ = 2.0;

    frame_.width = width_;
    frame_.height = height_;
    VectorShape placeholder;
    placeholder.has_fill = true;
    placeholder.fill.color[0] = 0.9f;
    placeholder.fill.color[1] = 0.3f;
    placeholder.fill.color[2] = 0.6f;
    placeholder.fill.color[3] = 0.8f;
    PathCommand move{PathVerb::MoveTo, {{0, 0}}};
    PathCommand line1{PathVerb::LineTo, {{width_, 0}}};
    PathCommand line2{PathVerb::LineTo, {{width_, height_}}};
    PathCommand line3{PathVerb::LineTo, {{0, height_}}};
    PathCommand close{PathVerb::Close, {}};
    placeholder.commands = {move, line1, line2, line3, close};
    frame_.shapes.push_back(std::move(placeholder));

    return true;
}

bool RiveAnimation::loadFile(const std::string& path) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f.is_open()) return false;
    size_t sz = static_cast<size_t>(f.tellg());
    f.seekg(0);
    std::vector<char> buf(sz);
    f.read(buf.data(), sz);
    return load(buf.data(), sz);
}

void RiveAnimation::tick(float dt) {
    if (state_ != PlayState::Playing) return;
    current_time_ += dt * speed_;
    if (current_time_ >= duration_) {
        current_time_ = std::fmod(current_time_, std::max(duration_, 0.001));
    }
}

int RiveAnimation::playStateMachine(const char* name) {
    if (!name) return -1;
    active_state_machine_ = name;
    DONG_LOG_INFO("[Vector] Rive state machine: %s", name);
    return 0;
}

int RiveAnimation::setInputBool(const char* name, bool value) {
    DONG_LOG_DEBUG("[Vector] Rive input bool: %s = %d", name ? name : "", value);
    return 0;
}

int RiveAnimation::setInputNumber(const char* name, double value) {
    DONG_LOG_DEBUG("[Vector] Rive input number: %s = %f", name ? name : "", value);
    return 0;
}

int RiveAnimation::fireTrigger(const char* name) {
    DONG_LOG_DEBUG("[Vector] Rive trigger: %s", name ? name : "");
    return 0;
}

// ============================================================================
// DongVectorBridge: render vector frame to display list
// ============================================================================

void renderVectorFrame(const VectorFrame& frame,
                       float dest_x, float dest_y, float dest_w, float dest_h,
                       render::DisplayListBuilder& builder) {
    if (frame.shapes.empty()) return;

    // Scale from source coordinates to destination rect
    float sx = (frame.width > 0) ? dest_w / frame.width : 1.0f;
    float sy = (frame.height > 0) ? dest_h / frame.height : 1.0f;

    // V1 fallback rendering: render each shape as a colored rect (bounding box).
    // V2 will use Slug pipeline for true bezier rendering.
    for (const auto& shape : frame.shapes) {
        if (!shape.has_fill) continue;

        // Compute bounding box of path
        float min_x = 1e9f, min_y = 1e9f, max_x = -1e9f, max_y = -1e9f;
        for (const auto& cmd : shape.commands) {
            int point_count = 0;
            switch (cmd.verb) {
                case PathVerb::MoveTo:
                case PathVerb::LineTo: point_count = 1; break;
                case PathVerb::QuadTo: point_count = 2; break;
                case PathVerb::CubicTo: point_count = 3; break;
                case PathVerb::Close: continue;
            }
            for (int i = 0; i < point_count; ++i) {
                min_x = std::min(min_x, cmd.points[i].x);
                min_y = std::min(min_y, cmd.points[i].y);
                max_x = std::max(max_x, cmd.points[i].x);
                max_y = std::max(max_y, cmd.points[i].y);
            }
        }

        if (min_x >= max_x || min_y >= max_y) continue;

        // Transform to destination coordinates
        render::Rect r;
        r.x = dest_x + min_x * sx;
        r.y = dest_y + min_y * sy;
        r.width = (max_x - min_x) * sx;
        r.height = (max_y - min_y) * sy;

        render::Color c;
        c.r = shape.fill.color[0];
        c.g = shape.fill.color[1];
        c.b = shape.fill.color[2];
        c.a = shape.fill.color[3] * shape.fill.opacity;

        builder.addRect(r, c);
    }
}

} // namespace dong::vector
