

void SDLGPUDriver::executeDispatchCommand(const GPUCommand& cmd, ExecuteContext& ctx) {
    if (ctx.aborted) return;

    // 当在非脏隔离图层内部时，跳过除 Begin/EndIsolatedLayer 和 BeginPass/EndPass 以外的命令
    if (ctx.skip_draw_depth > 0 && cmd.type != GPUCommandType::BeginIsolatedLayer &&
        cmd.type != GPUCommandType::EndIsolatedLayer &&
        cmd.type != GPUCommandType::BeginPass &&
        cmd.type != GPUCommandType::EndPass) {
        return;
    }

    switch (cmd.type) {
    case GPUCommandType::BeginFrame:
    case GPUCommandType::EndFrame:
        // beginFrame/endFrame 由调用方负责
        break;
    case GPUCommandType::BeginPass:
        executeBeginPass(ctx);
        break;
    case GPUCommandType::EndPass:
        executeEndPass(ctx);
        break;
    case GPUCommandType::PushClipRect:
        executePushClipRect(ctx, cmd);
        break;
    case GPUCommandType::PopClip:
        executePopClip(ctx);
        break;
    case GPUCommandType::BeginIsolatedLayer:
        executeBeginIsolatedLayer(ctx, cmd);
        break;
    case GPUCommandType::EndIsolatedLayer:
        executeEndIsolatedLayer(ctx, cmd);
        break;
    case GPUCommandType::DrawInstancedQuads:
        executeDrawRect(ctx, cmd);
        break;
    case GPUCommandType::DrawRoundedRectQuad:
        executeDrawRoundedRect(ctx, cmd);
        break;
    case GPUCommandType::DrawShadowQuad:
        executeDrawShadow(ctx, cmd);
        break;
    case GPUCommandType::DrawImageQuad:
        executeDrawImage(ctx, cmd);
        break;
    case GPUCommandType::DrawText:
        executeDrawText(ctx, cmd);
        break;
    default:
        break;
    }
}

void SDLGPUDriver::execute(const GPUCommandList& commands) {
    DONG_PROFILE_SCOPE_CAT("GPU::execute", "gpu");

    if (!in_frame_ || !current_cmd_buf_ || !gpu_device_) {
        DONG_LOG_ERROR("SDLGPUDriver::execute: invalid state");
        return;
    }

    SDL_GPUDevice* dev = gpu_device_->getHandle();

    ExecuteContext ctx(current_cmd_buf_, dev, window_);
    ctx.debug_rt_enabled = debug_rt_enabled_;
    ctx.debug_log_draw_batches = debug_log_draw_batches_;
    ctx.debug_log_layer_cache = debug_log_layer_cache_;
    ctx.layer_cache_enabled = layer_cache_enabled_;
    ctx.split_cmd_buf_for_isolated_layers = split_cmd_buf_for_isolated_layers_;
    ctx.offscreen = (offscreen_target_ != nullptr);
    ctx.frame_index = frame_index_;

    if (!executeSetupMainTarget(ctx)) return;

    // 计算 device pixel ratio
    ctx.device_pixel_ratio = 1.0f;
    if (window_) {
        int logical_w = 0, logical_h = 0;
        int drawable_w = 0, drawable_h = 0;
        SDL_GetWindowSize(window_, &logical_w, &logical_h);
        SDL_GetWindowSizeInPixels(window_, &drawable_w, &drawable_h);
        if (logical_w > 0 && drawable_w > 0) {
            ctx.device_pixel_ratio = static_cast<float>(drawable_w) / static_cast<float>(logical_w);
        }
    }
    ctx.inv_device_pixel_ratio = ctx.device_pixel_ratio > 0.0f ? (1.0f / ctx.device_pixel_ratio) : 1.0f;

    const bool present_only = (!offscreen_target_ && ctx.use_intermediate && intermediate_texture_ && commands.commands.empty());

    // Present-only 快路径
    if (present_only) {
        DONG_PROFILE_SCOPE_CAT("GPU::presentOnly", "gpu");
        if (!intermediate_valid_) {
            executeBeginPass(ctx);
            if (ctx.aborted) return;
        }
        executeEndPass(ctx);
        return;
    }

    executePreuploadImages(commands);

    {
        DONG_PROFILE_SCOPE_CAT("GPU::executeCommands", "gpu");
        for (const auto& cmd : commands.commands) {
            executeDispatchCommand(cmd, ctx);
            if (ctx.aborted) break;
        }
    }

    if (ctx.pass) {
        SDL_EndGPURenderPass(ctx.pass);
        ctx.pass = nullptr;
    }
}

} // namespace render
} // namespace dong
