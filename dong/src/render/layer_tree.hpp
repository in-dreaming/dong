#pragma once

#include <cstdint>
#include <vector>

#include "display_list.hpp"

namespace dong::render {

// Layer 类型：后续可以扩展更多细分类型
enum class LayerType : uint8_t {
    Surface,   // 具有自己离屏表面的图层（如滚动容器、孤立层）
    Transform, // 主要携带变换信息的图层
    Opacity    // 主要携带透明度信息的图层
};

// 简单的 2D 仿射变换：
//   [ m0 m1 m2 ]   [ x ]   [ x']
//   [ m3 m4 m5 ] * [ y ] = [ y']
//   [  0  0  1 ]   [ 1 ]   [ 1 ]
struct LayerTransform {
    float m[6] = {1.0f, 0.0f, 0.0f,
                  0.0f, 1.0f, 0.0f};

    static LayerTransform identity() {
        LayerTransform t;
        t.m[0] = 1.0f; t.m[1] = 0.0f; t.m[2] = 0.0f;
        t.m[3] = 0.0f; t.m[4] = 1.0f; t.m[5] = 0.0f;
        return t;
    }
};

struct LayerNode {
    uint64_t id = 0;           // 与 DisplayList 的 LayerData.id 对应
    LayerType type = LayerType::Surface;

    Rect bounds;               // 本地坐标系下的边界
    float opacity = 1.0f;      // 合成时使用的透明度
    LayerTransform transform;  // 本层到父层的 2D 变换

    float scroll_x = 0.0f;     // 滚动偏移（暂用像素表示，后续可细化）
    float scroll_y = 0.0f;

    bool is_surface = false;   // 是否拥有独立的离屏 render target

    // Dirty 标记预留：
    bool content_dirty = true;     // 内容发生变化，需要重新 raster
    bool transform_dirty = false;  // 仅变换变化
    bool opacity_dirty = false;    // 仅透明度变化
    bool scroll_dirty = false;     // 仅滚动偏移变化

    int parent = -1;
    std::vector<int> children;
};

struct LayerTree {
    std::vector<LayerNode> nodes;
    int root_index = -1;

    void clear() {
        nodes.clear();
        root_index = -1;
    }

    bool empty() const { return nodes.empty(); }

    LayerNode* getRoot() {
        if (root_index < 0 || root_index >= static_cast<int>(nodes.size())) {
            return nullptr;
        }
        return &nodes[static_cast<std::size_t>(root_index)];
    }

    const LayerNode* getRoot() const {
        if (root_index < 0 || root_index >= static_cast<int>(nodes.size())) {
            return nullptr;
        }
        return &nodes[static_cast<std::size_t>(root_index)];
    }

    int addNode(const LayerNode& node, int parent_index) {
        const int index = static_cast<int>(nodes.size());
        nodes.push_back(node);
        nodes.back().parent = parent_index;
        if (parent_index >= 0 && parent_index < index) {
            nodes[static_cast<std::size_t>(parent_index)].children.push_back(index);
        } else {
            // 没有父节点时，视为新的 root（当前实现假定只有一个 root）
            root_index = index;
        }
        return index;
    }

    const LayerNode* findById(uint64_t id) const {
        for (const auto& node : nodes) {
            if (node.id == id) {
                return &node;
            }
        }
        return nullptr;
    }
};

} // namespace dong::render
