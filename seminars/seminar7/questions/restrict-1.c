/* restrict-1.c */

void f(int * restrict x, int * restrict add) {
  *x += *add;
  *x += *add;
}
