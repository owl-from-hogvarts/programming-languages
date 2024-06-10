#include <stddef.h>
#define _DEFAULT_SOURCE

#include "mem.h"

#include "mem_internals.h"
#include "util.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

void debug_block(struct block_header * b, const char * fmt, ...);
void debug(const char * fmt, ...);

extern inline block_size size_from_capacity(block_capacity cap);
extern inline block_capacity capacity_from_size(block_size sz);

static bool block_is_big_enough(size_t query, struct block_header * block) {
  return block->capacity.bytes >= query;
}
static size_t pages_count(size_t mem) {
  return mem / getpagesize() + ((mem % getpagesize()) > 0);
}
static size_t round_pages(size_t mem) {
  return getpagesize() * pages_count(mem);
}

static void block_init(void * restrict addr, block_size block_sz,
                       void * restrict next) {
  *((struct block_header *)addr) = (struct block_header){
      .next = next, .capacity = capacity_from_size(block_sz), .is_free = true};
}

static size_t region_actual_size(size_t query) {
  return size_max(round_pages(query), REGION_MIN_SIZE);
}

extern inline bool region_is_invalid(const struct region * r);

const int PAGE_PROTRECTION = PROT_READ | PROT_WRITE;
const int PAGE_MAP_DEFAULT_OPTIONS = MAP_PRIVATE | MAP_ANONYMOUS;
// required to be -1 for anonymus mappings
const int PAGE_FD = -1;
// requried to be 0 for anonymus mappings
const __off_t PAGE_OFFSET = 0;

static void * map_pages(void const * addr, size_t length,
                        int additional_flags) {
  return mmap((void *)addr,
              length,
              PAGE_PROTRECTION,
              PAGE_MAP_DEFAULT_OPTIONS | additional_flags,
              PAGE_FD,
              PAGE_OFFSET);
}

static bool should_retry_allocate(void * const addr) {
  // in real world mmap could for different reasons
  // but tests do not simulate or requires to cover
  // such scenarious
  return addr == MAP_FAILED /* && (errno == EEXIST || errno == EINVAL) */;
}

/*  аллоцировать регион памяти и инициализировать его блоком */
static struct region alloc_region(void * const addr, size_t query) {
  // create region
  // const size_t regions_desired_size = query + offsetof()
  const size_t region_size
      = region_actual_size(size_from_capacity((block_capacity){query}).bytes);
  // forcing alignment breaks allocation:
  // if unaligned address is passed, should use the smallest
  // aligned address greater than unaligned one
  // const size_t alignment = getpagesize();
  // const size_t aligned_address = (((size_t)addr) & ~(alignment - 1));

  void * mapped_addr = map_pages(addr, region_size, MAP_FIXED_NOREPLACE);
  // check for EINVAL 'cause platform specific address requirments may not be
  // satisfied
  if (should_retry_allocate(mapped_addr)) {
    // try again without MAP_FIXED to allocate
    // at least somewhere
    mapped_addr = map_pages(addr, region_size, 0);
  }

  if (mapped_addr == MAP_FAILED) {
    return REGION_INVALID;
  }

  // init block
  block_init(mapped_addr, (block_size){.bytes = region_size}, NULL);

  return (struct region){
      .addr = mapped_addr,
      .size = region_size,
      .extends = mapped_addr == addr,
  };
}

static void * block_after(struct block_header const * block);

void * heap_init(size_t initial) {
  const struct region region = alloc_region(HEAP_START, initial);
  if (region_is_invalid(&region)) {
    return NULL;
  }
  return region.addr;
}

static bool is_on_same_page(const void * const one, const void * const two) {
  if (!one && !two) {
    return false;
  }
  int64_t difference = (int64_t)((int64_t)one - (int64_t)two);
  // abs
  difference = difference < 0 ? -difference : difference;
  return difference < getpagesize();
}

/*  освободить всю память, выделенную под кучу */
void heap_term() {
  struct block_header * block = HEAP_START;
  while (block) {
    // next part of the code is really questionable
    // need insight on how to handle same page blocks better

    // probaly this is a piece of shit
    // fixme: remove duplicate `block->next`
    struct block_header * next = block->next;
    const size_t length_to_clean = size_from_capacity(block->capacity).bytes;
    // check range and not srict equality:
    // there can be multiple small blocks placed continiously
    // whithin same page as block_after(block) so they
    // will be cleaned too
    while (is_on_same_page(block_after(block), next)) {
      next = next->next;
    }
    if (munmap(block, length_to_clean) != 0) {
      err("munmap failed with error %" PRId8, errno);
    }
    block = next;
  }
}

#define BLOCK_MIN_CAPACITY 24

/*  --- Разделение блоков (если найденный свободный блок слишком большой )--- */

static bool block_splittable(struct block_header * restrict block,
                             size_t query) {
  return block->is_free
      && query + offsetof(struct block_header, contents) + BLOCK_MIN_CAPACITY
             <= block->capacity.bytes;
}

