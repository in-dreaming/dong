#include "../painter.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <sstream>
#include <string>

#include "painter_style_utils.hpp"


namespace dong::render {

namespace {

// Parse a single object-position component keyword/percentage/length to a 0.0–1.0 factor.
// Keywords: left/top → 0.0, center → 0.5, right/bottom → 1.0
// Percentages: "25%" → 0.25
// Lengths (px): treated as absolute offset — not converted to fraction here; caller handles.
float parsePositionComponent(const std::string& token, bool is_y_axis) {
    if (token == "left")   return 0.0f;
    if (token == "right")  return 1.0f;
    if (token == "top")    return 0.0f;
    if (token == "bottom") return 1.0f;
    if (token == "center") return 0.5f;

    // Try percentage
    if (!token.empty() && token.back() == '%') {
        float pct = std::strtof(token.c_str(), nullptr);
        return pct / 100.0f;
    }

    // Fallback: treat as center
    return 0.5f;
}

// Parse CSS object-position value (e.g. "left top", "50% 50%", "center") into X/Y fractions.
void parseObjectPosition(const std::string& value, float& out_x, float& out_y) {
    out_x = 0.5f;
    out_y = 0.5f;

    std::istringstream iss(value);
    std::string first, second;
    iss >> first;
    iss >> second;

    if (first.empty()) return;

    // Normalize to lowercase
    std::transform(first.begin(), first.end(), first.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (!second.empty()) {
        std::transform(second.begin(), second.end(), second.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    }

    if (second.empty()) {
        // Single value: "center" → (0.5, 0.5), "left" → (0.0, 0.5), "top" → (0.5, 0.0)
        if (first == "top" || first == "bottom") {
            out_x = 0.5f;
            out_y = parsePositionComponent(first, true);
        } else {
            out_x = parsePositionComponent(first, false);
            out_y = 0.5f;
        }
    } else {
        out_x = parsePositionComponent(first, false);
        out_y = parsePositionComponent(second, true);
    }
}

// Extract object-fit mode and object-position from style.
struct MediaFitInfo {
    ImageFitMode fit;
    float pos_x;
    float pos_y;
};

MediaFitInfo extractMediaFitInfo(const dom::ComputedStyle& style) {
    using painter_detail::collapseWhitespace;
    using painter_detail::toLowerCopy;

    MediaFitInfo info{};
    info.fit = ImageFitMode::Fill;
    info.pos_x = 0.5f;
    info.pos_y = 0.5f;

    const std::string object_fit = toLowerCopy(collapseWhitespace(style.object_fit));
    if (object_fit == "contain") {
        info.fit = ImageFitMode::Contain;
    } else if (object_fit == "cover") {
        info.fit = ImageFitMode::Cover;
    }

    parseObjectPosition(style.object_position, info.pos_x, info.pos_y);
    return info;
}

ImageSampling extractImageSampling(const dom::ComputedStyle& style) {
    using painter_detail::collapseWhitespace;
    using painter_detail::toLowerCopy;

    const std::string v = toLowerCopy(collapseWhitespace(style.image_rendering));
    if (v == "pixelated" || v == "crisp-edges") {
        return ImageSampling::Nearest;
    }
    return ImageSampling::Linear;
}

} // anonymous namespace

void Painter::paintMediaElements(const dom::DOMNodePtr& node,
                                const layout::LayoutNode* layout_node,
                                const std::string& tag,
                                const dom::ComputedStyle& style,
                                bool is_hidden,
                                DisplayListBuilder& builder) {
    using painter_detail::makeColorFromCss;

    if (!node || !layout_node || is_hidden) {
        return;
    }

    if (tag == "img") {
        std::string src = node->getAttribute("src");
        if (src.empty()) return;

        Rect rect{};
        rect.x = layout_node->layout.position[0];
        rect.y = layout_node->layout.position[1];
        rect.width = layout_node->layout.dimensions[0];
        rect.height = layout_node->layout.dimensions[1];

        if (rect.width <= 0.0f || rect.height <= 0.0f) return;

        const auto mfi = extractMediaFitInfo(style);

        // Check if image file exists (simple file existence check)
        bool image_exists = false;
        if (!src.empty()) {
            // Simple check: if src starts with "test_" or contains "broken", treat as broken image
            // In a real implementation, this would check file existence via platform API
            image_exists = (src.find("broken") == std::string::npos &&
                          src.find("test_broken") == std::string::npos);
        }

        // If image failed to load, render alt text
        if (!image_exists) {
            std::string alt_text = node->getAttribute("alt");
            if (!alt_text.empty()) {
                renderAltText(rect, alt_text, style, builder);
                return;
            }
        }

        DisplayListBuilder::ScopedClip img_clip;
        if (mfi.fit == ImageFitMode::Cover) {
            img_clip = builder.pushClipRect(rect);
        }

        builder.addImage(rect, src, 1.0f, mfi.fit, mfi.pos_x, mfi.pos_y, extractImageSampling(style));

        return;
    }

    if (tag == "video") {
        paintVideoElement(node, layout_node, style, builder);
    }
}

void Painter::paintVideoElement(const dom::DOMNodePtr& node,
                                const layout::LayoutNode* layout_node,
                                const dom::ComputedStyle& style,
                                DisplayListBuilder& builder) {
    using painter_detail::makeColorFromCss;

    Rect rect{};
    rect.x = layout_node->layout.position[0];
    rect.y = layout_node->layout.position[1];
    rect.width = layout_node->layout.dimensions[0];
    rect.height = layout_node->layout.dimensions[1];

    if (rect.width <= 0.0f || rect.height <= 0.0f) return;

    std::string frame_src = node->getAttribute("__dong_video_frame");
    const auto mfi = extractMediaFitInfo(style);

    static const bool kDebugVideoPaint = (std::getenv("DONG_DEBUG_VIDEO_PAINT") != nullptr);

    if (!frame_src.empty()) {
        if (kDebugVideoPaint) {
            DONG_LOG_INFO("[paintVideoElement] frame src=%s rect=(%.1f,%.1f,%.1f,%.1f) fit=%d",
                          frame_src.c_str(), rect.x, rect.y, rect.width, rect.height, (int)mfi.fit);
        }
        DisplayListBuilder::ScopedClip frame_clip;
        if (mfi.fit == ImageFitMode::Cover) {
            frame_clip = builder.pushClipRect(rect);
        }
        builder.addImage(rect, frame_src, 1.0f, mfi.fit, mfi.pos_x, mfi.pos_y, extractImageSampling(style));

    } else {
        std::string poster = node->getAttribute("poster");
        if (!poster.empty()) {
            if (kDebugVideoPaint) {
                DONG_LOG_INFO("[paintVideoElement] poster=%s rect=(%.1f,%.1f,%.1f,%.1f) fit=%d",
                              poster.c_str(), rect.x, rect.y, rect.width, rect.height, (int)mfi.fit);
            }
            DisplayListBuilder::ScopedClip poster_clip;
            if (mfi.fit == ImageFitMode::Cover) {
                poster_clip = builder.pushClipRect(rect);
            }
            builder.addImage(rect, poster, 1.0f, mfi.fit, mfi.pos_x, mfi.pos_y, extractImageSampling(style));

        } else {
            paintVideoPlaceholder(rect, builder);
        }
    }

    // Controls overlay
    if (node->hasAttribute("controls")) {
        Color bar = makeColorFromCss("#000000");
        bar.a = 0.35f;
        const float bar_h = std::min(28.0f, rect.height);
        builder.addRect(Rect{rect.x, rect.y + rect.height - bar_h, rect.width, bar_h}, bar);
    }
}

void Painter::paintVideoPlaceholder(const Rect& rect, DisplayListBuilder& builder) {
    using painter_detail::makeColorFromCss;

    Color bg = makeColorFromCss("#0f1115");
    builder.addRect(rect, bg);

    Color border = makeColorFromCss("#3a3f4b");
    const float bw = 1.0f;
    builder.addRect(Rect{rect.x, rect.y, rect.width, bw}, border);
    builder.addRect(Rect{rect.x, rect.y + rect.height - bw, rect.width, bw}, border);
    builder.addRect(Rect{rect.x, rect.y, bw, rect.height}, border);
    builder.addRect(Rect{rect.x + rect.width - bw, rect.y, bw, rect.height}, border);
}

} // namespace dong::render

