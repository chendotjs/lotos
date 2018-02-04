#include "../connection.h"
#include "../misc.h"
#include "minctest.h"
#include <stdio.h>
#include <string.h>

/* lotos_connections is seen as a binary min heap */
connection_t *lotos_connections[MAX_CONNECTION] = {0};
static int heap_size = 0;

#define LCHILD(x) (((x) << 1) + 1)
#define RCHILD(x) (LCHILD(x) + 1)
#define PARENT(x) ((x - 1) >> 1)
#define INHEAP(n, x) (((-1) < (x)) && ((x) < (n)))

inline static void c_swap(int x, int y) {
  lok(x >= 0 && x < heap_size && y >= 0 && y < heap_size);
  connection_t *tmp = lotos_connections[x];
  lotos_connections[x] = lotos_connections[y];
  lotos_connections[y] = tmp;
  // update heap_idx
  lotos_connections[x]->heap_idx = x;
  lotos_connections[y]->heap_idx = y;
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

/* used for extracting or active_time update larger */
static void heap_bubble_down(int idx) {
  while (TRUE) {
    int proper_child;
    int lchild = INHEAP(heap_size, LCHILD(idx)) ? LCHILD(idx) : (heap_size + 1);
    int rchild = INHEAP(heap_size, RCHILD(idx)) ? RCHILD(idx) : (heap_size + 1);
    if (lchild > heap_size && rchild > heap_size) { // no children
      break;
    } else if (INHEAP(heap_size, lchild) && INHEAP(heap_size, rchild)) {
      proper_child = lotos_connections[lchild]->active_time <
                             lotos_connections[rchild]->active_time
                         ? lchild
                         : rchild;
    } else if (lchild > heap_size) {
      proper_child = rchild;
    } else {
      proper_child = lchild;
    }
    // idx is the smaller than children
    if (lotos_connections[idx]->active_time <=
        lotos_connections[proper_child]->active_time)
      break;
    lok(INHEAP(heap_size, proper_child));
    c_swap(idx, proper_child);
    idx = proper_child;
  }
}

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
static int test_bubble_up = 0;
void TEST_BUBBLE_UP(int input[], int ans[], size_t n) {
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
  printf("%s %d:\n", "TEST_BUBBLE_UP CASE", test_bubble_up++);
  lok(heap_size == n);
  for (int i = 0; i < n; i++) {
    printf("{%d %lu} ", i, lotos_connections[i]->active_time);
    lok(ans[i] == lotos_connections[i]->active_time);
  }
  printf("\n");
  free(conn_arr);
}

static int test_bubble_down = 0;
void TEST_BUBBLE_DOWN(int input[], int ans[], size_t n, int pos, int nval) {
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

  // update value
  lotos_connections[pos]->active_time = nval;
  heap_bubble_down(pos);

  // TEST
  printf("%s %d:\n", "TEST_BUBBLE_DOWN CASE", test_bubble_down++);
  lok(heap_size == n);
  for (int i = 0; i < n; i++) {
    printf("{%d %lu} ", i, lotos_connections[i]->active_time);
    lok(ans[i] == lotos_connections[i]->active_time);
  }
  printf("\n");
  free(conn_arr);
}

int main(int argc, char const *argv[]) {
  {
    int arr[] = {4, 1, 5, 9, 6, 0};
    int ans[] = {0, 4, 1, 9, 6, 5};
    TEST_BUBBLE_UP(arr, ans, sizeof(arr) / sizeof(int));
  }

  {
    int arr[] = {5, 8, 12, 19, 28, 20, 15, 22, 3};
    int ans[] = {3, 5, 12, 8, 28, 20, 15, 22, 19};
    TEST_BUBBLE_UP(arr, ans, sizeof(arr) / sizeof(int));
  }

  {
    int arr[] = {9, 12, 17, 30, 50, 20, 60, 65, 4, 19};
    int ans[] = {4, 9, 17, 12, 19, 20, 60, 65, 30, 50};
    TEST_BUBBLE_UP(arr, ans, sizeof(arr) / sizeof(int));
  }

  printf("\n\n");

  {
    int arr[] = {4, 1, 5, 9, 6, 0};
    int ans[] = {1, 4, 5, 9, 6, 100};
    TEST_BUBBLE_DOWN(arr, ans, sizeof(arr) / sizeof(int), 0, 100);
  }

  {
    int arr[] = {4, 1, 5, 9, 6, 0};
    int ans[] = {0, 6, 1, 9, 100, 5};
    TEST_BUBBLE_DOWN(arr, ans, sizeof(arr) / sizeof(int), 1, 100);
  }

  {
    int arr[] = {4, 1, 5, 9, 6, 0};
    int ans[] = {0, 6, 1, 9, 8, 5};
    TEST_BUBBLE_DOWN(arr, ans, sizeof(arr) / sizeof(int), 1, 8);
  }

  {
    int arr[] = {4, 1, 5, 9, 6, 0};
    int ans[] = {0, 5, 1, 9, 6, 5};
    TEST_BUBBLE_DOWN(arr, ans, sizeof(arr) / sizeof(int), 1, 5);
  }

  {
    int arr[] = {4, 1, 5, 9, 6, 0};
    int ans[] = {0, 4, 1, 9, 7, 5};
    TEST_BUBBLE_DOWN(arr, ans, sizeof(arr) / sizeof(int), 4, 7);
  }

  {
    int arr[] = {4, 1, 5, 9, 6, 0};
    int ans[] = {0, 4, 1, 9, 6, 10};
    TEST_BUBBLE_DOWN(arr, ans, sizeof(arr) / sizeof(int), 5, 10);
  }

  {
    int arr[] = {4, 1, 5, 9, 6, 0};
    int ans[] = {0, 4, 3, 9, 6, 5};
    TEST_BUBBLE_DOWN(arr, ans, sizeof(arr) / sizeof(int), 2, 3);
  }

  {
    int arr[] = {4, 1, 5, 9, 6, 0};
    int ans[] = {0, 4, 5, 9, 6, 10};
    TEST_BUBBLE_DOWN(arr, ans, sizeof(arr) / sizeof(int), 2, 10);
  }

  {
    int arr[] = {1};
    int ans[] = {10};
    TEST_BUBBLE_DOWN(arr, ans, sizeof(arr) / sizeof(int), 0, 10);
  }
  lresults();
  printf("\n\n");
  return lfails != 0;
}
