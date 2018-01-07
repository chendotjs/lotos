#include "../connection.h"
#include "../misc.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

connection_t *lotos_connections[MAX_CONNECTION] = {0};
static int heap_size = 0;

#define LCHILD(x) (((x) << 1) + 1)
#define RCHILD(x) (LCHILD(x) + 1)
#define PARENT(x) ((x - 1) >> 1)
#define INHEAP(n, x) (((-1) < (i)) && ((i) < (n)))

inline static void c_swap(int x, int y) {
  assert(x >= 0 && x < heap_size && y >= 0 && y < heap_size);
  connection_t *tmp = lotos_connections[x];
  lotos_connections[x] = lotos_connections[y];
  lotos_connections[y] = tmp;
  // update heap_idx
  lotos_connections[x]->heap_idx = y;
  lotos_connections[y]->heap_idx = x;
}

/* used for inserting */
static void heap_bubble_up(int idx) {
  while (PARENT(idx) >= 0) {
    int fidx = PARENT(idx); // fidx is father of idx;
    connection_t *c = lotos_connections[idx];
    connection_t *fc = lotos_connections[fidx];
    if (c->active_time >= fc->active_time)
      break;
    c_swap(idx, fidx);
    idx = fidx;
  }
}

// TODO: a bit confusing...
/* used for extracting */
static void heap_bubble_down(int idx) {}

static int heap_insert(connection_t *c) {
  if (heap_size >= MAX_CONNECTION) {
    return ERROR;
  }
  lotos_connections[heap_size++] = c;
  c->heap_idx = heap_size - 1;
  heap_bubble_up(heap_size - 1);
  return 0;
}

/******************************  TEST CASES  **********************************/

void TEST_CASE(int input[], int ans[], size_t n) {
  // init
  memset(lotos_connections, 0, sizeof(lotos_connections));
  heap_size = 0;
  int *arr = input;

  connection_t *conn_arr = malloc(sizeof(connection_t) * n);
  ERR_ON(conn_arr == NULL, "malloc");
  for (int i = 0; i < n; i++) {
    connection_t *c = conn_arr + i;
    c->active_time = arr[i];
    heap_insert(c);
  }

  // TEST
  assert(heap_size == n);
  for (int i = 0; i < n; i++) {
    printf("{%d %lu} ", i, lotos_connections[i]->active_time);
    assert(ans[i] == lotos_connections[i]->active_time);
  }
  printf("\n\n");
  free(conn_arr);
}

int main(int argc, char const *argv[]) {
  {
    int arr[] = {4, 1, 5, 9, 6, 0};
    int ans[] = {0, 4, 1, 9, 6, 5};
    TEST_CASE(arr, ans, sizeof(arr) / sizeof(int));
  }

  {
    int arr[] = {5, 8, 12, 19, 28, 20, 15, 22, 3};
    int ans[] = {3, 5, 12, 8, 28, 20, 15, 22, 19};
    TEST_CASE(arr, ans, sizeof(arr) / sizeof(int));
  }

  {
    int arr[] = {9, 12, 17, 30, 50, 20, 60, 65, 4, 19};
    int ans[] = {4, 9, 17, 12, 19, 20, 60, 65, 30, 50};
    TEST_CASE(arr, ans, sizeof(arr) / sizeof(int));
  }
  return 0;
}
