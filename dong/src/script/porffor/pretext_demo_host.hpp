#pragma once

#include <cstdint>
#include <string>

namespace dong::render {
class OverlayDraw;
}

namespace dong::script {

class JSBindings;

void pretextDualModeOverlayTick(int mode, int t, JSBindings* bindings, render::OverlayDraw* overlay);
int pretextFlowDynamicTick(int mode, uint64_t lines_id, uint64_t hud_id, uint64_t obs_a_id,
                           uint64_t obs_b_id, uint64_t obs_c_id, int t, int fps,
                           JSBindings* bindings, render::OverlayDraw* overlay);
void pretextDomOnlyInit(JSBindings* bindings);
void pretextDomOnlyTick(int t, JSBindings* bindings);
std::string pretextDynamicHud(int fps, int lines, int pool_size);

} // namespace dong::script
