/* generic_list.c */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

void error(const char *s) {
  fprintf(stderr, "%s", s);
  abort();
}

#define DEFINE_LIST(type)                                                                                                                    \
  struct list_##type {                                                                                                                       \
    type value;                                                                                                                              \
    struct list_##type* next;                                                                                                                \
  };                                                                                                                                         \
                                                                                                                                             \
struct list_##type* node_##type##_create( type value ) {                                                                                     \
  struct list_##type* node = malloc(sizeof(struct list_##type));                                                                             \
  node->value = value;                                                                                                                       \
  node->next = NULL;                                                                                                                         \
                                                                                                                                             \
  return node;                                                                                                                               \
}                                                                                                                                            \
                                                                                                                                             \
struct list_##type* list_##type##_last( struct list_##type * list ) {                                                                        \
    while (list) {                                                                                                                           \
        if (!list->next) {                                                                                                                   \
            return list;                                                                                                                     \
        }                                                                                                                                    \
                                                                                                                                             \
        list = list->next;                                                                                                                   \
    }                                                                                                                                        \
                                                                                                                                             \
    return NULL;                                                                                                                             \
}                                                                                                                                            \
                                                                                                                                             \
void list_##type##_push(struct list_##type ** list, type value) {                                                                            \
  struct list_##type * const last_node = list_##type##_last(*list);                                                                          \
  struct list_##type * const new_node = node_##type##_create(value);                                                                         \
  if (last_node == NULL) {                                                                                                                   \
    *list = new_node;                                                                                                                        \
    return;                                                                                                                                  \
  }                                                                                                                                          \
                                                                                                                                             \
  last_node->next = new_node;                                                                                                                \
}                                                                                                                                            \
                                                                                                                                             \
void list_##type##_print(struct list_##type * list) {                                                                                        \
  while (list != NULL) {                                                                                                                     \
    printf(_Generic((list->value), int64_t : "%" PRId64, double : "%f", uint8_t : "%" PRIu8, default : "Unsupported type!"), list->value);   \
    puts("");                                                                                                                                \
    list = list->next;                                                                                                                       \
  }                                                                                                                                          \
}                                                                                                                                            \

DEFINE_LIST(int64_t)
DEFINE_LIST(double)
DEFINE_LIST(uint8_t)


int main(void) {
  struct list_int64_t * int_list = NULL;
  list_int64_t_push(&int_list, 10);
  list_int64_t_push(&int_list, 73);
  list_int64_t_push(&int_list, 42);

  list_int64_t_print(int_list);
  puts("");
  
  struct list_double * double_list = NULL;
  list_double_push(&double_list, 1.0);
  list_double_push(&double_list, 1.0);

  list_double_print(double_list);
  puts("");

  struct list_uint8_t * byte_list = NULL;
  list_uint8_t_push(&byte_list, 1);

  list_uint8_t_print(byte_list);
  puts("");

 }
// list
