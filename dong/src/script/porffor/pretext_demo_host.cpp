#include "pretext_demo_host.hpp"

#include "js_bindings_porffor.hpp"
#include "js_text_layout_porffor.hpp"
#include "porffor_mini_json.hpp"

#include "../../dom/dom_manager.hpp"
#include "../../render/overlay_draw.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <random>
#include <sstream>
#include <vector>

namespace dong::script {
namespace {

constexpr float kPanelLeft = 180.0f;
constexpr float kPanelTop = 36.0f;
constexpr float kPanelRight = 720.0f;
constexpr float kPanelBottom = 600.0f;
constexpr float kPanelW = kPanelRight - kPanelLeft;
constexpr float kPanelH = kPanelBottom - kPanelTop;
constexpr int kMaxParticles = 120;
constexpr int kDomParticles = 60;
constexpr int kMaxTextLines = 40;

struct Particle {
    float x = 0;
    float y = 0;
    float vx = 0;
    float vy = 0;
    float r = 0;
    float life = 0;
    float decay = 0;
    int hue = 0;
};

struct Obstacle {
    float cx = 0;
    float cy = 0;
    float r = 0;
    float speed = 0;
    float phase = 0;
};

struct ChartBar {
    float value = 0;
    float target = 0;
};

struct DualModeState {
    std::vector<Particle> particles;
    std::vector<Obstacle> obs;
    std::vector<ChartBar> chart_bars;
    int frame_count = 0;
    int fps = 0;
    double last_fps_time = 0;
    bool inited = false;
};

struct DomOnlyState {
    std::vector<Particle> particles;
    std::vector<Obstacle> obs;
    std::vector<uint64_t> particle_nodes;
    std::vector<uint64_t> obstacle_nodes;
    std::vector<uint64_t> text_nodes;
    uint64_t hud_id = 0;
    uint64_t dom_count_id = 0;
    uint64_t p_count_id = 0;
    int frame_count = 0;
    int fps = 0;
    double last_fps_time = 0;
    bool inited = false;
};

DualModeState g_dual;
DomOnlyState g_dom;

std::mt19937& rng() {
    static std::mt19937 gen{std::random_device{}()};
    return gen;
}

float randFloat(float lo, float hi) {
    std::uniform_real_distribution<float> dist(lo, hi);
    return dist(rng());
}

std::string hslToHex(int h, int s, int l, int alpha = 255) {
    const float sf = s / 100.0f;
    const float lf = l / 100.0f;
    const float a = sf * std::min(lf, 1.0f - lf);
    auto channel = [&](int n) {
        const float k = std::fmod(n + h / 30.0f, 12.0f);
        const float c = lf - a * std::max(std::min({k - 3.0f, 9.0f - k, 1.0f}), -1.0f);
        return static_cast<int>(std::lround(255.0f * c));
    };
    char buf[16];
    if (alpha < 255) {
        std::snprintf(buf, sizeof(buf), "#%02x%02x%02x%02x", channel(0), channel(8), channel(4), alpha);
    } else {
        std::snprintf(buf, sizeof(buf), "#%02x%02x%02x", channel(0), channel(8), channel(4));
    }
    return buf;
}

void setStylePx(JSBindings* bindings, uint64_t node_id, const char* prop, float value) {
    if (!bindings || node_id == 0) {
        return;
    }
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%dpx", static_cast<int>(std::lround(value)));
    bindings->styleSet(node_id, prop, buf);
}

float bouncePosition(float start, float min_value, float max_value, float delta) {
    const float range = max_value - min_value;
    if (range <= 0.0f) {
        return min_value;
    }
    float x = std::fmod(start + delta - min_value, range * 2.0f);
    if (x < 0.0f) {
        x += range * 2.0f;
    }
    return x <= range ? min_value + x : max_value - (x - range);
}

Particle spawnParticle() {
    Particle p;
    p.x = kPanelLeft + 40.0f + randFloat(0.0f, kPanelW - 80.0f);
    p.y = kPanelBottom - 20.0f;
    p.vx = randFloat(-1.0f, 1.0f);
    p.vy = -1.5f - randFloat(0.0f, 2.5f);
    p.r = 3.0f + randFloat(0.0f, 5.0f);
    p.life = 1.0f;
    p.decay = 0.005f + randFloat(0.0f, 0.01f);
    p.hue = static_cast<int>(randFloat(0.0f, 360.0f));
    return p;
}

void initDualMode() {
    if (g_dual.inited) {
        return;
    }
    g_dual.particles.clear();
    g_dual.particles.reserve(kMaxParticles);
    for (int i = 0; i < kMaxParticles; ++i) {
        Particle p = spawnParticle();
        p.life = randFloat(0.0f, 1.0f);
        p.y = kPanelTop + 60.0f + randFloat(0.0f, kPanelH - 120.0f);
        g_dual.particles.push_back(p);
    }
    g_dual.obs = {
        {350.0f, 200.0f, 40.0f, 0.8f, 0.0f},
        {500.0f, 350.0f, 30.0f, 1.2f, 2.1f},
        {600.0f, 250.0f, 25.0f, 0.6f, 4.0f},
    };
    g_dual.chart_bars.clear();
    for (int i = 0; i < 12; ++i) {
        g_dual.chart_bars.push_back({0.3f + randFloat(0.0f, 0.7f), 0.3f + randFloat(0.0f, 0.7f)});
    }
    g_dual.inited = true;
}

void updateFps(DualModeState& st, int t) {
    (void)t;
    ++st.frame_count;
    if (st.last_fps_time <= 0.0) {
        st.last_fps_time = static_cast<double>(st.frame_count);
        return;
    }
    if (st.frame_count >= 60) {
        st.fps = st.frame_count;
        st.frame_count = 0;
    }
}

void updateParticles(std::vector<Particle>& particles) {
    for (auto& p : particles) {
        p.x += p.vx;
        p.y += p.vy;
        p.vy += 0.02f;
        p.life -= p.decay;
        if (p.life <= 0.0f) {
            p = spawnParticle();
        }
    }
}

void updateObstacles(std::vector<Obstacle>& obs, int t) {
    const float center_x = (kPanelLeft + kPanelRight) * 0.5f;
    const float center_y = (kPanelTop + 80.0f + kPanelBottom) * 0.5f;
    for (auto& o : obs) {
        o.cx = center_x + std::sin(t * 0.02f * o.speed + o.phase) * 160.0f;
        o.cy = center_y + std::cos(t * 0.015f * o.speed + o.phase) * 120.0f;
    }
}

int drawParticles(JSBindings* bindings, const std::vector<Particle>& particles, int limit) {
    (void)bindings;
    int n = 0;
    const int count = std::min(limit, static_cast<int>(particles.size()));
    for (int i = 0; i < count; ++i) {
        const auto& p = particles[i];
        const int alpha = static_cast<int>(std::max(0.0f, p.life) * 200.0f);
        std::ostringstream cfg;
        cfg << "{\"cx\":" << p.x << ",\"cy\":" << p.y << ",\"r\":" << p.r * std::max(0.0f, p.life)
            << ",\"color\":\"" << hslToHex(p.hue, 70, 55, alpha) << "\"}";
        bindings->drawCircle(cfg.str());
        ++n;
    }
    return n;
}

int drawObstacles(JSBindings* bindings, const std::vector<Obstacle>& obs) {
    int n = 0;
    for (const auto& o : obs) {
        bindings->drawCircle(
            "{\"cx\":" + std::to_string(o.cx) + ",\"cy\":" + std::to_string(o.cy) +
            ",\"r\":" + std::to_string(o.r) + ",\"color\":\"#58a6ff20\",\"strokeWidth\":1.5}");
        bindings->drawCircle("{\"cx\":" + std::to_string(o.cx) + ",\"cy\":" + std::to_string(o.cy) +
                             ",\"r\":" + std::to_string(o.r * 0.6f) + ",\"color\":\"#58a6ff10\"}");
        n += 2;
    }
    return n;
}

int drawTextFlow(JSBindings* bindings) {
    std::ostringstream circles;
    circles << '[';
    for (size_t i = 0; i < g_dual.obs.size(); ++i) {
        if (i > 0) {
            circles << ',';
        }
        const auto& o = g_dual.obs[i];
        circles << "{\"cx\":" << o.cx << ",\"cy\":" << o.cy << ",\"r\":" << o.r
                << ",\"hPad\":10,\"vPad\":3}";
    }
    circles << ']';
    const std::string col =
        "[{\"x\":" + std::to_string(kPanelLeft + 20.0f) + ",\"y\":" +
        std::to_string(kPanelTop + 80.0f) + ",\"width\":" + std::to_string(kPanelW - 40.0f) +
        ",\"height\":" + std::to_string(kPanelH - 100.0f) + "}]";
    const std::string cfg = pretextDualModeTextConfig(col, circles.str());
    return porfforTextLayoutRenderOverlay(cfg, "#b1bac4", bindings->overlay_draw_);
}

int drawGrid(JSBindings* bindings) {
    int n = 0;
    for (float gx = kPanelLeft + 60.0f; gx < kPanelRight; gx += 80.0f) {
        bindings->drawRect("{\"x\":" + std::to_string(gx) + ",\"y\":" +
                           std::to_string(kPanelTop + 60.0f) + ",\"w\":1,\"h\":" +
                           std::to_string(kPanelH - 80.0f) + ",\"color\":\"#21262d40\"}");
        ++n;
    }
    for (float gy = kPanelTop + 80.0f; gy < kPanelBottom - 20.0f; gy += 60.0f) {
        bindings->drawRect("{\"x\":" + std::to_string(kPanelLeft + 20.0f) + ",\"y\":" +
                           std::to_string(gy) + ",\"w\":" + std::to_string(kPanelW - 40.0f) +
                           ",\"h\":1,\"color\":\"#21262d40\"}");
        ++n;
    }
    return n;
}

int drawCharts(JSBindings* bindings) {
    int n = 0;
    const float bar_w = 30.0f;
    const float gap = 10.0f;
    const int num_bars = static_cast<int>(g_dual.chart_bars.size());
    const float total_w = num_bars * (bar_w + gap) - gap;
    const float start_x = kPanelLeft + (kPanelW - total_w) * 0.5f;
    const float max_h = kPanelH - 160.0f;
    const float base_y = kPanelBottom - 40.0f;
    const char* colors[] = {"#58a6ff", "#3fb950", "#f97316", "#d2a8ff", "#f778ba", "#79c0ff",
                            "#56d364", "#ffa657", "#bc8cff", "#ff7eb6", "#a5d6ff", "#7ee787"};
    for (int bi = 0; bi < num_bars; ++bi) {
        auto& b = g_dual.chart_bars[bi];
        b.value += (b.target - b.value) * 0.05f;
        if (std::abs(b.value - b.target) < 0.01f) {
            b.target = 0.1f + randFloat(0.0f, 0.9f);
        }
        const float bar_h = b.value * max_h;
        const float x = start_x + bi * (bar_w + gap);
        bindings->drawRect("{\"x\":" + std::to_string(x) + ",\"y\":" + std::to_string(base_y - bar_h) +
                           ",\"w\":" + std::to_string(bar_w) + ",\"h\":" + std::to_string(bar_h) +
                           ",\"color\":\"" + colors[bi % 12] + "\",\"radius\":3}");
        ++n;
        const int pct = static_cast<int>(std::lround(b.value * 100.0f));
        bindings->renderText("{\"font\":{\"family\":\"sans-serif\",\"size\":10},\"color\":\"#8b949e\","
                           "\"lines\":[{\"x\":" +
                           std::to_string(x + 4.0f) + ",\"y\":" + std::to_string(base_y + 6.0f) +
                           ",\"text\":\"" + std::to_string(pct) + "%\"}]}");
        ++n;
    }
    bindings->renderText(
        "{\"font\":{\"family\":\"sans-serif\",\"size\":14},\"color\":\"#f0f6fc\",\"lines\":[{\"x\":" +
        std::to_string(start_x) + ",\"y\":" + std::to_string(kPanelTop + 60.0f) +
        ",\"text\":\"Real-Time Metrics (all via dong.drawRect)\"}]}");
    ++n;
    return n;
}

int drawSettings(JSBindings* bindings) {
    const char* info[] = {
        "Architecture: Dual-Mode Rendering Engine", "",
        "Retained Mode (DOM):", "  - HTML/CSS parsed into DOM tree",
        "  - Yoga flexbox layout engine", "  - Display list cached when DOM unchanged",
        "  - Used for: menus, panels, status bars", "",
        "Immediate Mode (Overlay):", "  - dong.drawRect()   -> rounded rectangles",
        "  - dong.drawCircle() -> circles / arcs", "  - dong.renderText() -> batched glyph runs",
        "  - dong.clearOverlay() -> frame reset", "  - Zero DOM tax on animation frames", "",
        "Skip-if-clean optimization:", "  When only overlay changes, DOM display list",
        "  is restored from cache — no DOM traversal."};
    std::ostringstream lines_json;
    lines_json << '[';
    bool first = true;
    const float x0 = kPanelLeft + 30.0f;
    const float y0 = kPanelTop + 80.0f;
    int row = 0;
    for (const char* line : info) {
        if (line[0] == '\0') {
            ++row;
            continue;
        }
        if (!first) {
            lines_json << ',';
        }
        first = false;
        lines_json << "{\"x\":" << x0 << ",\"y\":" << (y0 + row * 22.0f) << ",\"text\":\""
                   << porffor_json::escapeJson(line) << "\"}";
        ++row;
    }
    lines_json << ']';
    bindings->renderText(
        "{\"font\":{\"family\":\"sans-serif\",\"size\":13},\"color\":\"#b1bac4\",\"lines\":" +
        lines_json.str() + "}");
    return 1;
}

const char* modeName(int mode) {
    switch (mode) {
    case 0:
        return "dashboard";
    case 1:
        return "textflow";
    case 2:
        return "particles";
    case 3:
        return "charts";
    case 4:
        return "settings";
    default:
        return "dashboard";
    }
}

uint64_t elementById(JSBindings* bindings, const char* id) {
    if (!bindings || !bindings->dom_manager_) {
        return 0;
    }
    const auto node = bindings->dom_manager_->getElementById(id);
    if (!node) {
        return 0;
    }
    return bindings->getNodeIdFor(node);
}

} // namespace

std::string pretextDynamicHud(int fps, int lines, int pool_size) {
    std::ostringstream out;
    out << "Dynamic Text Flow | " << fps << " fps | " << lines << " lines";
    if (pool_size > 0) {
        out << " | pool: " << pool_size;
    }
    return out.str();
}

int pretextFlowDynamicTick(int mode, uint64_t lines_id, uint64_t hud_id, uint64_t obs_a_id,
                           uint64_t obs_b_id, uint64_t obs_c_id, int t, int fps,
                           JSBindings* bindings, render::OverlayDraw* overlay) {
    if (!bindings) {
        return 0;
    }

    constexpr float col_a_y = 46.0f;
    constexpr float col_a_h = 510.0f;
    constexpr float col_b_x = 444.0f;
    constexpr float col_b_y = 46.0f;
    constexpr float col_b_w = 436.0f;

    constexpr float obs_a_cx = 200.0f;
    constexpr float obs_a_r = 55.0f;
    constexpr float obs_b_w = 100.0f;
    constexpr float obs_b_h = 70.0f;
    constexpr float obs_b_cy = 250.0f;
    constexpr float obs_c_r = 30.0f;

    const float obs_a_cy =
        bouncePosition(200.0f, col_a_y + obs_a_r, col_a_y + col_a_h - obs_a_r, 1.2f * t);
    const float obs_b_cx =
        bouncePosition(650.0f, col_b_x + obs_b_w * 0.5f, col_b_x + col_b_w - obs_b_w * 0.5f,
                       1.5f * t);
    const float obs_c_cx = col_b_x + col_b_w * 0.5f + std::sin(t * 0.03f) * 150.0f;
    const float obs_c_cy = col_b_y + 100.0f + std::sin(t * 0.05f) * 60.0f;

    setStylePx(bindings, obs_a_id, "left", obs_a_cx - obs_a_r);
    setStylePx(bindings, obs_a_id, "top", obs_a_cy - obs_a_r);
    setStylePx(bindings, obs_a_id, "width", obs_a_r * 2.0f);
    setStylePx(bindings, obs_a_id, "height", obs_a_r * 2.0f);

    const float bx = obs_b_cx - obs_b_w * 0.5f;
    const float by = obs_b_cy - obs_b_h * 0.5f;
    setStylePx(bindings, obs_b_id, "left", bx);
    setStylePx(bindings, obs_b_id, "top", by);
    setStylePx(bindings, obs_b_id, "width", obs_b_w);
    setStylePx(bindings, obs_b_id, "height", obs_b_h);

    setStylePx(bindings, obs_c_id, "left", obs_c_cx - obs_c_r);
    setStylePx(bindings, obs_c_id, "top", obs_c_cy - obs_c_r);
    setStylePx(bindings, obs_c_id, "width", obs_c_r * 2.0f);
    setStylePx(bindings, obs_c_id, "height", obs_c_r * 2.0f);

    const std::string obstacles = pretextFlowObstaclesJson(
        obs_a_cx, obs_a_cy, obs_a_r, bx, by, obs_b_w, obs_b_h, obs_c_cx, obs_c_cy,
        obs_c_r);
    const std::string cfg = pretextTypoConfig(pretextFlowColumnsDynamic(), obstacles);

    int line_count = 0;
    if (mode == 1) {
        if (overlay) {
            bindings->clearOverlay();
            line_count = porfforTextLayoutRenderOverlay(cfg, "#c9d1d9", overlay);
        }
    } else {
        line_count = porfforTextLayoutMountLines(lines_id, cfg, "line", false, bindings);
    }

    if (hud_id != 0) {
        std::ostringstream hud;
        if (mode == 1) {
            hud << "Direct Draw | " << fps << " fps | " << line_count << " lines";
        } else {
            hud << pretextDynamicHud(fps, line_count, 0);
        }
        bindings->setNodeTextContent(hud_id, hud.str());
    }
    return line_count;
}

void pretextDualModeOverlayTick(int mode, int t, JSBindings* bindings, render::OverlayDraw* overlay) {
    if (!bindings || !overlay) {
        return;
    }
    initDualMode();
    updateFps(g_dual, t);
    updateParticles(g_dual.particles);
    updateObstacles(g_dual.obs, t);

    bindings->clearOverlay();
    int overlay_count = 0;
    int line_count = 0;

    if (mode == 0) {
        overlay_count += drawParticles(bindings, g_dual.particles, 60);
        overlay_count += drawObstacles(bindings, g_dual.obs);
        line_count = drawTextFlow(bindings);
        overlay_count += 1;
        overlay_count += drawGrid(bindings);
    } else if (mode == 1) {
        overlay_count += drawObstacles(bindings, g_dual.obs);
        line_count = drawTextFlow(bindings);
        overlay_count += 1;
    } else if (mode == 2) {
        overlay_count += drawParticles(bindings, g_dual.particles, kMaxParticles);
    } else if (mode == 3) {
        overlay_count += drawCharts(bindings);
    } else if (mode == 4) {
        overlay_count += drawSettings(bindings);
    }

    bindings->renderText(
        "{\"font\":{\"family\":\"sans-serif\",\"size\":11},\"color\":\"#484f58\",\"lines\":[{\"x\":" +
        std::to_string(kPanelLeft + 16.0f) + ",\"y\":" + std::to_string(kPanelBottom - 14.0f) +
        ",\"text\":\"" + std::to_string(g_dual.fps) + " fps | mode: " + modeName(mode) +
        " | overlay: " + std::to_string(overlay_count) + "\"}]}");

    if (g_dual.frame_count == 1) {
        const uint64_t ov_count = elementById(bindings, "ov-count");
        const uint64_t p_count = elementById(bindings, "p-count");
        if (ov_count) {
            bindings->setNodeTextContent(ov_count, std::to_string(overlay_count));
        }
        if (p_count) {
            int pval = (mode == 2) ? kMaxParticles : (mode == 0 ? 60 : 0);
            bindings->setNodeTextContent(p_count, std::to_string(pval));
        }
    }
    (void)line_count;
}

void pretextDomOnlyInit(JSBindings* bindings) {
    if (!bindings || g_dom.inited) {
        return;
    }
    const uint64_t container = elementById(bindings, "dynamic-container");
    if (!container) {
        return;
    }
    g_dom.particle_nodes.clear();
    for (int i = 0; i < kDomParticles; ++i) {
        const uint64_t id = bindings->parseHtmlDetached("<div class=\"particle\"></div>");
        if (id) {
            bindings->appendChild(container, id);
            g_dom.particle_nodes.push_back(id);
        }
    }
    g_dom.obstacle_nodes.clear();
    for (int i = 0; i < 3; ++i) {
        const uint64_t id = bindings->parseHtmlDetached("<div class=\"obstacle\"></div>");
        if (id) {
            bindings->appendChild(container, id);
            g_dom.obstacle_nodes.push_back(id);
        }
    }
    g_dom.text_nodes.clear();
    for (int i = 0; i < kMaxTextLines; ++i) {
        const uint64_t id =
            bindings->parseHtmlDetached("<div class=\"text-line\" style=\"display:none\"></div>");
        if (id) {
            bindings->appendChild(container, id);
            g_dom.text_nodes.push_back(id);
        }
    }
    g_dom.hud_id = elementById(bindings, "hud");
    g_dom.dom_count_id = elementById(bindings, "dom-count");
    g_dom.p_count_id = elementById(bindings, "p-count");

    g_dom.particles.clear();
    for (int i = 0; i < kDomParticles; ++i) {
        Particle p = spawnParticle();
        p.life = randFloat(0.0f, 1.0f);
        p.y = kPanelTop + 60.0f + randFloat(0.0f, kPanelH - 120.0f);
        g_dom.particles.push_back(p);
    }
    g_dom.obs = {
        {350.0f, 200.0f, 40.0f, 0.8f, 0.0f},
        {500.0f, 350.0f, 30.0f, 1.2f, 2.1f},
        {600.0f, 250.0f, 25.0f, 0.6f, 4.0f},
    };
    g_dom.inited = true;
}

void pretextDomOnlyTick(int t, JSBindings* bindings) {
    if (!bindings) {
        return;
    }
    pretextDomOnlyInit(bindings);
    ++g_dom.frame_count;
    if (g_dom.frame_count >= 60) {
        g_dom.fps = g_dom.frame_count;
        g_dom.frame_count = 0;
    }

    int alive = 0;
    for (size_t i = 0; i < g_dom.particles.size(); ++i) {
        auto& p = g_dom.particles[i];
        p.x += p.vx;
        p.y += p.vy;
        p.vy += 0.02f;
        p.life -= p.decay;
        if (p.life <= 0.0f) {
            p = spawnParticle();
        }
        ++alive;
        if (i < g_dom.particle_nodes.size()) {
            const float sz = p.r * p.life * 2.0f;
            const uint64_t el = g_dom.particle_nodes[i];
            setStylePx(bindings, el, "left", p.x - sz * 0.5f);
            setStylePx(bindings, el, "top", p.y - sz * 0.5f);
            setStylePx(bindings, el, "width", sz);
            setStylePx(bindings, el, "height", sz);
            bindings->styleSet(el, "background", hslToHex(p.hue, 70, 55));
            char opacity[16];
            std::snprintf(opacity, sizeof(opacity), "%.3f", p.life);
            bindings->styleSet(el, "opacity", opacity);
        }
    }

    updateObstacles(g_dom.obs, t);
    for (size_t oi = 0; oi < g_dom.obs.size() && oi < g_dom.obstacle_nodes.size(); ++oi) {
        const auto& o = g_dom.obs[oi];
        const uint64_t el = g_dom.obstacle_nodes[oi];
        setStylePx(bindings, el, "left", o.cx - o.r);
        setStylePx(bindings, el, "top", o.cy - o.r);
        setStylePx(bindings, el, "width", o.r * 2.0f);
        setStylePx(bindings, el, "height", o.r * 2.0f);
    }

    std::ostringstream circles;
    circles << '[';
    for (size_t i = 0; i < g_dom.obs.size(); ++i) {
        if (i > 0) {
            circles << ',';
        }
        const auto& o = g_dom.obs[i];
        circles << "{\"cx\":" << o.cx << ",\"cy\":" << o.cy << ",\"r\":" << o.r
                << ",\"hPad\":10,\"vPad\":3}";
    }
    circles << ']';
    const std::string col =
        "[{\"x\":" + std::to_string(kPanelLeft + 20.0f) + ",\"y\":" +
        std::to_string(kPanelTop + 80.0f) + ",\"width\":" + std::to_string(kPanelW - 40.0f) +
        ",\"height\":" + std::to_string(kPanelH - 100.0f) + "}]";
    const std::string cfg = pretextDualModeTextConfig(col, circles.str());
    const std::string layout = bindings->textLayout(cfg);
    const auto lines = pretextParseLayoutLines(layout);

    for (int i = 0; i < kMaxTextLines; ++i) {
        if (static_cast<size_t>(i) >= g_dom.text_nodes.size()) {
            break;
        }
        const uint64_t el = g_dom.text_nodes[i];
        if (static_cast<size_t>(i) < lines.size()) {
            bindings->styleSet(el, "display", "block");
            setStylePx(bindings, el, "left", lines[i].x);
            setStylePx(bindings, el, "top", lines[i].y);
            bindings->setNodeTextContent(el, lines[i].text);
        } else {
            bindings->styleSet(el, "display", "none");
        }
    }

    const int dom_count =
        kDomParticles + 3 + static_cast<int>(lines.size()) + static_cast<int>(g_dom.text_nodes.size());
    if (g_dom.hud_id) {
        bindings->setNodeTextContent(
            g_dom.hud_id, std::to_string(g_dom.fps) + " fps | dom-items: " + std::to_string(dom_count) +
                              " | particles: " + std::to_string(alive) + " | lines: " +
                              std::to_string(lines.size()) + " | mode: DOM-only");
    }
    if (g_dom.dom_count_id) {
        bindings->setNodeTextContent(g_dom.dom_count_id, std::to_string(dom_count));
    }
    if (g_dom.p_count_id) {
        bindings->setNodeTextContent(g_dom.p_count_id, std::to_string(alive));
    }
}

} // namespace dong::script
