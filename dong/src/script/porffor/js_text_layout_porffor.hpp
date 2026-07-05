#pragma once

#include <string>

namespace dong::render {
class OverlayDraw;
}

namespace dong::script {

std::string porfforTextLayout(const std::string& config_json);
void porfforClearOverlay(dong::render::OverlayDraw* overlay);
void porfforRenderText(const std::string& config_json, dong::render::OverlayDraw* overlay);
void porfforDrawRect(const std::string& config_json, dong::render::OverlayDraw* overlay);
void porfforDrawCircle(const std::string& config_json, dong::render::OverlayDraw* overlay);

} // namespace dong::script
