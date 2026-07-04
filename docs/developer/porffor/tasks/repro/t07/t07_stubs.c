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

#define MAX_NODES 2048

typedef struct Node {
  int used;
  int parent;
  int first_child;
  int next_sibling;
  int is_text;
  char tag[32];
  char text[256];
} Node;

static Node g_nodes[MAX_NODES];
static int g_next_id = 2;
static f64 g_state_nums[256];

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

static int live_count(void) {
  int n = 0;
  for (int i = 1; i < MAX_NODES; ++i) {
    if (g_nodes[i].used) {
      n++;
    }
  }
  return n;
}

static void update_live_state(void) {
  g_state_nums[0] = (f64)live_count();
}

static int alloc_node(int is_text, const char* tag, const char* text) {
  if (g_next_id >= MAX_NODES) {
    return 0;
  }
  int id = g_next_id++;
  Node* n = &g_nodes[id];
  memset(n, 0, sizeof(*n));
  n->used = 1;
  n->is_text = is_text;
  if (tag) {
    strncpy(n->tag, tag, sizeof(n->tag) - 1);
  }
  if (text) {
    strncpy(n->text, text, sizeof(n->text) - 1);
  }
  update_live_state();
  return id;
}

static int valid_id(int id) {
  return id > 0 && id < MAX_NODES && g_nodes[id].used;
}

static void unlink_node(int id) {
  Node* n = &g_nodes[id];
  int p = n->parent;
  if (p > 0 && valid_id(p)) {
    Node* parent = &g_nodes[p];
    if (parent->first_child == id) {
      parent->first_child = n->next_sibling;
    } else {
      int cur = parent->first_child;
      while (cur > 0) {
        if (g_nodes[cur].next_sibling == id) {
          g_nodes[cur].next_sibling = n->next_sibling;
          break;
        }
        cur = g_nodes[cur].next_sibling;
      }
    }
  }
  n->parent = 0;
  n->next_sibling = 0;
}

static void append_child(int parent_id, int child_id) {
  if (!valid_id(parent_id) || !valid_id(child_id)) {
    return;
  }
  unlink_node(child_id);
  Node* parent = &g_nodes[parent_id];
  Node* child = &g_nodes[child_id];
  child->parent = parent_id;
  if (parent->first_child == 0) {
    parent->first_child = child_id;
  } else {
    int cur = parent->first_child;
    while (g_nodes[cur].next_sibling != 0) {
      cur = g_nodes[cur].next_sibling;
    }
    g_nodes[cur].next_sibling = child_id;
  }
}

static void insert_before(int parent_id, int new_id, int ref_id) {
  if (!valid_id(parent_id) || !valid_id(new_id)) {
    return;
  }
  if (ref_id == 0) {
    append_child(parent_id, new_id);
    return;
  }
  if (!valid_id(ref_id) || g_nodes[ref_id].parent != parent_id) {
    return;
  }
  unlink_node(new_id);
  Node* parent = &g_nodes[parent_id];
  Node* child = &g_nodes[new_id];
  child->parent = parent_id;
  if (parent->first_child == ref_id) {
    child->next_sibling = ref_id;
    parent->first_child = new_id;
    return;
  }
  int cur = parent->first_child;
  while (cur > 0 && g_nodes[cur].next_sibling != ref_id) {
    cur = g_nodes[cur].next_sibling;
  }
  if (cur > 0) {
    child->next_sibling = ref_id;
    g_nodes[cur].next_sibling = new_id;
  }
}

