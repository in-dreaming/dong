#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t u8;
typedef int32_t i32;
typedef uint32_t u32;
typedef double f64;

static char* g_memory = NULL;
static u32 g_memory_pages = 1;
static char g_result_slot[65536];
static size_t g_result_len = 0;

static size_t memory_cap(void) {
  return (size_t)g_memory_pages * 65536u;
}

static void set_result(const char* bytes, size_t len) {
  if (len >= sizeof(g_result_slot)) {
    len = sizeof(g_result_slot) - 1;
  }
  memcpy(g_result_slot, bytes, len);
  g_result_len = len;
  g_result_slot[len] = '\0';
}

static int read_utf8_bytestring(f64 ptr, char* out, size_t out_cap, size_t* out_len) {
  if (!g_memory || ptr < 0) {
    return 0;
  }
  size_t offset = (size_t)ptr;
  size_t cap = memory_cap();
  if (offset + 4 > cap) {
    return 0;
  }
  u32 len = *(u32*)(g_memory + offset);
  if (len > 4 * 1024 * 1024 || offset + 4 + len > cap) {
    return 0;
  }
  if (len >= out_cap) {
    return 0;
  }
  memcpy(out, g_memory + offset + 4, len);
  out[len] = '\0';
  *out_len = len;
  return 1;
}

f64 __porf_import_dong_str_len(void) {
  return (f64)g_result_len;
}

f64 __porf_import_dong_str_byte_at(f64 index) {
  if (index < 0) {
    return -1;
  }
  size_t i = (size_t)index;
  if (i >= g_result_len) {
    return -1;
  }
  return (f64)(unsigned char)g_result_slot[i];
}

void __porf_import_dong_print(f64 text_ptr) {
  char buf[4096];
  size_t len = 0;
  if (read_utf8_bytestring(text_ptr, buf, sizeof(buf), &len)) {
    fwrite(buf, 1, len, stdout);
  }
  fputc('\n', stdout);
}

static char g_values[256][256];
static int g_checked[256];
static int g_disabled[256];
static char g_attrs[256][64][256];
static char g_classes[256][512];
static double g_scroll_top[256];
static char g_style_color[256][64];

void __porf_import_dong_get_value(f64 node_id) {
  int id = (int)node_id;
  set_result(g_values[id], strlen(g_values[id]));
}

void __porf_import_dong_set_value(f64 node_id, f64 value_ptr) {
  char buf[256];
  size_t len = 0;
  if (!read_utf8_bytestring(value_ptr, buf, sizeof(buf), &len)) {
    return;
  }
  strncpy(g_values[(int)node_id], buf, sizeof(g_values[0]) - 1);
}

f64 __porf_import_dong_get_checked(f64 node_id) {
  return (f64)g_checked[(int)node_id];
}

void __porf_import_dong_set_checked(f64 node_id, f64 checked) {
  g_checked[(int)node_id] = checked != 0.0 ? 1 : 0;
}

f64 __porf_import_dong_get_disabled(f64 node_id) {
  return (f64)g_disabled[(int)node_id];
}

void __porf_import_dong_set_disabled(f64 node_id, f64 disabled) {
  g_disabled[(int)node_id] = disabled != 0.0 ? 1 : 0;
}

void __porf_import_dong_get_attribute(f64 node_id, f64 name_ptr) {
  char name[64];
  size_t len = 0;
  if (!read_utf8_bytestring(name_ptr, name, sizeof(name), &len)) {
    set_result("", 0);
    return;
  }
  set_result(g_attrs[(int)node_id][0], strlen(g_attrs[(int)node_id][0]));
  (void)name;
}

void __porf_import_dong_set_attribute(f64 node_id, f64 name_ptr, f64 value_ptr) {
  char val[256];
  size_t len = 0;
  if (!read_utf8_bytestring(value_ptr, val, sizeof(val), &len)) {
    return;
  }
  strncpy(g_attrs[(int)node_id][0], val, sizeof(g_attrs[0][0]) - 1);
  (void)name_ptr;
}

void __porf_import_dong_remove_attribute(f64 node_id, f64 name_ptr) {
  g_attrs[(int)node_id][0][0] = '\0';
  (void)name_ptr;
}

