/* return-value.c */

#include <stdio.h>

struct string_buffer {
  char data[1024];
};

__attribute__((noinline)) struct string_buffer sb_init() {
  return (struct string_buffer){.data = "hey"};
}

volatile struct string_buffer sb;
int main() {
  sb = sb_init();
  printf("%s", sb.data);
  return 0;
}
