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

static f64 g_stage_0 = 0;
static f64 g_stage_1 = 0;

typedef struct {
  int active;
  i32 request_id;
  i32 status;
  int ok;
  char body[4096];
  char error[512];
} FetchSlot;

static FetchSlot g_fetch_slot;
static FetchSlot g_fetch_stack[8];
static int g_fetch_stack_depth = 0;

typedef struct {
  u32 id;
  char export_name[64];
  char url[256];
} PendingFetch;

typedef struct {
  u32 id;
  char export_name[64];
  i32 status;
  int ok;
  char body[4096];
  char error[512];
} FetchCompletion;

static PendingFetch g_pending[16];
static int g_pending_count = 0;
static FetchCompletion g_completions[16];
static int g_completion_count = 0;
static u32 g_next_fetch_id = 1;

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

static void push_fetch_slot(void) {
  if (g_fetch_stack_depth >= 8) {
    return;
  }
  g_fetch_stack[g_fetch_stack_depth++] = g_fetch_slot;
  memset(&g_fetch_slot, 0, sizeof(g_fetch_slot));
}

static void pop_fetch_slot(void) {
  if (g_fetch_stack_depth <= 0) {
    memset(&g_fetch_slot, 0, sizeof(g_fetch_slot));
    return;
  }
  g_fetch_slot = g_fetch_stack[--g_fetch_stack_depth];
}

static int pending_index(u32 id) {
  for (int i = 0; i < g_pending_count; ++i) {
    if (g_pending[i].id == id) {
      return i;
    }
  }
  return -1;
}

static void remove_pending_at(int idx) {
  for (int j = idx; j < g_pending_count - 1; ++j) {
    g_pending[j] = g_pending[j + 1];
  }
  g_pending_count--;
}

static void fill_completion(FetchCompletion* c, const char* url) {
  c->ok = 0;
  c->status = 0;
  c->body[0] = '\0';
  c->error[0] = '\0';

  if (strcmp(url, "good.json") == 0) {
    c->ok = 1;
    c->status = 200;
    strncpy(c->body, "{\"title\":\"hello\"}", sizeof(c->body) - 1);
  } else if (strcmp(url, "missing.json") == 0) {
    c->status = 404;
    strncpy(c->error, "HTTP 404 for: missing.json", sizeof(c->error) - 1);
  } else if (strncmp(url, "bad://", 6) == 0) {
    strncpy(c->error, "Invalid URL", sizeof(c->error) - 1);
  } else if (strcmp(url, "a.json") == 0) {
    c->ok = 1;
    c->status = 200;
    strncpy(c->body, "A", sizeof(c->body) - 1);
  } else if (strcmp(url, "b.json") == 0) {
    c->ok = 1;
    c->status = 200;
    strncpy(c->body, "B", sizeof(c->body) - 1);
  } else if (strcmp(url, "slow.json") == 0) {
    c->ok = 1;
    c->status = 200;
    strncpy(c->body, "slow", sizeof(c->body) - 1);
  }
}

void __porf_import_dong_print(f64 text_ptr);
f64 __porf_import_dong_str_len(void);
f64 __porf_import_dong_str_byte_at(f64 index);
f64 __porf_import_dong_fetch_start(f64 url_ptr, f64 export_ptr);
void __porf_import_dong_fetch_abort(f64 request_id);
f64 __porf_import_dong_fetch_request_id(void);
f64 __porf_import_dong_fetch_status(void);
f64 __porf_import_dong_fetch_ok(void);
void __porf_import_dong_fetch_body(void);
void __porf_import_dong_fetch_error(void);
void __porf_import_dong_state_set_num(f64 slot, f64 value);
f64 __porf_import_dong_state_get_num(f64 slot);

int t10_main(void);
int t10_export_runFetchTests(void);
int t10_export_onLocalData(void);
int t10_export_onError(void);
int t10_export_onConcurrent(void);
int t10_export_onAborted(void);
int t10_export_probeOutsideSlot(void);

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

