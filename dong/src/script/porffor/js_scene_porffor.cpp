#include "js_scene_porffor.hpp"

#include "porffor_mini_json.hpp"
#include "porffor_script_registry.hpp"
#include "../../render/scene_graph.hpp"
#include "../../render/painter/painter_style_utils.hpp"
#include <cstdlib>

namespace dong::script {
namespace {

namespace pj = porffor_json;

static render::SceneGraph& sceneInstance() {
    static render::SceneGraph sg;
    return sg;
}

render::Color parseSceneColor(const std::string& s, render::Color fallback = {0, 0, 0, 0}) {
    if (s.empty()) {
        return fallback;
    }
    return render::painter_detail::makeColorFromCss(s);
}

} // namespace

render::SceneGraph& getGlobalSceneGraph() {
    return sceneInstance();
}

uint32_t porfforSceneAddNode(const std::string& config_json) {
    render::SceneNode node;
    pj::extractString(config_json, "name", node.name);

    double num = 0;
    if (pj::extractNumber(config_json, "x", num)) {
        node.x = static_cast<float>(num);
    }
    if (pj::extractNumber(config_json, "y", num)) {
        node.y = static_cast<float>(num);
    }
    if (pj::extractNumber(config_json, "w", num) || pj::extractNumber(config_json, "width", num)) {
        node.width = static_cast<float>(num);
    }
    if (pj::extractNumber(config_json, "h", num) || pj::extractNumber(config_json, "height", num)) {
        node.height = static_cast<float>(num);
    }
    if (pj::extractNumber(config_json, "zOrder", num)) {
        node.z_order = static_cast<int>(num);
    }
    if (pj::extractNumber(config_json, "opacity", num)) {
        node.opacity = static_cast<float>(num);
    }
    if (pj::extractNumber(config_json, "borderWidth", num)) {
        node.border_width = static_cast<float>(num);
    }
    if (pj::extractNumber(config_json, "borderRadius", num)) {
        node.border_radius = static_cast<float>(num);
    }
    if (pj::extractNumber(config_json, "fontSize", num)) {
        node.font_size = static_cast<float>(num);
    }
    if (pj::extractNumber(config_json, "lineHeight", num)) {
        node.line_height = static_cast<float>(num);
    }

    bool visible = true;
    if (pj::extractBool(config_json, "visible", visible)) {
        node.visible = visible;
    }

    std::string s;
    if (pj::extractString(config_json, "fontFamily", s)) {
        node.font_family = s;
    }
    if (pj::extractString(config_json, "fontWeight", s)) {
        node.font_weight = s;
    }
    if (pj::extractString(config_json, "textAlign", s)) {
        node.text_align = s;
    }
    if (pj::extractString(config_json, "text", s)) {
        node.text = s;
    }
    if (pj::extractString(config_json, "imageSrc", s)) {
        node.image_src = s;
    }
    if (pj::extractString(config_json, "background", s)) {
        node.background_color = parseSceneColor(s);
    }
    if (pj::extractString(config_json, "border", s)) {
        node.border_color = parseSceneColor(s);
    }
    if (pj::extractString(config_json, "color", s)) {
        node.text_color = parseSceneColor(s, {1, 1, 1, 1});
    }

    if (pj::extractNumber(config_json, "parent", num)) {
        node.parent = static_cast<uint32_t>(num);
    }

    return sceneInstance().addNode(std::move(node));
}

void porfforSceneRemove(uint32_t id) {
    sceneInstance().removeNode(id);
}

void porfforSceneSet(uint32_t id, const std::string& prop, const std::string& value) {
    auto& sg = sceneInstance();
    if (prop == "visible") {
        bool b = (value == "true" || value == "1");
        sg.setBool(id, prop, b);
        return;
    }

    char* end = nullptr;
    const double d = std::strtod(value.c_str(), &end);
    if (end && end != value.c_str() && *end == '\0') {
        sg.setFloat(id, prop, static_cast<float>(d));
        return;
    }

    if (prop == "background" || prop == "borderColor" || prop == "color" || prop == "borderTopColor" ||
        prop == "borderRightColor" || prop == "borderBottomColor" || prop == "borderLeftColor") {
        sg.setColor(id, prop, parseSceneColor(value));
        return;
    }

    sg.setString(id, prop, value);
}

int32_t porfforSceneFind(const std::string& name) {
    auto* node = sceneInstance().findByName(name);
    return node ? static_cast<int32_t>(node->id) : -1;
}

void porfforSceneOn(uint32_t id, const std::string& type, const std::string& export_name,
                    const std::string& module_name, PorfforScriptRegistry* registry) {
    if (!registry || export_name.empty()) {
        return;
    }
    std::string mod = module_name;
    std::string exp = export_name;
    sceneInstance().addEventListener(id, type,
                                     [registry, mod, exp](uint32_t node_id, float x, float y) {
                                         double args[3] = {static_cast<double>(node_id),
                                                           static_cast<double>(x),
                                                           static_cast<double>(y)};
                                         registry->callExport(mod, exp, args, 3);
                                     });
}

void porfforSceneClear() {
    sceneInstance().clear();
}

uint32_t porfforSceneCount() {
    return static_cast<uint32_t>(sceneInstance().nodeCount());
}

} // namespace dong::script
