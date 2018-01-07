#include "connection.h"
#include "misc.h"
#include "server.h"
#include <assert.h>
#include <fcntl.h>
#include <time.h>

connection_t *lotos_connections[MAX_CONNECTION];
static int heap_size = 0;

int connection_accept(int fd, struct sockaddr_in *paddr) {
  // Too many malloc would be slow
  connection_t *c = malloc(sizeof(connection_t));
  assert(c != NULL);
  if (c == NULL) {
    // TODO: close connection
  }

  /* fill in connection_t */
  c->fd = fd;
  c->event.events = EPOLLIN | EPOLLET; // edge trigger
  if (paddr)
    c->saddr = *paddr;
  c->active_time = time(NULL);

  if (connection_register(c) == ERROR) {
    // TODO: close connection
  }

#ifndef NDEBUG
  char ip_addr[32];
  uint16_t port;
  get_internet_address(ip_addr, 32, &port, &c->saddr);
  printf("fd: %2d %s:%u\n", fd, ip_addr, port);
#endif
  return 0;
}

int connection_register(connection_t *c) {
  if (heap_size + 1 > MAX_CONNECTION) {
    return ERROR;
  }
  lotos_connections[heap_size++] = c;
  //TODO: heap adjust

  return OK;
}

int set_fd_nonblocking(int fd) {
  int flag = fcntl(fd, F_GETFL, 0);
  ABORT_ON(flag == ERROR, "fcntl: F_GETFL");
  flag |= O_NONBLOCK;
  ABORT_ON(fcntl(fd, F_SETFL, flag) == ERROR, "fcntl: FSETFL");
  return 0;
}
