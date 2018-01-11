#include "connection.h"
#include "lotos_epoll.h"
#include "misc.h"
#include "server.h"
#include <assert.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

/**************************  heap operation start  ****************************/

/* lotos_connections is seen as a binary min heap */
connection_t *lotos_connections[MAX_CONNECTION] = {0};
static int heap_size = 0;

#define LCHILD(x) (((x) << 1) + 1)
#define RCHILD(x) (LCHILD(x) + 1)
#define PARENT(x) ((x - 1) >> 1)
#define INHEAP(n, x) (((-1) < (x)) && ((x) < (n)))

inline static void c_swap(int x, int y) {
  assert(x >= 0 && x < heap_size && y >= 0 && y < heap_size);
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
    assert(INHEAP(heap_size, proper_child));
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

static void heap_print() {
  connection_t *c;
  int i;
  printf("----------------heap---------------\n");
  for (i = 0; i < heap_size; i++) {
    c = lotos_connections[i];
    printf("[%2d] %p fd: %2d heap_idx: %2d active_time: %lu\n", i, c, c->fd,
           c->heap_idx, c->active_time);
  }
  printf("----------------heap---------------\n");
}

/**************************  heap operation end  ******************************/

connection_t *connection_accept(int fd, struct sockaddr_in *paddr) {
  // Too many malloc would be slow
  connection_t *c = malloc(sizeof(connection_t));
  assert(c != NULL);
  if (c == NULL) { // malloc fail
    connection_close(c);
    return NULL;
  }

  /* fill in connection_t */
  c->fd = fd;
  if (paddr)
    c->saddr = *paddr;
  c->active_time = time(NULL);

  set_fd_nonblocking(c->fd);

  if (connection_register(c) == ERROR) {
    connection_close(c);
    return NULL;
  }

  if (lotos_epoll_add(epoll_fd, c, EPOLLIN | EPOLLET, &c->event) == ERROR) {
    connection_close(c);
    return NULL;
  }

#ifndef NDEBUG
  char ip_addr[32];
  uint16_t port;
  get_internet_address(ip_addr, 32, &port, &c->saddr);
  printf("fd: %2d %s:%u\n", fd, ip_addr, port);
  printf("malloc %p %d\n", c, heap_size);
#endif

  return c;
}

int connection_register(connection_t *c) {
  if (heap_size >= MAX_CONNECTION) {
    return ERROR;
  }
  return heap_insert(c);
}

void connection_unregister(connection_t *c) {
  assert(heap_size >= 1);
  lotos_connections[c->heap_idx] = lotos_connections[heap_size - 1];
  lotos_connections[c->heap_idx]->heap_idx = c->heap_idx;
  heap_size--;
  heap_bubble_down(c->heap_idx);
}

/* close connection, free memory */
int connection_close(connection_t *c) {
  if (c == NULL)
    return OK;
  /*
   * explicitly delete fd from epoll_set, see `man 7 epoll` Q6
   * Q6  Will closing a file descriptor cause it to be removed from all epoll
   * sets automatically?
   */
  lotos_epoll_del(epoll_fd, c, 0, NULL);
  close(c->fd);
  connection_unregister(c);
  free(c);
  return OK;
}

void connection_prune() {
  while (heap_size > 0) {
    connection_t *c = lotos_connections[0];
    if (time(NULL) - c->active_time >= server_config.timeout) {
#ifndef NDEBUG
      // heap_print();
      printf("prune %p %d\n", c, heap_size);
#endif
      connection_close(c);
    } else
      break;
  }
}

int set_fd_nonblocking(int fd) {
  int flag = fcntl(fd, F_GETFL, 0);
  ABORT_ON(flag == ERROR, "fcntl: F_GETFL");
  flag |= O_NONBLOCK;
  ABORT_ON(fcntl(fd, F_SETFL, flag) == ERROR, "fcntl: FSETFL");
  return 0;
}
