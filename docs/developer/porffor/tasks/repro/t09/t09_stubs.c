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

#define MAX_TIMERS 32
#define MAX_RAF 64
#define MAX_STR 256

typedef struct {
  int active;
  uint64_t id;
  double fire_at_ms;
  double interval_ms;
  char export_name[64];
} TimerEntry;

typedef struct {
  int active;
  uint64_t id;
  char export_name[64];
} RafEntry;

static char* g_memory = NULL;
static u32 g_memory_pages = 16;
static double g_now_ms = 0.0;
static double g_raf_timestamp = 0.0;
static uint64_t g_next_timer_id = 1;
static uint64_t g_next_raf_id = 1;
static TimerEntry g_timers[MAX_TIMERS];
static RafEntry g_raf_queue[MAX_RAF];
static int g_raf_count = 0;
static f64 g_state_nums[32];
static f64 g_stage_0 = 0.0;
static f64 g_stage_1 = 0.0;

extern int t09_main(void);
extern int t09_export_onIntervalTick(void);
extern int t09_export_onRafFrame(void);
extern int t09_export_onRafWithArg(f64 p0);
extern int t09_export_onCancelProbe(void);

static int read_byte_string(f64 ptr, char* out, size_t out_cap) {
  if (!g_memory || ptr < 0) {
    return 0;
  }
  size_t offset = (size_t)ptr;
  size_t cap = (size_t)g_memory_pages * 65536u;
  if (offset + 4 > cap) {
    return 0;
  }
  u32 len = *(u32*)(g_memory + offset);
  if (len >= out_cap || offset + 4 + len > cap) {
    return 0;
  }
  memcpy(out, g_memory + offset + 4, len);
  out[len] = '\0';
  return 1;
}

static void call_export0(const char* name) {
  if (strcmp(name, "onIntervalTick") == 0) {
    t09_export_onIntervalTick();
  } else if (strcmp(name, "onRafFrame") == 0) {
    t09_export_onRafFrame();
  } else if (strcmp(name, "onCancelProbe") == 0) {
    t09_export_onCancelProbe();
  }
}

static void call_export1(const char* name, f64 arg) {
  if (strcmp(name, "onRafWithArg") == 0) {
    t09_export_onRafWithArg(arg);
  }
}

static uint64_t schedule_timer(f64 handler_ptr, f64 delay_ms, f64 interval_ms) {
  char name[64];
  if (!read_byte_string(handler_ptr, name, sizeof(name))) {
    return 0;
  }
  for (int i = 0; i < MAX_TIMERS; ++i) {
    if (!g_timers[i].active) {
      g_timers[i].active = 1;
      g_timers[i].id = g_next_timer_id++;
      g_timers[i].fire_at_ms = g_now_ms + (delay_ms < 0 ? 0 : delay_ms);
      g_timers[i].interval_ms = interval_ms;
      strncpy(g_timers[i].export_name, name, sizeof(g_timers[i].export_name) - 1);
      return g_timers[i].id;
    }
  }
  return 0;
}

static void process_timers(double now_ms) {
  for (int pass = 0; pass < MAX_TIMERS; ++pass) {
  uint64_t due_id = 0;
  int due_idx = -1;
  for (int i = 0; i < MAX_TIMERS; ++i) {
    if (g_timers[i].active && g_timers[i].fire_at_ms <= now_ms) {
      due_id = g_timers[i].id;
      due_idx = i;
      break;
    }
  }
  if (due_idx < 0) {
    break;
  }
  TimerEntry t = g_timers[due_idx];
  if (t.interval_ms >= 0.0) {
    g_timers[due_idx].fire_at_ms = now_ms + t.interval_ms;
    call_export0(t.export_name);
  } else {
    g_timers[due_idx].active = 0;
    call_export0(t.export_name);
  }
  (void)due_id;
  }
}

static void process_raf(double ts) {
  if (g_raf_count == 0) {
    return;
  }
  g_raf_timestamp = ts;
  RafEntry batch[MAX_RAF];
  int n = g_raf_count;
  memcpy(batch, g_raf_queue, (size_t)n * sizeof(RafEntry));
  g_raf_count = 0;
  for (int i = 0; i < n; ++i) {
    if (strcmp(batch[i].export_name, "onRafWithArg") == 0) {
      call_export1(batch[i].export_name, ts);
    } else {
      call_export0(batch[i].export_name);
    }
  }
}

f64 __porf_import_dong_time_now(void) {
  return g_now_ms;
}

