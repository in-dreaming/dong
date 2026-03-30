#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "../core/log.h"
#include "text_renderer_mode.hpp"


namespace dong::render {

struct Color {
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;
};

struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

// DisplayList 中支持的最小 UI 原语集合
enum class DisplayItemType : uint8_t {
    DrawRect,
    DrawRoundedRect,
    DrawShadow,      // box-shadow with blur
    DrawImage,
    DrawGlyphRun,
    DrawLinearGradient,  // CSS linear-gradient background
    PushLayer,
    PopLayer,
    PushClipRect,
    PushClipRoundedRect,
    PopClip,
};

struct DrawRectData {
    Rect rect;
    Color color;
};

struct DrawRoundedRectData {
    Rect rect;
    Color color;
    float radius = 0.0f;
    float stroke_width = 0.0f; // 0 = filled, >0 = stroke (border ring)
};


struct DrawShadowData {
    Rect rect;           // 阴影的目标矩形（已包含 offset 和 spread）
    Color color;
    float radius = 0.0f; // 圆角半径
    float blur = 0.0f;   // 模糊半径
};

enum class ImageFitMode : uint8_t {
    Fill,
    Contain,
    Cover,
};

enum class ImageSampling : uint8_t {
    Linear,
    Nearest,
};

struct DrawImageData {
    Rect rect;
    std::string src;   // 图片资源标识（路径或其它 key）
    float opacity = 1.0f;
    ImageFitMode fit = ImageFitMode::Fill;
    float position_x = 0.5f;  // object-position X (0.0=left, 0.5=center, 1.0=right)
    float position_y = 0.5f;  // object-position Y (0.0=top, 0.5=center, 1.0=bottom)
    ImageSampling sampling = ImageSampling::Linear;
};


struct GlyphInstance {
    uint32_t glyph_id = 0;
    float pen_x_units = 0.0f;      // design units（相对于文本起点）
    float pen_y_units = 0.0f;      // design units
    uint16_t font_path_index = 0;  // 指向 DrawGlyphRunData::font_paths 的索引
    uint32_t units_per_em = 0;     // 该字体的 units_per_em
};

struct DrawGlyphRunData {
    Rect rect;
    Color color;
    float font_size = 16.0f;
    std::string font_family;
    std::string font_weight;
    std::string font_style; // CSS font-style: normal/italic/oblique

    // 字体路径表：索引 0 为主字体；后续为回退字体。
    std::vector<std::string> font_paths;

    // 为了兼容旧代码：主字体路径（等价于 font_paths[0]）。
    std::string font_path;

    float baseline_x = 0.0f;
    float baseline_y = 0.0f;
    std::vector<GlyphInstance> glyphs;
    
    // 新增：design units 元数据
    uint32_t units_per_em = 0;
    float scale_to_pixels = 1.0f;  // font_size / units_per_em


    // text-shadow 支持
    float text_shadow_offset_x = 0.0f;
    float text_shadow_offset_y = 0.0f;
    float text_shadow_blur = 0.0f;
    Color text_shadow_color;
    bool has_text_shadow = false;

    // Text renderer backend preference
    TextRendererMode text_renderer_mode = TextRendererMode::Auto;
};

struct ClipData {
    Rect rect;
    float radius = 0.0f;
    bool is_rounded = false;
};

// Linear gradient stop (resolved to Color + position)
struct GradientColorStop {
    Color color;
    float position = 0.0f;  // 0.0 to 1.0
};

// Max stops passed via uniforms to GPU shader
static constexpr int kMaxGradientStops = 8;

struct DrawLinearGradientData {
    Rect rect;
    float angle_deg = 180.0f;           // CSS angle in degrees
    float radius = 0.0f;                // border-radius for clipping
    int stop_count = 0;
    GradientColorStop stops[kMaxGradientStops];
};

struct LayerData {
    Rect bounds;
    float opacity = 1.0f;
    bool isolate = false;
    uint64_t id = 0;      // 跨帧标识图层的稳定 ID（例如 DOM 节点指针）
    bool is_dirty = true; // 本帧是否需要重新栅格（否则可复用缓存纹理）
};

struct DisplayItem {
    DisplayItemType type;

    // 为简化实现，这里不用 union，而是根据 type 只访问对应字段
    DrawRectData rect;
    DrawRoundedRectData rounded_rect;
    DrawShadowData shadow;
    DrawImageData image;
    DrawGlyphRunData glyph_run;
    DrawLinearGradientData gradient;
    ClipData clip;
    LayerData layer;
};

struct DisplayList {
    std::vector<DisplayItem> items;
};

// 一个轻量的 builder，方便从 DOM/布局阶段生成 DisplayList
class DisplayListBuilder {
public:
    class ScopedLayer {
    public:
        ScopedLayer() = default;
        ScopedLayer(const ScopedLayer&) = delete;
        ScopedLayer& operator=(const ScopedLayer&) = delete;
        ScopedLayer(ScopedLayer&& other) noexcept {
            builder_ = other.builder_;
            active_ = other.active_;
            other.builder_ = nullptr;
            other.active_ = false;
        }
        ScopedLayer& operator=(ScopedLayer&& other) noexcept {
            if (this != &other) {
                release();
                builder_ = other.builder_;
                active_ = other.active_;
                other.builder_ = nullptr;
                other.active_ = false;
            }
            return *this;
        }
        ~ScopedLayer() { release(); }

