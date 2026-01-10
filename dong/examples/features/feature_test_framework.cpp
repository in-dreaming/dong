#include "feature_test_framework.hpp"
#include <dong.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <map>

namespace dong::test {

// Static renderer state
static SDL_Window* s_window = nullptr;
static SDL_GPUDevice* s_device = nullptr;
static dong_context_t* s_ctx = nullptr;
static dong_view_t* s_view = nullptr;
static int s_current_width = 0;
static int s_current_height = 0;

FeatureTestRunner::FeatureTestRunner() = default;

FeatureTestRunner::~FeatureTestRunner() {
    cleanupRenderer();
}

void FeatureTestRunner::addTest(const FeatureTestCase& test) {
    tests_.push_back(test);
}

void FeatureTestRunner::addTests(const std::vector<FeatureTestCase>& tests) {
    tests_.insert(tests_.end(), tests.begin(), tests.end());
}

bool FeatureTestRunner::initializeRenderer(int width, int height) {
    if (s_device && s_current_width == width && s_current_height == height) {
        return true;  // Already initialized with same dimensions
    }
    
    cleanupRenderer();
    
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("ERROR: SDL_Init failed: %s", SDL_GetError());
        return false;
    }
    
    s_window = SDL_CreateWindow("Feature Test", width, height, SDL_WINDOW_HIDDEN);
    if (!s_window) {
        SDL_Log("ERROR: SDL_CreateWindow failed: %s", SDL_GetError());
        return false;
    }
    
    s_device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
                                   false, nullptr);
    if (!s_device) {
        SDL_Log("ERROR: SDL_CreateGPUDevice failed: %s", SDL_GetError());
        return false;
    }
    
    if (!SDL_ClaimWindowForGPUDevice(s_device, s_window)) {
        SDL_Log("ERROR: SDL_ClaimWindowForGPUDevice failed: %s", SDL_GetError());
        return false;
    }
    
    s_ctx = dong_create_context();
    if (!s_ctx) {
        SDL_Log("ERROR: dong_create_context failed");
        return false;
    }
    
    s_view = dong_view_create(s_ctx, width, height);
    if (!s_view) {
        SDL_Log("ERROR: dong_view_create failed");
        return false;
    }
    
    dong_view_set_external_gpu_device(s_view, static_cast<void*>(s_device), static_cast<void*>(s_window));
    
    s_current_width = width;
    s_current_height = height;
    
    return true;
}

void FeatureTestRunner::cleanupRenderer() {
    if (s_view) {
        dong_view_destroy(s_view);
        s_view = nullptr;
    }
    if (s_ctx) {
        dong_destroy_context(s_ctx);
        s_ctx = nullptr;
    }
    if (s_device) {
        if (s_window) {
            SDL_ReleaseWindowFromGPUDevice(s_device, s_window);
        }
        SDL_DestroyGPUDevice(s_device);
        s_device = nullptr;
    }
    if (s_window) {
        SDL_DestroyWindow(s_window);
        s_window = nullptr;
    }
    s_current_width = 0;
    s_current_height = 0;
}

bool FeatureTestRunner::renderToPixels(const std::string& html, int width, int height, 
                                        std::vector<uint8_t>& pixels) {
    if (!initializeRenderer(width, height)) {
        return false;
    }
    
    dong_view_load_html(s_view, html.c_str());
    
    pixels.resize(width * height * 4);
    return dong_view_render_offscreen(s_view, static_cast<void*>(s_device), 
                                       width, height, pixels.data());
}

bool FeatureTestRunner::saveBMP(const std::string& path, int width, int height, const uint8_t* pixels) {
    FILE* f = fopen(path.c_str(), "wb");
    if (!f) return false;

    uint32_t filesize = 54 + width * height * 3;
    uint8_t bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
    bmpfileheader[2] = (uint8_t)(filesize);
    bmpfileheader[3] = (uint8_t)(filesize >> 8);
    bmpfileheader[4] = (uint8_t)(filesize >> 16);
    bmpfileheader[5] = (uint8_t)(filesize >> 24);

    uint8_t bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
    bmpinfoheader[4]  = (uint8_t)(width);
    bmpinfoheader[5]  = (uint8_t)(width >> 8);
    bmpinfoheader[6]  = (uint8_t)(width >> 16);
    bmpinfoheader[7]  = (uint8_t)(width >> 24);
    bmpinfoheader[8]  = (uint8_t)(height);
    bmpinfoheader[9]  = (uint8_t)(height >> 8);
    bmpinfoheader[10] = (uint8_t)(height >> 16);
    bmpinfoheader[11] = (uint8_t)(height >> 24);

    fwrite(bmpfileheader, 1, 14, f);
    fwrite(bmpinfoheader, 1, 40, f);

    for (int y = height - 1; y >= 0; --y) {
        for (int x = 0; x < width; ++x) {
            int idx = (y * width + x) * 4;
            uint8_t rgb[3] = {pixels[idx + 2], pixels[idx + 1], pixels[idx + 0]};
            fwrite(rgb, 3, 1, f);
        }
    }

    fclose(f);
    return true;
}

