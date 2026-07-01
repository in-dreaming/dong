#include "ui_pass_compiler.hpp"

namespace dong::render {

DongUiPassBundle UiPassCompiler::compile(const GPUCommandList& list) {
    DongUiPassBundle bundle{};
    bundle.draw_command_count = static_cast<uint32_t>(list.commands.size());
    bundle.command_list = &list;
    for (const auto& cmd : list.commands) {
        switch (cmd.type) {
        case GPUCommandType::BeginIsolatedLayer:
            bundle.has_layer_raster = 1;
            break;
        case GPUCommandType::DrawImageQuad:
        case GPUCommandType::DrawText:
        case GPUCommandType::UberQuadBatch:
        case GPUCommandType::DrawInstancedQuads:
        case GPUCommandType::DrawRoundedRectQuad:
        case GPUCommandType::DrawShadowQuad:
        case GPUCommandType::DrawGradientQuad:
        case GPUCommandType::DrawConicGradientQuad:
        case GPUCommandType::DrawNineSliceQuad:
            bundle.has_ui_main = 1;
            break;
        default:
            break;
        }
    }
    if (bundle.draw_command_count > 0) {
        bundle.has_upload = 1;
    }
    return bundle;
}

} // namespace dong::render
