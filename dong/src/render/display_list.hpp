#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

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
    DrawImage,
    DrawGlyphRun,
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
};

struct DrawImageData {
    Rect rect;
    std::string src;   // 图片资源标识（路径或其它 key）
    float opacity = 1.0f;
};

struct GlyphInstance {
    uint32_t glyph_id = 0;
    float pen_x_units = 0.0f;      // design units（相对于文本起点）
    float pen_y_units = 0.0f;      // design units
};

struct DrawGlyphRunData {
    Rect rect;
    Color color;
    float font_size = 16.0f;
    std::string font_family;
    std::string font_weight;
    std::string font_path;
    float baseline_x = 0.0f;
    float baseline_y = 0.0f;
    std::vector<GlyphInstance> glyphs;
    
    // 新增：design units 元数据
    uint32_t units_per_em = 0;
    float scale_to_pixels = 1.0f;  // font_size / units_per_em
};

struct ClipData {
    Rect rect;
    float radius = 0.0f;
    bool is_rounded = false;
};

struct LayerData {
    Rect bounds;
    float opacity = 1.0f;
    bool isolate = false;
};

struct DisplayItem {
    DisplayItemType type;

    // 为简化实现，这里不用 union，而是根据 type 只访问对应字段
    DrawRectData rect;
    DrawRoundedRectData rounded_rect;
    DrawImageData image;
    DrawGlyphRunData glyph_run;
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
    }

    void addRect(const Rect& rect, const Color& color) {
        DisplayItem item{};
        item.type = DisplayItemType::DrawRect;
        item.rect.rect = rect;
        item.rect.color = color;
        list_.items.push_back(std::move(item));
    }

    void addRoundedRect(const Rect& rect, const Color& color, float radius) {
        DisplayItem item{};
        item.type = DisplayItemType::DrawRoundedRect;
        item.rounded_rect.rect = rect;
        item.rounded_rect.color = color;
        item.rounded_rect.radius = radius;
        list_.items.push_back(std::move(item));
    }

    void addImage(const Rect& rect, const std::string& src, float opacity) {
        DisplayItem item{};
        item.type = DisplayItemType::DrawImage;
        item.image.rect = rect;
        item.image.src = src;
        item.image.opacity = opacity;
        list_.items.push_back(std::move(item));
    }

    void addGlyphRun(DrawGlyphRunData data) {
        DisplayItem item{};
        item.type = DisplayItemType::DrawGlyphRun;
        item.glyph_run = std::move(data);
        list_.items.push_back(std::move(item));
    }

    ScopedLayer pushLayer(float opacity, bool isolate = false, const Rect& bounds = Rect{}) {
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
        item.clip.rect = rect;
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
        item.clip.rect = rect;
        item.clip.radius = std::max(0.0f, radius);
        item.clip.is_rounded = true;
        list_.items.push_back(item);
        ++clip_depth_;
        return ScopedClip(this, true);
    }

    const DisplayList& get() const { return list_; }
    DisplayList&& take() { return std::move(list_); }

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

    DisplayList list_;
    int layer_depth_ = 0;
    int clip_depth_ = 0;
};

} // namespace dong::render
