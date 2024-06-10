#include <stdint.h>
#include <stdio.h>

void print_string(char str[]);
void print_file(char filename[]);

int main() {
  print_string("Please, enter file name: ");
  char input_string[256];
  scanf("%s", input_string);

  print_file(input_string);
}
