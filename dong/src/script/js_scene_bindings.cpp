#include "quickjs_compat.h"
#include "../render/scene_graph.hpp"
#include "../render/painter/painter_style_utils.hpp"
#include <string>
#include <cstdlib>

namespace dong::script {

static render::SceneGraph& sceneInstance() {
    static render::SceneGraph sg;
    return sg;
}

render::SceneGraph& getGlobalSceneGraph() {
    return sceneInstance();
}

static std::string jsStr(JSContext* ctx, JSValueConst obj, const char* key,
                         const char* fallback = "") {
    JSValue v = JS_GetPropertyStr(ctx, obj, key);
    if (JS_IsUndefined(v) || JS_IsNull(v)) { JS_FreeValue(ctx, v); return fallback; }
    const char* s = JS_ToCString(ctx, v);
    std::string r = s ? s : fallback;
    if (s) JS_FreeCString(ctx, s);
    JS_FreeValue(ctx, v);
    return r;
}

static double jsNum(JSContext* ctx, JSValueConst obj, const char* key, double fallback = 0) {
    JSValue v = JS_GetPropertyStr(ctx, obj, key);
    if (JS_IsUndefined(v) || JS_IsNull(v)) { JS_FreeValue(ctx, v); return fallback; }
    double d = 0; JS_ToFloat64(ctx, &d, v); JS_FreeValue(ctx, v); return d;
}

static bool jsBool(JSContext* ctx, JSValueConst obj, const char* key, bool fallback = true) {
    JSValue v = JS_GetPropertyStr(ctx, obj, key);
    if (JS_IsUndefined(v) || JS_IsNull(v)) { JS_FreeValue(ctx, v); return fallback; }
    int b = JS_ToBool(ctx, v);
    JS_FreeValue(ctx, v);
    return b > 0;
}

static render::Color parseColor(const std::string& s,
                                render::Color fallback = {0, 0, 0, 0}) {
    if (s.empty()) return fallback;
    return render::painter_detail::makeColorFromCss(s);
}

// dong.scene.addNode({name, x, y, w, h, background, border, ...})
static JSValue js_scene_addNode(JSContext* ctx, JSValueConst, int argc, JSValueConst* argv) {
    if (argc < 1 || !JS_IsObject(argv[0]))
        return JS_ThrowTypeError(ctx, "scene.addNode: config object required");
    JSValueConst cfg = argv[0];

    render::SceneNode node;
    node.name = jsStr(ctx, cfg, "name");
    node.x = (float)jsNum(ctx, cfg, "x");
    node.y = (float)jsNum(ctx, cfg, "y");
    node.width = (float)jsNum(ctx, cfg, "w", jsNum(ctx, cfg, "width"));
    node.height = (float)jsNum(ctx, cfg, "h", jsNum(ctx, cfg, "height"));
    node.z_order = (int)jsNum(ctx, cfg, "zOrder", 0);
    node.opacity = (float)jsNum(ctx, cfg, "opacity", 1.0);
    node.visible = jsBool(ctx, cfg, "visible", true);
    node.border_width = (float)jsNum(ctx, cfg, "borderWidth");
    node.border_radius = (float)jsNum(ctx, cfg, "borderRadius");
    node.font_size = (float)jsNum(ctx, cfg, "fontSize", 16);
    node.line_height = (float)jsNum(ctx, cfg, "lineHeight", 0);
    node.font_family = jsStr(ctx, cfg, "fontFamily", "sans-serif");
    node.font_weight = jsStr(ctx, cfg, "fontWeight", "normal");
    node.text_align = jsStr(ctx, cfg, "textAlign", "left");
    node.text = jsStr(ctx, cfg, "text");
    node.image_src = jsStr(ctx, cfg, "imageSrc");

    node.background_color = parseColor(jsStr(ctx, cfg, "background"));
    node.border_color = parseColor(jsStr(ctx, cfg, "border"));
    node.text_color = parseColor(jsStr(ctx, cfg, "color"), {1, 1, 1, 1});

    node.border_top_width = (float)jsNum(ctx, cfg, "borderTopWidth", -1);
    node.border_right_width = (float)jsNum(ctx, cfg, "borderRightWidth", -1);
    node.border_bottom_width = (float)jsNum(ctx, cfg, "borderBottomWidth", -1);
    node.border_left_width = (float)jsNum(ctx, cfg, "borderLeftWidth", -1);
    auto btc = jsStr(ctx, cfg, "borderTopColor");
    auto brc = jsStr(ctx, cfg, "borderRightColor");
    auto bbc = jsStr(ctx, cfg, "borderBottomColor");
    auto blc = jsStr(ctx, cfg, "borderLeftColor");
    if (!btc.empty()) node.border_top_color = parseColor(btc);
    if (!brc.empty()) node.border_right_color = parseColor(brc);
    if (!bbc.empty()) node.border_bottom_color = parseColor(bbc);
    if (!blc.empty()) node.border_left_color = parseColor(blc);

    uint32_t parent = (uint32_t)jsNum(ctx, cfg, "parent", UINT32_MAX);
    node.parent = parent;

    uint32_t id = sceneInstance().addNode(std::move(node));
    return JS_NewInt32(ctx, (int32_t)id);
}

// dong.scene.remove(id)
static JSValue js_scene_remove(JSContext* ctx, JSValueConst, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_UNDEFINED;
    int32_t id = 0;
    JS_ToInt32(ctx, &id, argv[0]);
    sceneInstance().removeNode((uint32_t)id);
    return JS_UNDEFINED;
}

// dong.scene.set(id, prop, value)
static JSValue js_scene_set(JSContext* ctx, JSValueConst, int argc, JSValueConst* argv) {
    if (argc < 3) return JS_ThrowTypeError(ctx, "scene.set: (id, prop, value) required");
    int32_t id = 0;
    JS_ToInt32(ctx, &id, argv[0]);
    const char* prop_s = JS_ToCString(ctx, argv[1]);
    if (!prop_s) return JS_UNDEFINED;
    std::string prop(prop_s);
    JS_FreeCString(ctx, prop_s);

    auto& sg = sceneInstance();

    // Detect value type
    if (JS_IsBool(argv[2])) {
        int b = JS_ToBool(ctx, argv[2]);
        sg.setBool((uint32_t)id, prop, b > 0);
    } else if (JS_IsNumber(argv[2])) {
        double d = 0;
        JS_ToFloat64(ctx, &d, argv[2]);
        sg.setFloat((uint32_t)id, prop, (float)d);
    } else {
        const char* vs = JS_ToCString(ctx, argv[2]);
        if (vs) {
            std::string val(vs);
            JS_FreeCString(ctx, vs);
            if (prop == "background" || prop == "borderColor" || prop == "color" ||
                prop == "borderTopColor" || prop == "borderRightColor" ||
                prop == "borderBottomColor" || prop == "borderLeftColor") {
                sg.setColor((uint32_t)id, prop, parseColor(val));
            } else {
                sg.setString((uint32_t)id, prop, val);
            }
        }
    }
    return JS_UNDEFINED;
}

// dong.scene.find(name) -> id or -1
static JSValue js_scene_find(JSContext* ctx, JSValueConst, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_NewInt32(ctx, -1);
    const char* name = JS_ToCString(ctx, argv[0]);
    if (!name) return JS_NewInt32(ctx, -1);
    auto* node = sceneInstance().findByName(name);
    int32_t result = node ? (int32_t)node->id : -1;
    JS_FreeCString(ctx, name);
    return JS_NewInt32(ctx, result);
}

// dong.scene.on(id, type, callback)
static JSValue js_scene_on(JSContext* ctx, JSValueConst, int argc, JSValueConst* argv) {
    if (argc < 3) return JS_ThrowTypeError(ctx, "scene.on: (id, type, callback) required");
    int32_t id = 0;
    JS_ToInt32(ctx, &id, argv[0]);
    const char* type_s = JS_ToCString(ctx, argv[1]);
    if (!type_s) return JS_UNDEFINED;
    std::string type(type_s);
    JS_FreeCString(ctx, type_s);

    if (!JS_IsFunction(ctx, argv[2])) {
        return JS_ThrowTypeError(ctx, "scene.on: third arg must be function");
    }

    JSValue func = JS_DupValue(ctx, argv[2]);
    JSContext* captured_ctx = ctx;

    sceneInstance().addEventListener((uint32_t)id, type,
        [captured_ctx, func](uint32_t node_id, float x, float y) {
            JSValue args[3];
            args[0] = JS_NewInt32(captured_ctx, (int32_t)node_id);
            args[1] = JS_NewFloat64(captured_ctx, x);
            args[2] = JS_NewFloat64(captured_ctx, y);
            JSValue global = JS_GetGlobalObject(captured_ctx);
            JSValue ret = JS_Call(captured_ctx, func, global, 3, args);
            JS_FreeValue(captured_ctx, ret);
            JS_FreeValue(captured_ctx, global);
            for (int i = 0; i < 3; i++) JS_FreeValue(captured_ctx, args[i]);
        });

    return JS_UNDEFINED;
}

// dong.scene.clear()
static JSValue js_scene_clear(JSContext* ctx, JSValueConst, int, JSValueConst*) {
    sceneInstance().clear();
    return JS_UNDEFINED;
}

// dong.scene.count() -> number of nodes
static JSValue js_scene_count(JSContext* ctx, JSValueConst, int, JSValueConst*) {
    return JS_NewInt32(ctx, (int32_t)sceneInstance().nodeCount());
}

void registerSceneGraphAPI(JSContext* ctx) {
    JSValue global = JS_GetGlobalObject(ctx);
    JSValue dong_obj = JS_GetPropertyStr(ctx, global, "dong");
    if (JS_IsUndefined(dong_obj) || JS_IsNull(dong_obj)) {
        JS_FreeValue(ctx, dong_obj);
        dong_obj = JS_NewObject(ctx);
    }

    JSValue scene_obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, scene_obj, "addNode",
                      JS_NewCFunction(ctx, js_scene_addNode, "addNode", 1));
    JS_SetPropertyStr(ctx, scene_obj, "remove",
                      JS_NewCFunction(ctx, js_scene_remove, "remove", 1));
    JS_SetPropertyStr(ctx, scene_obj, "set",
                      JS_NewCFunction(ctx, js_scene_set, "set", 3));
    JS_SetPropertyStr(ctx, scene_obj, "find",
                      JS_NewCFunction(ctx, js_scene_find, "find", 1));
    JS_SetPropertyStr(ctx, scene_obj, "on",
                      JS_NewCFunction(ctx, js_scene_on, "on", 3));
    JS_SetPropertyStr(ctx, scene_obj, "clear",
                      JS_NewCFunction(ctx, js_scene_clear, "clear", 0));
    JS_SetPropertyStr(ctx, scene_obj, "count",
                      JS_NewCFunction(ctx, js_scene_count, "count", 0));

    JS_SetPropertyStr(ctx, dong_obj, "scene", scene_obj);
    JS_SetPropertyStr(ctx, global, "dong", dong_obj);
    JS_FreeValue(ctx, global);
}

} // namespace dong::script
