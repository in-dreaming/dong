// Porffor prelude — flat helpers only (no object method calls).
// Multi-arg host calls use single-arg stage + commit imports (Porffor 2c limitation).
// String channel: T16 pull-based host→wasm + UTF-8 toUtf8 for wasm→host.

function utf8AppendCodePoint(out, cp) {
  if (cp < 128) {
    return out + String.fromCharCode(cp);
  }
  if (cp < 2048) {
    return out + String.fromCharCode(192 | (cp >> 6), 128 | (cp & 63));
  }
  if (cp < 65536) {
    return out + String.fromCharCode(224 | (cp >> 12), 128 | ((cp >> 6) & 63), 128 | (cp & 63));
  }
  return out + String.fromCharCode(240 | (cp >> 18), 128 | ((cp >> 12) & 63), 128 | ((cp >> 6) & 63), 128 | (cp & 63));
}

function toUtf8(s) {
  var out = "";
  var i = 0;
  var len = s.length;
  while (i < len) {
    var cp = s.charCodeAt(i);
    if (cp >= 55296 && cp <= 56319 && i + 1 < len) {
      var next = s.charCodeAt(i + 1);
      if (next >= 56320 && next <= 57343) {
        cp = 65536 + ((cp - 55296) << 10) + (next - 56320);
        i = i + 1;
      }
    }
    out = utf8AppendCodePoint(out, cp);
    i = i + 1;
  }
  return out;
}

function toBytes(s) {
  return toUtf8(s);
}

function pullHostString() {
  var len = dong_str_len();
  if (len === 0) {
    return "";
  }
  var out = "";
  var i = 0;
  var b0 = 0;
  var b1 = 0;
  var b2 = 0;
  var b3 = 0;
  var cp = 0;
  while (i < len) {
    b0 = dong_str_byte_at(i);
    if (b0 < 0) {
      break;
    }
    if (b0 < 128) {
      out = out + String.fromCharCode(b0);
      i = i + 1;
    } else if ((b0 & 224) === 192) {
      if (i + 1 >= len) {
        break;
      }
      b1 = dong_str_byte_at(i + 1);
      cp = ((b0 & 31) << 6) | (b1 & 63);
      out = out + String.fromCharCode(cp);
      i = i + 2;
    } else if ((b0 & 240) === 224) {
      if (i + 2 >= len) {
        break;
      }
      b1 = dong_str_byte_at(i + 1);
      b2 = dong_str_byte_at(i + 2);
      cp = ((b0 & 15) << 12) | ((b1 & 63) << 6) | (b2 & 63);
      out = out + String.fromCharCode(cp);
      i = i + 3;
    } else if ((b0 & 248) === 240) {
      if (i + 3 >= len) {
        break;
      }
      b1 = dong_str_byte_at(i + 1);
      b2 = dong_str_byte_at(i + 2);
      b3 = dong_str_byte_at(i + 3);
      cp = ((b0 & 7) << 18) | ((b1 & 63) << 12) | ((b2 & 63) << 6) | (b3 & 63);
      if (cp > 65535) {
        cp = cp - 65536;
        out = out + String.fromCharCode(55296 + (cp >> 10), 56320 + (cp & 1023));
      } else {
        out = out + String.fromCharCode(cp);
      }
      i = i + 4;
    } else {
      i = i + 1;
    }
  }
  return out;
}

function getElementById(id) {
  return dong_dom_getElementById(toUtf8(id));
}

function setTextContent(nodeId, text) {
  dong_stage_0(nodeId);
  dong_stage_1(toUtf8(text));
  dong_commit_set_textContent();
}

function getTextContent(nodeId) {
  dong_dom_get_textContent(nodeId);
  return pullHostString();
}

function addEventListener(nodeId, type, handlerName) {
  dong_stage_0(nodeId);
  dong_stage_1(toUtf8(type));
  dong_stage_2(toUtf8(handlerName));
  dong_commit_addEventListener();
}

function dongLog(msg) {
  dong_print(toUtf8(msg));
}

function benchLog(msg) {
  dong_bench_log(toUtf8(msg));
}

function setTimeout(handlerName, ms) {
  dong_stage_0(ms);
  dong_stage_1(toUtf8(handlerName));
  return dong_commit_setTimeout();
}

function setInterval(handlerName, ms) {
  dong_stage_0(ms);
  dong_stage_1(toUtf8(handlerName));
  return dong_commit_setInterval();
}

function clearTimeout(timerId) {
  dong_clear_timeout(timerId);
}

function clearInterval(timerId) {
  dong_clear_interval(timerId);
}