FeatureTestResult FeatureTestRunner::runTest(const FeatureTestCase& test, const std::string& output_dir) {
    FeatureTestResult result;
    result.name = test.name;
    result.category = test.category;
    result.description = test.description;
    
    // Create output directory
    std::filesystem::create_directories(output_dir);
    
    // Generate screenshot filename
    std::string safe_name = test.name;
    for (char& c : safe_name) {
        if (!std::isalnum(c) && c != '_' && c != '-') c = '_';
    }
    result.screenshot_path = output_dir + "/" + test.category + "_" + safe_name + ".bmp";
    
    // Render
    std::vector<uint8_t> pixels;
    if (!renderToPixels(test.html, test.width, test.height, pixels)) {
        result.passed = false;
        result.error_message = "Failed to render HTML";
        return result;
    }
    
    // Save screenshot
    if (!saveBMP(result.screenshot_path, test.width, test.height, pixels.data())) {
        result.passed = false;
        result.error_message = "Failed to save screenshot";
        return result;
    }
    
    // Run validator if provided
    if (test.validator) {
        result.passed = test.validator(pixels.data(), test.width, test.height);
        if (!result.passed) {
            result.error_message = "Validation failed";
        }
    } else {
        // Without validator, assume passed if render succeeded
        result.passed = true;
    }
    
    return result;
}

std::vector<FeatureTestResult> FeatureTestRunner::runAll(const std::string& output_dir) {
    std::vector<FeatureTestResult> results;
    
    SDL_Log("Running %zu feature tests...", tests_.size());
    
    for (size_t i = 0; i < tests_.size(); ++i) {
        SDL_Log("[%zu/%zu] Testing: %s - %s", 
                i + 1, tests_.size(), tests_[i].category.c_str(), tests_[i].name.c_str());
        
        auto result = runTest(tests_[i], output_dir);
        results.push_back(result);
        
        SDL_Log("  %s: %s", result.passed ? "PASS" : "FAIL", 
                result.error_message.empty() ? result.screenshot_path.c_str() : result.error_message.c_str());
    }
    
    // Summary
    int passed = 0, failed = 0;
    for (const auto& r : results) {
        if (r.passed) ++passed;
        else ++failed;
    }
    SDL_Log("\n=== Test Summary ===");
    SDL_Log("Passed: %d, Failed: %d, Total: %zu", passed, failed, results.size());
    
    return results;
}

