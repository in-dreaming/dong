// Image Decoder Unit Tests
// Compile: cl /EHsc test_image_decoder.c /I../include /I../backends/sdl /I../third_party/sdl/src/video /link dong_sdl_backend.lib
// Or use CMake test target

#include "dong_image_decoder.h"
#include "dong_sdl_image_decoder.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// Test Framework (minimal)
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
// Test Data - Minimal PNG (1x1 red pixel)
// =============================================================================

// This is a valid 1x1 red PNG image
static const unsigned char g_test_png_1x1_red[] = {
    0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,  // PNG signature
    0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,  // IHDR chunk
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,  // width=1, height=1
    0x08, 0x02, 0x00, 0x00, 0x00, 0x90, 0x77, 0x53,  // bit_depth=8, color_type=2 (RGB)
    0xDE, 0x00, 0x00, 0x00, 0x0C, 0x49, 0x44, 0x41,  // IDAT chunk
    0x54, 0x08, 0xD7, 0x63, 0xF8, 0xCF, 0xC0, 0x00,  // compressed pixel data
    0x00, 0x00, 0x03, 0x00, 0x01, 0x00, 0x18, 0xDD,
    0x8D, 0xB4, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45,  // IEND chunk
    0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82
};
static const size_t g_test_png_1x1_red_size = sizeof(g_test_png_1x1_red);

// =============================================================================
// Tests
// =============================================================================

TEST(decoder_create_destroy) {
    DongImageDecoder* decoder = dong_sdl_image_decoder_create();
    ASSERT(decoder != NULL);
    ASSERT(decoder->vtable != NULL);

    dong_sdl_image_decoder_destroy(decoder);
}

TEST(format_utilities) {
    // Test block size
    uint32_t bw, bh;

    dong_image_format_block_size(DONG_IMAGE_FORMAT_RGBA8, &bw, &bh);
    ASSERT_EQ(bw, 1);
    ASSERT_EQ(bh, 1);

    dong_image_format_block_size(DONG_IMAGE_FORMAT_ASTC_4x4, &bw, &bh);
    ASSERT_EQ(bw, 4);
    ASSERT_EQ(bh, 4);

    dong_image_format_block_size(DONG_IMAGE_FORMAT_ASTC_8x8, &bw, &bh);
    ASSERT_EQ(bw, 8);
    ASSERT_EQ(bh, 8);

    dong_image_format_block_size(DONG_IMAGE_FORMAT_BC7, &bw, &bh);
    ASSERT_EQ(bw, 4);
    ASSERT_EQ(bh, 4);

    // Test bytes per block
    ASSERT_EQ(dong_image_format_bytes_per_block(DONG_IMAGE_FORMAT_RGBA8), 4);
    ASSERT_EQ(dong_image_format_bytes_per_block(DONG_IMAGE_FORMAT_RGB8), 3);
    ASSERT_EQ(dong_image_format_bytes_per_block(DONG_IMAGE_FORMAT_ASTC_4x4), 16);
    ASSERT_EQ(dong_image_format_bytes_per_block(DONG_IMAGE_FORMAT_BC7), 16);
    ASSERT_EQ(dong_image_format_bytes_per_block(DONG_IMAGE_FORMAT_BC1), 8);

    // Test size calculation
    size_t size;

    // 256x256 RGBA8 = 256 * 256 * 4 = 262144
    size = dong_image_format_calc_size(DONG_IMAGE_FORMAT_RGBA8, 256, 256);
    ASSERT_EQ(size, 262144);

    // 256x256 ASTC 4x4 = (256/4) * (256/4) * 16 = 64 * 64 * 16 = 65536
    size = dong_image_format_calc_size(DONG_IMAGE_FORMAT_ASTC_4x4, 256, 256);
    ASSERT_EQ(size, 65536);

    // 256x256 BC1 = (256/4) * (256/4) * 8 = 64 * 64 * 8 = 32768
    size = dong_image_format_calc_size(DONG_IMAGE_FORMAT_BC1, 256, 256);
    ASSERT_EQ(size, 32768);

    // Test is_compressed
    ASSERT_EQ(dong_image_format_is_compressed(DONG_IMAGE_FORMAT_RGBA8), 0);
    ASSERT_EQ(dong_image_format_is_compressed(DONG_IMAGE_FORMAT_ASTC_4x4), 1);
    ASSERT_EQ(dong_image_format_is_compressed(DONG_IMAGE_FORMAT_BC7), 1);

    // Test is_astc/is_bc
    ASSERT_EQ(dong_image_format_is_astc(DONG_IMAGE_FORMAT_ASTC_4x4), 1);
    ASSERT_EQ(dong_image_format_is_astc(DONG_IMAGE_FORMAT_BC7), 0);
    ASSERT_EQ(dong_image_format_is_bc(DONG_IMAGE_FORMAT_BC7), 1);
    ASSERT_EQ(dong_image_format_is_bc(DONG_IMAGE_FORMAT_ASTC_4x4), 0);
}