function requestAnimationFrame(handlerName) {
  dong_stage_0(toUtf8(handlerName));
  return dong_commit_requestAnimationFrame();
}

function cancelAnimationFrame(rafId) {
  dong_cancel_animation_frame(rafId);
}

function rafTimestamp() {
  return dong_raf_timestamp();
}

function dongStateSetNum(slot, v) {
  dong_state_set_num(slot, v);
}

function dongStateGetNum(slot) {
  return dong_state_get_num(slot);
}

var METRIC_OFFSET_WIDTH = 0;
var METRIC_OFFSET_HEIGHT = 1;
var METRIC_OFFSET_TOP = 2;
var METRIC_OFFSET_LEFT = 3;
var METRIC_CLIENT_WIDTH = 4;
var METRIC_CLIENT_HEIGHT = 5;
var METRIC_SCROLL_WIDTH = 6;
var METRIC_SCROLL_HEIGHT = 7;

function getValue(nodeId) {
  dong_get_value(nodeId);
  return pullHostString();
}

function setValue(nodeId, v) {
  dong_set_value(nodeId, toUtf8(v));
}

function getChecked(nodeId) {
  return dong_get_checked(nodeId);
}

function setChecked(nodeId, v) {
  dong_set_checked(nodeId, v ? 1 : 0);
}

function getDisabled(nodeId) {
  return dong_get_disabled(nodeId);
}

function setDisabled(nodeId, v) {
  dong_set_disabled(nodeId, v ? 1 : 0);
}

function getAttribute(nodeId, name) {
  dong_get_attribute(nodeId, toUtf8(name));
  return pullHostString();
}

function setAttribute(nodeId, name, value) {
  dong_set_attribute(nodeId, toUtf8(name), toUtf8(value));
}

function removeAttribute(nodeId, name) {
  dong_remove_attribute(nodeId, toUtf8(name));
}

function setInnerHTML(nodeId, html) {
  dong_set_inner_html(nodeId, toUtf8(html));
}

function querySelector(rootId, selector) {
  return dong_query_selector(rootId, toUtf8(selector));
}

function querySelectorAll(rootId, selector) {
  dong_query_selector_all(rootId, toUtf8(selector));
  return pullHostString();
}

function getElementsByTagName(rootId, tag) {
  dong_get_elements_by_tag_name(rootId, toUtf8(tag));
  return pullHostString();
}

function classAdd(nodeId, cls) {
  dong_class_add(nodeId, toUtf8(cls));
}

function classRemove(nodeId, cls) {
  dong_class_remove(nodeId, toUtf8(cls));
}

function classToggle(nodeId, cls) {
  return dong_class_toggle(nodeId, toUtf8(cls));
}

function classContains(nodeId, cls) {
  return dong_class_contains(nodeId, toUtf8(cls));
}

function setStyle(nodeId, prop, value) {
  dong_style_set(nodeId, toUtf8(prop), toUtf8(value));
}

function getStyle(nodeId, prop) {
  dong_style_get(nodeId, toUtf8(prop));
  return pullHostString();
}

function getComputedStyleProp(nodeId, prop) {
  dong_computed_style_get(nodeId, toUtf8(prop));
  return pullHostString();
}

function getRect(nodeId) {
  dong_get_rect(nodeId);
  return pullHostString();
}

function getMetric(nodeId, metricId) {
  return dong_get_metric(nodeId, metricId);
}

function getScrollTop(nodeId) {
  return dong_get_scroll_top(nodeId);
}

function setScrollTop(nodeId, v) {
  dong_set_scroll_top(nodeId, v);
}

function getScrollLeft(nodeId) {
  return dong_get_scroll_left(nodeId);
}

function setScrollLeft(nodeId, v) {
  dong_set_scroll_left(nodeId, v);
}

function focusNode(nodeId) {
  dong_focus(nodeId);
}

function blurNode(nodeId) {
  dong_blur(nodeId);
}

function clickNode(nodeId) {
  dong_click(nodeId);
}

function matchesSelector(nodeId, selector) {
  return dong_matches(nodeId, toUtf8(selector));
}

function closestSelector(nodeId, selector) {
  return dong_closest(nodeId, toUtf8(selector));
}

function createElement(tag) {
  return dong_create_element(toUtf8(tag));
}

function createTextNode(text) {
  return dong_create_text_node(toUtf8(text));
}

function appendChild(parentId, childId) {
  dong_append_child(parentId, childId);
}

function insertBefore(parentId, newId, refId) {
  dong_insert_before(parentId, newId, refId);
}

function removeNode(nodeId) {
  dong_remove(nodeId);
}