void FeatureTestRunner::generateReport(const std::vector<FeatureTestResult>& results, 
                                         const std::string& output_path) {
    std::ofstream file(output_path);
    if (!file) return;
    
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    file << "<!DOCTYPE html>\n<html>\n<head>\n";
    file << "<meta charset=\"UTF-8\">\n";
    file << "<title>Feature Test Report</title>\n";
    file << "<style>\n";
    file << "body { font-family: Arial, sans-serif; margin: 20px; background: #1a1a2e; color: #eee; }\n";
    file << "h1 { color: #e94560; }\n";
    file << ".summary { background: #16213e; padding: 15px; border-radius: 8px; margin-bottom: 20px; }\n";
    file << ".test { background: #16213e; padding: 15px; border-radius: 8px; margin-bottom: 10px; }\n";
    file << ".pass { border-left: 4px solid #00ff88; }\n";
    file << ".fail { border-left: 4px solid #e94560; }\n";
    file << ".category { color: #00d4ff; font-size: 12px; }\n";
    file << ".name { font-weight: bold; margin: 5px 0; }\n";
    file << ".screenshot { max-width: 400px; margin-top: 10px; border-radius: 4px; }\n";
    file << ".error { color: #e94560; font-size: 12px; }\n";
    file << "</style>\n</head>\n<body>\n";
    
    file << "<h1>Feature Test Report</h1>\n";
    file << "<p>Generated: " << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "</p>\n";
    
    // Summary
    int passed = 0, failed = 0;
    for (const auto& r : results) {
        if (r.passed) ++passed;
        else ++failed;
    }
    
    file << "<div class=\"summary\">\n";
    file << "<h2>Summary</h2>\n";
    file << "<p>Total: " << results.size() << " | ";
    file << "Passed: <span style=\"color:#00ff88\">" << passed << "</span> | ";
    file << "Failed: <span style=\"color:#e94560\">" << failed << "</span></p>\n";
    file << "</div>\n";
    
    // Group by category
    std::map<std::string, std::vector<const FeatureTestResult*>> by_category;
    for (const auto& r : results) {
        by_category[r.category].push_back(&r);
    }
    
    for (const auto& [category, tests] : by_category) {
        file << "<h2>" << category << "</h2>\n";
        
        for (const auto* r : tests) {
            file << "<div class=\"test " << (r->passed ? "pass" : "fail") << "\">\n";
            file << "<div class=\"category\">" << r->category << "</div>\n";
            file << "<div class=\"name\">" << r->name << " - " << (r->passed ? "PASS" : "FAIL") << "</div>\n";
            if (!r->description.empty()) {
                file << "<div>" << r->description << "</div>\n";
            }
            if (!r->error_message.empty()) {
                file << "<div class=\"error\">" << r->error_message << "</div>\n";
            }
            if (!r->screenshot_path.empty()) {
                // Use relative path
                std::filesystem::path p(r->screenshot_path);
                file << "<img class=\"screenshot\" src=\"" << p.filename().string() << "\" alt=\"screenshot\">\n";
            }
            file << "</div>\n";
        }
    }
    
    file << "</body>\n</html>\n";
}

