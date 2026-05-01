#pragma once

#include "../../include/dong_drawlist.h"

namespace dong::render {

struct GPUCommandList;

/// Translate the internal GPUCommandList into a flat dong_draw_cmd_t array.
/// The emitted commands remain valid until the next call to emitDrawList().
/// UberQuadBatch commands are expanded into individual primitive draw commands.
void emitDrawList(const GPUCommandList& gpu_cmds);

/// Get pointer to the emitted draw command array and its count.
/// Returns nullptr if no commands have been emitted (count set to 0).
const dong_draw_cmd_t* getEmittedCommands(uint32_t* out_count);

} // namespace dong::render