    private:
        friend class DisplayListBuilder;
        ScopedLayer(DisplayListBuilder* builder, bool active)
            : builder_(builder), active_(active) {}

        void release() {
            if (active_ && builder_) {
                builder_->popLayer();
            }
            builder_ = nullptr;
            active_ = false;
        }

        DisplayListBuilder* builder_ = nullptr;
        bool active_ = false;
    };

    class ScopedClip {
    public:
        ScopedClip() = default;
        ScopedClip(const ScopedClip&) = delete;
        ScopedClip& operator=(const ScopedClip&) = delete;
        ScopedClip(ScopedClip&& other) noexcept {
            builder_ = other.builder_;
            active_ = other.active_;
            other.builder_ = nullptr;
            other.active_ = false;
        }
        ScopedClip& operator=(ScopedClip&& other) noexcept {
            if (this != &other) {
                release();
                builder_ = other.builder_;
                active_ = other.active_;
                other.builder_ = nullptr;
                other.active_ = false;
            }
            return *this;
        }
        ~ScopedClip() { release(); }

    private:
        friend class DisplayListBuilder;
        ScopedClip(DisplayListBuilder* builder, bool active)
            : builder_(builder), active_(active) {}

        void release() {
            if (active_ && builder_) {
                builder_->popClip();
            }
            builder_ = nullptr;
            active_ = false;
        }

        DisplayListBuilder* builder_ = nullptr;
        bool active_ = false;
    };

    void clear() {
        list_.items.clear();
        layer_depth_ = 0;
        clip_depth_ = 0;
        translate_x_ = 0.0f;
        translate_y_ = 0.0f;
    }

    // 滚动偏移管理
    void pushTranslate(float dx, float dy) {
        translate_stack_.push_back({translate_x_, translate_y_});
        translate_x_ += dx;
        translate_y_ += dy;
    }

    void popTranslate() {
        if (!translate_stack_.empty()) {
            auto prev = translate_stack_.back();
            translate_stack_.pop_back();
            translate_x_ = prev.first;
            translate_y_ = prev.second;
        }
    }

    float getTranslateX() const { return translate_x_; }
    float getTranslateY() const { return translate_y_; }


    void addRect(const Rect& rect, const Color& color) {
        DisplayItem item{};
        item.type = DisplayItemType::DrawRect;
        item.rect.rect = applyTranslate(rect);
        item.rect.color = color;
        list_.items.push_back(std::move(item));
    }

    void addRoundedRect(const Rect& rect, const Color& color, float radius, float stroke_width = 0.0f) {
        DisplayItem item{};
        item.type = DisplayItemType::DrawRoundedRect;
        Rect translated = applyTranslate(rect);
        // if (translate_y_ != 0.0f) {
        //     SDL_Log("[DisplayList] addRoundedRect: input_y=%.1f translate_y=%.1f output_y=%.1f",
        //             rect.y, translate_y_, translated.y);
        // }
        item.rounded_rect.rect = translated;
        item.rounded_rect.color = color;
        item.rounded_rect.radius = radius;
        item.rounded_rect.stroke_width = stroke_width;
        list_.items.push_back(std::move(item));
    }


    void addShadow(const Rect& rect, const Color& color, float radius, float blur) {
        DisplayItem item{};
        item.type = DisplayItemType::DrawShadow;
        item.shadow.rect = applyTranslate(rect);
        item.shadow.color = color;
        item.shadow.radius = radius;
        item.shadow.blur = blur;
        list_.items.push_back(std::move(item));
    }

    void addImage(const Rect& rect, const std::string& src, float opacity,
                  ImageFitMode fit = ImageFitMode::Fill,
                  float pos_x = 0.5f, float pos_y = 0.5f,
                  ImageSampling sampling = ImageSampling::Linear) {
        DisplayItem item{};
        item.type = DisplayItemType::DrawImage;
        item.image.rect = applyTranslate(rect);
        item.image.src = src;
        item.image.opacity = opacity;
        item.image.fit = fit;
        item.image.position_x = pos_x;
        item.image.position_y = pos_y;
        item.image.sampling = sampling;
        list_.items.push_back(std::move(item));
    }

    void addLinearGradient(const DrawLinearGradientData& data) {
        DisplayItem item{};
        item.type = DisplayItemType::DrawLinearGradient;
        item.gradient = data;
        item.gradient.rect = applyTranslate(data.rect);
        list_.items.push_back(std::move(item));
    }