// Pre-defined test cases
namespace tests {

std::vector<FeatureTestCase> getBoxModelTests() {
    return {
        {
            "margin_padding",
            "BoxModel",
            R"(<!DOCTYPE html>
<html><head><style>
body { margin: 0; padding: 20px; background: #1a1a2e; }
.box { width: 100px; height: 100px; background: #e94560; margin: 20px; padding: 15px; }
.inner { width: 100%; height: 100%; background: #00d4ff; }
</style></head><body>
<div class="box"><div class="inner"></div></div>
</body></html>)",
            "Tests margin and padding properties"
        },
        {
            "border_radius",
            "BoxModel",
            R"(<!DOCTYPE html>
<html><head><style>
body { margin: 0; padding: 20px; background: #1a1a2e; display: flex; gap: 20px; }
.box { width: 80px; height: 80px; background: #e94560; }
.r1 { border-radius: 10px; }
.r2 { border-radius: 20px; }
.r3 { border-radius: 50%; }
.r4 { border-radius: 40px 10px; }
</style></head><body>
<div class="box r1"></div>
<div class="box r2"></div>
<div class="box r3"></div>
<div class="box r4"></div>
</body></html>)",
            "Tests border-radius variations"
        },
        {
            "box_shadow",
            "BoxModel",
            R"(<!DOCTYPE html>
<html><head><style>
body { margin: 0; padding: 40px; background: #1a1a2e; display: flex; gap: 40px; }
.box { width: 80px; height: 80px; background: #16213e; border-radius: 8px; }
.s1 { box-shadow: 5px 5px 10px rgba(0,0,0,0.5); }
.s2 { box-shadow: 0 0 20px #e94560; }
.s3 { box-shadow: inset 0 0 10px rgba(0,0,0,0.8); }
.s4 { box-shadow: 5px 5px 0 #00d4ff, 10px 10px 0 #e94560; }
</style></head><body>
<div class="box s1"></div>
<div class="box s2"></div>
<div class="box s3"></div>
<div class="box s4"></div>
</body></html>)",
            "Tests box-shadow variations"
        }
    };
}

std::vector<FeatureTestCase> getFlexboxTests() {
    return {
        {
            "flex_direction",
            "Flexbox",
            R"(<!DOCTYPE html>
<html><head><style>
body { margin: 0; padding: 20px; background: #1a1a2e; }
.container { display: flex; gap: 10px; margin-bottom: 20px; padding: 10px; background: #16213e; }
.row { flex-direction: row; }
.column { flex-direction: column; }
.box { width: 50px; height: 50px; background: #e94560; border-radius: 4px; }
</style></head><body>
<div class="container row">
  <div class="box"></div><div class="box"></div><div class="box"></div>
</div>
<div class="container column">
  <div class="box"></div><div class="box"></div><div class="box"></div>
</div>
</body></html>)",
            "Tests flex-direction row and column"
        },
        {
            "justify_content",
            "Flexbox",
            R"(<!DOCTYPE html>
<html><head><style>
body { margin: 0; padding: 20px; background: #1a1a2e; }
.container { display: flex; height: 60px; margin-bottom: 10px; padding: 5px; background: #16213e; }
.start { justify-content: flex-start; }
.center { justify-content: center; }
.end { justify-content: flex-end; }
.between { justify-content: space-between; }
.around { justify-content: space-around; }
.box { width: 40px; height: 40px; background: #e94560; border-radius: 4px; }
</style></head><body>
<div class="container start"><div class="box"></div><div class="box"></div><div class="box"></div></div>
<div class="container center"><div class="box"></div><div class="box"></div><div class="box"></div></div>
<div class="container end"><div class="box"></div><div class="box"></div><div class="box"></div></div>
<div class="container between"><div class="box"></div><div class="box"></div><div class="box"></div></div>
<div class="container around"><div class="box"></div><div class="box"></div><div class="box"></div></div>
</body></html>)",
            "Tests justify-content variations"
        },
        {
            "align_items",
            "Flexbox",
            R"(<!DOCTYPE html>
<html><head><style>
body { margin: 0; padding: 20px; background: #1a1a2e; display: flex; gap: 10px; }
.container { display: flex; width: 100px; height: 120px; padding: 5px; background: #16213e; }
.start { align-items: flex-start; }
.center { align-items: center; }
.end { align-items: flex-end; }
.stretch { align-items: stretch; }
.box { width: 30px; background: #e94560; border-radius: 4px; }
.box1 { height: 30px; }
.box2 { height: 50px; }
.box3 { height: 40px; }
</style></head><body>
<div class="container start"><div class="box box1"></div><div class="box box2"></div><div class="box box3"></div></div>
<div class="container center"><div class="box box1"></div><div class="box box2"></div><div class="box box3"></div></div>
<div class="container end"><div class="box box1"></div><div class="box box2"></div><div class="box box3"></div></div>
<div class="container stretch"><div class="box"></div><div class="box"></div><div class="box"></div></div>
</body></html>)",
            "Tests align-items variations"
        },
        {
            "flex_wrap",
            "Flexbox",
            R"(<!DOCTYPE html>
<html><head><style>
body { margin: 0; padding: 20px; background: #1a1a2e; }
.container { display: flex; flex-wrap: wrap; gap: 10px; width: 200px; padding: 10px; background: #16213e; }
.box { width: 50px; height: 50px; background: #e94560; border-radius: 4px; }
</style></head><body>
<div class="container">
  <div class="box"></div><div class="box"></div><div class="box"></div>
  <div class="box"></div><div class="box"></div><div class="box"></div>
</div>
</body></html>)",
            "Tests flex-wrap"
        }
    };
}

std::vector<FeatureTestCase> getTransformTests() {
    return {
        {
            "rotate",
            "Transform",
            R"(<!DOCTYPE html>
<html><head><style>
body { margin: 0; padding: 40px; background: #1a1a2e; display: flex; gap: 40px; }
.box { width: 60px; height: 60px; background: #e94560; border-radius: 4px; }
.r15 { transform: rotate(15deg); }
.r45 { transform: rotate(45deg); }
.r90 { transform: rotate(90deg); }
.rn30 { transform: rotate(-30deg); }
</style></head><body>
<div class="box r15"></div>
<div class="box r45"></div>
<div class="box r90"></div>
<div class="box rn30"></div>
</body></html>)",
            "Tests transform: rotate()"
        },
        {
            "scale",
            "Transform",
            R"(<!DOCTYPE html>
<html><head><style>
body { margin: 0; padding: 60px; background: #1a1a2e; display: flex; gap: 60px; }
.box { width: 40px; height: 40px; background: #e94560; border-radius: 4px; }
.s05 { transform: scale(0.5); }
.s10 { transform: scale(1); }
.s15 { transform: scale(1.5); }
.sxy { transform: scale(1.5, 0.8); }
</style></head><body>
<div class="box s05"></div>
<div class="box s10"></div>
<div class="box s15"></div>
<div class="box sxy"></div>
</body></html>)",
            "Tests transform: scale()"
        },
        {
            "translate",
            "Transform",
            R"(<!DOCTYPE html>
<html><head><style>
body { margin: 0; padding: 40px; background: #1a1a2e; }
.container { position: relative; width: 200px; height: 200px; background: #16213e; }
.box { position: absolute; width: 40px; height: 40px; background: #e94560; border-radius: 4px; }
.t1 { transform: translate(20px, 20px); }
.t2 { transform: translateX(100px); }
.t3 { transform: translateY(100px); }
.t4 { transform: translate(100px, 100px); }
</style></head><body>
<div class="container">
  <div class="box t1"></div>
  <div class="box t2"></div>
  <div class="box t3"></div>
  <div class="box t4"></div>
</div>
</body></html>)",
            "Tests transform: translate()"
        },
        {
            "skew",
            "Transform",
            R"(<!DOCTYPE html>
<html><head><style>
body { margin: 0; padding: 40px; background: #1a1a2e; display: flex; gap: 40px; }
.box { width: 60px; height: 60px; background: #e94560; border-radius: 4px; }
.sx { transform: skewX(15deg); }
.sy { transform: skewY(15deg); }
.sxy { transform: skew(10deg, 5deg); }
</style></head><body>
<div class="box sx"></div>
<div class="box sy"></div>
<div class="box sxy"></div>
</body></html>)",
            "Tests transform: skew()"
        },
        {
            "combined",
            "Transform",
            R"(<!DOCTYPE html>
<html><head><style>
body { margin: 0; padding: 60px; background: #1a1a2e; display: flex; gap: 60px; }
.box { width: 50px; height: 50px; background: #e94560; border-radius: 4px; }
.c1 { transform: rotate(15deg) scale(1.2); }
.c2 { transform: translate(10px, 10px) rotate(45deg); }
.c3 { transform: scale(0.8) skewX(10deg) rotate(-15deg); }
</style></head><body>
<div class="box c1"></div>
<div class="box c2"></div>
<div class="box c3"></div>
</body></html>)",
            "Tests combined transforms"
        }
    };
}

std::vector<FeatureTestCase> getVisualEffectsTests() {
    return {
        {
            "linear_gradient",
            "Gradient",
            R"(<!DOCTYPE html>
<html><head><style>
body { margin: 0; padding: 20px; background: #1a1a2e; display: flex; gap: 20px; flex-wrap: wrap; }
.box { width: 100px; height: 100px; border-radius: 8px; }
.g1 { background: linear-gradient(to right, #e94560, #00d4ff); }
.g2 { background: linear-gradient(45deg, #ff6b35, #ffd93d); }
.g3 { background: linear-gradient(to bottom, #7b2cbf, #e94560, #ff6b35); }
.g4 { background: linear-gradient(135deg, #00ff88 0%, #00d4ff 50%, #9d4edd 100%); }
</style></head><body>
<div class="box g1"></div>
<div class="box g2"></div>
<div class="box g3"></div>
<div class="box g4"></div>
</body></html>)",
            "Tests linear-gradient"
        },
        {
            "radial_gradient",
            "Gradient",
            R"(<!DOCTYPE html>
<html><head><style>
body { margin: 0; padding: 20px; background: #1a1a2e; display: flex; gap: 20px; }
.box { width: 100px; height: 100px; border-radius: 8px; }
.g1 { background: radial-gradient(circle, #e94560, #1a1a2e); }
.g2 { background: radial-gradient(ellipse, #00d4ff, #16213e); }
.g3 { background: radial-gradient(circle at 30% 30%, #ffd93d, #ff6b35, #1a1a2e); }
</style></head><body>
<div class="box g1"></div>
<div class="box g2"></div>
<div class="box g3"></div>
</body></html>)",
            "Tests radial-gradient"
        },
        {
            "filter_blur",
            "Filter",
            R"(<!DOCTYPE html>
<html><head><style>
body { margin: 0; padding: 20px; background: #1a1a2e; display: flex; gap: 20px; }
.box { width: 80px; height: 80px; background: #e94560; border-radius: 8px; }
.b0 { filter: blur(0px); }
.b2 { filter: blur(2px); }
.b5 { filter: blur(5px); }
.b10 { filter: blur(10px); }
</style></head><body>
<div class="box b0"></div>
<div class="box b2"></div>
<div class="box b5"></div>
<div class="box b10"></div>
</body></html>)",
            "Tests filter: blur()"
        },
        {
            "filter_effects",
            "Filter",
            R"(<!DOCTYPE html>
<html><head><style>
body { margin: 0; padding: 20px; background: #1a1a2e; display: flex; gap: 20px; flex-wrap: wrap; }
.box { width: 80px; height: 80px; background: #e94560; border-radius: 8px; }
.gray { filter: grayscale(100%); }
.sepia { filter: sepia(100%); }
.bright { filter: brightness(1.5); }
.contrast { filter: contrast(1.5); }
.invert { filter: invert(100%); }
.saturate { filter: saturate(2); }
</style></head><body>
<div class="box gray"></div>
<div class="box sepia"></div>
<div class="box bright"></div>
<div class="box contrast"></div>
<div class="box invert"></div>
<div class="box saturate"></div>
</body></html>)",
            "Tests various filter effects"
        },
        {
            "opacity",
            "Visual",
            R"(<!DOCTYPE html>
<html><head><style>
body { margin: 0; padding: 20px; background: #1a1a2e; display: flex; gap: 20px; }
.box { width: 80px; height: 80px; background: #e94560; border-radius: 8px; }
.o100 { opacity: 1.0; }
.o75 { opacity: 0.75; }
.o50 { opacity: 0.5; }
.o25 { opacity: 0.25; }
</style></head><body>
<div class="box o100"></div>
<div class="box o75"></div>
<div class="box o50"></div>
<div class="box o25"></div>
</body></html>)",
            "Tests opacity"
        }
    };
}

std::vector<FeatureTestCase> getTextTests() {
    return {
        {
            "font_size",
            "Text",
            R"(<!DOCTYPE html>
<html><head><style>
body { margin: 0; padding: 20px; background: #1a1a2e; color: #eee; font-family: Arial; }
.s12 { font-size: 12px; }
.s16 { font-size: 16px; }
.s24 { font-size: 24px; }
.s32 { font-size: 32px; }
</style></head><body>
<p class="s12">Font size 12px</p>
<p class="s16">Font size 16px</p>
<p class="s24">Font size 24px</p>
<p class="s32">Font size 32px</p>
</body></html>)",
            "Tests font-size"
        },
        {
            "font_weight",
            "Text",
            R"(<!DOCTYPE html>
<html><head><style>
body { margin: 0; padding: 20px; background: #1a1a2e; color: #eee; font-family: Arial; font-size: 18px; }
.normal { font-weight: normal; }
.bold { font-weight: bold; }
.w300 { font-weight: 300; }
.w700 { font-weight: 700; }
</style></head><body>
<p class="normal">Normal weight</p>
<p class="bold">Bold weight</p>
<p class="w300">Weight 300</p>
<p class="w700">Weight 700</p>
</body></html>)",
            "Tests font-weight"
        },
        {
            "text_align",
            "Text",
            R"(<!DOCTYPE html>
<html><head><style>
body { margin: 0; padding: 20px; background: #1a1a2e; color: #eee; font-family: Arial; }
.box { width: 200px; padding: 10px; background: #16213e; margin-bottom: 10px; }
.left { text-align: left; }
.center { text-align: center; }
.right { text-align: right; }
</style></head><body>
<div class="box left">Left aligned</div>
<div class="box center">Center aligned</div>
<div class="box right">Right aligned</div>
</body></html>)",
            "Tests text-align"
        },
        {
            "text_decoration",
            "Text",
            R"(<!DOCTYPE html>
<html><head><style>
body { margin: 0; padding: 20px; background: #1a1a2e; color: #eee; font-family: Arial; font-size: 18px; }
.underline { text-decoration: underline; }
.overline { text-decoration: overline; }
.linethrough { text-decoration: line-through; }
.none { text-decoration: none; }
</style></head><body>
<p class="underline">Underlined text</p>
<p class="overline">Overlined text</p>
<p class="linethrough">Line-through text</p>
<p class="none">No decoration</p>
</body></html>)",
            "Tests text-decoration"
        },
        {
            "text_shadow",
            "Text",
            R"(<!DOCTYPE html>
<html><head><style>
body { margin: 0; padding: 20px; background: #1a1a2e; color: #eee; font-family: Arial; font-size: 24px; }
.s1 { text-shadow: 2px 2px 4px rgba(0,0,0,0.5); }
.s2 { text-shadow: 0 0 10px #e94560; }
.s3 { text-shadow: 1px 1px 0 #e94560, 2px 2px 0 #00d4ff; }
</style></head><body>
<p class="s1">Drop shadow</p>
<p class="s2">Glow effect</p>
<p class="s3">Multiple shadows</p>
</body></html>)",
            "Tests text-shadow"
        }
    };
}

std::vector<FeatureTestCase> getAnimationTests() {
    // Note: These are static snapshots, animations would need frame-by-frame testing
    return {
        {
            "transition_property",
            "Animation",
            R"(<!DOCTYPE html>
<html><head><style>
body { margin: 0; padding: 20px; background: #1a1a2e; color: #eee; font-family: Arial; }
.box { 
  width: 100px; height: 100px; 
  background: #e94560; 
  border-radius: 8px;
  transition: all 0.3s ease;
}
.info { margin-top: 10px; font-size: 12px; color: #888; }
</style></head><body>
<div class="box"></div>
<p class="info">Transition: all 0.3s ease (hover to see effect)</p>
</body></html>)",
            "Tests transition property parsing"
        },
        {
            "keyframes_definition",
            "Animation",
            R"(<!DOCTYPE html>
<html><head><style>
body { margin: 0; padding: 20px; background: #1a1a2e; color: #eee; font-family: Arial; }
@keyframes pulse {
  0% { transform: scale(1); opacity: 1; }
  50% { transform: scale(1.1); opacity: 0.8; }
  100% { transform: scale(1); opacity: 1; }
}
.box { 
  width: 100px; height: 100px; 
  background: #e94560; 
  border-radius: 8px;
  animation: pulse 2s infinite;
}
.info { margin-top: 10px; font-size: 12px; color: #888; }
</style></head><body>
<div class="box"></div>
<p class="info">Animation: pulse 2s infinite</p>
</body></html>)",
            "Tests @keyframes definition"
        }
    };
}

std::vector<FeatureTestCase> getDOMAPITests() {
    return {
        {
            "classlist_api",
            "DOM",
            R"(<!DOCTYPE html>
<html><head><style>
body { margin: 0; padding: 20px; background: #1a1a2e; color: #eee; font-family: Arial; }
.box { width: 100px; height: 100px; background: #16213e; border-radius: 8px; margin-bottom: 10px; }
.active { background: #e94560; }
.highlight { box-shadow: 0 0 10px #00d4ff; }
</style></head>
<body>
<div id="box1" class="box"></div>
<div id="box2" class="box active"></div>
<div id="box3" class="box active highlight"></div>
<script>
// classList API test (visual result shows initial state)
</script>
</body></html>)",
            "Tests classList API visual result"
        },
        {
            "query_selector",
            "DOM",
            R"(<!DOCTYPE html>
<html><head><style>
body { margin: 0; padding: 20px; background: #1a1a2e; color: #eee; font-family: Arial; }
.container { display: flex; gap: 10px; }
.box { width: 50px; height: 50px; background: #16213e; border-radius: 4px; }
.box:first-child { background: #e94560; }
.box:last-child { background: #00d4ff; }
.box:nth-child(2) { background: #ffd93d; }
</style></head>
<body>
<div class="container">
  <div class="box"></div>
  <div class="box"></div>
  <div class="box"></div>
  <div class="box"></div>
  <div class="box"></div>
</div>
</body></html>)",
            "Tests querySelector with pseudo-selectors"
        }
    };
}

std::vector<FeatureTestCase> getAllTests() {
    std::vector<FeatureTestCase> all;
    
    auto boxModel = getBoxModelTests();
    all.insert(all.end(), boxModel.begin(), boxModel.end());
    
    auto flexbox = getFlexboxTests();
    all.insert(all.end(), flexbox.begin(), flexbox.end());
    
    auto transform = getTransformTests();
    all.insert(all.end(), transform.begin(), transform.end());
    
    auto visual = getVisualEffectsTests();
    all.insert(all.end(), visual.begin(), visual.end());
    
    auto text = getTextTests();
    all.insert(all.end(), text.begin(), text.end());
    
    auto animation = getAnimationTests();
    all.insert(all.end(), animation.begin(), animation.end());
    
    auto dom = getDOMAPITests();
    all.insert(all.end(), dom.begin(), dom.end());
    
    return all;
}

} // namespace tests

} // namespace dong::test
