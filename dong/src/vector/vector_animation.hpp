#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <cmath>

namespace dong::render {
class DisplayListBuilder;
}

namespace dong::vector {

// Abstract path command for DongVectorBridge
enum class PathVerb : uint8_t {
    MoveTo,
    LineTo,
    QuadTo,
    CubicTo,
    Close,
};

struct PathPoint {
    float x, y;
};

struct PathCommand {
    PathVerb verb;
    PathPoint points[3];  // max 3 control points (cubic)
};

// Paint style for a vector path
struct VectorPaint {
    enum class Type : uint8_t { Solid, LinearGradient, RadialGradient };
    Type type = Type::Solid;
    float color[4] = {1, 1, 1, 1};  // RGBA for solid
    float opacity = 1.0f;
    bool is_stroke = false;
    float stroke_width = 1.0f;
};

// A single vector shape (path + paint)
struct VectorShape {
    std::vector<PathCommand> commands;
    VectorPaint fill;
    VectorPaint stroke;
    bool has_fill = true;
    bool has_stroke = false;
};

// A single frame of vector animation data
struct VectorFrame {
    std::vector<VectorShape> shapes;
    float width = 0;
    float height = 0;
};

// Animation state
enum class PlayState : uint8_t { Stopped, Playing, Paused };

// Abstract vector animation instance
class VectorAnimation {
public:
    virtual ~VectorAnimation() = default;

    virtual double duration() const = 0;
    virtual float sourceWidth() const = 0;
    virtual float sourceHeight() const = 0;

    virtual void tick(float dt) = 0;
    virtual const VectorFrame& currentFrame() const = 0;

    // Playback control
    void play() { state_ = PlayState::Playing; }
    void pause() { state_ = PlayState::Paused; }
    void stop() { state_ = PlayState::Stopped; current_time_ = 0.0; }
    void seek(double t) { current_time_ = t; }
    void setLoop(int count) { loop_count_ = count; }
    void setSpeed(float s) { speed_ = s; }

    PlayState state() const { return state_; }
    double currentTime() const { return current_time_; }

protected:
    PlayState state_ = PlayState::Stopped;
    double current_time_ = 0.0;
    float speed_ = 1.0f;
    int loop_count_ = 0;  // 0 = once, -1 = forever, N = N times
    int loops_done_ = 0;
};

// Stub Lottie animation (placeholder until rlottie is integrated)
class LottieAnimation : public VectorAnimation {
public:
    bool load(const void* json_data, size_t len);
    bool loadFile(const std::string& path);

    double duration() const override { return duration_; }
    float sourceWidth() const override { return width_; }
    float sourceHeight() const override { return height_; }

    void tick(float dt) override;
    const VectorFrame& currentFrame() const override { return frame_; }

private:
    double duration_ = 1.0;
    float width_ = 100.0f;
    float height_ = 100.0f;
    float frame_rate_ = 30.0f;
    VectorFrame frame_;
};

// Stub Rive animation (placeholder until rive-cpp is integrated)
class RiveAnimation : public VectorAnimation {
public:
    bool load(const void* data, size_t len);
    bool loadFile(const std::string& path);

    double duration() const override { return duration_; }
    float sourceWidth() const override { return width_; }
    float sourceHeight() const override { return height_; }

    void tick(float dt) override;
    const VectorFrame& currentFrame() const override { return frame_; }

    // State machine
    int playStateMachine(const char* name);
    int setInputBool(const char* name, bool value);
    int setInputNumber(const char* name, double value);
    int fireTrigger(const char* name);

private:
    double duration_ = 1.0;
    float width_ = 100.0f;
    float height_ = 100.0f;
    VectorFrame frame_;
    std::string active_state_machine_;
};

// DongVectorBridge: converts VectorFrame paths into display list items
// V1: Renders as colored rects (placeholder).
// V2: Will use Slug GPU pipeline for true bezier rendering.
void renderVectorFrame(const VectorFrame& frame,
                       float dest_x, float dest_y, float dest_w, float dest_h,
                       render::DisplayListBuilder& builder);

} // namespace dong::vector
