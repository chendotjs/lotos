#include "mem_pool.h"
#include "misc.h"
#include <stdlib.h>
#include <string.h>

static inline void *last_block(mem_pool_t *pool) {
  return ((char *)pool->blocks) + (pool->block_num - 1) * pool->block_size;
}

int pool_create(mem_pool_t *pool, size_t nmemb, size_t size) {
  memset(pool, 0, sizeof(mem_pool_t));

  pool->block_num = nmemb;
  pool->block_size = size > sizeof(void *) ? size : sizeof(void *);

  pool->blocks = calloc(pool->block_num, pool->block_size);
  if (pool->blocks == NULL) {
    lotos_log(LOG_ERR, "memory pool creating failed...exit");
    exit(1);
  }

  pool->next = pool->blocks;

  /**
   * similar to what we do in ACM, pseudo linkedlist
   *
   *
   * suppose block_size = 30 and block_num = 4, the `blocks` memory:
   * 0          30         60         90        120
   * ———————————————————————————————————————————
   * |30        |60        |90        |NULL    |
   * ———————————————————————————————————————————
   *
   */
  char *char_ptr = pool->blocks;
  void **ptr_ptr;
  for (; char_ptr < (char *)last_block(pool);) {
    ptr_ptr = (void **)char_ptr;
    *ptr_ptr = (char_ptr += pool->block_size);
  }
  ptr_ptr = (void **)char_ptr;
  *ptr_ptr = NULL;
  return OK;
}

void pool_destroy(mem_pool_t *pool) {
  if (pool->blocks) {
    free(pool->blocks);
    pool->blocks = NULL;
  }
}

void *pool_alloc(mem_pool_t *pool) {
  if (pool->block_allocated >= pool->block_num)
    return NULL;

  void *cur = pool->next;
  void **ptr_ptr = cur;
  if (ptr_ptr) {
    pool->next = *ptr_ptr;
    pool->block_allocated++;
  }
  return cur;
}

void pool_free(mem_pool_t *pool, void *ptr) {
  if (ptr == NULL)
    return;

  void **ptr_ptr = ptr;
  *ptr_ptr = pool->next;
  pool->next = ptr;
  pool->block_allocated--;
}