f64 __porf_import_dong_timer_setTimeout(f64 handler_ptr, f64 delay_ms) {
  return (f64)schedule_timer(handler_ptr, delay_ms, -1.0);
}

f64 __porf_import_dong_set_interval(f64 handler_ptr, f64 interval_ms) {
  return (f64)schedule_timer(handler_ptr, interval_ms, interval_ms);
}

void __porf_import_dong_clear_interval(f64 timer_id) {
  for (int i = 0; i < MAX_TIMERS; ++i) {
    if (g_timers[i].active && g_timers[i].id == (uint64_t)timer_id) {
      g_timers[i].active = 0;
    }
  }
}

void __porf_import_dong_clear_timeout(f64 timer_id) {
  __porf_import_dong_clear_interval(timer_id);
}

f64 __porf_import_dong_request_animation_frame(f64 handler_ptr) {
  char name[64];
  if (!read_byte_string(handler_ptr, name, sizeof(name))) {
    return 0;
  }
  if (g_raf_count >= MAX_RAF) {
    return 0;
  }
  RafEntry* e = &g_raf_queue[g_raf_count++];
  e->active = 1;
  e->id = g_next_raf_id++;
  strncpy(e->export_name, name, sizeof(e->export_name) - 1);
  return (f64)e->id;
}

void __porf_import_dong_cancel_animation_frame(f64 raf_id) {
  for (int i = 0; i < g_raf_count; ++i) {
    if (g_raf_queue[i].id == (uint64_t)raf_id) {
      for (int j = i; j < g_raf_count - 1; ++j) {
        g_raf_queue[j] = g_raf_queue[j + 1];
      }
      g_raf_count--;
      return;
    }
  }
}

f64 __porf_import_dong_raf_timestamp(void) {
  return g_raf_timestamp;
}

void __porf_import_dong_stage_0(f64 v) { g_stage_0 = v; }
void __porf_import_dong_stage_1(f64 v) { g_stage_1 = v; }
void __porf_import_dong_stage_2(f64 v) { (void)v; }
void __porf_import_dong_commit_set_textContent(void) {}
void __porf_import_dong_commit_addEventListener(void) {}
f64 __porf_import_dong_commit_setTimeout(void) {
  return (f64)schedule_timer(g_stage_1, g_stage_0, -1.0);
}
f64 __porf_import_dong_commit_setInterval(void) {
  return (f64)schedule_timer(g_stage_1, g_stage_0, g_stage_0);
}
f64 __porf_import_dong_commit_requestAnimationFrame(void) {
  return __porf_import_dong_request_animation_frame(g_stage_0);
}
void __porf_import_dong_print(f64 p) { (void)p; }
void __porf_import_dong_bench_log(f64 p) { (void)p; }
f64 __porf_import_dong_dom_getElementById(f64 p) { (void)p; return 0; }
void __porf_import_dong_dom_set_textContent(f64 a, f64 b) { (void)a; (void)b; }
void __porf_import_dong_dom_get_textContent(f64 a) { (void)a; }
f64 __porf_import_dong_str_len(void) { return 0; }
f64 __porf_import_dong_str_read(f64 a, f64 b) { (void)a; (void)b; return 0; }
f64 __porf_import_dong_str_byte_at(f64 a) { (void)a; return -1; }
void __porf_import_dong_dom_addEventListener(f64 a, f64 b, f64 c) { (void)a; (void)b; (void)c; }

void __porf_import_dong_state_set_num(f64 slot, f64 value) {
  size_t idx = (size_t)slot;
  if (idx < 32) {
    g_state_nums[idx] = value;
  }
}

f64 __porf_import_dong_state_get_num(f64 slot) {
  size_t idx = (size_t)slot;
  if (idx < 32) {
    return g_state_nums[idx];
  }
  return 0;
}

