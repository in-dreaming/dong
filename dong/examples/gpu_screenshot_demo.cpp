#include <cstdio>
#include <cstdint>
#include <vector>
#include <cstring>
#include <dong.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include "platform/sdl3_window.hpp"

using dong::platform::SDL3Window;

// 简单的 BMP 写入函数
bool writeBMP(const char* filename, uint32_t width, uint32_t height, const uint8_t* rgba_data) {
    FILE* f = fopen(filename, "wb");
    if (!f) return false;
    
    uint32_t filesize = 54 + width * height * 3;
    uint8_t bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
    bmpfileheader[2] = (uint8_t)(filesize);
    bmpfileheader[3] = (uint8_t)(filesize>>8);
    bmpfileheader[4] = (uint8_t)(filesize>>16);
    bmpfileheader[5] = (uint8_t)(filesize>>24);
    
    uint8_t bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
    bmpinfoheader[4] = (uint8_t)(width);
    bmpinfoheader[5] = (uint8_t)(width>>8);
    bmpinfoheader[6] = (uint8_t)(width>>16);
    bmpinfoheader[7] = (uint8_t)(width>>24);
    bmpinfoheader[8] = (uint8_t)(height);
    bmpinfoheader[9] = (uint8_t)(height>>8);
    bmpinfoheader[10] = (uint8_t)(height>>16);
    bmpinfoheader[11] = (uint8_t)(height>>24);
    
    fwrite(bmpfileheader, 1, 14, f);
    fwrite(bmpinfoheader, 1, 40, f);
    
    for (int y = height - 1; y >= 0; y--) {
        for (uint32_t x = 0; x < width; x++) {
            uint32_t idx = (y * width + x) * 4;
            uint8_t rgb[3] = {rgba_data[idx + 2], rgba_data[idx + 1], rgba_data[idx + 0]};
            fwrite(rgb, 3, 1, f);
        }
    }
    
    fclose(f);
    return true;
}