/** split into base and next blocks
 * block will be located like this in memory:
 * base block
 * next block
 */
static bool split_if_too_big(struct block_header * block, size_t query) {
  const size_t base_block_capacity = size_max(query, BLOCK_MIN_CAPACITY);
  const block_capacity full_block_capacity = block->capacity;

  if (!block_splittable(block, base_block_capacity)) {
    return false;
  }
  block->capacity.bytes = base_block_capacity;
  void * const next_block_address = block_after(block);

  int a;
  block_init(next_block_address,
             ((block_size){full_block_capacity.bytes - base_block_capacity}),
             block->next);

  block->next = next_block_address;

  return true;
}

/*  --- Слияние соседних свободных блоков --- */

static void * block_after(struct block_header const * block) {
  if (block == NULL) {
    return NULL;
  }
  return (void *)(block->contents + block->capacity.bytes);
}
static bool blocks_continuous(struct block_header const * fst,
                              struct block_header const * snd) {
  return (void *)snd == block_after(fst);
}

static bool mergeable(struct block_header const * restrict fst,
                      struct block_header const * restrict snd) {
  return fst->is_free && snd->is_free && blocks_continuous(fst, snd);
}

static bool try_merge_with_next(struct block_header * block) {
  struct block_header * next = block->next;
  if (next == NULL) {
    return false;
  }
  if (!mergeable(block, next)) {
    return false;
  }

  block->next = next->next;
  block->capacity.bytes += size_from_capacity(next->capacity).bytes;

  return true;
}

/*  --- ... ecли размера кучи хватает --- */

struct block_search_result {
  enum { BSR_FOUND_GOOD_BLOCK, BSR_REACHED_END_NOT_FOUND, BSR_CORRUPTED } type;
  struct block_header * block;
};

static struct block_search_result
find_good_or_last(struct block_header * restrict block, size_t sz) {
  if (!block) {
    return (struct block_search_result){.type = BSR_CORRUPTED, .block = NULL};
  }

  // non-last block matches
  // last block matches
  // no block matches

  while (block) {
    // `continue` is required for clang-format
    // to put loop onto single line
    while (try_merge_with_next(block)) continue;
    if (block->is_free && block->capacity.bytes >= sz) {
      return (struct block_search_result){.type = BSR_FOUND_GOOD_BLOCK,
                                          .block = block};
    }

    if (!block->next) {
      break;
    }

    block = block->next;
  }

  return (struct block_search_result){.type = BSR_REACHED_END_NOT_FOUND,
                                      .block = block};
}

/*  Попробовать выделить память в куче начиная с блока `block` не пытаясь
 расширить кучу Можно переиспользовать как только кучу расширили. */
static struct block_search_result
try_memalloc_existing(size_t query, struct block_header * block) {
  // find appropriate block
  const struct block_search_result result = find_good_or_last(block, query);
  if (result.type == BSR_FOUND_GOOD_BLOCK) {
    split_if_too_big(result.block, query);
    result.block->is_free = false;
  }
  return result;
}

static struct block_header * grow_heap(struct block_header * restrict last,
                                       size_t query) {
  void * const region_desired_base_address = block_after(last);
  const struct region region = alloc_region(region_desired_base_address, query);
  struct block_header * const allocated_block = region.addr;
  // set breakpoint here
  last->next = allocated_block;
  const bool is_merged = try_merge_with_next(last);
  if (!is_merged) {
    return allocated_block;
  }

  return last;
}

/*  Реализует основную логику malloc и возвращает заголовок выделенного блока */
static struct block_header * memalloc(size_t query,
                                      struct block_header * heap_start) {
  if (!heap_start) {
    return NULL;
  }

  const struct block_search_result result
      = try_memalloc_existing(query, heap_start);

  if (result.type == BSR_CORRUPTED || result.block == NULL) {
    return NULL;
  }

  if (result.type == BSR_FOUND_GOOD_BLOCK) {
    return result.block;
  }

  if (result.type == BSR_REACHED_END_NOT_FOUND) {
    const struct block_search_result retried = try_memalloc_existing(query, grow_heap(result.block, query));
    if (retried.type != BSR_FOUND_GOOD_BLOCK) {
      return NULL;
    }

    return retried.block;
  }

  return NULL;
}

void * _malloc(size_t query) {
  struct block_header * const addr
      = memalloc(query, (struct block_header *)HEAP_START);
  if (addr)
    return addr->contents;
  else
    return NULL;
}

static struct block_header * block_get_header(void * contents) {
  return (struct block_header *)(((uint8_t *)contents)
                                 - offsetof(struct block_header, contents));
}

void _free(void * mem) {
  if (!mem)
    return;
  struct block_header * header = block_get_header(mem);
  header->is_free = true;
  try_merge_with_next(header);
}
