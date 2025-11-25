#include "skia_backend.hpp"
#include "render_surface.hpp"
#include "resource_manager.hpp"

#include <core/SkCanvas.h>
#include <core/SkPaint.h>
#include <core/SkFont.h>
#include <core/SkGraphics.h>
#include <core/SkTypeface.h>
#include <core/SkTextBlob.h>
#include <core/SkPath.h>
#include <core/SkRRect.h>
#include <core/SkColor.h>
#include <core/SkImageInfo.h>
#include <core/SkImage.h>
#include <core/SkFontStyle.h>
#include <core/SkFontTypes.h>

#include <cstdio>
#include <cstring>
#include <memory>
#include <mutex>

namespace dong::render {

namespace {
std::once_flag g_skia_init_flag;
}

SkiaBackend::SkiaBackend(RenderSurface* surface)
    : render_surface_(surface), sk_canvas_(nullptr),
      default_font_(nullptr), default_font_size_(16.0f),
      resource_manager_(std::make_unique<ResourceManager>()) {
    std::call_once(g_skia_init_flag, []() {
        SkGraphics::Init();
    });
}

SkiaBackend::~SkiaBackend() {
    resource_manager_.reset();
    
    if (sk_canvas_) {
        sk_canvas_ = nullptr;
    }
    sk_canvas_holder_.reset();
    
    if (default_font_) {
        SkFont* font = reinterpret_cast<SkFont*>(default_font_);
        delete font;
        default_font_ = nullptr;
    }
}

bool SkiaBackend::initialize() {
    if (!render_surface_) return false;
    if (render_surface_->getType() != RenderSurface::Type::CPU_BUFFER) {
        return false;
    }

    // Get CPU buffer dimensions
    uint32_t width = render_surface_->getWidth();
    uint32_t height = render_surface_->getHeight();

    // Create Skia image info for RGBA
    SkImageInfo info = SkImageInfo::Make(
        width, height,
        kRGBA_8888_SkColorType,
        kOpaque_SkAlphaType
    );

    // Get the CPU buffer from render surface
    void* pixels = render_surface_->getCPUBuffer();
    if (!pixels) return false;

    // Create a raster canvas that wraps the CPU buffer
    size_t row_bytes = width * 4; // RGBA = 4 bytes per pixel
    auto canvas = SkCanvas::MakeRasterDirect(info, pixels, row_bytes);
    if (!canvas) {
        return false;
    }

    sk_canvas_holder_ = std::move(canvas);
    sk_canvas_ = sk_canvas_holder_.get();

    // Create default font with default typeface
    SkFont* font = new SkFont();
    font->setSize(default_font_size_);
    default_font_ = font;

    return true;
}

void SkiaBackend::setDefaultFont(const std::string& font_name, float font_size) {
    default_font_size_ = font_size;
    
    if (default_font_) {
        SkFont* font = reinterpret_cast<SkFont*>(default_font_);
        font->setSize(font_size);
    }
}

void SkiaBackend::drawRect(float x, float y, float width, float height,
                          uint8_t r, uint8_t g, uint8_t b, uint8_t a, float stroke_width) {
    if (!sk_canvas_) return;

    SkCanvas* canvas = reinterpret_cast<SkCanvas*>(sk_canvas_);
    SkPaint paint;
    paint.setColor(SkColorSetARGB(a, r, g, b));

    if (stroke_width > 0) {
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(stroke_width);
    } else {
        paint.setStyle(SkPaint::kFill_Style);
    }

    canvas->drawRect(SkRect::MakeXYWH(x, y, width, height), paint);
    
    render_surface_->markDirty();
}

void SkiaBackend::drawRoundRect(float x, float y, float width, float height, float radius,
                               uint8_t r, uint8_t g, uint8_t b, uint8_t a, float stroke_width) {
    if (!sk_canvas_) return;

    SkCanvas* canvas = reinterpret_cast<SkCanvas*>(sk_canvas_);
    SkPaint paint;
    paint.setColor(SkColorSetARGB(a, r, g, b));

    if (stroke_width > 0) {
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(stroke_width);
    } else {
        paint.setStyle(SkPaint::kFill_Style);
    }

    SkRRect rrect = SkRRect::MakeRectXY(
        SkRect::MakeXYWH(x, y, width, height),
        radius, radius
    );
    
    canvas->drawRRect(rrect, paint);
    
    render_surface_->markDirty();
}

void SkiaBackend::drawStroke(float x, float y, float width, float height, float stroke_width,
                            uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    drawRect(x, y, width, height, r, g, b, a, stroke_width);
}

void SkiaBackend::drawShadow(float x, float y, float width, float height, float blur_radius,
                            float offset_x, float offset_y, uint8_t shadow_color_alpha) {
    if (!sk_canvas_) return;

    SkCanvas* canvas = reinterpret_cast<SkCanvas*>(sk_canvas_);
    SkPaint paint;
    paint.setColor(SkColorSetARGB(shadow_color_alpha / 2, 0, 0, 0));
    paint.setStyle(SkPaint::kFill_Style);

    // Draw shadow with offset
    canvas->drawRect(
        SkRect::MakeXYWH(x + offset_x, y + offset_y, width, height),
        paint
    );
    
    render_surface_->markDirty();
}

void SkiaBackend::drawText(const std::string& text, float x, float y,
                           float font_size, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    // 保持兼容旧接口：使用默认字体族和权重
    drawTextStyled(text, x, y, font_size, "", "", r, g, b, a);
}

void SkiaBackend::drawTextStyled(const std::string& text, float x, float y,
                                 float font_size, const std::string& font_family,
                                 const std::string& font_weight,
                                 uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (!sk_canvas_) return;

    SkCanvas* canvas = reinterpret_cast<SkCanvas*>(sk_canvas_);

    // 准备字体对象
    float size = font_size > 0.0f ? font_size : default_font_size_;

    // 使用 SkFontMgr 从系统加载字体，尽量按 font-family / font-weight 匹配
    SkFont font;
    if (default_font_) {
        font = *reinterpret_cast<SkFont*>(default_font_);
    }
    font.setSize(size);

    // 开启子像素抗锯齿，让文字尽可能清晰
    font.setEdging(SkFont::Edging::kSubpixelAntiAlias);

    SkPaint paint;
    paint.setColor(SkColorSetARGB(a, r, g, b));
    paint.setAntiAlias(true);

    std::fprintf(stderr,
                 "[SkiaBackend] drawTextStyled '%s' at (%.1f,%.1f) size=%.1f family='%s' weight='%s' rgba=(%u,%u,%u,%u)\n",
                 text.c_str(), x, y, size, font_family.c_str(), font_weight.c_str(), r, g, b, a);

    // 使用 SkCanvas::drawSimpleText 直接根据 UTF-8 文本绘制
    canvas->drawSimpleText(
        text.c_str(),
        text.length(),
        SkTextEncoding::kUTF8,
        x,
        y,
        font,
        paint
    );

    render_surface_->markDirty();
}

float SkiaBackend::measureTextWidth(const std::string& text, float font_size,
                                   const std::string& font_family,
                                   const std::string& font_weight) {
    // 复用 drawTextStyled 中的字体构造逻辑，但不真正绘制，只做度量
    float size = font_size > 0.0f ? font_size : default_font_size_;

    SkFont font;
    if (default_font_) {
        font = *reinterpret_cast<SkFont*>(default_font_);
    }
    font.setSize(size);

    SkRect bounds;
    font.measureText(text.c_str(), text.length(), SkTextEncoding::kUTF8, &bounds);
    return bounds.width();
}

void SkiaBackend::drawParagraph(const std::string& text, float x, float y, float max_width,
                               float font_size, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    // Simplified paragraph drawing - just use drawText for now
    // A proper implementation would use Skia's paragraph module
    drawText(text, x, y, font_size, r, g, b, a);
}

void* SkiaBackend::loadTypeface(const std::string& font_name) {
    if (!resource_manager_) return nullptr;
    
    FontResource* res = resource_manager_->getSystemFont(font_name, default_font_size_);
    if (res) {
        return res->sk_typeface;
    }
    return nullptr;
}

void* SkiaBackend::loadImage(const std::string& image_path) {
    if (!resource_manager_) return nullptr;
    
    ImageResource* res = resource_manager_->loadImage(image_path);
    if (res) {
        return res->sk_image;
    }
    return nullptr;
}

void SkiaBackend::drawImage(const std::string& image_path, float x, float y,
                           float width, float height, uint8_t alpha) {
    if (!sk_canvas_ || !resource_manager_) return;

    // If width or height is 0, try to load image and use its intrinsic size
    float actual_width = width;
    float actual_height = height;

    ImageResource* img_res = resource_manager_->getImage(image_path);
    if (!img_res) {
        img_res = resource_manager_->loadImage(image_path);
    }
    
    if (img_res && img_res->sk_image) {
        // If no explicit size, use image's intrinsic size
        if (actual_width <= 0.0f || actual_height <= 0.0f) {
            actual_width = (float)img_res->width;
            actual_height = (float)img_res->height;
        }
        
        SkImage* image = reinterpret_cast<SkImage*>(img_res->sk_image);
        SkCanvas* canvas = reinterpret_cast<SkCanvas*>(sk_canvas_);
        
        SkPaint paint;
        paint.setAlpha(alpha);
        
        SkRect dest = SkRect::MakeXYWH(x, y, actual_width, actual_height);
        canvas->drawImageRect(image, dest, SkSamplingOptions(), &paint);
    }
    
    render_surface_->markDirty();
}

void SkiaBackend::saveState() {
    if (!sk_canvas_) return;

    SkCanvas* canvas = reinterpret_cast<SkCanvas*>(sk_canvas_);
    canvas->save();
}

void SkiaBackend::restoreState() {
    if (!sk_canvas_) return;

    SkCanvas* canvas = reinterpret_cast<SkCanvas*>(sk_canvas_);
    canvas->restore();
}

void SkiaBackend::clipRect(float x, float y, float width, float height) {
    if (!sk_canvas_) return;

    SkCanvas* canvas = reinterpret_cast<SkCanvas*>(sk_canvas_);
    canvas->clipRect(SkRect::MakeXYWH(x, y, width, height));
}

void SkiaBackend::flush() {
    // For CPU buffer surface, no explicit flush needed
    // Pixels are directly written to the buffer
}

} // namespace dong::render
