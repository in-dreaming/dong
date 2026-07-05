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

static char g_clipboard[512];
static int g_viewport_w = 800;

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

void __porf_import_dong_print(f64 text_ptr);
f64 __porf_import_dong_str_len(void);
f64 __porf_import_dong_str_byte_at(f64 index);
void __porf_import_dong_clipboard_read(void);
void __porf_import_dong_clipboard_write(f64 text_ptr);
f64 __porf_import_dong_match_media(f64 query_ptr);
f64 __porf_import_dong_css_supports(f64 prop_ptr, f64 value_ptr);
f64 __porf_import_dong_scene_add_node(f64 config_ptr);
void __porf_import_dong_scene_remove(f64 id);
void __porf_import_dong_scene_set(f64 id, f64 prop_ptr, f64 value_ptr);
f64 __porf_import_dong_scene_find(f64 name_ptr);
void __porf_import_dong_scene_on(f64 id, f64 type_ptr, f64 handler_ptr);
void __porf_import_dong_scene_clear(void);
f64 __porf_import_dong_scene_count(void);
void __porf_import_dong_text_layout(f64 config_ptr);
void __porf_import_dong_clear_overlay(void);
void __porf_import_dong_render_text(f64 config_ptr);
void __porf_import_dong_draw_rect(f64 config_ptr);
void __porf_import_dong_draw_circle(f64 config_ptr);

int t11_main(void);
int t11_export_probeClipboardCn(void);
int t11_export_probeMatchMedia(void);
int t11_export_probeCssSupports(void);
int t11_export_probeScene(void);
int t11_export_probeTextLayout(void);

static u32 g_scene_next = 1;
static u32 g_scene_count = 0;
static char g_scene_name[64];

void __porf_import_dong_print(f64 text_ptr) {
  char buf[4096];
  size_t len = 0;
  if (read_utf8_bytestring(text_ptr, buf, sizeof(buf), &len)) {
    fputs(buf, stdout);
    fputc('\n', stdout);
  }
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

void __porf_import_dong_clipboard_read(void) {
  set_result(g_clipboard, strlen(g_clipboard));
}

void __porf_import_dong_clipboard_write(f64 text_ptr) {
  char buf[512];
  size_t len = 0;
  if (read_utf8_bytestring(text_ptr, buf, sizeof(buf), &len)) {
    strncpy(g_clipboard, buf, sizeof(g_clipboard) - 1);
    g_clipboard[sizeof(g_clipboard) - 1] = '\0';
  }
}

f64 __porf_import_dong_match_media(f64 query_ptr) {
  char buf[256];
  size_t len = 0;
  if (!read_utf8_bytestring(query_ptr, buf, sizeof(buf), &len)) {
    return 0;
  }
  if (strstr(buf, "min-width:") != NULL) {
    const char* p = strstr(buf, "min-width:") + 10;
    int min_w = atoi(p);
    return g_viewport_w >= min_w ? 1.0 : 0.0;
  }
  return 0;
}

f64 __porf_import_dong_css_supports(f64 prop_ptr, f64 value_ptr) {
  char prop[128];
  char val[128];
  size_t len = 0;
  if (!read_utf8_bytestring(prop_ptr, prop, sizeof(prop), &len)) {
    return 0;
  }
  if (!read_utf8_bytestring(value_ptr, val, sizeof(val), &len)) {
    return 0;
  }
  if (strcmp(prop, "display") != 0) {
    return 0;
  }
  return strstr(val, "grid") != NULL ? 0.0 : 1.0;
}

f64 __porf_import_dong_scene_add_node(f64 config_ptr) {
  char buf[1024];
  size_t len = 0;
  if (!read_utf8_bytestring(config_ptr, buf, sizeof(buf), &len)) {
    return 0;
  }
  const char* name_key = strstr(buf, "\"name\"");
  if (name_key) {
    const char* q = strchr(name_key + 6, '"');
    if (q) {
      q = strchr(q + 1, '"');
      if (q) {
        const char* start = q + 1;
        const char* end = strchr(start, '"');
        if (end && (size_t)(end - start) < sizeof(g_scene_name)) {
          memcpy(g_scene_name, start, (size_t)(end - start));
          g_scene_name[end - start] = '\0';
        }
      }
    }
  }
  g_scene_count++;
  return (f64)g_scene_next++;
}

void __porf_import_dong_scene_remove(f64 id) {
  (void)id;
  if (g_scene_count > 0) {
    g_scene_count--;
  }
}

void __porf_import_dong_scene_set(f64 id, f64 prop_ptr, f64 value_ptr) {
  (void)id;
  (void)prop_ptr;
  (void)value_ptr;
}

f64 __porf_import_dong_scene_find(f64 name_ptr) {
  char buf[128];
  size_t len = 0;
  if (!read_utf8_bytestring(name_ptr, buf, sizeof(buf), &len)) {
    return -1;
  }
  return strcmp(buf, g_scene_name) == 0 ? 1.0 : -1.0;
}

void __porf_import_dong_scene_on(f64 id, f64 type_ptr, f64 handler_ptr) {
  (void)id;
  (void)type_ptr;
  (void)handler_ptr;
}

void __porf_import_dong_scene_clear(void) {
  g_scene_count = 0;
  g_scene_name[0] = '\0';
}

f64 __porf_import_dong_scene_count(void) {
  return (f64)g_scene_count;
}

void __porf_import_dong_text_layout(f64 config_ptr) {
  char buf[2048];
  size_t len = 0;
  if (!read_utf8_bytestring(config_ptr, buf, sizeof(buf), &len)) {
    set_result("{}", 2);
    return;
  }
  set_result("{\"columns\":[{\"lines\":[{\"text\":\"Hello Dong\"}]}]}", 48);
}

void __porf_import_dong_clear_overlay(void) {}
void __porf_import_dong_render_text(f64 config_ptr) { (void)config_ptr; }
void __porf_import_dong_draw_rect(f64 config_ptr) { (void)config_ptr; }
void __porf_import_dong_draw_circle(f64 config_ptr) { (void)config_ptr; }

#include "t11_platform.c"

int main(void) {
  g_memory = (char*)calloc(1, 65536);
  if (!g_memory) {
    return 1;
  }

  t11_main();
  t11_export_probeClipboardCn();
  t11_export_probeMatchMedia();
  g_viewport_w = 1200;
  t11_export_probeMatchMedia();
  t11_export_probeCssSupports();
  t11_export_probeScene();
  t11_export_probeTextLayout();

  printf("T11_PASS\n");
  return 0;
}