static void free_node(int id) {
  if (!valid_id(id)) {
    return;
  }
  unlink_node(id);
  memset(&g_nodes[id], 0, sizeof(Node));
  update_live_state();
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

void __porf_import_dong_state_set_num(f64 slot, f64 value) {
  int idx = (int)slot;
  if (idx >= 0 && idx < 256) {
    g_state_nums[idx] = value;
  }
}

f64 __porf_import_dong_state_get_num(f64 slot) {
  int idx = (int)slot;
  if (idx >= 0 && idx < 256) {
    return g_state_nums[idx];
  }
  return 0;
}

static f64 g_stage_0;
static f64 g_stage_1;

void __porf_import_dong_stage_0(f64 v) {
  g_stage_0 = v;
}
void __porf_import_dong_stage_1(f64 v) {
  g_stage_1 = v;
}
void __porf_import_dong_stage_2(f64 v) {
  (void)v;
}
void __porf_import_dong_commit_set_textContent(void) {
  int id = (int)g_stage_0;
  char buf[256];
  size_t len = 0;
  if (!valid_id(id) || !read_utf8_bytestring(g_stage_1, buf, sizeof(buf), &len)) {
    return;
  }
  strncpy(g_nodes[id].text, buf, sizeof(g_nodes[id].text) - 1);
}

void __porf_import_dong_dom_get_textContent(f64 node_id) {
  int id = (int)node_id;
  if (!valid_id(id)) {
    set_result("", 0);
    return;
  }
  set_result(g_nodes[id].text, strlen(g_nodes[id].text));
}

f64 __porf_import_dong_create_element(f64 tag_ptr) {
  char tag[64];
  size_t len = 0;
  if (!read_utf8_bytestring(tag_ptr, tag, sizeof(tag), &len)) {
    return 0;
  }
  return (f64)alloc_node(0, tag, NULL);
}

f64 __porf_import_dong_create_text_node(f64 text_ptr) {
  char text[256];
  size_t len = 0;
  if (!read_utf8_bytestring(text_ptr, text, sizeof(text), &len)) {
    return 0;
  }
  return (f64)alloc_node(1, NULL, text);
}

void __porf_import_dong_append_child(f64 parent_id, f64 child_id) {
  append_child((int)parent_id, (int)child_id);
}

void __porf_import_dong_insert_before(f64 parent_id, f64 new_id, f64 ref_id) {
  insert_before((int)parent_id, (int)new_id, (int)ref_id);
}

void __porf_import_dong_remove(f64 node_id) {
  free_node((int)node_id);
}

void __porf_import_dong_replace_child(f64 parent_id, f64 new_id, f64 old_id) {
  int p = (int)parent_id;
  int neu = (int)new_id;
  int old = (int)old_id;
  if (!valid_id(p) || !valid_id(neu) || !valid_id(old)) {
    return;
  }
  insert_before(p, neu, old);
  int next = g_nodes[old].next_sibling;
  g_nodes[neu].next_sibling = next;
  if (g_nodes[p].first_child == old) {
    g_nodes[p].first_child = neu;
  } else {
    int cur = g_nodes[p].first_child;
    while (cur > 0 && g_nodes[cur].next_sibling != old) {
      cur = g_nodes[cur].next_sibling;
    }
    if (cur > 0) {
      g_nodes[cur].next_sibling = neu;
    }
  }
  g_nodes[neu].parent = p;
  free_node(old);
}

f64 __porf_import_dong_parent(f64 node_id) {
  int id = (int)node_id;
  if (!valid_id(id)) {
    return 0;
  }
  return (f64)g_nodes[id].parent;
}

f64 __porf_import_dong_first_child(f64 node_id) {
  int id = (int)node_id;
  if (!valid_id(id)) {
    return 0;
  }
  return (f64)g_nodes[id].first_child;
}

f64 __porf_import_dong_next_sibling(f64 node_id) {
  int id = (int)node_id;
  if (!valid_id(id)) {
    return 0;
  }
  return (f64)g_nodes[id].next_sibling;
}

f64 __porf_import_dong_clone_node(f64 node_id, f64 deep) {
  int id = (int)node_id;
  if (!valid_id(id)) {
    return 0;
  }
  (void)deep;
  return (f64)alloc_node(g_nodes[id].is_text, g_nodes[id].tag, g_nodes[id].text);
}

#include "t07_mutation.c"

int main(void) {
  g_memory = (char*)calloc(1, g_memory_pages * 65536u);
  if (!g_memory) {
    return 1;
  }
  g_nodes[1].used = 1;
  strncpy(g_nodes[1].tag, "root", sizeof(g_nodes[1].tag) - 1);
  update_live_state();
  g_state_nums[0] = (f64)live_count();
  t07_main();
  return 0;
}
