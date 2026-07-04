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

#include "t17_counter.c"

typedef struct {
  char* memory;
  u32 memory_pages;
  t17_state_t state;
  int initialized;
} ViewInstance;

static void init_view(ViewInstance* view) {
  if (view->initialized) {
    return;
  }
  view->memory_pages = t17_memory_pages;
  view->memory = calloc(1, (size_t)view->memory_pages * 65536u);
  t17_memory = NULL;
  t17__porf_init();
  if (t17_memory) {
    memcpy(view->memory, t17_memory, (size_t)view->memory_pages * 65536u);
    free(t17_memory);
  }
  t17_memory = view->memory;
  t17_state_capture(&view->state);
  view->initialized = 1;
}

static void activate_view(ViewInstance* view) {
  init_view(view);
  t17_memory = view->memory;
  t17_state_apply(&view->state);
}

static void deactivate_view(ViewInstance* view) {
  t17_state_capture(&view->state);
}

static f64 view_count(const ViewInstance* view) {
  return view->state.t17_count;
}

int main(void) {
  ViewInstance view_a = {0};
  ViewInstance view_b = {0};

  activate_view(&view_a);
  t17_export_increment();
  t17_export_increment();
  deactivate_view(&view_a);

  activate_view(&view_b);
  t17_export_increment();
  deactivate_view(&view_b);

  if (view_count(&view_a) != 2) {
    fprintf(stderr, "view_a count expected 2, got %.0f\n", view_count(&view_a));
    return 1;
  }

  if (view_count(&view_b) != 1) {
    fprintf(stderr, "view_b count expected 1, got %.0f\n", view_count(&view_b));
    return 1;
  }

  return 0;
}
