#include <stdio.h>
#define print_var(x) printf(#x " is %d\n", x )
#define CONSTANT 42

int main(void) {
  int some_var = 15;
  
  print_var(some_var);
  print_var(CONSTANT);
  print_var(7);
  

  return 0;
}
