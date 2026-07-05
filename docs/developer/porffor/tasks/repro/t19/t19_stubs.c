#include <stdio.h>
#include <stdint.h>

typedef double f64;

static void utf8_putchar(unsigned int cp) {
  if (cp <= 0x7F) {
    fputc((int)cp, stdout);
  } else if (cp <= 0x7FF) {
    fputc(0xC0 | (cp >> 6), stdout);
    fputc(0x80 | (cp & 0x3F), stdout);
  } else {
    fputc(0xE0 | (cp >> 12), stdout);
    fputc(0x80 | ((cp >> 6) & 0x3F), stdout);
    fputc(0x80 | (cp & 0x3F), stdout);
  }
}

void __porf_import_print(f64 x) {
  char buf[64];
  snprintf(buf, sizeof(buf), "%.15g", x);
  fputs(buf, stdout);
}

void __porf_import_printChar(f64 x) {
  utf8_putchar((unsigned int)(int)x);
}

extern int t19_promise_cases_main(void);

int main(void) {
  return t19_promise_cases_main();
}