function replaceChild(parentId, newId, oldId) {
  dong_replace_child(parentId, newId, oldId);
}

function getParentId(nodeId) {
  return dong_parent(nodeId);
}

function getFirstChildId(nodeId) {
  return dong_first_child(nodeId);
}

function getNextSiblingId(nodeId) {
  return dong_next_sibling(nodeId);
}

function cloneNodeId(nodeId, deep) {
  return dong_clone_node(nodeId, deep ? 1 : 0);
}

function eventType() {
  dong_event_type();
  return pullHostString();
}

function eventTarget() {
  return dong_event_target();
}

function eventKey() {
  dong_event_key();
  return pullHostString();
}

function eventKeyCode() {
  return dong_event_key_code();
}

function eventX() {
  return dong_event_x();
}

function eventY() {
  return dong_event_y();
}

function eventButton() {
  return dong_event_button();
}

function eventModifiers() {
  return dong_event_modifiers();
}

function eventValue() {
  dong_event_value();
  return pullHostString();
}

function preventDefault() {
  dong_event_prevent_default();
}

function stopPropagation() {
  dong_event_stop_propagation();
}

function parseHtml(html) {
  dong_parse_html(toUtf8(html));
  var idStr = pullHostString();
  if (idStr === "") {
    return 0;
  }
  return idStr - 0;
}

function formSerialize(formId) {
  dong_form_serialize(formId);
  return pullHostString();
}

function selectionText() {
  dong_selection_text();
  return pullHostString();
}

function dongFetch(url, exportName) {
  return dong_fetch_start(toUtf8(url), toUtf8(exportName));
}

function fetchAbort(requestId) {
  dong_fetch_abort(requestId);
}

function fetchRequestId() {
  return dong_fetch_request_id();
}

function fetchStatus() {
  return dong_fetch_status();
}

function fetchOk() {
  return dong_fetch_ok();
}

function fetchBody() {
  dong_fetch_body();
  return pullHostString();
}

function fetchError() {
  dong_fetch_error();
  return pullHostString();
}

function fetchHeader(name) {
  dong_fetch_header(toUtf8(name));
  return pullHostString();
}

function clipboardWrite(text) {
  dong_clipboard_write(toUtf8(text));
}

function clipboardRead() {
  dong_clipboard_read();
  return pullHostString();
}

function matchMedia(query) {
  return dong_match_media(toUtf8(query)) !== 0;
}

function cssSupports(prop, value) {
  return dong_css_supports(toUtf8(prop), toUtf8(value)) !== 0;
}

function dialogShow(nodeId) {
  dong_dialog_show(nodeId);
}

function dialogShowModal(nodeId) {
  dong_dialog_show_modal(nodeId);
}

function dialogClose(nodeId, returnValue) {
  dong_dialog_close(nodeId, toUtf8(returnValue || ""));
}

function dialogReturnValue(nodeId) {
  dong_dialog_return_value(nodeId);
  return pullHostString();
}

function dialogOpen(nodeId) {
  return dong_dialog_open(nodeId) !== 0;
}

function sceneAddNode(configJson) {
  return dong_scene_add_node(toUtf8(configJson));
}

function sceneRemove(id) {
  dong_scene_remove(id);
}

function sceneSet(id, prop, value) {
  dong_stage_0(id);
  dong_stage_1(toUtf8(prop));
  dong_stage_2(toUtf8(value));
  dong_scene_set(id, toUtf8(prop), toUtf8(value));
}

function sceneFind(name) {
  return dong_scene_find(toUtf8(name));
}

function sceneOn(id, type, exportName) {
  dong_stage_0(id);
  dong_stage_1(toUtf8(type));
  dong_stage_2(toUtf8(exportName));
  dong_scene_on(id, toUtf8(type), toUtf8(exportName));
}

function sceneClear() {
  dong_scene_clear();
}

function sceneCount() {
  return dong_scene_count();
}

function textLayout(configJson) {
  dong_text_layout(toUtf8(configJson));
  return pullHostString();
}

function clearOverlay() {
  dong_clear_overlay();
}

function renderText(configJson) {
  dong_render_text(toUtf8(configJson));
}

function drawRect(configJson) {
  dong_draw_rect(toUtf8(configJson));
}

function drawCircle(configJson) {
  dong_draw_circle(toUtf8(configJson));
}

var nId = 0;

function porfPatch_count() {
  setTextContent(nId, String(count));
}

function porfInit() {
  nId = getElementById('n');
  porfPatch_count();
  dongLog('porfInit');
}

var count = 0;

porfInit();
