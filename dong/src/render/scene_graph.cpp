#include "scene_graph.hpp"
#include "../core/log.h"
#include <algorithm>
#include <cmath>

namespace dong::render {

uint32_t SceneGraph::addNode(SceneNode node) {
    uint32_t id = next_id_++;
    node.id = id;
    node.dirty = true;

    if (!node.name.empty()) {
        name_map_[node.name] = id;
    }

    if (node.parent == UINT32_MAX) {
        root_ids_.push_back(id);
    } else {
        auto pit = nodes_.find(node.parent);
        if (pit != nodes_.end()) {
            pit->second.children.push_back(id);
        }
    }

    nodes_.emplace(id, std::move(node));
    any_dirty_ = true;
    return id;
}

void SceneGraph::removeNode(uint32_t id) {
    auto it = nodes_.find(id);
    if (it == nodes_.end()) return;

    // Remove children recursively
    auto children_copy = it->second.children;
    for (auto cid : children_copy) {
        removeNode(cid);
    }

    if (!it->second.name.empty()) {
        name_map_.erase(it->second.name);
    }

    // Remove from parent's children list
    if (it->second.parent != UINT32_MAX) {
        auto pit = nodes_.find(it->second.parent);
        if (pit != nodes_.end()) {
            auto& pc = pit->second.children;
            pc.erase(std::remove(pc.begin(), pc.end(), id), pc.end());
        }
    }

    root_ids_.erase(std::remove(root_ids_.begin(), root_ids_.end(), id), root_ids_.end());
    listeners_.erase(id);
    nodes_.erase(it);
    any_dirty_ = true;
}

SceneNode* SceneGraph::getNode(uint32_t id) {
    auto it = nodes_.find(id);
    return it != nodes_.end() ? &it->second : nullptr;
}

SceneNode* SceneGraph::findByName(const std::string& name) {
    auto it = name_map_.find(name);
    if (it == name_map_.end()) return nullptr;
    return getNode(it->second);
}

void SceneGraph::clear() {
    nodes_.clear();
    name_map_.clear();
    root_ids_.clear();
    listeners_.clear();
    cached_display_items_.clear();
    any_dirty_ = true;
    next_id_ = 1;
}

static void markDirty(SceneNode& node) {
    node.dirty = true;
    node.cached_items.clear();
}

void SceneGraph::setFloat(uint32_t id, const std::string& prop, float value) {
    auto* n = getNode(id);
    if (!n) return;

    if (prop == "x") n->x = value;
    else if (prop == "y") n->y = value;
    else if (prop == "width" || prop == "w") n->width = value;
    else if (prop == "height" || prop == "h") n->height = value;
    else if (prop == "borderWidth") n->border_width = value;
    else if (prop == "borderTopWidth") n->border_top_width = value;
    else if (prop == "borderRightWidth") n->border_right_width = value;
    else if (prop == "borderBottomWidth") n->border_bottom_width = value;
    else if (prop == "borderLeftWidth") n->border_left_width = value;
    else if (prop == "borderRadius") n->border_radius = value;
    else if (prop == "opacity") n->opacity = value;
    else if (prop == "fontSize") n->font_size = value;
    else if (prop == "lineHeight") n->line_height = value;
    else if (prop == "zOrder") n->z_order = static_cast<int>(value);
    else return;

    markDirty(*n);
    any_dirty_ = true;
}

void SceneGraph::setString(uint32_t id, const std::string& prop, const std::string& value) {
    auto* n = getNode(id);
    if (!n) return;

    if (prop == "text") n->text = value;
    else if (prop == "name") {
        if (!n->name.empty()) name_map_.erase(n->name);
        n->name = value;
        if (!value.empty()) name_map_[value] = id;
    }
    else if (prop == "fontFamily") n->font_family = value;
    else if (prop == "fontWeight") n->font_weight = value;
    else if (prop == "textAlign") n->text_align = value;
    else if (prop == "imageSrc") n->image_src = value;
    else return;

    markDirty(*n);
    any_dirty_ = true;
}

void SceneGraph::setColor(uint32_t id, const std::string& prop, const Color& color) {
    auto* n = getNode(id);
    if (!n) return;

    if (prop == "background") n->background_color = color;
    else if (prop == "borderColor") n->border_color = color;
    else if (prop == "borderTopColor") n->border_top_color = color;
    else if (prop == "borderRightColor") n->border_right_color = color;
    else if (prop == "borderBottomColor") n->border_bottom_color = color;
    else if (prop == "borderLeftColor") n->border_left_color = color;
    else if (prop == "color") n->text_color = color;
    else return;

    markDirty(*n);
    any_dirty_ = true;
}

void SceneGraph::setBool(uint32_t id, const std::string& prop, bool value) {
    auto* n = getNode(id);
    if (!n) return;

    if (prop == "visible") n->visible = value;
    else return;

    markDirty(*n);
    any_dirty_ = true;
}

// --- Rendering ---

void SceneGraph::buildNodeItems(SceneNode& node) {
    node.cached_items.clear();

    if (!node.visible || node.opacity <= 0.001f) {
        node.dirty = false;
        return;
    }

    Rect bounds{node.x, node.y, node.width, node.height};
    bool has_radius = node.border_radius > 0.01f;

    // Background
    if (node.background_color.a > 0.001f) {
        DisplayItem item{};
        Color bg = node.background_color;
        bg.a *= node.opacity;
        if (has_radius) {
            item.type = DisplayItemType::DrawRoundedRect;
            item.rounded_rect = {bounds, bg, node.border_radius, 0.0f};
        } else {
            item.type = DisplayItemType::DrawRect;
            item.rect = {bounds, bg};
        }
        node.cached_items.push_back(std::move(item));
    }

    // Border (per-side aware)
    {
        auto ew = [&](float side) -> float {
            return (side >= 0) ? side : std::max(0.0f, node.border_width);
        };
        auto ec = [&](const Color& side) -> Color {
            return (side.a > 0.001f) ? side : node.border_color;
        };
        float bt = ew(node.border_top_width);
        float br = ew(node.border_right_width);
        float bb = ew(node.border_bottom_width);
        float bl = ew(node.border_left_width);
        float bmax = std::max(std::max(bt, bb), std::max(bl, br));

        if (bmax > 0.01f) {
            Color ct = ec(node.border_top_color);
            Color cr = ec(node.border_right_color);
            Color cb = ec(node.border_bottom_color);
            Color cl = ec(node.border_left_color);

            auto nearEq = [](float a, float b) { return std::fabs(a - b) < 0.01f; };
            auto colorEq = [](const Color& a, const Color& b) {
                return std::fabs(a.r - b.r) < 0.004f && std::fabs(a.g - b.g) < 0.004f &&
                       std::fabs(a.b - b.b) < 0.004f && std::fabs(a.a - b.a) < 0.004f;
            };
            bool uniform = nearEq(bt, br) && nearEq(bt, bb) && nearEq(bt, bl) &&
                           colorEq(ct, cr) && colorEq(ct, cb) && colorEq(ct, cl);

            if (uniform) {
                DisplayItem item{};
                item.type = DisplayItemType::DrawRoundedRect;
                Color bc = ct;
                bc.a *= node.opacity;
                item.rounded_rect = {bounds, bc, node.border_radius, bt};
                node.cached_items.push_back(std::move(item));
            } else {
                float inner_h = std::max(0.0f, bounds.height - bt - bb);
                auto addSide = [&](Rect r, Color c) {
                    if (c.a <= 0.001f) return;
                    c.a *= node.opacity;
                    DisplayItem item{};
                    item.type = DisplayItemType::DrawRect;
                    item.rect = {r, c};
                    node.cached_items.push_back(std::move(item));
                };
                if (bt > 0.01f) addSide({bounds.x, bounds.y, bounds.width, bt}, ct);
                if (bb > 0.01f) addSide({bounds.x, bounds.y + bounds.height - bb, bounds.width, bb}, cb);
                if (bl > 0.01f) addSide({bounds.x, bounds.y + bt, bl, inner_h}, cl);
                if (br > 0.01f) addSide({bounds.x + bounds.width - br, bounds.y + bt, br, inner_h}, cr);
            }
        }
    }

    // Image
    if (!node.image_src.empty()) {
        DisplayItem item{};
        item.type = DisplayItemType::DrawImage;
        item.image = {bounds, node.image_src, node.opacity};
        node.cached_items.push_back(std::move(item));
    }

    // Text
    if (!node.text.empty()) {
        TextShaper shaper;
        TextShapeRequest req;
        req.text = node.text;
        req.font_family = node.font_family;
        req.font_weight = node.font_weight;
        req.font_size = node.font_size;

        ShapedText shaped;
        if (shaper.shape(req, shaped) && !shaped.glyphs.empty()) {
            float text_x = node.x;
            if (node.text_align == "center") {
                float tw = shaped.width_units * shaped.scale_to_pixels;
                text_x = node.x + (node.width - tw) / 2.0f;
            } else if (node.text_align == "right") {
                float tw = shaped.width_units * shaped.scale_to_pixels;
                text_x = node.x + node.width - tw;
            }

            float scale = shaped.scale_to_pixels;
            float lh = node.line_height > 0 ? node.line_height : node.font_size * 1.2f;
            float ascent_u = shaped.ascent_units > 0.0f
                           ? shaped.ascent_units
                           : node.font_size / std::max(scale, 1e-3f);
            float descent_abs_u = shaped.descent_units < 0.0f
                                ? -shaped.descent_units : 0.0f;
            float lh_u = lh / std::max(scale, 1e-3f);
            float extra_u = std::max(lh_u - (ascent_u + descent_abs_u), 0.0f);
            float baseline_off = (extra_u * 0.5f + ascent_u) * scale;
            float text_y = node.y + (node.height - lh) / 2.0f + baseline_off;

            float inv_scale = shaped.scale_to_pixels > 0 ? (1.0f / shaped.scale_to_pixels) : 1.0f;
            float ox = text_x * inv_scale;
            float oy = text_y * inv_scale;

            DrawGlyphRunData grd;
            grd.color = node.text_color;
            grd.color.a *= node.opacity;
            grd.font_size = node.font_size;
            grd.font_family = node.font_family;
            grd.font_weight = node.font_weight;
            grd.font_paths = shaped.font_paths;
            grd.font_path = shaped.font_path;
            grd.units_per_em = shaped.units_per_em;
            grd.scale_to_pixels = shaped.scale_to_pixels;
            grd.baseline_x = 0;
            grd.baseline_y = 0;
            grd.rect = bounds;

            for (const auto& sg : shaped.glyphs) {
                GlyphInstance gi;
                gi.glyph_id = sg.glyph_id;
                gi.pen_x_units = sg.pen_x_units + ox;
                gi.pen_y_units = sg.pen_y_units + oy;
                gi.font_path_index = sg.font_path_index;
                gi.units_per_em = sg.units_per_em;
                grd.glyphs.push_back(gi);
            }

            DisplayItem item{};
            item.type = DisplayItemType::DrawGlyphRun;
            item.glyph_run = std::move(grd);
            node.cached_items.push_back(std::move(item));
        }
    }

    node.dirty = false;
}

void SceneGraph::collectItems(uint32_t id, std::vector<DisplayItem>& out) {
    auto* node = getNode(id);
    if (!node || !node->visible) return;

    if (node->dirty) {
        buildNodeItems(*node);
    }

    out.insert(out.end(), node->cached_items.begin(), node->cached_items.end());

    // Children sorted by z_order
    if (!node->children.empty()) {
        auto sorted = node->children;
        std::sort(sorted.begin(), sorted.end(), [this](uint32_t a, uint32_t b) {
            auto* na = getNode(a);
            auto* nb = getNode(b);
            if (!na || !nb) return false;
            return na->z_order < nb->z_order;
        });
        for (auto cid : sorted) {
            collectItems(cid, out);
        }
    }
}

const std::vector<DisplayItem>& SceneGraph::buildDisplayItems() {
    if (!any_dirty_) return cached_display_items_;

    cached_display_items_.clear();

    auto sorted_roots = root_ids_;
    std::sort(sorted_roots.begin(), sorted_roots.end(), [this](uint32_t a, uint32_t b) {
        auto* na = getNode(a);
        auto* nb = getNode(b);
        if (!na || !nb) return false;
        return na->z_order < nb->z_order;
    });

    for (auto rid : sorted_roots) {
        collectItems(rid, cached_display_items_);
    }

    any_dirty_ = false;
    return cached_display_items_;
}

// --- Hit-testing ---

SceneNode* SceneGraph::hitTestNode(uint32_t id, float x, float y) {
    auto* node = getNode(id);
    if (!node || !node->visible) return nullptr;

    // Children in reverse z-order (topmost first)
    if (!node->children.empty()) {
        auto sorted = node->children;
        std::sort(sorted.begin(), sorted.end(), [this](uint32_t a, uint32_t b) {
            auto* na = getNode(a);
            auto* nb = getNode(b);
            if (!na || !nb) return false;
            return na->z_order > nb->z_order;
        });
        for (auto cid : sorted) {
            auto* hit = hitTestNode(cid, x, y);
            if (hit) return hit;
        }
    }

    if (x >= node->x && x <= node->x + node->width &&
        y >= node->y && y <= node->y + node->height) {
        return node;
    }
    return nullptr;
}

SceneNode* SceneGraph::hitTest(float x, float y) {
    auto sorted_roots = root_ids_;
    std::sort(sorted_roots.begin(), sorted_roots.end(), [this](uint32_t a, uint32_t b) {
        auto* na = getNode(a);
        auto* nb = getNode(b);
        if (!na || !nb) return false;
        return na->z_order > nb->z_order;
    });
    for (auto rid : sorted_roots) {
        auto* hit = hitTestNode(rid, x, y);
        if (hit) return hit;
    }
    return nullptr;
}

// --- Events ---

void SceneGraph::addEventListener(uint32_t node_id, const std::string& type, SceneEventCallback cb) {
    listeners_[node_id].push_back({type, std::move(cb)});
}

bool SceneGraph::dispatchEvent(const std::string& type, float x, float y) {
    auto* hit = hitTest(x, y);
    if (!hit) return false;

    // Walk from hit node up through parents (bubbling)
    uint32_t current = hit->id;
    while (current != UINT32_MAX) {
        auto lit = listeners_.find(current);
        if (lit != listeners_.end()) {
            for (auto& entry : lit->second) {
                if (entry.type == type) {
                    entry.callback(hit->id, x, y);
                }
            }
        }
        auto* node = getNode(current);
        current = node ? node->parent : UINT32_MAX;
    }
    return true;
}

} // namespace dong::render
