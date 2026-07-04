#include "render/scene_graph.hpp"

namespace dong::script {

static render::SceneGraph& sceneInstance() {
    static render::SceneGraph sg;
    return sg;
}

render::SceneGraph& getGlobalSceneGraph() {
    return sceneInstance();
}

} // namespace dong::script
