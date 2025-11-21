#include "skia_backend.hpp"
#include "render_surface.hpp"

#include <core/SkCanvas.h>
#include <core/SkSurface.h>
#include <core/SkPaint.h>
#include <core/SkFont.h>
#include <core/SkTypeface.h>
#include <core/SkTextBlob.h>
#include <core/SkPath.h>
#include <core/SkRRect.h>
#include <core/SkColor.h>
#include <core/SkImageInfo.h>
#include <core/SkImage.h>

#include <cstdio>
#include <cstring>
#include <memory>

namespace dong::render {

SkiaBackend::SkiaBackend(RenderSurface* surface)
    : render_surface_(surface), sk_canvas_(nullptr), sk_surface_(nullptr),
      default_font_(nullptr), default_font_size_(16.0f) {
}

SkiaBackend::~SkiaBackend() {
    font_cache_.clear();
    image_cache_.clear();
    
    if (sk_canvas_) {
        // sk_canvas_ is owned by sk_surface_, don't delete directly
        sk_canvas_ = nullptr;
    }
    
    if (sk_surface_) {
        SkSurface* surface = reinterpret_cast<SkSurface*>(sk_surface_);
        SkSafeUnref(surface);
        sk_surface_ = nullptr;
    }
    
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

    // Create Skia surface wrapping the CPU buffer
    size_t row_bytes = width * 4; // RGBA = 4 bytes per pixel
    sk_sp<SkSurface> surface = SkSurfaces::WrapPixels(
        info,
        pixels,
        row_bytes
    );

    if (!surface) {
        return false;
    }

    // Store canvas from surface
    SkCanvas* canvas = surface->getCanvas();
    if (!canvas) {
        return false;
    }

    // Store as opaque pointers
    sk_surface_ = surface.release();
    sk_canvas_ = canvas;

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
    
    font_cache_[font_name] = nullptr;
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
    if (!sk_canvas_ || !default_font_) return;

    SkCanvas* canvas = reinterpret_cast<SkCanvas*>(sk_canvas_);
    SkFont* font = reinterpret_cast<SkFont*>(default_font_);
    
    SkPaint paint;
    paint.setColor(SkColorSetARGB(a, r, g, b));

    // Create text blob
    sk_sp<SkTextBlob> blob = SkTextBlob::MakeFromText(
        text.c_str(),
        text.length(),
        *font
    );

    if (blob) {
        canvas->drawTextBlob(blob, x, y, paint);
    }
    
    render_surface_->markDirty();
}

void SkiaBackend::drawParagraph(const std::string& text, float x, float y, float max_width,
                               float font_size, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    // Simplified paragraph drawing - just use drawText for now
    // A proper implementation would use Skia's paragraph module
    drawText(text, x, y, font_size, r, g, b, a);
}

void* SkiaBackend::loadTypeface(const std::string& font_name) {
    auto it = font_cache_.find(font_name);
    if (it != font_cache_.end()) {
        return it->second;
    }
    
    // For now, just return nullptr and use default typeface
    // A proper implementation would use SkFontMgr to load typefaces
    return nullptr;
}

void* SkiaBackend::loadImage(const std::string& image_path) {
    auto it = image_cache_.find(image_path);
    if (it != image_cache_.end()) {
        return it->second;
    }
    
    // TODO: Implement image loading from file
    // This would require SkImage::MakeFromEncoded or similar
    return nullptr;
}

void SkiaBackend::drawImage(const std::string& image_path, float x, float y,
                           float width, float height, uint8_t alpha) {
    if (!sk_canvas_) return;

    // TODO: Load image and draw it
    // void* img_ptr = loadImage(image_path);
    // if (img_ptr) {
    //     SkImage* image = reinterpret_cast<SkImage*>(img_ptr);
    //     SkCanvas* canvas = reinterpret_cast<SkCanvas*>(sk_canvas_);
    //     SkPaint paint;
    //     paint.setAlpha(alpha);
    //     canvas->drawImageRect(image, SkRect::MakeXYWH(x, y, width, height), paint);
    // }
    
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