TEST(can_decode_png) {
    DongImageDecoder* decoder = dong_sdl_image_decoder_create();
    ASSERT(decoder != NULL);

    // Should recognize PNG by magic
    int can = dong_image_can_decode(decoder, g_test_png_1x1_red, g_test_png_1x1_red_size, NULL);
    ASSERT_EQ(can, 1);

    // Should also recognize by extension
    can = dong_image_can_decode(decoder, "dummy", 5, "test.png");
    ASSERT_EQ(can, 1);

    // Should not recognize random data
    const char* random_data = "not an image";
    can = dong_image_can_decode(decoder, random_data, strlen(random_data), NULL);
    ASSERT_EQ(can, 0);

    dong_sdl_image_decoder_destroy(decoder);
}

TEST(decode_png) {
    DongImageDecoder* decoder = dong_sdl_image_decoder_create();
    ASSERT(decoder != NULL);

    DongDecodedImage image = {0};
    DongImageDecoderResult result = dong_image_decode(decoder, g_test_png_1x1_red, g_test_png_1x1_red_size, &image);

    ASSERT_EQ(result, DONG_IMAGE_OK);
    ASSERT(image.data != NULL);
    ASSERT_EQ(image.width, 1);
    ASSERT_EQ(image.height, 1);
    ASSERT_EQ(image.format, DONG_IMAGE_FORMAT_RGBA8);
    ASSERT_EQ(image.row_bytes, 4);
    ASSERT_EQ(image.mip_levels, 1);

    // Verify pixel data is red (stb_image outputs RGBA)
    const unsigned char* pixels = (const unsigned char*)image.data;
    // Note: The test PNG is RGB, so alpha will be 255
    ASSERT(pixels[0] >= 200); // R should be high
    // G and B values depend on the actual PNG encoding

    dong_image_free(decoder, &image);
    dong_sdl_image_decoder_destroy(decoder);
}

TEST(decode_invalid_data) {
    DongImageDecoder* decoder = dong_sdl_image_decoder_create();
    ASSERT(decoder != NULL);

    DongDecodedImage image = {0};
    const char* invalid = "not an image";

    DongImageDecoderResult result = dong_image_decode(decoder, invalid, strlen(invalid), &image);
    ASSERT(result != DONG_IMAGE_OK);
    ASSERT(image.data == NULL);

    dong_sdl_image_decoder_destroy(decoder);
}

TEST(encode_not_implemented) {
    DongImageDecoder* decoder = dong_sdl_image_decoder_create();
    ASSERT(decoder != NULL);

    // ASTC and BC7 encoding should now be supported
    int can = dong_image_can_encode(decoder, DONG_IMAGE_FORMAT_RGBA8, DONG_IMAGE_FORMAT_ASTC_4x4);
    ASSERT_EQ(can, 1);  // ASTC encoder available

    can = dong_image_can_encode(decoder, DONG_IMAGE_FORMAT_RGBA8, DONG_IMAGE_FORMAT_BC7);
    ASSERT_EQ(can, 1);  // BC7 encoder available

    // Unsupported: compressed -> compressed
    can = dong_image_can_encode(decoder, DONG_IMAGE_FORMAT_ASTC_4x4, DONG_IMAGE_FORMAT_BC7);
    ASSERT_EQ(can, 0);

    dong_sdl_image_decoder_destroy(decoder);
}