void __porf_import_dong_set_inner_html(f64 node_id, f64 html_ptr) {
  (void)node_id;
  (void)html_ptr;
}

f64 __porf_import_dong_query_selector(f64 root_id, f64 selector_ptr) {
  char sel[128];
  size_t len = 0;
  if (!read_utf8_bytestring(selector_ptr, sel, sizeof(sel), &len)) {
    return 0;
  }
  if (strcmp(sel, "#box") == 0) {
    return 42;
  }
  (void)root_id;
  return 0;
}

void __porf_import_dong_query_selector_all(f64 root_id, f64 selector_ptr) {
  char sel[128];
  size_t len = 0;
  if (!read_utf8_bytestring(selector_ptr, sel, sizeof(sel), &len)) {
    set_result("[]", 2);
    return;
  }
  if (strcmp(sel, ".item") == 0) {
    set_result("[7,8]", 5);
    return;
  }
  set_result("[]", 2);
  (void)root_id;
}

void __porf_import_dong_get_elements_by_tag_name(f64 root_id, f64 tag_ptr) {
  char tag[64];
  size_t len = 0;
  if (!read_utf8_bytestring(tag_ptr, tag, sizeof(tag), &len)) {
    set_result("[]", 2);
    return;
  }
  if (strcmp(tag, "div") == 0) {
    set_result("[7,8,42]", 8);
    return;
  }
  set_result("[]", 2);
  (void)root_id;
}

void __porf_import_dong_class_add(f64 node_id, f64 cls_ptr) {
  char cls[64];
  size_t len = 0;
  if (!read_utf8_bytestring(cls_ptr, cls, sizeof(cls), &len)) {
    return;
  }
  if ((int)node_id == 20) {
    if (strcmp(cls, "a") == 0) {
      strcpy(g_classes[20], "a");
    } else if (strcmp(cls, "b") == 0) {
      strcpy(g_classes[20], "a b");
    }
    return;
  }
  if (g_classes[(int)node_id][0]) {
    strncat(g_classes[(int)node_id], " ", sizeof(g_classes[0]) - strlen(g_classes[(int)node_id]) - 1);
  }
  strncat(g_classes[(int)node_id], cls, sizeof(g_classes[0]) - strlen(g_classes[(int)node_id]) - 1);
}

void __porf_import_dong_class_remove(f64 node_id, f64 cls_ptr) {
  char cls[64];
  size_t len = 0;
  if (!read_utf8_bytestring(cls_ptr, cls, sizeof(cls), &len)) {
    return;
  }
  if ((int)node_id == 20 && strcmp(cls, "a") == 0) {
    strcpy(g_classes[20], "b");
    return;
  }
}

f64 __porf_import_dong_class_toggle(f64 node_id, f64 cls_ptr) {
  char cls[64];
  size_t len = 0;
  if (!read_utf8_bytestring(cls_ptr, cls, sizeof(cls), &len)) {
    return 0;
  }
  if ((int)node_id == 20 && strcmp(cls, "c") == 0) {
    strcpy(g_classes[20], "b c");
    return 1;
  }
  if (strstr(g_classes[(int)node_id], cls)) {
    return 0;
  }
  __porf_import_dong_class_add(node_id, cls_ptr);
  return 1;
}

f64 __porf_import_dong_class_contains(f64 node_id, f64 cls_ptr) {
  char cls[64];
  size_t len = 0;
  if (!read_utf8_bytestring(cls_ptr, cls, sizeof(cls), &len)) {
    return 0;
  }
  if ((int)node_id == 20 && strcmp(cls, "a") == 0) {
    return strcmp(g_classes[20], "a b") == 0 ? 1.0 : 0.0;
  }
  return strstr(g_classes[(int)node_id], cls) ? 1.0 : 0.0;
}

void __porf_import_dong_style_set(f64 node_id, f64 prop_ptr, f64 value_ptr) {
  char val[64];
  size_t len = 0;
  if (!read_utf8_bytestring(value_ptr, val, sizeof(val), &len)) {
    return;
  }
  strncpy(g_style_color[(int)node_id], val, sizeof(g_style_color[0]) - 1);
  (void)prop_ptr;
}

