#include "../painter.hpp"

#include <algorithm>
#include <cctype>
#include <string>

#include "painter_style_utils.hpp"

namespace dong::render {

void Painter::paintMediaElements(const dom::DOMNodePtr& node,
                                const layout::LayoutNode* layout_node,
                                const std::string& tag,
                                const dom::ComputedStyle& style,
                                bool is_hidden,
                                DisplayListBuilder& builder) {
    using painter_detail::collapseWhitespace;
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

        auto toLowerCopy = [](std::string s) {
            std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            return s;
        };

        const std::string object_fit = toLowerCopy(collapseWhitespace(style.object_fit));
        ImageFitMode fit = ImageFitMode::Fill;
        if (object_fit == "contain") {
            fit = ImageFitMode::Contain;
        } else if (object_fit == "cover") {
            fit = ImageFitMode::Cover;
        }

        // object-fit: cover needs cropping inside the element box.
        DisplayListBuilder::ScopedClip img_clip;
        if (fit == ImageFitMode::Cover) {
            img_clip = builder.pushClipRect(rect);
        }

        builder.addImage(rect, src, 1.0f, fit);
        return;
    }

    // Video (poster or placeholder)
    if (tag == "video") {
        Rect rect{};
        rect.x = layout_node->layout.position[0];
        rect.y = layout_node->layout.position[1];
        rect.width = layout_node->layout.dimensions[0];
        rect.height = layout_node->layout.dimensions[1];

        if (rect.width <= 0.0f || rect.height <= 0.0f) return;

        // If video decoder has produced a frame, View will set an internal attribute
        // ("__dong_video_frame") with a special src key ("video://...").
        std::string frame_src = node->getAttribute("__dong_video_frame");

        auto toLowerCopy = [](std::string s) {
            std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            return s;
        };

        const std::string object_fit = toLowerCopy(collapseWhitespace(style.object_fit));
        ImageFitMode fit = ImageFitMode::Fill;
        if (object_fit == "contain") {
            fit = ImageFitMode::Contain;
        } else if (object_fit == "cover") {
            fit = ImageFitMode::Cover;
        }

        if (!frame_src.empty()) {
            DisplayListBuilder::ScopedClip frame_clip;
            if (fit == ImageFitMode::Cover) {
                frame_clip = builder.pushClipRect(rect);
            }
            builder.addImage(rect, frame_src, 1.0f, fit);
        } else {
            std::string poster = node->getAttribute("poster");
            if (!poster.empty()) {
                DisplayListBuilder::ScopedClip poster_clip;
                if (fit == ImageFitMode::Cover) {
                    poster_clip = builder.pushClipRect(rect);
                }

                builder.addImage(rect, poster, 1.0f, fit);
            } else {
                // Minimal placeholder: black-ish background + subtle border + optional controls bar.
                Color bg = makeColorFromCss("#0f1115");
                builder.addRect(rect, bg);

                Color border = makeColorFromCss("#3a3f4b");
                const float bw = 1.0f;
                builder.addRect(Rect{rect.x, rect.y, rect.width, bw}, border);
                builder.addRect(Rect{rect.x, rect.y + rect.height - bw, rect.width, bw}, border);
                builder.addRect(Rect{rect.x, rect.y, bw, rect.height}, border);
                builder.addRect(Rect{rect.x + rect.width - bw, rect.y, bw, rect.height}, border);
            }
        }

        // Controls overlay (still minimal, but clickable behavior is handled by View input).
        if (node->hasAttribute("controls")) {
            Color bar = makeColorFromCss("#000000");
            bar.a = 0.35f;
            const float bar_h = std::min(28.0f, rect.height);
            builder.addRect(Rect{rect.x, rect.y + rect.height - bar_h, rect.width, bar_h}, bar);
        }
    }
}

} // namespace dong::render
