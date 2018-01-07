#include "connection.h"
#include "lotos_epoll.h"
#include "misc.h"
#include "server.h"
#include <assert.h>
#include <fcntl.h>
#include <time.h>

/* lotos_connections is seen as a binary min heap */
connection_t *lotos_connections[MAX_CONNECTION] = {0};
static int heap_size = 0;

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
#endif

  return c;
}

int connection_register(connection_t *c) {
  if (heap_size >= MAX_CONNECTION) {
    return ERROR;
  }
  lotos_connections[heap_size++] = c;
  // TODO: heap adjust

  return OK;
}

// TODO: close connection, free memory
int connection_close(connection_t *c) {
  if (c == NULL)
    return OK;
  return OK;
}

int set_fd_nonblocking(int fd) {
  int flag = fcntl(fd, F_GETFL, 0);
  ABORT_ON(flag == ERROR, "fcntl: F_GETFL");
  flag |= O_NONBLOCK;
  ABORT_ON(fcntl(fd, F_SETFL, flag) == ERROR, "fcntl: FSETFL");
  return 0;
}
