#pragma once

#include "../render/scene_graph.hpp"
#include "dom/dom_node.hpp"

namespace dong::dom {

class SceneCompiler {
public:
    // Check if a DOM tree is suitable for scene graph compilation:
    // all positioned elements with explicit pixel coordinates, no flexbox/grid.
    static bool canCompile(const DOMNodePtr& root);

    // Compile a DOM tree into a SceneGraph, extracting visual properties.
    // Returns true if compilation succeeded.
    static bool compile(const DOMNodePtr& root, render::SceneGraph& sg);

private:
    static void compileNode(const DOMNodePtr& node, render::SceneGraph& sg,
                            uint32_t parent_id, float parent_x, float parent_y);
};

} // namespace dong::dom
