#pragma once

#include "display_list.hpp"
#include "text_shaper.hpp"
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace dong::render {

struct SceneNode {
    uint32_t id = 0;
    std::string name;

    float x = 0, y = 0, width = 0, height = 0;
    int z_order = 0;

    Color background_color{0, 0, 0, 0};
    Color border_color{0, 0, 0, 0};
    float border_width = 0;
    float border_radius = 0;
    float opacity = 1.0f;
    bool visible = true;

    // Per-side border (-1 = use uniform border_width/border_color)
    float border_top_width = -1, border_right_width = -1;
    float border_bottom_width = -1, border_left_width = -1;
    Color border_top_color{0, 0, 0, 0};
    Color border_right_color{0, 0, 0, 0};
    Color border_bottom_color{0, 0, 0, 0};
    Color border_left_color{0, 0, 0, 0};

    std::string text;
    Color text_color{1, 1, 1, 1};
    float font_size = 16;
    std::string font_family = "sans-serif";
    std::string font_weight = "normal";
    std::string text_align = "left";
    float line_height = 0;

    std::string image_src;

    uint32_t parent = UINT32_MAX;
    std::vector<uint32_t> children;

    bool dirty = true;
    std::vector<DisplayItem> cached_items;
};

using SceneEventCallback = std::function<void(uint32_t node_id, float x, float y)>;

class SceneGraph {
public:
    uint32_t addNode(SceneNode node);
    void removeNode(uint32_t id);
    SceneNode* getNode(uint32_t id);
    SceneNode* findByName(const std::string& name);
    void clear();

    void setFloat(uint32_t id, const std::string& prop, float value);
    void setString(uint32_t id, const std::string& prop, const std::string& value);
    void setColor(uint32_t id, const std::string& prop, const Color& color);
    void setBool(uint32_t id, const std::string& prop, bool value);

    const std::vector<DisplayItem>& buildDisplayItems();
    bool isDirty() const { return any_dirty_; }

    SceneNode* hitTest(float x, float y);

    void addEventListener(uint32_t node_id, const std::string& type, SceneEventCallback cb);
    bool dispatchEvent(const std::string& type, float x, float y);

    bool empty() const { return nodes_.empty(); }
    size_t nodeCount() const { return nodes_.size(); }

private:
    void buildNodeItems(SceneNode& node);
    void collectItems(uint32_t id, std::vector<DisplayItem>& out);
    SceneNode* hitTestNode(uint32_t id, float x, float y);

    std::unordered_map<uint32_t, SceneNode> nodes_;
    std::unordered_map<std::string, uint32_t> name_map_;
    std::vector<uint32_t> root_ids_;
    uint32_t next_id_ = 1;
    bool any_dirty_ = true;
    std::vector<DisplayItem> cached_display_items_;

    struct ListenerEntry {
        std::string type;
        SceneEventCallback callback;
    };
    std::unordered_map<uint32_t, std::vector<ListenerEntry>> listeners_;
};

} // namespace dong::render
