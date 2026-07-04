#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t u8;
typedef int32_t i32;
typedef uint32_t u32;
typedef double f64;

struct ReturnValue {
  f64 value;
  i32 type;
};

static char* g_memory = NULL;
static u32 g_memory_pages = 1;
static char g_result_slot[65536];
static size_t g_result_len = 0;
static u8 g_guard_byte = 0xA5;

static size_t memory_cap(void) {
  return (size_t)g_memory_pages * 65536u;
}

static void set_result_utf8(const char* bytes, size_t len) {
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

f64 __porf_import_dong_host_set_slot(f64 text_ptr);
f64 __porf_import_dong_str_len(void);
f64 __porf_import_dong_str_read(f64 dest_ptr, f64 max_len);
f64 __porf_import_dong_str_byte_at(f64 index);
void __porf_import_dong_print(f64 text_ptr);
void __porf_import_dong_dom_get_textContent(f64 node_id);

f64 __porf_import_dong_host_set_slot(f64 text_ptr) {
  char buf[4096];
  size_t len = 0;
  if (!read_utf8_bytestring(text_ptr, buf, sizeof(buf), &len)) {
    g_result_len = 0;
    return 0;
  }
  set_result_utf8(buf, len);
  return 1;
}

f64 __porf_import_dong_str_len(void) {
  return (f64)g_result_len;
}

f64 __porf_import_dong_str_read(f64 dest_ptr, f64 max_len) {
  if (!g_memory || dest_ptr < 0 || max_len <= 0 || g_result_len == 0) {
    return 0;
  }
  size_t offset = (size_t)dest_ptr;
  size_t cap = memory_cap();
  if (offset >= cap) {
    return 0;
  }
  size_t avail = cap - offset;
  size_t want = g_result_len;
  if (want > (size_t)max_len) {
    want = (size_t)max_len;
  }
  if (want > avail) {
    want = avail;
  }
  memcpy(g_memory + offset, g_result_slot, want);
  return (f64)want;
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

void __porf_import_dong_dom_get_textContent(f64 node_id) {
  (void)node_id;
  set_result_utf8("dom-中文", strlen("dom-中文"));
}

#include "t16_channel.c"

int main(void) {
  g_memory = (char*)calloc(1, g_memory_pages * 65536u);
  if (!g_memory) {
    return 1;
  }
  g_memory[65536 - 1] = (char)g_guard_byte;

  t16_main();

  if (g_memory[65536 - 1] != (char)g_guard_byte) {
    fprintf(stderr, "guard byte corrupted\n");
    return 2;
  }
  return 0;
}
