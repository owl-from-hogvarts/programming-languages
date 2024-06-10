#ifndef OWL_LINKED_LIST_H
#define OWL_LINKED_LIST_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

enum move_dir { MD_UP, MD_RIGHT, MD_DOWN, MD_LEFT, MD_NONE };

// Определите тип обработчика событий move_callback с помощью typedef
typedef void move_callback(const enum move_dir direction);

struct list {
  move_callback * value;
  struct list * next;
};

struct maybe_move_callback {
  move_callback * value;
  bool isValid;
};

struct maybe_move_callback none_move_callback = { 0 };
struct maybe_move_callback some_move_callback(move_callback value) {
  const struct maybe_move_callback some = { .value = value, .isValid = true };
  return some;
}

struct list* node_create( move_callback value ) {
  struct list* node = malloc(sizeof(struct list));
  node->value = value;
  node->next = NULL;
    
  return node;
}

void list_add_front( struct list** old, move_callback value ) {
    struct list* new_node = node_create(value);
    new_node->next = *old;
    *old = new_node;
}

size_t list_length(const struct list* list ) {
    size_t counter = 0;
    while (list) {
        counter += 1;
        list = list->next;
    }
    
    return counter;
}

void list_destroy( struct list* list ) {
   while (list) {
      struct list* const current_elem = list;
      list = current_elem -> next; 
      free(current_elem);
   }
}

struct list* list_last( struct list * list ) {
    while (list) {
        if (!list->next) {
            return list;
        }
        
        list = list->next;
    }
    
    return NULL;
}

/*
 * adds `value` to the end of the `old` list
 */
void list_add_back( struct list** old, move_callback value ) {
    if (*old == NULL) {
        *old = node_create(value);
        return;
    }
    
    struct list * const new_node = node_create(value);
    struct list* const last_node = list_last(*old);
    last_node->next = new_node;
}

struct maybe_move_callback list_at( const struct list* list, size_t idx ) {
    size_t current_index = 0;
    while (list && current_index < idx) {
        list = list->next;
        current_index += 1;
    }
    
    if (!list) {
        return none_move_callback;
    }
    
    return some_move_callback(list->value);
}


#endif
