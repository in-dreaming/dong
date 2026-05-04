/**
 * @file test_image_atlas.c
 * @brief Unit tests for ImageAtlas
 *
 * Note: These tests require a valid SDL_GPUDevice.
 * For CI without GPU, use mock tests or skip GPU-dependent tests.
 */

#include "dong_image_atlas.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// Test Framework (minimal, same as test_image_decoder.c)
// =============================================================================

static int g_tests_run = 0;
static int g_tests_passed = 0;

#define TEST(name) \
    static void test_##name(void); \
    static void run_test_##name(void) { \
        g_tests_run++; \
        printf("  Running: %s ... ", #name); \
        test_##name(); \
        g_tests_passed++; \
        printf("PASS\n"); \
    } \
    static void test_##name(void)

#define ASSERT(cond) \
    do { \
        if (!(cond)) { \
            printf("FAIL\n    Assert failed: %s\n    At %s:%d\n", #cond, __FILE__, __LINE__); \
            exit(1); \
        } \
    } while (0)

#define ASSERT_EQ(a, b) \
    do { \
        if ((a) != (b)) { \
            printf("FAIL\n    Assert failed: %s == %s\n    Got: %d vs %d\n    At %s:%d\n", \
                   #a, #b, (int)(a), (int)(b), __FILE__, __LINE__); \
            exit(1); \
        } \
    } while (0)

#define RUN_TEST(name) run_test_##name()

// =============================================================================
// Tests for Atlas Helper Functions
// =============================================================================

TEST(config_default) {
    DongAtlasConfig cfg = dong_atlas_config_default();
    ASSERT_EQ(cfg.width, 2048);
    ASSERT_EQ(cfg.height, 2048);
    ASSERT_EQ(cfg.max_pages, 4);
    ASSERT_EQ(cfg.format, DONG_IMAGE_FORMAT_RGBA8);
    ASSERT_EQ(cfg.padding, 2);
}

TEST(align_to_block_rgba8) {
    // RGBA8 has block size 1x1, so no alignment needed
    ASSERT_EQ(dong_atlas_align_to_block(0, DONG_IMAGE_FORMAT_RGBA8), 0);
    ASSERT_EQ(dong_atlas_align_to_block(1, DONG_IMAGE_FORMAT_RGBA8), 1);
    ASSERT_EQ(dong_atlas_align_to_block(100, DONG_IMAGE_FORMAT_RGBA8), 100);
    ASSERT_EQ(dong_atlas_align_to_block(255, DONG_IMAGE_FORMAT_RGBA8), 255);
}

TEST(align_to_block_astc_4x4) {
    // ASTC 4x4 has block size 4x4
    ASSERT_EQ(dong_atlas_align_to_block(0, DONG_IMAGE_FORMAT_ASTC_4x4), 0);
    ASSERT_EQ(dong_atlas_align_to_block(1, DONG_IMAGE_FORMAT_ASTC_4x4), 4);
    ASSERT_EQ(dong_atlas_align_to_block(4, DONG_IMAGE_FORMAT_ASTC_4x4), 4);
    ASSERT_EQ(dong_atlas_align_to_block(5, DONG_IMAGE_FORMAT_ASTC_4x4), 8);
    ASSERT_EQ(dong_atlas_align_to_block(7, DONG_IMAGE_FORMAT_ASTC_4x4), 8);
    ASSERT_EQ(dong_atlas_align_to_block(8, DONG_IMAGE_FORMAT_ASTC_4x4), 8);
    ASSERT_EQ(dong_atlas_align_to_block(100, DONG_IMAGE_FORMAT_ASTC_4x4), 100);
    ASSERT_EQ(dong_atlas_align_to_block(101, DONG_IMAGE_FORMAT_ASTC_4x4), 104);
}

TEST(align_to_block_astc_8x8) {
    // ASTC 8x8 has block size 8x8
    ASSERT_EQ(dong_atlas_align_to_block(0, DONG_IMAGE_FORMAT_ASTC_8x8), 0);
    ASSERT_EQ(dong_atlas_align_to_block(1, DONG_IMAGE_FORMAT_ASTC_8x8), 8);
    ASSERT_EQ(dong_atlas_align_to_block(8, DONG_IMAGE_FORMAT_ASTC_8x8), 8);
    ASSERT_EQ(dong_atlas_align_to_block(9, DONG_IMAGE_FORMAT_ASTC_8x8), 16);
    ASSERT_EQ(dong_atlas_align_to_block(15, DONG_IMAGE_FORMAT_ASTC_8x8), 16);
    ASSERT_EQ(dong_atlas_align_to_block(16, DONG_IMAGE_FORMAT_ASTC_8x8), 16);
}

TEST(align_to_block_bc7) {
    // BC7 has block size 4x4
    ASSERT_EQ(dong_atlas_align_to_block(0, DONG_IMAGE_FORMAT_BC7), 0);
    ASSERT_EQ(dong_atlas_align_to_block(1, DONG_IMAGE_FORMAT_BC7), 4);
    ASSERT_EQ(dong_atlas_align_to_block(4, DONG_IMAGE_FORMAT_BC7), 4);
    ASSERT_EQ(dong_atlas_align_to_block(5, DONG_IMAGE_FORMAT_BC7), 8);
}

TEST(min_alloc_size_rgba8) {
    uint32_t w, h;

    dong_atlas_min_alloc_size(DONG_IMAGE_FORMAT_RGBA8, 100, 50, &w, &h);
    ASSERT_EQ(w, 100);
    ASSERT_EQ(h, 50);

    dong_atlas_min_alloc_size(DONG_IMAGE_FORMAT_RGBA8, 1, 1, &w, &h);
    ASSERT_EQ(w, 1);
    ASSERT_EQ(h, 1);
}

TEST(min_alloc_size_astc_4x4) {
    uint32_t w, h;

    // Exact multiple of 4
    dong_atlas_min_alloc_size(DONG_IMAGE_FORMAT_ASTC_4x4, 100, 100, &w, &h);
    ASSERT_EQ(w, 100);
    ASSERT_EQ(h, 100);

    // Not multiple of 4 - should round up
    dong_atlas_min_alloc_size(DONG_IMAGE_FORMAT_ASTC_4x4, 101, 102, &w, &h);
    ASSERT_EQ(w, 104);
    ASSERT_EQ(h, 104);

    dong_atlas_min_alloc_size(DONG_IMAGE_FORMAT_ASTC_4x4, 1, 1, &w, &h);
    ASSERT_EQ(w, 4);
    ASSERT_EQ(h, 4);

    dong_atlas_min_alloc_size(DONG_IMAGE_FORMAT_ASTC_4x4, 5, 7, &w, &h);
    ASSERT_EQ(w, 8);
    ASSERT_EQ(h, 8);
}

TEST(min_alloc_size_bc7) {
    uint32_t w, h;

    dong_atlas_min_alloc_size(DONG_IMAGE_FORMAT_BC7, 64, 64, &w, &h);
    ASSERT_EQ(w, 64);
    ASSERT_EQ(h, 64);

    dong_atlas_min_alloc_size(DONG_IMAGE_FORMAT_BC7, 65, 63, &w, &h);
    ASSERT_EQ(w, 68);
    ASSERT_EQ(h, 64);
}

TEST(null_atlas_safety) {
    // All functions should handle NULL safely
    DongAtlasEntry entry = {0};

    ASSERT_EQ(dong_atlas_alloc(NULL, 100, 100, &entry), DONG_ATLAS_ERR_INVALID_ARG);
    ASSERT_EQ(dong_atlas_upload(NULL, &entry, "data", 4), DONG_ATLAS_ERR_INVALID_ARG);
    ASSERT(dong_atlas_get_texture(NULL, 0) == NULL);

    // These should not crash
    dong_atlas_clear(NULL);
    dong_atlas_destroy(NULL);
}

// =============================================================================
// Main
// =============================================================================

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    printf("Image Atlas Tests\n");
    printf("=================\n\n");

    printf("Helper function tests:\n");
    RUN_TEST(config_default);
    RUN_TEST(align_to_block_rgba8);
    RUN_TEST(align_to_block_astc_4x4);
    RUN_TEST(align_to_block_astc_8x8);
    RUN_TEST(align_to_block_bc7);
    RUN_TEST(min_alloc_size_rgba8);
    RUN_TEST(min_alloc_size_astc_4x4);
    RUN_TEST(min_alloc_size_bc7);
    RUN_TEST(null_atlas_safety);

    printf("\n=================\n");
    printf("Results: %d/%d tests passed\n", g_tests_passed, g_tests_run);
    printf("\nNote: GPU-dependent tests require SDL_GPUDevice and are skipped in CI.\n");

    return (g_tests_passed == g_tests_run) ? 0 : 1;
}
