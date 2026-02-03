cmake_minimum_required(VERSION 3.16)
project(dong_tests LANGUAGES C)

set(CMAKE_C_STANDARD 11)

# Test executable for image decoder
add_executable(test_image_decoder
    test_image_decoder.c
)

# Include paths
target_include_directories(test_image_decoder PRIVATE
    ${CMAKE_SOURCE_DIR}/../include
    ${CMAKE_SOURCE_DIR}/../backends/sdl
    ${CMAKE_SOURCE_DIR}/../third_party/sdl/src/video
)

# Link against dong_sdl_backend
# For standalone test, we compile the sources directly
target_sources(test_image_decoder PRIVATE
    ${CMAKE_SOURCE_DIR}/../backends/sdl/dong_sdl_image_decoder.c
    ${CMAKE_SOURCE_DIR}/../backends/sdl/dong_astc_encoder.c
    ${CMAKE_SOURCE_DIR}/../backends/sdl/dong_bc_encoder.c
)

# Math library (Linux)
if(UNIX AND NOT APPLE)
    target_link_libraries(test_image_decoder PRIVATE m)
endif()

# Enable testing
enable_testing()
add_test(NAME ImageDecoderTests COMMAND test_image_decoder)

# Test executable for image atlas (helper functions only, no GPU required)
add_executable(test_image_atlas
    test_image_atlas.c
)

target_include_directories(test_image_atlas PRIVATE
    ${CMAKE_SOURCE_DIR}/../include
)

add_test(NAME ImageAtlasTests COMMAND test_image_atlas)
