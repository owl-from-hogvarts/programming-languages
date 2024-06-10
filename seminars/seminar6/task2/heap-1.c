/* heap-1.c */

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define HEAP_BLOCKS 16
#define BLOCK_CAPACITY 1024

enum block_status { BLK_FREE = 0, BLK_ONE, BLK_FIRST, BLK_CONT, BLK_LAST };

struct heap {

  struct block {
    uint8_t contents[BLOCK_CAPACITY];
  } blocks[HEAP_BLOCKS];

  enum block_status status[HEAP_BLOCKS];
} global_heap = {0};

struct block_id {
  /* offset into heap, by which this block is located */
  size_t       heap_offset;
  bool         valid;
  struct heap* heap;
};

struct block_id block_id_new(size_t heap_offset, struct heap* from) {
  return (struct block_id){.valid = true, .heap_offset = heap_offset, .heap = from};
}
struct block_id block_id_invalid() {
  return (struct block_id){.valid = false};
}

bool block_id_is_valid(struct block_id bid) {
  return bid.valid && bid.heap_offset < HEAP_BLOCKS;
}

/* Find block
 * Negative number means no block is found
 */
ssize_t heap_find_available_block(struct heap * heap, size_t start_index) {
  // can't search starting from negative index, since this leads
  // to out of bounds memory access
  assert(start_index >= 0);
  for (size_t i = start_index; i < HEAP_BLOCKS; i++) {
    if (heap->status[i] == BLK_FREE) {
      return i;
    }
  }

  return -1;
}

ssize_t heap_find_first_available_block(struct heap * heap) {
  return heap_find_available_block(heap, 0);
}

bool block_is_free(struct block_id bid) {
  if (!block_id_is_valid(bid))
    return false;
  return bid.heap->status[bid.heap_offset] == BLK_FREE;
}

struct block_id block_allocate_single(struct heap * heap) {
  const ssize_t index = heap_find_first_available_block(heap);
  if (index < 0) {
    return block_id_invalid();
  }

  heap->status[index] = BLK_ONE;
  return block_id_new(index, heap);
}

// both indexes exclusively
void heap_mark_continues_blocks(struct heap * const heap, size_t start, size_t end) {
  // +1 to make start exclusive
  for (size_t index = start + 1; index < end; index++) {
    heap->status[index] = BLK_CONT;
  }
}

void block_id_set_status(struct block_id block, enum block_status new_status) {
  block.heap->status[block.heap_offset] = new_status;
}

enum block_status block_id_get_status(struct block_id block) {
  return block.heap->status[block.heap_offset];
}

struct block_id block_allocate_region(struct heap * heap, size_t size) {
  size_t last_unchecked_index = 0;
  // +1 to account for floor rounding
  const size_t blocks_required = ceil((float)size / BLOCK_CAPACITY);

  try_next_region: while (last_unchecked_index < HEAP_BLOCKS) {
    const ssize_t region_start_index = heap_find_available_block(heap, last_unchecked_index);
    // if no free block found, abort
    if (region_start_index < 0) {
      return block_id_invalid();
    }

    size_t blocks_found = 0;
    // use region_start_index as base index for search
    for (size_t current_block = region_start_index; (current_block < HEAP_BLOCKS) && (blocks_found < blocks_required); current_block++) {
      if (heap->status[current_block] != BLK_FREE) {
        // skip already checked blocks
        // +1 to account for current block
        last_unchecked_index += blocks_found + 1;
        goto try_next_region;
      }
      blocks_found += 1;
    }


    heap->status[region_start_index] = BLK_FIRST;
    // -1 to convert blocks_found which is length to index
    const size_t end_index = region_start_index + blocks_found - 1;
    heap->status[end_index] = BLK_LAST;
    heap_mark_continues_blocks(heap, region_start_index, end_index);
    
    return block_id_new(region_start_index, heap);
  }

  return block_id_invalid();
}

/* Allocate */
struct block_id block_allocate(struct heap* heap, size_t size) {
  if (size <= BLOCK_CAPACITY) {
    return block_allocate_single(heap);
  }

  return block_allocate_region(heap, size);
}

/* Free */

void block_id_free_sinle(struct block_id * block) {
  block_id_set_status(*block, BLK_FREE);
  block->valid = false;
}

void block_id_free(struct block_id * const  block) {
  if (!block->valid) {
    return;
  }

  const enum block_status status = block_id_get_status(*block);

  switch (status) {
    case BLK_ONE: {
      block_id_free_sinle(block);
      break;
    }
    case BLK_FIRST: {
      // go until LAST block is found
      struct heap * const heap = block->heap;
      size_t block_index = block->heap_offset;
      while (heap->status[block_index] != BLK_LAST) {
        assert(block_index < HEAP_BLOCKS);
        const enum block_status status = heap->status[block_index];
        assert(status == BLK_CONT || status == BLK_FIRST || status == BLK_LAST);
        heap->status[block_index] = BLK_FREE;
        block_index += 1;
      }

      // clear last
      heap->status[block_index] = BLK_FREE;

      block->valid = false;
    }
    case BLK_CONT:
    case BLK_LAST:
    // do nothing 
    case BLK_FREE:
      break;
  }

}

/* Printer */
const char* block_repr(struct block_id b) {
  static const char* const repr[] = {[BLK_FREE] = " .",
                                     [BLK_ONE] = " *",
                                     [BLK_FIRST] = "[=",
                                     [BLK_LAST] = "=]",
                                     [BLK_CONT] = " ="};
  if (b.valid)
    return repr[b.heap->status[b.heap_offset]];
  else
    return "INVALID";
}

void block_debug_info(struct block_id b, FILE* f) {
  fprintf(f, "%s", block_repr(b));
}

void block_foreach_printer(struct heap* h, size_t count,
                           void printer(struct block_id, FILE* f), FILE* f) {
  for (size_t c = 0; c < count; c++)
    printer(block_id_new(c, h), f);
}

void heap_debug_info(struct heap* h, FILE* f) {
  block_foreach_printer(h, HEAP_BLOCKS, block_debug_info, f);
  fprintf(f, "\n");
}
/*  -------- */

int main() {
  heap_debug_info(&global_heap, stdout);
  struct block_id first_block = block_allocate(&global_heap, 1023);
  block_allocate(&global_heap, 1024);
  block_id_free(&first_block);
  heap_debug_info(&global_heap, stdout);
  block_allocate(&global_heap, 1026);
  heap_debug_info(&global_heap, stdout);
  struct block_id long_block = block_allocate(&global_heap, 1024*4);
  heap_debug_info(&global_heap, stdout);
  block_allocate(&global_heap, 1024*3);
  block_id_free(&long_block);
  heap_debug_info(&global_heap, stdout);
  block_allocate(&global_heap, 1024*2);
  block_allocate(&global_heap, 1024*2);
  heap_debug_info(&global_heap, stdout);
  return 0;
}
