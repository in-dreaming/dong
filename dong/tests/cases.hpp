#pragma once

// Test case definition for dong_runner
// This file is auto-generated from cases.json for C++ embedding

struct TestCase {
    const char* name;
    bool needs_data_dir;
    bool set_layer_cache_env;
    int timeout_seconds; // 0 = use default
};

static const TestCase kTestCases[] = {
    {"3d_cube", false, false, 5},
    {"3d_screen", false, false, 5},
    {"3d_screen_html", true, false, 10},
    {"3d_screen_script", true, false, 10},

    {"gpu_screenshot_demo", false, false, 10},
    {"gpu_screenshot_demo_fontonly", false, false, 10},
    {"gpu_screenshot_demo_basic_layout", false, false, 10},
    {"gpu_screenshot_demo_glyph_stress", false, false, 15},
    {"gpu_screenshot_analysis", false, false, 10},

    {"interactive_demo", false, true, 5},

    {"integration_demo", false, false, 5},
    {"sdl_gpu_demo", false, false, 5},

    {"dong_app", false, false, 5},
};

static constexpr int kDefaultTimeoutSeconds = 10;
