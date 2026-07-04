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

void __porf_import_dong_print(f64) {}
void __porf_import_dong_dom_set_textContent(f64, f64) {}
