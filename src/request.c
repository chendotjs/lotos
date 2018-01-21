#define _GNU_SOURCE
#include "buffer.h"
#include "connection.h"
#include "http_parser.h"
#include "misc.h"
#include "server.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int request_init(request_t *r, connection_t *c) {
  assert(r != NULL);
  memset(r, 0, sizeof(request_t));
  r->c = c;
  r->b = buffer_init();
  if (r->b == NULL)
    return ERROR;
  parse_archive_init(&r->par, r->b);
  return OK;
}

static int request_recv(request_t *r) {
  char buf[BUFSIZ];
  int len;
  while (TRUE) {
    len = recv(r->c->fd, buf, sizeof(buf), 0);
    // https://stackoverflow.com/questions/2416944/can-read-function-on-a-connected-socket-return-zero-bytes
    if (len == 0) { /* if client close, will read EOF */
      return OK;
    }
    if (len == ERROR) {
      if (errno != EAGAIN) {
        perror("recv");
        return ERROR;
      } else
        return AGAIN; /* does not have data now */
    }
    buffer_cat(r->b, buf, len); /* append new data to buffer */
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
  if (status == ERROR || status == OK) /* error or client close */
    return ERROR;
  /**
   * TODO:
   * parse request main process:
   *
   * do {
   *  status = parse_request_line()
   *  status = parse_header()
   *  *status = parse_body()
   * }  while (status != error  && !parse_done)
   */

  return status;
}

static int request_handle_request_line(request_t *r) {
  int status;
  status = parse_request_line(r->b, &r->par);
  if (status == AGAIN) // not a complete request line
    return AGAIN;
  if (status != OK) { // INVALID_REQUEST or URL_OUT_OF_RANGE
    // TODO: send error response to client
    return status;
  }
  // status = OK now
  parse_archive *ar = &(r->par);
  /* check http version */
  if (ar->version.http_major > 1 || ar->version.http_minor > 1) {
    // TODO: send 505 error response to client
  }
  ar->keep_alive = (ar->version.http_major == 1 && ar->version.http_minor == 1);

  /* check request_path */
  const char *relative_path = NULL;
  relative_path = strlen(ar->request_path) == 1 && ar->request_path[0] == '/'
                      ? "./"
                      : ar->request_path + 1;

  int fd = openat(server_config.rootdir_fd, relative_path, O_RDONLY);
  if (fd == ERROR) {
    // TODO: send 404 error response to client
  }
  struct stat st;
  fstat(fd, &st);

  if (S_ISDIR(st.st_mode)) { // substitute dir to index.html
    // fd is a dir fildes
    int html_fd = openat(fd, "index.html", O_RDONLY);
    close(fd);
    if (fd == ERROR) {
      // TODO: send 404 error response to client
    }
    fd = html_fd;
    fstat(fd, &st);
    strncpy(ar->mime_extention, "html", sizeof(ar->mime_extention));
  }
  r->resource_fd = fd;
  r->resource_size = st.st_size;

  return OK;
}
