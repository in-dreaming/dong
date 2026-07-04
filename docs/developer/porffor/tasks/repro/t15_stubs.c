#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t u8;
typedef int32_t i32;
typedef uint32_t u32;
typedef double f64;

struct ReturnValue {
  f64 value;
  i32 type;
};

#include "t15_wasm_state.c"

static f64 readCount(void) {
  struct ReturnValue rv = t15_getCount(0, 0, 0, 0);
  return rv.value;
}

int main(void) {
  t15_main();
  if (readCount() != 0) {
    fprintf(stderr, "expected count 0 after main\n");
    return 1;
  }

  t15_export_onClick();
  t15_export_onClick();
  if (readCount() != 2) {
    fprintf(stderr, "expected count 2 after two clicks, got %.0f\n", readCount());
    return 1;
  }

  t15_export_echoArg(99.0);
  struct ReturnValue rv = t15_echoArg(0, 0, 0, 0, 99.0, 1);
  if (rv.value != 99.0) {
    fprintf(stderr, "expected echoArg 99, got %.1f\n", rv.value);
    return 1;
  }

  return 0;
}
