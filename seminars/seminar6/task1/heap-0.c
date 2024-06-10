/* heap-0.c */

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define heap_blocks 16
#define block_capacity 1024

struct heap {

  struct block {
    char contents[block_capacity];
  } blocks[heap_blocks];

  bool is_occupied[heap_blocks];
} global_heap = {0};

struct block_id {
  /* denotes offset into heap by which the block is located */
  size_t heap_offset;
  bool   valid;
  struct heap* heap;
};

struct block_id block_id_new(size_t heap_offset, struct heap* from) {
  return (struct block_id){.valid = true, .heap_offset = heap_offset, .heap = from};
}
struct block_id block_id_invalid() { return (struct block_id){.valid = false}; }

bool block_id_is_valid(struct block_id bid) {
  return bid.valid && bid.heap_offset < heap_blocks;
}
/* find block */
bool block_is_free(struct block_id bid) {
  return false;
}

/* allocate */
/* find a free block, reserve it and return its id */
struct block_id block_allocate(struct heap* heap) {
  // search through heap
  for (size_t i = 0; i < heap_blocks; i++) {
    if (heap->is_occupied[i]) {
      continue;
    }

    heap->is_occupied[i] = true;
    return block_id_new(i, heap);
  }

  return block_id_invalid();
}

/* mark block as 'free' */
void block_free(struct block_id b) {
  b.valid = false;
  b.heap->is_occupied[b.heap_offset] = false;
}

/* printer */
const char* block_repr(struct block_id b) {
  static const char* const repr[] = {[false] = " .", [true] = " ="};
  if (b.valid)
    return repr[b.heap->is_occupied[b.heap_offset]];
  else
    return "x";
}

void block_debug_info(struct block_id b, FILE* f) {
  fprintf(f, "%s", block_repr( b));
}

void block_foreach_printer(struct heap* h, size_t count,
                           void  printer(struct block_id, FILE* f),
                           FILE* f) {
  for (size_t c = 0; c < count; c++)
    printer(block_id_new(c,h), f);
}

void heap_debug_info(struct heap* h, FILE* f) {
  block_foreach_printer(h, heap_blocks, block_debug_info, f);
  fprintf(f, "\n");
}
/*  -------- */

int main() {
  heap_debug_info(&global_heap, stdout);
  block_allocate(&global_heap);
  struct block_id bid = block_allocate(&global_heap);
  block_allocate(&global_heap);

  block_free(bid);
  heap_debug_info(&global_heap, stdout);
  return 0;
}
