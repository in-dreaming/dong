#include <stdio.h>
extern int mod_a_main(void);
extern int mod_b_main(void);
int main(void) {
  int a = mod_a_main();
  int b = mod_b_main();
  return (a == 3 && b == 3) ? 0 : 1;
}
