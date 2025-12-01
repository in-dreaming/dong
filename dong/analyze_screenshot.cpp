// 独立的截图分析工具
// 用法: ./analyze_screenshot <image.bmp>
// 
// 分析 MSDF 渲染结果的像素分布，判断渲染是否正常

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <vector>

struct BMPHeader {
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
} __attribute__((packed));

struct BMPInfoHeader {
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bits;
    uint32_t compression;
    uint32_t imagesize;
    int32_t xresolution;
    int32_t yresolution;
    uint32_t ncolors;
    uint32_t importantcolors;
} __attribute__((packed));

bool loadBMP(const char* filename, std::vector<uint8_t>& pixels, uint32_t& width, uint32_t& height) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        std::printf("ERROR: Cannot open file: %s\n", filename);
        return false;
    }

    BMPHeader header;
    fread(&header, sizeof(BMPHeader), 1, f);
    
    if (header.type != 0x4D42) {  // "BM"
        std::printf("ERROR: Not a valid BMP file\n");
        fclose(f);
        return false;
    }

    BMPInfoHeader info;
    fread(&info, sizeof(BMPInfoHeader), 1, f);

    width = info.width;
    height = abs(info.height);
    
    std::printf("[Load] %s: %ux%u, %u bits\n", filename, width, height, info.bits);

    if (info.bits != 24 && info.bits != 32) {
        std::printf("ERROR: Only 24-bit and 32-bit BMP supported\n");
        fclose(f);
        return false;
    }

    fseek(f, header.offset, SEEK_SET);

    int bytes_per_pixel = info.bits / 8;
    int row_size = ((width * bytes_per_pixel + 3) / 4) * 4;  // 4-byte aligned
    
    pixels.resize(width * height * 4);  // Always RGBA

    for (int y = height - 1; y >= 0; y--) {
        for (uint32_t x = 0; x < width; x++) {
            uint8_t bgr[4] = {0, 0, 0, 255};
            fread(bgr, bytes_per_pixel, 1, f);
            
            int idx = (y * width + x) * 4;
            pixels[idx + 0] = bgr[2];  // R
            pixels[idx + 1] = bgr[1];  // G
            pixels[idx + 2] = bgr[0];  // B
            pixels[idx + 3] = (bytes_per_pixel == 4) ? bgr[3] : 255;  // A
        }
        
        // Skip padding
        int padding = row_size - width * bytes_per_pixel;
        if (padding > 0) {
            fseek(f, padding, SEEK_CUR);
        }
    }

    fclose(f);
    return true;
}

