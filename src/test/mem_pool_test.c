#include "../mem_pool.h"
#include "minctest.h"
#include <stdio.h>
#include <string.h>

void test1() {
  mem_pool_t pool;
  pool_create(&pool, 4, 20);

  void **q = pool.blocks;
  lok(*q == pool.blocks + 20);

  q = pool.blocks + 20;
  lok(*q == pool.blocks + 40);

  q = pool.blocks + 60;
  lok(*q == 0);

  pool_destroy(&pool);
}

void test2() {
  mem_pool_t pool;
  pool_create(&pool, 4, 20);
  lok(pool.block_num == 4);
  lok(pool.block_size == 20);
  lok(pool.block_allocated == 0);

  void *p;

  p = pool_alloc(&pool);
  lok(p == pool.blocks);
  lok(pool.block_allocated == 1);

  p = pool_alloc(&pool);
  lok(p == pool.blocks + 20);

  p = pool_alloc(&pool);
  lok(p == pool.blocks + 40);

  p = pool_alloc(&pool);
  lok(p == pool.blocks + 60);
  lok(pool.block_allocated == 4);

  p = pool_alloc(&pool);
  lok(p == NULL);
  lok(pool.block_allocated == 4);

  p = pool_alloc(&pool);
  lok(p == NULL);
  lok(pool.block_allocated == 4);

  pool_destroy(&pool);
}

void test3() {
  mem_pool_t pool;
  pool_create(&pool, 4, 20);
  lok(pool.block_num == 4);
  lok(pool.block_size == 20);
  lok(pool.block_allocated == 0);

  void *p;

  p = pool_alloc(&pool);
  lok(p == pool.blocks);
  lok(pool.block_allocated == 1);

  p = pool_alloc(&pool);
  lok(p == pool.blocks + 20);

  p = pool_alloc(&pool);
  lok(p == pool.blocks + 40);

  pool_free(&pool, p);
  lok(pool.block_allocated == 2);

  p = pool_alloc(&pool);
  lok(p == pool.blocks + 40);
  lok(pool.block_allocated == 3);

  p = pool_alloc(&pool);
  lok(p == pool.blocks + 60);
  lok(pool.block_allocated == 4);

  p = pool_alloc(&pool);
  lok(p == NULL);
  lok(pool.block_allocated == 4);

  p = pool_alloc(&pool);
  lok(p == NULL);
  lok(pool.block_allocated == 4);

  pool_destroy(&pool);
}

int main(int argc, char const *argv[]) {
  lrun("test1", test1);
  lrun("test2", test2);
  lrun("test3", test3);
  lresults();
  printf("\n\n");
  return lfails != 0;
}
