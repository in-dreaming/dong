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

typedef struct {
  char type[64];
  f64 target;
  char key[64];
  f64 key_code;
  f64 x;
  f64 y;
  f64 button;
  f64 modifiers;
  char value[256];
  int prevent_default;
  int stop_propagation;
} EventSlot;

static EventSlot g_event_slot;
static EventSlot g_event_stack[8];
static int g_event_stack_depth = 0;
static int g_prevent_flag = 0;
static int g_stop_flag = 0;

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

static void push_event_slot(void) {
  if (g_event_stack_depth >= 8) {
    return;
  }
  g_event_stack[g_event_stack_depth++] = g_event_slot;
  memset(&g_event_slot, 0, sizeof(g_event_slot));
}

static void pop_event_slot(void) {
  if (g_event_stack_depth <= 0) {
    memset(&g_event_slot, 0, sizeof(g_event_slot));
    return;
  }
  g_event_slot = g_event_stack[--g_event_stack_depth];
}

static void fill_sample_event(void) {
  strncpy(g_event_slot.type, "click", sizeof(g_event_slot.type) - 1);
  g_event_slot.target = 42;
  strncpy(g_event_slot.key, "Enter", sizeof(g_event_slot.key) - 1);
  g_event_slot.key_code = 13;
  g_event_slot.x = 100;
  g_event_slot.y = 200;
  g_event_slot.button = 0;
  g_event_slot.modifiers = 3;
  strncpy(g_event_slot.value, "hello", sizeof(g_event_slot.value) - 1);
}

void __porf_import_dong_print(f64 text_ptr);
f64 __porf_import_dong_str_len(void);
f64 __porf_import_dong_str_byte_at(f64 index);
void __porf_import_dong_event_type(void);
f64 __porf_import_dong_event_target(void);
void __porf_import_dong_event_key(void);
f64 __porf_import_dong_event_key_code(void);
f64 __porf_import_dong_event_x(void);
f64 __porf_import_dong_event_y(void);
f64 __porf_import_dong_event_button(void);
f64 __porf_import_dong_event_modifiers(void);
void __porf_import_dong_event_value(void);
void __porf_import_dong_event_prevent_default(void);
void __porf_import_dong_event_stop_propagation(void);
void __porf_import_dong_state_set_num(f64 slot, f64 value);
f64 __porf_import_dong_state_get_num(f64 slot);

int t08_main(void);
int t08_export_probeEventSlot(void);
int t08_export_markPrevent(void);
int t08_export_readTypeDuringNested(void);

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

void __porf_import_dong_event_type(void) {
  set_result(g_event_slot.type, strlen(g_event_slot.type));
}

f64 __porf_import_dong_event_target(void) {
  return g_event_slot.target;
}

void __porf_import_dong_event_key(void) {
  set_result(g_event_slot.key, strlen(g_event_slot.key));
}

f64 __porf_import_dong_event_key_code(void) {
  return g_event_slot.key_code;
}

f64 __porf_import_dong_event_x(void) {
  return g_event_slot.x;
}

f64 __porf_import_dong_event_y(void) {
  return g_event_slot.y;
}

f64 __porf_import_dong_event_button(void) {
  return g_event_slot.button;
}

f64 __porf_import_dong_event_modifiers(void) {
  return g_event_slot.modifiers;
}

void __porf_import_dong_event_value(void) {
  set_result(g_event_slot.value, strlen(g_event_slot.value));
}

void __porf_import_dong_event_prevent_default(void) {
  g_event_slot.prevent_default = 1;
  g_prevent_flag = 1;
}

void __porf_import_dong_event_stop_propagation(void) {
  g_event_slot.stop_propagation = 1;
  g_stop_flag = 1;
}

void __porf_import_dong_state_set_num(f64 slot, f64 value) {
  (void)slot;
  (void)value;
}

f64 __porf_import_dong_state_get_num(f64 slot) {
  (void)slot;
  return 0;
}

#include "t08_events.c"

int main(void) {
  g_memory = (char*)calloc(1, 65536);
  if (!g_memory) {
    return 1;
  }

  t08_main();

  fill_sample_event();
  t08_export_probeEventSlot();

  g_prevent_flag = 0;
  g_stop_flag = 0;
  t08_export_markPrevent();
  if (!g_prevent_flag || !g_stop_flag) {
    fprintf(stderr, "prevent/stop flags not set\n");
    return 1;
  }

  push_event_slot();
  strncpy(g_event_slot.type, "outer", sizeof(g_event_slot.type) - 1);
  push_event_slot();
  strncpy(g_event_slot.type, "inner", sizeof(g_event_slot.type) - 1);
  t08_export_readTypeDuringNested();
  pop_event_slot();
  t08_export_readTypeDuringNested();
  pop_event_slot();

  printf("T08_PASS\n");
  return 0;
}