void analyzePixels(const uint8_t* pixels, uint32_t width, uint32_t height) {
    std::printf("\n========================================\n");
    std::printf("=== COMPREHENSIVE PIXEL ANALYSIS ===\n");
    std::printf("========================================\n\n");

    int total_pixels = width * height;
    int histogram[256] = {0};
    int total_black = 0, total_white = 0, total_gray = 0;
    int total_dark = 0, total_light = 0;
    
    for (uint32_t i = 0; i < total_pixels; i++) {
        int idx = i * 4;
        uint8_t r = pixels[idx];
        uint8_t g = pixels[idx + 1];
        uint8_t b = pixels[idx + 2];
        
        int brightness = (r + g + b) / 3;
        histogram[brightness]++;
        
        if (brightness < 50) total_black++;
        else if (brightness < 100) total_dark++;
        else if (brightness < 150) total_gray++;
        else if (brightness < 200) total_light++;
        else total_white++;
    }
    
    std::printf("Resolution: %ux%u (%d pixels)\n\n", width, height, total_pixels);
    
    std::printf("Brightness Distribution:\n");
    std::printf("  Black (<50):         %7d (%5.2f%%)\n", total_black, 100.0*total_black/total_pixels);
    std::printf("  Dark (50-100):       %7d (%5.2f%%)\n", total_dark, 100.0*total_dark/total_pixels);
    std::printf("  Gray (100-150):      %7d (%5.2f%%)\n", total_gray, 100.0*total_gray/total_pixels);
    std::printf("  Light (150-200):     %7d (%5.2f%%)\n", total_light, 100.0*total_light/total_pixels);
    std::printf("  White (>200):        %7d (%5.2f%%)\n\n", total_white, 100.0*total_white/total_pixels);

    // 显示直方图（精简版）
    std::printf("Brightness Histogram (每10个级别):\n");
    for (int i = 0; i < 256; i += 10) {
        int sum = 0;
        for (int j = i; j < i + 10 && j < 256; j++) {
            sum += histogram[j];
        }
        float percent = 100.0f * sum / total_pixels;
        if (percent > 0.1f) {
            std::printf("  [%3d-%3d]: %7d (%5.2f%%)  ", i, i+9, sum, percent);
            int bars = (int)(percent * 2);
            for (int b = 0; b < bars && b < 50; b++) std::printf("█");
            std::printf("\n");
        }
    }
    std::printf("\n");

    // 文本检测
    int text_pixels = total_black + total_dark;
    int anti_alias_pixels = total_gray;
    float text_ratio = 100.0f * text_pixels / total_pixels;
    float aa_ratio = 100.0f * anti_alias_pixels / total_pixels;
    
    std::printf("Text Detection:\n");
    std::printf("  Text cores (< 100):   %7d (%5.2f%%)\n", text_pixels, text_ratio);
    std::printf("  Anti-aliasing edges:  %7d (%5.2f%%)\n", anti_alias_pixels, aa_ratio);
    std::printf("  Background (> 200):   %7d (%5.2f%%)\n\n", total_white, 100.0*total_white/total_pixels);

    // 区域采样
    std::printf("Sample Points:\n");
    struct {int x, y; const char* desc;} samples[] = {
        {50, 50, "Top-left"},
        {(int)(width/2), (int)(height/2), "Center"},
        {(int)width-50, (int)height-50, "Bottom-right"},
        {100, 100, "Expected text area"},
        {(int)(width/2), 50, "Top-center"},
    };
    
    for (const auto& s : samples) {
        if (s.x < (int)width && s.y < (int)height) {
            int idx = (s.y * width + s.x) * 4;
            uint8_t r = pixels[idx];
            uint8_t g = pixels[idx + 1];
            uint8_t b = pixels[idx + 2];
            int brightness = (r + g + b) / 3;
            std::printf("  (%4d,%4d) %-20s: RGB(%3d,%3d,%3d) = %3d\n",
                       s.x, s.y, s.desc, r, g, b, brightness);
        }
    }
    std::printf("\n");

    // 最终判断
    std::printf("========================================\n");
    std::printf("FINAL VERDICT:\n");
    std::printf("========================================\n");
    
    if (total_black + total_dark + total_gray == 0) {
        std::printf("❌ RENDERING COMPLETELY FAILED\n");
        std::printf("   All pixels are bright (>150)\n");
        std::printf("   No text rendering occurred!\n");
    } else if (text_ratio < 0.5f) {
        std::printf("❌ RENDERING FAILED\n");
        std::printf("   Only %.2f%% text pixels (expected >1%%)\n", text_ratio);
        std::printf("   MSDF text is not rendering correctly!\n");
    } else if (text_ratio < 2.0f) {
        std::printf("⚠️  RENDERING PARTIALLY WORKING\n");
        std::printf("   %.2f%% text pixels (expected >2%%)\n", text_ratio);
        std::printf("   Text may be too small or sparse.\n");
    } else if (aa_ratio < 0.5f) {
        std::printf("⚠️  RENDERING WITHOUT ANTI-ALIASING\n");
        std::printf("   %.2f%% text, but only %.2f%% AA edges\n", text_ratio, aa_ratio);
        std::printf("   MSDF may not be smoothing edges properly.\n");
    } else {
        std::printf("✅ RENDERING SUCCESS!\n");
        std::printf("   %.2f%% text pixels\n", text_ratio);
        std::printf("   %.2f%% anti-aliased edges\n", aa_ratio);
        std::printf("   MSDF text is rendering correctly!\n");
    }
    std::printf("========================================\n");
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::printf("Usage: %s <screenshot.bmp>\n", argv[0]);
        std::printf("\nAnalyzes a BMP screenshot to verify MSDF text rendering.\n");
        return 1;
    }

    const char* filename = argv[1];
    
    std::vector<uint8_t> pixels;
    uint32_t width, height;
    
    if (!loadBMP(filename, pixels, width, height)) {
        return 1;
    }

    analyzePixels(pixels.data(), width, height);
    
    return 0;
}
