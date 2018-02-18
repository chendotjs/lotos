#ifndef _MEM_POOL_H__
#define _MEM_POOL_H__
#include <stdlib.h>

typedef struct {
  size_t block_num;       /* num of blocks expected to alloc */
  size_t block_size;      /* size of each block */
  size_t block_allocated; /* num of allocated blocks */
  void *next;             /* next block to be allocated */
  void *blocks;           /* store data */
} mem_pool_t;

int pool_create(mem_pool_t *pool, size_t nmemb, size_t size);
void pool_destroy(mem_pool_t *pool);
void *pool_alloc(mem_pool_t *pool);
void pool_free(mem_pool_t *pool, void *ptr);
#endif
