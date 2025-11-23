#include <cstdio>
#include <cstdint>

#include "render/render_surface.hpp"
#include "render/skia_backend.hpp"

using namespace dong::render;

int main() {
    std::printf("=== Skia CPUBuffer Demo ===\n\n");

    const uint32_t width = 256;
    const uint32_t height = 256;

    std::fprintf(stderr, "[skia_demo] constructing surface\\n");
    CPUBufferSurface surface(width, height);

    std::fprintf(stderr, "[skia_demo] constructing backend\\n");
    SkiaBackend backend(&surface);

    std::fprintf(stderr, "[skia_demo] calling initialize()\\n");
    if (!backend.initialize()) {
        std::fprintf(stderr, "[skia_demo] initialize() returned false\\n");
        std::printf("ERROR: SkiaBackend initialization failed (WrapPixels or canvas setup).\n");
        return 1;
    }
    std::fprintf(stderr, "[skia_demo] initialize() succeeded\\n");

    // Clear to white via RenderSurface API
    std::fprintf(stderr, "[skia_demo] clearing surface\\n");
    surface.clear(255, 255, 255, 255);

    // Draw a few primitives to verify basic Skia rendering
    std::fprintf(stderr, "[skia_demo] drawing rect\\n");
    backend.drawRect(20, 20, 80, 40, 255, 0, 0, 255);              // solid red rect
    std::fprintf(stderr, "[skia_demo] drawing round rect\\n");
    backend.drawRoundRect(60, 80, 140, 80, 12, 0, 200, 0, 255);    // green rounded rect
    std::fprintf(stderr, "[skia_demo] drawing stroke\\n");
    backend.drawStroke(10, 10, 200, 150, 3.0f, 0, 0, 255, 255);    // blue border
    std::fprintf(stderr, "[skia_demo] drawing text\\n");
    backend.drawText("Skia OK", 40, 200, 20.0f, 0, 0, 0, 255);    // black text

    backend.flush();

    // Inspect a few pixels to make sure something was rendered
    auto* buf = static_cast<uint8_t*>(surface.getCPUBuffer());
    if (!buf) {
        std::printf("ERROR: CPU buffer is null.\n");
        return 1;
    }

    std::uint64_t checksum = 0;
    const std::size_t total_bytes = static_cast<std::size_t>(width) * height * 4;
    for (std::size_t i = 0; i < total_bytes; i += 31) {
        checksum += buf[i];
    }

    const uint8_t r = buf[0];
    const uint8_t g = buf[1];
    const uint8_t b = buf[2];
    const uint8_t a = buf[3];

    std::printf("Top-left pixel RGBA = (%u, %u, %u, %u)\n", r, g, b, a);
    std::printf("Sampled checksum = %llu\n", (unsigned long long)checksum);

    std::printf("Skia CPUBuffer demo finished.\n");
    return 0;
}