    void addGlyphRun(DrawGlyphRunData data) {
        DisplayItem item{};
        item.type = DisplayItemType::DrawGlyphRun;
        // 应用 translate 到 glyph run 的位置
        data.rect = applyTranslate(data.rect);
        data.baseline_x += translate_x_;
        data.baseline_y += translate_y_;
        // Propagate default text renderer mode if not explicitly set
        if (data.text_renderer_mode == TextRendererMode::Auto) {
            data.text_renderer_mode = default_text_renderer_mode_;
        }
        item.glyph_run = std::move(data);
        DONG_LOG_DEBUG("[DisplayListBuilder] addGlyphRun: glyphs=%zu baseline=(%.1f,%.1f) font='%s'",
                      item.glyph_run.glyphs.size(),
                      item.glyph_run.baseline_x,
                      item.glyph_run.baseline_y,
                      item.glyph_run.font_family.c_str());
        list_.items.push_back(std::move(item));
    }

    // Set the default text renderer mode for all subsequent glyph runs.
    void setDefaultTextRendererMode(TextRendererMode mode) {
        default_text_renderer_mode_ = mode;
    }

    ScopedLayer pushLayer(float opacity,
                          bool isolate = false,
                          const Rect& bounds = Rect{},
                          uint64_t layer_id = 0,
                          bool is_dirty = true) {
        float clamped = std::clamp(opacity, 0.0f, 1.0f);
        bool requires_layer = isolate || clamped < 0.999f;
        
        if (!requires_layer) {
            return ScopedLayer();
        }

        DisplayItem item{};
        item.type = DisplayItemType::PushLayer;
        item.layer.bounds = bounds;
        item.layer.opacity = clamped;
        item.layer.isolate = isolate;
        item.layer.id = layer_id;
        item.layer.is_dirty = is_dirty;
        list_.items.push_back(item);
        ++layer_depth_;
        return ScopedLayer(this, true);
    }

    ScopedClip pushClipRect(const Rect& rect) {
        if (!isValidRect(rect)) {
            return ScopedClip();
        }
        DisplayItem item{};
        item.type = DisplayItemType::PushClipRect;
        item.clip.rect = applyTranslate(rect);
        item.clip.radius = 0.0f;
        item.clip.is_rounded = false;
        list_.items.push_back(item);
        ++clip_depth_;
        return ScopedClip(this, true);
    }

    ScopedClip pushRoundedClip(const Rect& rect, float radius) {
        if (!isValidRect(rect)) {
            return ScopedClip();
        }
        DisplayItem item{};
        item.type = DisplayItemType::PushClipRoundedRect;
        item.clip.rect = applyTranslate(rect);
        item.clip.radius = std::max(0.0f, radius);
        item.clip.is_rounded = true;
        list_.items.push_back(item);
        ++clip_depth_;
        return ScopedClip(this, true);
    }


    const DisplayList& get() const { return list_; }
    DisplayList&& take() { return std::move(list_); }

    // Append pre-built items (e.g. from overlay direct-draw).
    void appendItems(const std::vector<DisplayItem>& items) {
        list_.items.insert(list_.items.end(), items.begin(), items.end());
    }

    // Returns the current number of items in the display list.
    // Use this to record an insertion point before emitting child items.
    size_t getItemCount() const { return list_.items.size(); }

    // Insert a rect at position `index` (0-based).  Items at `index` and beyond
    // are shifted one position to the right.  Use this to emit a background rect
    // before glyph runs that were already pushed to the list.
    void insertRectAt(size_t index, const Rect& rect, const Color& color) {
        DisplayItem item{};
        item.type = DisplayItemType::DrawRect;
        item.rect.rect = applyTranslate(rect);
        item.rect.color = color;
        auto it = list_.items.begin() + static_cast<ptrdiff_t>(
            std::min(index, list_.items.size()));
        list_.items.insert(it, std::move(item));
    }

    void insertRoundedRectAt(size_t index, const Rect& rect, const Color& color, float radius) {
        DisplayItem item{};
        item.type = DisplayItemType::DrawRoundedRect;
        item.rounded_rect.rect = applyTranslate(rect);
        item.rounded_rect.color = color;
        item.rounded_rect.radius = radius;
        item.rounded_rect.stroke_width = 0.0f;
        auto it = list_.items.begin() + static_cast<ptrdiff_t>(
            std::min(index, list_.items.size()));
        list_.items.insert(it, std::move(item));
    }

private:
    void popLayer() {
        if (layer_depth_ <= 0) {
            return;
        }
        DisplayItem item{};
        item.type = DisplayItemType::PopLayer;
        list_.items.push_back(item);
        --layer_depth_;
    }

    void popClip() {
        if (clip_depth_ <= 0) {
            return;
        }
        DisplayItem item{};
        item.type = DisplayItemType::PopClip;
        list_.items.push_back(item);
        --clip_depth_;
    }

    static bool isValidRect(const Rect& rect) {
        return rect.width > 0.0f && rect.height > 0.0f;
    }

    Rect applyTranslate(const Rect& rect) const {
        Rect result = rect;
        result.x += translate_x_;
        result.y += translate_y_;
        return result;
    }

    DisplayList list_;
    int layer_depth_ = 0;
    int clip_depth_ = 0;
    float translate_x_ = 0.0f;
    float translate_y_ = 0.0f;
    std::vector<std::pair<float, float>> translate_stack_;
    TextRendererMode default_text_renderer_mode_ = TextRendererMode::Auto;
};

} // namespace dong::render
