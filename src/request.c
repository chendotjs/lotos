#include "buffer.h"
#include "connection.h"
#include "misc.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

int request_init(request_t *r, connection_t *c) {
  assert(r != NULL);
  memset(r, 0, sizeof(request_t));
  r->c = c;
  r->b = buffer_init();
  if (r->b == NULL)
    return ERROR;
  return OK;
}

static int request_recv(request_t *r) {
  char buf[BUFSIZ];
  int len;
  while (TRUE) {
    len = recv(r->c->fd, buf, sizeof(buf), 0);
    if (len == 0) {
      return OK;
    }
    if (len == ERROR) {
      if (errno != EAGAIN) {
        perror("recv");
        return ERROR;
      } else
        return AGAIN; // does not have data now
    }
    buffer_cat(r->b, buf, len);
#ifndef NDEBUG
    printf("recv %d bytes:\n", len);
    buffer_print(r->b);
#endif
  }
  return AGAIN;
}

int request_handle(connection_t *c) {
  request_t *r = &c->req;
  int status = request_recv(r);
  if (status == ERROR)
    return ERROR;
  /**
   * TODO: parse request
   * do {
   *  status = parse()
   * }  while (status != error  && !parse_done)
   */
  return status;
}
