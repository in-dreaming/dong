#pragma once

#include <cstdint>
#include <string>
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

struct DrawGlyphRunData {
    // 这里先放一个占位结构，后续接入真正的布局结果
    std::string text;
    Rect rect;     // 文本包围盒
    Color color;
    float font_size = 16.0f;
    std::string font_family;
    std::string font_weight;
};

struct DisplayItem {
    DisplayItemType type;

    // 为简化实现，这里不用 union，而是根据 type 只访问对应字段
    DrawRectData rect;
    DrawRoundedRectData rounded_rect;
    DrawImageData image;
    DrawGlyphRunData glyph_run;
};

struct DisplayList {
    std::vector<DisplayItem> items;
};

// 一个轻量的 builder，方便从 DOM/布局阶段生成 DisplayList
class DisplayListBuilder {
public:
    void clear() {
        list_.items.clear();
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

    void addGlyphRun(const DrawGlyphRunData& data) {
        DisplayItem item{};
        item.type = DisplayItemType::DrawGlyphRun;
        item.glyph_run = data;
        list_.items.push_back(item);
    }

    const DisplayList& get() const { return list_; }
    DisplayList&& take() { return std::move(list_); }

private:
    DisplayList list_;
};

} // namespace dong::render
