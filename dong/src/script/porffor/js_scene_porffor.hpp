#pragma once

#include <cstdint>
#include <string>

namespace dong::render {
class SceneGraph;
}

namespace dong::script {

class PorfforScriptRegistry;

render::SceneGraph& getGlobalSceneGraph();

uint32_t porfforSceneAddNode(const std::string& config_json);
void porfforSceneRemove(uint32_t id);
void porfforSceneSet(uint32_t id, const std::string& prop, const std::string& value);
int32_t porfforSceneFind(const std::string& name);
void porfforSceneOn(uint32_t id, const std::string& type, const std::string& export_name,
                    const std::string& module_name, PorfforScriptRegistry* registry);
void porfforSceneClear();
uint32_t porfforSceneCount();

} // namespace dong::script
