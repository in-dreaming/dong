#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace dong::render {
class OverlayDraw;
}

namespace dong::script {

std::string porfforTextLayout(const std::string& config_json);
void porfforClearOverlay(dong::render::OverlayDraw* overlay);
void porfforRenderText(const std::string& config_json, dong::render::OverlayDraw* overlay);
void porfforDrawRect(const std::string& config_json, dong::render::OverlayDraw* overlay);
void porfforDrawCircle(const std::string& config_json, dong::render::OverlayDraw* overlay);

std::string pretextTypoConfig(const std::string& columns_json, const std::string& obstacles_json);
std::string pretextDualModeTextConfig(const std::string& columns_json, const std::string& circles_json);
std::string pretextFlowObstaclesJson(float obs_a_cx, float obs_a_cy, float obs_a_r, float bx, float by,
                                     float obs_b_w, float obs_b_h, float obs_c_cx, float obs_c_cy,
                                     float obs_c_r);
std::string pretextFlowColumnsStatic();
std::string pretextFlowColumnsDynamic();
int porfforTextLayoutMountLines(uint64_t node_id, const std::string& config_json,
                                const std::string& line_class, bool stats_footer, class JSBindings* bindings);
int porfforTextLayoutRenderOverlay(const std::string& config_json, const std::string& color,
                                   dong::render::OverlayDraw* overlay);
struct PretextLayoutLine {
    float x = 0;
    float y = 0;
    float width = 0;
    std::string text;
};

std::vector<PretextLayoutLine> pretextParseLayoutLines(const std::string& layout_json);
int porfforLastLayoutLineCount();

} // namespace dong::script