TEST(encode_astc_4x4) {
    DongImageDecoder* decoder = dong_sdl_image_decoder_create();
    ASSERT(decoder != NULL);

    // Create a test 8x8 RGBA image (solid red)
    uint8_t test_pixels[8 * 8 * 4];
    for (int i = 0; i < 8 * 8; i++) {
        test_pixels[i * 4 + 0] = 255;  // R
        test_pixels[i * 4 + 1] = 0;    // G
        test_pixels[i * 4 + 2] = 0;    // B
        test_pixels[i * 4 + 3] = 255;  // A
    }

    DongDecodedImage src = {0};
    src.data = test_pixels;
    src.data_size = sizeof(test_pixels);
    src.width = 8;
    src.height = 8;
    src.row_bytes = 8 * 4;
    src.format = DONG_IMAGE_FORMAT_RGBA8;
    src.mip_levels = 1;

    DongDecodedImage encoded = {0};
    DongEncodeOptions opts = dong_encode_options_default();

    DongImageDecoderResult result = dong_image_encode(decoder, &src, DONG_IMAGE_FORMAT_ASTC_4x4, &opts, &encoded);

    ASSERT_EQ(result, DONG_IMAGE_OK);
    ASSERT(encoded.data != NULL);
    ASSERT_EQ(encoded.format, DONG_IMAGE_FORMAT_ASTC_4x4);
    ASSERT_EQ(encoded.width, 8);
    ASSERT_EQ(encoded.height, 8);

    // 8x8 image with 4x4 blocks = 2x2 = 4 blocks * 16 bytes = 64 bytes
    size_t expected_size = dong_image_format_calc_size(DONG_IMAGE_FORMAT_ASTC_4x4, 8, 8);
    ASSERT_EQ(encoded.data_size, expected_size);
    ASSERT_EQ(expected_size, 64);

    dong_image_free(decoder, &encoded);
    dong_sdl_image_decoder_destroy(decoder);
}

TEST(encode_bc7) {
    DongImageDecoder* decoder = dong_sdl_image_decoder_create();
    ASSERT(decoder != NULL);

    // Create a test 8x8 RGBA image (gradient)
    uint8_t test_pixels[8 * 8 * 4];
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            int i = y * 8 + x;
            test_pixels[i * 4 + 0] = (uint8_t)(x * 32);  // R gradient
            test_pixels[i * 4 + 1] = (uint8_t)(y * 32);  // G gradient
            test_pixels[i * 4 + 2] = 128;                 // B constant
            test_pixels[i * 4 + 3] = 255;                 // A opaque
        }
    }

    DongDecodedImage src = {0};
    src.data = test_pixels;
    src.data_size = sizeof(test_pixels);
    src.width = 8;
    src.height = 8;
    src.row_bytes = 8 * 4;
    src.format = DONG_IMAGE_FORMAT_RGBA8;
    src.mip_levels = 1;

    DongDecodedImage encoded = {0};

    DongImageDecoderResult result = dong_image_encode(decoder, &src, DONG_IMAGE_FORMAT_BC7, NULL, &encoded);

    ASSERT_EQ(result, DONG_IMAGE_OK);
    ASSERT(encoded.data != NULL);
    ASSERT_EQ(encoded.format, DONG_IMAGE_FORMAT_BC7);
    ASSERT_EQ(encoded.width, 8);
    ASSERT_EQ(encoded.height, 8);

    // 8x8 image with 4x4 blocks = 2x2 = 4 blocks * 16 bytes = 64 bytes
    size_t expected_size = dong_image_format_calc_size(DONG_IMAGE_FORMAT_BC7, 8, 8);
    ASSERT_EQ(encoded.data_size, expected_size);
    ASSERT_EQ(expected_size, 64);

    dong_image_free(decoder, &encoded);
    dong_sdl_image_decoder_destroy(decoder);
}

TEST(format_names) {
    ASSERT(strcmp(dong_image_format_name(DONG_IMAGE_FORMAT_RGBA8), "RGBA8") == 0);
    ASSERT(strcmp(dong_image_format_name(DONG_IMAGE_FORMAT_ASTC_4x4), "ASTC_4x4") == 0);
    ASSERT(strcmp(dong_image_format_name(DONG_IMAGE_FORMAT_BC7), "BC7") == 0);
    ASSERT(strcmp(dong_image_format_name(DONG_IMAGE_FORMAT_UNKNOWN), "UNKNOWN") == 0);
}

// =============================================================================
// Main
// =============================================================================

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    printf("Image Decoder Tests\n");
    printf("===================\n\n");

    RUN_TEST(decoder_create_destroy);
    RUN_TEST(format_utilities);
    RUN_TEST(can_decode_png);
    RUN_TEST(decode_png);
    RUN_TEST(decode_invalid_data);
    RUN_TEST(encode_not_implemented);
    RUN_TEST(encode_astc_4x4);
    RUN_TEST(encode_bc7);
    RUN_TEST(format_names);

    printf("\n===================\n");
    printf("Results: %d/%d tests passed\n", g_tests_passed, g_tests_run);

    return (g_tests_passed == g_tests_run) ? 0 : 1;
}
