/* generic_list.c */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

void error(const char *s) {
  fprintf(stderr, "%s", s);
  abort();
}

#define _print(type, x) type##_print(x)

// Обратите внимание на обратные слэши в конце каждой строчки кроме последней!
// Они экранируют переводы строк, позволяя написать макрос во много строчек.
#define print(x)                                                        \
  _Generic((x),                                                         \
           int64_t : int64_t_print(x),                                  \
           double : double_print(x),                                    \
           default : error("Unsupported operation"))

void int64_t_print(int64_t i) { printf("%" PRId64 " ", i); }
void double_print(double d) { printf("%lf ", d); }
void newline_print() { puts(""); }

#define DEFINE_LIST(type)                                               \
  struct list_##type {                                                  \
    type value;                                                         \
    struct list_##type* next;                                           \
  };                                                                    \
                                                                        \
  void list_##type##_push(struct list_##type * l, type value) {         \
    if(!l) return;                                                      \
    while (l->next) l = l->next;                                        \
                                                                        \
    l->next = malloc(sizeof(struct list_##type));                       \
    l->next->value = value;                                             \
    l->next->next = NULL;                                               \
  }                                                                     \
                                                                        \
  void list_##type##_print(struct list_##type const* l) {               \
    while(l) {                                                          \
      print(l->value);                                                  \
      l = l->next;                                                      \
    }                                                                   \
  }                                                                     

DEFINE_LIST(int64_t)
DEFINE_LIST(double)

#define list_push(l, value)                                             \
    _Generic((l),                                                       \
           struct list_int64_t* : list_int64_t_push(l, value),          \
           struct list_double* : list_double_push(l, value),            \
           default : error("Unsupported operation"))

#define list_print(l)                                                   \
    _Generic((l),                                                       \
           struct list_int64_t* : list_int64_t_print(l),                \
           struct list_double* : list_double_print(l),                  \
           default : error("Unsupported operation"))

int main() {
  struct list_int64_t a = (struct list_int64_t) {.value = 1, .next=NULL};
  list_push(&a, 2);
  list_push(&a, 3);
  list_push(&a, 4);
  list_print(&a);

  struct list_double b = (struct list_double) {.value = 1.4f, .next=NULL};
  list_push(&b, 2.4f);
  list_push(&b, 3.4f);
  list_push(&b, 4.4f);
  list_print(&b);
}

// struct list_int64_t {
//   int64_t value;
//   struct list_int64_t* next;
// };


// void list_push(struct list_int64_t* l, int64_t value) { 
//   while (l->next != NULL) l = l->next;

//   l->next = malloc(sizeof(struct list_int64_t));
//   l->next->value = value;
//   l->next->next = NULL;
// }  

// void list_int64_t_print(struct list_int64_t* l) {
//   while(l->next != NULL) {
//     print(l->value);
//     l = l->next;
//   }
// }