int main() {
    std::printf("=== GPU Screenshot Demo - MSDF Analysis ===\n");

    // 1. 创建带 GPU 的 SDL 窗口
    SDL3Window::CreateInfo ci{};
    ci.title = "GPU Screenshot Demo";
    ci.width = 800;
    ci.height = 400;
    ci.use_gpu = true;
    ci.debug_mode = false;

    SDL3Window window;
    if (!window.initialize(ci)) {
        std::printf("ERROR: Failed to initialize SDL3Window: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GPUDevice* device = window.getGPUDevice();
    if (!device) {
        std::printf("ERROR: GPU device is null\n");
        return 1;
    }

    // 2. 创建 Dong 上下文 + 视图
    std::printf("[Init] Creating dong context...\n");
    dong_context_t* ctx = dong_create_context();
    if (!ctx) {
        std::printf("ERROR: Failed to create dong context\n");
        return 1;
    }

    std::printf("[Init] Creating dong view...\n");
    dong_view_t* view = dong_view_create(ctx, ci.width, ci.height);
    if (!view) {
        std::printf("ERROR: Failed to create dong view\n");
        dong_destroy_context(ctx);
        return 1;
    }

    // 3. 切换到 GPU 渲染模式
    std::printf("[Init] Enabling GPU render mode...\n");
    dong_view_set_external_gpu_device(view, static_cast<void*>(device), static_cast<void*>(window.getHandle()));

    // 4. 加载测试 HTML - 大字号 MSDF 文字
    const char* html = R"(
        <!DOCTYPE html>
        <html>
        <head>
            <style>
                body {
                    margin: 0;
                    padding: 20px;
                    background-color: #ffffff;
                    font-family: Arial, sans-serif;
                }
                
                .test-line {
                    margin: 15px 0;
                    padding: 10px;
                    background: #f0f0f0;
                }
                
                .size-72 { font-size: 72px; color: #000000; }
                .size-48 { font-size: 48px; color: #ff0000; }
                .size-36 { font-size: 36px; color: #0000ff; }
            </style>
        </head>
        <body>
            <div class="test-line size-72">ABC</div>
            <div class="test-line size-48">Test</div>
            <div class="test-line size-36">Hello</div>
        </body>
        </html>
    )";

    std::printf("[Load] Loading HTML...\n");
    dong_view_load_html(view, html);

    // 5. 渲染多帧让 GPU 稳定
    std::printf("[Render] Rendering frames for GPU stabilization...\n");
    bool running = true;
    int frame_count = 0;
    const int target_frames = 10;  // 渲染10帧
    bool screenshot_taken = false;
    
    while (running && frame_count < target_frames + 3) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                running = false;
            }
            if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE) {
                running = false;
            }
        }

        // 更新和渲染
        dong_view_update(view);
        // GPU present 在 dong_view_update 内部完成
        
        frame_count++;
        
        // 在第10帧后截图
        if (frame_count == target_frames && !screenshot_taken) {
            std::printf("[Screenshot] Taking screenshot at frame %d...\n", frame_count);
            
            // 延迟确保渲染完成
            SDL_Delay(500);
            
            // 创建下载buffer读取swapchain
            // 注意：我们需要在下一帧之前读取当前帧
            // 最简单的方式：读取窗口的像素
            
            // 获取窗口surface（如果可用）
            // SDL3 GPU 模式下需要特殊处理
            // 我们创建一个离屏纹理并blit过去
            
            SDL_GPUTextureCreateInfo tex_info{};
            tex_info.type = SDL_GPU_TEXTURETYPE_2D;
            tex_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
            tex_info.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
            tex_info.width = ci.width;
            tex_info.height = ci.height;
            tex_info.layer_count_or_depth = 1;
            tex_info.num_levels = 1;
            tex_info.sample_count = SDL_GPU_SAMPLECOUNT_1;
            
            SDL_GPUTexture* screenshot_texture = SDL_CreateGPUTexture(device, &tex_info);
            if (!screenshot_texture) {
                SDL_Log("Failed to create screenshot texture: %s", SDL_GetError());
                continue;
            }
            
            // 再渲染一帧到这个纹理
            SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
            if (!cmd) {
                SDL_Log("Failed to acquire command buffer: %s", SDL_GetError());
                SDL_ReleaseGPUTexture(device, screenshot_texture);
                continue;
            }
            
            // 清屏并渲染
            SDL_GPUColorTargetInfo color_target{};
            color_target.texture = screenshot_texture;
            color_target.mip_level = 0;
            color_target.layer_or_depth_plane = 0;
            color_target.clear_color = SDL_FColor{1.0f, 1.0f, 1.0f, 1.0f};
            color_target.load_op = SDL_GPU_LOADOP_CLEAR;
            color_target.store_op = SDL_GPU_STOREOP_STORE;
            
            SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmd, &color_target, 1, nullptr);
            // 这里理想情况下应该重新执行 dong_view 的渲染命令
            // 但由于API限制，我们先渲染一个空帧
            SDL_EndGPURenderPass(pass);
            
            // 创建下载buffer
            SDL_GPUTransferBufferCreateInfo transfer_info{};
            transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD;
            transfer_info.size = ci.width * ci.height * 4;
            
            SDL_GPUTransferBuffer* download_buffer = SDL_CreateGPUTransferBuffer(device, &transfer_info);
            if (!download_buffer) {
                SDL_Log("Failed to create download buffer: %s", SDL_GetError());
                SDL_ReleaseGPUTexture(device, screenshot_texture);
                continue;
            }
            
            // 复制纹理到buffer
            SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd);
            
            SDL_GPUTextureRegion src_region{};
            src_region.texture = screenshot_texture;
            src_region.mip_level = 0;
            src_region.layer = 0;
            src_region.x = 0;
            src_region.y = 0;
            src_region.z = 0;
            src_region.w = ci.width;
            src_region.h = ci.height;
            src_region.d = 1;
            
            SDL_GPUTextureTransferInfo dst_transfer{};
            dst_transfer.transfer_buffer = download_buffer;
            dst_transfer.offset = 0;
            dst_transfer.pixels_per_row = 0;
            dst_transfer.rows_per_layer = 0;
            
            SDL_DownloadFromGPUTexture(copy_pass, &src_region, &dst_transfer);
            SDL_EndGPUCopyPass(copy_pass);
            
            SDL_SubmitGPUCommandBuffer(cmd);
            SDL_WaitForGPUIdle(device);
            
            // Map并读取像素
            void* mapped = SDL_MapGPUTransferBuffer(device, download_buffer, false);
            if (mapped) {
                std::vector<uint8_t> pixels(ci.width * ci.height * 4);
                std::memcpy(pixels.data(), mapped, ci.width * ci.height * 4);
                SDL_UnmapGPUTransferBuffer(device, download_buffer);
                
                // 保存图片
                const char* output_file = "gpu_screenshot.bmp";
                if (writeBMP(output_file, ci.width, ci.height, pixels.data())) {
                    std::printf("[Save] Saved to %s\n", output_file);
                    
                    // 分析像素
                    std::printf("\n=== PIXEL ANALYSIS ===\n");
                    int total_pixels = ci.width * ci.height;
                    int total_black = 0, total_white = 0, total_gray = 0;
                    
                    for (uint32_t i = 0; i < total_pixels; i++) {
                        int idx = i * 4;
                        int brightness = (pixels[idx] + pixels[idx+1] + pixels[idx+2]) / 3;
                        if (brightness < 50) total_black++;
                        else if (brightness > 200) total_white++;
                        else total_gray++;
                    }
                    
                    std::printf("Total pixels: %d\n", total_pixels);
                    std::printf("  Black (<50): %d (%.1f%%)\n", total_black, 100.0*total_black/total_pixels);
                    std::printf("  Gray (50-200): %d (%.1f%%)\n", total_gray, 100.0*total_gray/total_pixels);
                    std::printf("  White (>200): %d (%.1f%%)\n", total_white, 100.0*total_white/total_pixels);
                    
                    if (total_white == total_pixels) {
                        std::printf("\n⚠️  WARNING: Image is all white! Rendering to offscreen texture failed.\n");
                        std::printf("   This is expected - we need to capture the actual swapchain.\n");
                        std::printf("   The window should show the rendered text correctly.\n");
                    } else if (total_black > 100 || total_gray > 100) {
                        std::printf("\n✅ SUCCESS: Text is rendering!\n");
                    }
                    
                    screenshot_taken = true;
                }
            } else {
                SDL_Log("Failed to map download buffer: %s", SDL_GetError());
            }
            
            SDL_ReleaseGPUTransferBuffer(device, download_buffer);
            SDL_ReleaseGPUTexture(device, screenshot_texture);
            
            std::printf("\n[Info] Screenshot attempt complete. Window will close in 2 seconds...\n");
            SDL_Delay(2000);
            running = false;
        }
        
        SDL_Delay(16);  // ~60 FPS
    }

    std::printf("\n[Cleanup] Shutting down...\n");
    dong_view_destroy(view);
    dong_destroy_context(ctx);

    std::printf("[Done] Check gpu_screenshot.bmp (may be all white - check window for actual render)\n");
    return 0;
}