void __porf_import_dong_get_value(f64 a) { (void)a; }
void __porf_import_dong_set_value(f64 a, f64 b) { (void)a; (void)b; }
f64 __porf_import_dong_get_checked(f64 a) { (void)a; return 0; }
void __porf_import_dong_set_checked(f64 a, f64 b) { (void)a; (void)b; }
f64 __porf_import_dong_get_disabled(f64 a) { (void)a; return 0; }
void __porf_import_dong_set_disabled(f64 a, f64 b) { (void)a; (void)b; }
void __porf_import_dong_get_attribute(f64 a, f64 b) { (void)a; (void)b; }
void __porf_import_dong_set_attribute(f64 a, f64 b, f64 c) { (void)a; (void)b; (void)c; }
void __porf_import_dong_remove_attribute(f64 a, f64 b) { (void)a; (void)b; }
void __porf_import_dong_set_inner_html(f64 a, f64 b) { (void)a; (void)b; }
f64 __porf_import_dong_query_selector(f64 a, f64 b) { (void)a; (void)b; return 0; }
void __porf_import_dong_query_selector_all(f64 a, f64 b) { (void)a; (void)b; }
void __porf_import_dong_get_elements_by_tag_name(f64 a, f64 b) { (void)a; (void)b; }
void __porf_import_dong_class_add(f64 a, f64 b) { (void)a; (void)b; }
void __porf_import_dong_class_remove(f64 a, f64 b) { (void)a; (void)b; }
f64 __porf_import_dong_class_toggle(f64 a, f64 b) { (void)a; (void)b; return 0; }
f64 __porf_import_dong_class_contains(f64 a, f64 b) { (void)a; (void)b; return 0; }
void __porf_import_dong_style_set(f64 a, f64 b, f64 c) { (void)a; (void)b; (void)c; }
void __porf_import_dong_style_get(f64 a, f64 b) { (void)a; (void)b; }
void __porf_import_dong_computed_style_get(f64 a, f64 b) { (void)a; (void)b; }
void __porf_import_dong_get_rect(f64 a) { (void)a; }
f64 __porf_import_dong_get_metric(f64 a, f64 b) { (void)a; (void)b; return 0; }
f64 __porf_import_dong_get_scroll_top(f64 a) { (void)a; return 0; }
void __porf_import_dong_set_scroll_top(f64 a, f64 b) { (void)a; (void)b; }
f64 __porf_import_dong_get_scroll_left(f64 a) { (void)a; return 0; }
void __porf_import_dong_set_scroll_left(f64 a, f64 b) { (void)a; (void)b; }
void __porf_import_dong_focus(f64 a) { (void)a; }
void __porf_import_dong_blur(f64 a) { (void)a; }
void __porf_import_dong_click(f64 a) { (void)a; }
f64 __porf_import_dong_matches(f64 a, f64 b) { (void)a; (void)b; return 0; }
f64 __porf_import_dong_closest(f64 a, f64 b) { (void)a; (void)b; return 0; }
f64 __porf_import_dong_create_element(f64 a) { (void)a; return 0; }
f64 __porf_import_dong_create_text_node(f64 a) { (void)a; return 0; }
void __porf_import_dong_append_child(f64 a, f64 b) { (void)a; (void)b; }
void __porf_import_dong_insert_before(f64 a, f64 b, f64 c) { (void)a; (void)b; (void)c; }
void __porf_import_dong_remove(f64 a) { (void)a; }
void __porf_import_dong_replace_child(f64 a, f64 b, f64 c) { (void)a; (void)b; (void)c; }
f64 __porf_import_dong_parent(f64 a) { (void)a; return 0; }
f64 __porf_import_dong_first_child(f64 a) { (void)a; return 0; }
f64 __porf_import_dong_next_sibling(f64 a) { (void)a; return 0; }
f64 __porf_import_dong_clone_node(f64 a, f64 b) { (void)a; (void)b; return 0; }

#include "t09_timers.c"

int main(void) {
  g_memory = (char*)calloc(1, (size_t)g_memory_pages * 65536u);
  if (!g_memory) {
    return 1;
  }

  t09_main();

  if (g_state_nums[4] != 1.0) {
    fprintf(stderr, "expected monotonic interval timer id 1, got %.0f\n", g_state_nums[4]);
    return 1;
  }

  for (int frame = 0; frame < 80; ++frame) {
    g_now_ms = (double)frame * 16.0;
    process_timers(g_now_ms);
    process_raf(g_now_ms);
  }

  if (g_state_nums[0] != 5.0) {
    fprintf(stderr, "expected interval count 5, got %.0f\n", g_state_nums[0]);
    return 1;
  }
  if (g_state_nums[1] != 60.0) {
    fprintf(stderr, "expected raf count 60, got %.0f\n", g_state_nums[1]);
    return 1;
  }
  if (g_state_nums[2] <= 0.0) {
    fprintf(stderr, "expected positive raf timestamp\n");
    return 1;
  }
  if (g_state_nums[3] <= 0.0) {
    fprintf(stderr, "expected onRafWithArg timestamp via f64 param\n");
    return 1;
  }
  if (g_state_nums[10] != 0.0) {
    fprintf(stderr, "expected cancelAnimationFrame to prevent callback, got %.0f\n",
            g_state_nums[10]);
    return 1;
  }

  printf("T09_PASS\n");
  return 0;
}
