#include <cstdio>
#include <cstdint>
#include <vector>
#include <cstring>
#include <dong.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

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
    std::printf("=== MSDF Soft Render - Output to Image ===\n");

    // 1. 初始化 SDL 和 GPU
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Failed to init SDL: %s", SDL_GetError());
        return 1;
    }

    // 创建隐藏窗口（SDL_gpu 需要窗口上下文）
    SDL_Window* window = SDL_CreateWindow("MSDF Soft Render", 800, 400, SDL_WINDOW_HIDDEN);
    if (!window) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_GPUDevice* device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_DXIL, false, nullptr);
    if (!device) {
        SDL_Log("Failed to create GPU device: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!SDL_ClaimWindowForGPUDevice(device, window)) {
        SDL_Log("Failed to claim window: %s", SDL_GetError());
        SDL_DestroyGPUDevice(device);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // 2. 创建 Dong 上下文和视图
    dong_context_t* ctx = dong_create_context();
    if (!ctx) {
        std::printf("ERROR: Failed to create context\n");
        SDL_DestroyGPUDevice(device);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    const uint32_t width = 800;
    const uint32_t height = 400;
    
    dong_view_t* view = dong_view_create(ctx, width, height);
    if (!view) {
        std::printf("ERROR: Failed to create view\n");
        dong_destroy_context(ctx);
        SDL_DestroyGPUDevice(device);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // 3. 切换到 GPU 渲染模式
    std::printf("[Init] Enabling GPU render mode...\n");
    dong_view_set_external_gpu_device(view, static_cast<void*>(device), static_cast<void*>(window));

    // 4. 加载测试 HTML
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
                    background: #f0f0f0;
                    padding: 10px;
                }
                
                .size-48 { font-size: 48px; color: #000000; }
                .size-32 { font-size: 32px; color: #ff0000; }
                .size-24 { font-size: 24px; color: #0000ff; }
            </style>
        </head>
        <body>
            <div class="test-line size-48">ABCDEFG</div>
            <div class="test-line size-32">Test</div>
            <div class="test-line size-24">Hello</div>
        </body>
        </html>
    )";

    std::printf("[Load] Loading HTML...\n");
    dong_view_load_html(view, html);

    // 5. 渲染多帧（GPU 渲染需要时间初始化）
    std::printf("[Render] Rendering multiple frames for GPU stabilization...\n");
    for (int i = 0; i < 5; i++) {
        dong_view_update(view);
        SDL_Delay(100);  // 延迟 100ms
        std::printf("  Frame %d/5 rendered\n", i + 1);
    }
    
    std::printf("[Render] Waiting for GPU to stabilize...\n");
    SDL_Delay(1000);  // 额外延迟 1秒确保渲染完成

    // 6. 现在读取最后一帧的渲染结果
    // 注意：dong_view_update 会渲染到 swapchain，我们需要捕获这一帧
    // 最简单的方式：让 dong 再渲染一次到我们的离屏纹理
    SDL_GPUTextureCreateInfo tex_info{};
    tex_info.type = SDL_GPU_TEXTURETYPE_2D;
    tex_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    tex_info.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
    tex_info.width = width;
    tex_info.height = height;
    tex_info.layer_count_or_depth = 1;
    tex_info.num_levels = 1;
    tex_info.sample_count = SDL_GPU_SAMPLECOUNT_1;

    SDL_GPUTexture* read_texture = SDL_CreateGPUTexture(device, &tex_info);
    if (!read_texture) {
        SDL_Log("Failed to create read texture: %s", SDL_GetError());
        dong_view_destroy(view);
        dong_destroy_context(ctx);
        SDL_DestroyGPUDevice(device);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // 7. 再次渲染到离屏纹理
    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
    if (!cmd) {
        SDL_Log("Failed to acquire command buffer: %s", SDL_GetError());
        SDL_ReleaseGPUTexture(device, read_texture);
        dong_view_destroy(view);
        dong_destroy_context(ctx);
        SDL_DestroyGPUDevice(device);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // 开始渲染通道
    SDL_GPUColorTargetInfo color_target{};
    color_target.texture = read_texture;
    color_target.mip_level = 0;
    color_target.layer_or_depth_plane = 0;
    color_target.clear_color = SDL_FColor{1.0f, 1.0f, 1.0f, 1.0f};
    color_target.load_op = SDL_GPU_LOADOP_CLEAR;
    color_target.store_op = SDL_GPU_STOREOP_STORE;

    SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmd, &color_target, 1, nullptr);
    // TODO: 这里需要调用 dong_view 的渲染命令
    // 由于 dong_view_update 已经执行过，我们需要让它再次提交命令到这个 pass
    SDL_EndGPURenderPass(pass);

    // 8. 读回像素
    SDL_GPUTransferBufferCreateInfo transfer_info{};
    transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD;
    transfer_info.size = width * height * 4;

    SDL_GPUTransferBuffer* download_buffer = SDL_CreateGPUTransferBuffer(device, &transfer_info);
    if (!download_buffer) {
        SDL_Log("Failed to create download buffer: %s", SDL_GetError());
        SDL_ReleaseGPUTexture(device, read_texture);
        dong_view_destroy(view);
        dong_destroy_context(ctx);
        SDL_DestroyGPUDevice(device);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd);

    SDL_GPUTextureRegion src_region{};
    src_region.texture = read_texture;
    src_region.mip_level = 0;
    src_region.layer = 0;
    src_region.x = 0;
    src_region.y = 0;
    src_region.z = 0;
    src_region.w = width;
    src_region.h = height;
    src_region.d = 1;

    SDL_GPUTextureTransferInfo dst_transfer{};
    dst_transfer.transfer_buffer = download_buffer;
    dst_transfer.offset = 0;
    dst_transfer.pixels_per_row = 0;  // 使用width作为默认值
    dst_transfer.rows_per_layer = 0;  // 使用height作为默认值

    SDL_DownloadFromGPUTexture(copy_pass, &src_region, &dst_transfer);
    SDL_EndGPUCopyPass(copy_pass);

    SDL_SubmitGPUCommandBuffer(cmd);
    SDL_WaitForGPUIdle(device);

    // 9. Map 并复制像素数据
    void* mapped = SDL_MapGPUTransferBuffer(device, download_buffer, false);
    if (!mapped) {
        SDL_Log("Failed to map download buffer: %s", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(device, download_buffer);
        SDL_ReleaseGPUTexture(device, read_texture);
        dong_view_destroy(view);
        dong_destroy_context(ctx);
        SDL_DestroyGPUDevice(device);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    std::vector<uint8_t> pixels(width * height * 4);
    std::memcpy(pixels.data(), mapped, width * height * 4);
    SDL_UnmapGPUTransferBuffer(device, download_buffer);

    // 10. 保存图片
    const char* output_file = "msdf_soft_render.bmp";
    if (writeBMP(output_file, width, height, pixels.data())) {
        std::printf("[Save] Saved to %s\n", output_file);
    } else {
        std::printf("ERROR: Failed to save BMP\n");
    }

    // 11. 分析像素
    std::printf("\n=== PIXEL ANALYSIS ===\n");
    
    int total_black = 0, total_white = 0, total_gray = 0;
    int total_pixels = width * height;
    
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
    
    if (total_black == total_pixels) {
        std::printf("\n⚠️  WARNING: Image is completely black! Rendering failed.\n");
    } else if (total_white == total_pixels) {
        std::printf("\n⚠️  WARNING: Image is completely white! No text rendered.\n");
    } else if (total_black > 0 && total_gray > 0) {
        std::printf("\n✓ Image contains text (has black and gray pixels)\n");
    }

    // 采样特定点
    std::printf("\n=== SAMPLE POINTS ===\n");
    struct {
        int x, y;
        const char* desc;
    } samples[] = {
        {100, 60, "Expected text area 1"},
        {200, 60, "Expected text area 2"},
        {100, 135, "Expected text area 3"},
        {200, 200, "Expected text area 4"},
    };

    for (const auto& sample : samples) {
        if (sample.x < (int)width && sample.y < (int)height) {
            int idx = (sample.y * width + sample.x) * 4;
            uint8_t r = pixels[idx];
            uint8_t g = pixels[idx + 1];
            uint8_t b = pixels[idx + 2];
            uint8_t a = pixels[idx + 3];
            
            std::printf("  (%d,%d) %s: RGBA(%d,%d,%d,%d)\n",
                       sample.x, sample.y, sample.desc, r, g, b, a);
        }
    }

    // 12. 清理
    SDL_ReleaseGPUTransferBuffer(device, download_buffer);
    SDL_ReleaseGPUTexture(device, read_texture);
    dong_view_destroy(view);
    dong_destroy_context(ctx);
    SDL_DestroyGPUDevice(device);
    SDL_DestroyWindow(window);
    SDL_Quit();

    std::printf("\n[Done] Check %s for visual verification\n", output_file);
    return 0;
}