f64 __porf_import_dong_fetch_start(f64 url_ptr, f64 export_ptr) {
  char url[256];
  char export_name[64];
  size_t url_len = 0;
  size_t export_len = 0;
  if (!read_utf8_bytestring(url_ptr, url, sizeof(url), &url_len)) {
    return 0;
  }
  if (!read_utf8_bytestring(export_ptr, export_name, sizeof(export_name), &export_len)) {
    return 0;
  }

  u32 id = g_next_fetch_id++;
  if (g_pending_count < 16) {
    PendingFetch* p = &g_pending[g_pending_count++];
    p->id = id;
    strncpy(p->url, url, sizeof(p->url) - 1);
    strncpy(p->export_name, export_name, sizeof(p->export_name) - 1);
  }

  if (g_completion_count < 16) {
    FetchCompletion* c = &g_completions[g_completion_count++];
    c->id = id;
    strncpy(c->export_name, export_name, sizeof(c->export_name) - 1);
    fill_completion(c, url);
  }

  return (f64)id;
}

void __porf_import_dong_fetch_abort(f64 request_id) {
  int idx = pending_index((u32)request_id);
  if (idx >= 0) {
    remove_pending_at(idx);
  }
}

f64 __porf_import_dong_fetch_request_id(void) {
  return g_fetch_slot.active ? (f64)g_fetch_slot.request_id : 0;
}

f64 __porf_import_dong_fetch_status(void) {
  return g_fetch_slot.active ? (f64)g_fetch_slot.status : 0;
}

f64 __porf_import_dong_fetch_ok(void) {
  return g_fetch_slot.active && g_fetch_slot.ok ? 1 : 0;
}

void __porf_import_dong_fetch_body(void) {
  if (!g_fetch_slot.active) {
    g_result_len = 0;
    return;
  }
  set_result(g_fetch_slot.body, strlen(g_fetch_slot.body));
}

void __porf_import_dong_fetch_error(void) {
  if (!g_fetch_slot.active) {
    g_result_len = 0;
    return;
  }
  set_result(g_fetch_slot.error, strlen(g_fetch_slot.error));
}

void __porf_import_dong_state_set_num(f64 slot, f64 value) {
  (void)slot;
  (void)value;
}

f64 __porf_import_dong_state_get_num(f64 slot) {
  (void)slot;
  return 0;
}

static void dispatch_completion(const FetchCompletion* c) {
  push_fetch_slot();
  g_fetch_slot.active = 1;
  g_fetch_slot.request_id = (i32)c->id;
  g_fetch_slot.status = c->status;
  g_fetch_slot.ok = c->ok;
  strncpy(g_fetch_slot.body, c->body, sizeof(g_fetch_slot.body) - 1);
  strncpy(g_fetch_slot.error, c->error, sizeof(g_fetch_slot.error) - 1);

  if (strcmp(c->export_name, "onLocalData") == 0) {
    t10_export_onLocalData();
  } else if (strcmp(c->export_name, "onError") == 0) {
    t10_export_onError();
  } else if (strcmp(c->export_name, "onConcurrent") == 0) {
    t10_export_onConcurrent();
  } else if (strcmp(c->export_name, "onAborted") == 0) {
    t10_export_onAborted();
  }

  g_fetch_slot.active = 0;
  pop_fetch_slot();
}

static void process_fetch_completions(void) {
  for (int i = 0; i < g_completion_count; ++i) {
    FetchCompletion* c = &g_completions[i];
    int idx = pending_index(c->id);
    if (idx < 0) {
      continue;
    }
    remove_pending_at(idx);
    dispatch_completion(c);
  }
  g_completion_count = 0;
}

#include "t10_fetch.c"

int main(void) {
  g_memory = (char*)calloc(1, 65536);
  if (!g_memory) {
    return 1;
  }

  t10_main();
  t10_export_runFetchTests();
  process_fetch_completions();
  t10_export_probeOutsideSlot();

  printf("T10_PASS\n");
  return 0;
}