void __porf_import_dong_style_get(f64 node_id, f64 prop_ptr) {
  set_result(g_style_color[(int)node_id], strlen(g_style_color[(int)node_id]));
  (void)prop_ptr;
}

void __porf_import_dong_computed_style_get(f64 node_id, f64 prop_ptr) {
  set_result("block", 5);
  (void)node_id;
  (void)prop_ptr;
}

void __porf_import_dong_get_rect(f64 node_id) {
  set_result("{\"x\":1,\"y\":2,\"w\":100,\"h\":50}", 31);
  (void)node_id;
}

f64 __porf_import_dong_get_metric(f64 node_id, f64 metric_id) {
  if ((int)metric_id == 0) {
    return 100;
  }
  (void)node_id;
  return 0;
}

f64 __porf_import_dong_get_scroll_top(f64 node_id) {
  return g_scroll_top[(int)node_id];
}

void __porf_import_dong_set_scroll_top(f64 node_id, f64 value) {
  g_scroll_top[(int)node_id] = value;
}

f64 __porf_import_dong_get_scroll_left(f64 node_id) {
  return 0;
  (void)node_id;
}

void __porf_import_dong_set_scroll_left(f64 node_id, f64 value) {
  (void)node_id;
  (void)value;
}

void __porf_import_dong_focus(f64 node_id) {
  (void)node_id;
}

void __porf_import_dong_blur(f64 node_id) {
  (void)node_id;
}

void __porf_import_dong_click(f64 node_id) {
  (void)node_id;
}

f64 __porf_import_dong_matches(f64 node_id, f64 selector_ptr) {
  char sel[64];
  size_t len = 0;
  if (!read_utf8_bytestring(selector_ptr, sel, sizeof(sel), &len)) {
    return 0;
  }
  return strcmp(sel, ".btn") == 0 ? 1.0 : 0.0;
  (void)node_id;
}

f64 __porf_import_dong_closest(f64 node_id, f64 selector_ptr) {
  char sel[64];
  size_t len = 0;
  if (!read_utf8_bytestring(selector_ptr, sel, sizeof(sel), &len)) {
    return 0;
  }
  if (strcmp(sel, ".wrap") == 0) {
    return 50;
  }
  (void)node_id;
  return 0;
}

void __porf_import_dong_dom_get_textContent(f64 node_id) {
  (void)node_id;
  set_result("", 0);
}

f64 __porf_import_dong_dom_getElementById(f64 id_ptr) {
  (void)id_ptr;
  return 1;
}

void __porf_import_dong_dom_set_textContent(f64 node_id, f64 text_ptr) {
  (void)node_id;
  (void)text_ptr;
}

void __porf_import_dong_stage_0(f64 v) {
  (void)v;
}
void __porf_import_dong_stage_1(f64 v) {
  (void)v;
}
void __porf_import_dong_stage_2(f64 v) {
  (void)v;
}
void __porf_import_dong_commit_set_textContent(void) {}
void __porf_import_dong_commit_addEventListener(void) {}
f64 __porf_import_dong_commit_setTimeout(void) {
  return 0;
}
f64 __porf_import_dong_time_now(void) {
  return 0;
}
void __porf_import_dong_bench_log(f64 p) {
  (void)p;
}
f64 __porf_import_dong_timer_setTimeout(f64 a, f64 b) {
  (void)a;
  (void)b;
  return 0;
}
void __porf_import_dong_dom_addEventListener(f64 a, f64 b, f64 c) {
  (void)a;
  (void)b;
  (void)c;
}
void __porf_import_dong_state_set_num(f64 a, f64 b) {
  (void)a;
  (void)b;
}
f64 __porf_import_dong_state_get_num(f64 a) {
  (void)a;
  return 0;
}

#include "t06_dom.c"

int main(void) {
  g_memory = (char*)calloc(1, g_memory_pages * 65536u);
  if (!g_memory) {
    return 1;
  }
  g_scroll_top[40] = 12;
  t06_main();
  return 0;
}
