#define _GNU_SOURCE
#include "buffer.h"
#include "connection.h"
#include "http_parser.h"
#include "lotos_epoll.h"
#include "misc.h"
#include "server.h"
#include "ssstr.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int request_handle_request_line(request_t *r);
static int request_handle_headers(request_t *r);
static int request_handle_body(request_t *r);

int request_init(request_t *r, connection_t *c) {
  assert(r != NULL);
  memset(r, 0, sizeof(request_t));
  r->c = c;
  r->ib = buffer_init();
  r->ob = buffer_init();
  if (r->ib == NULL || r->ob == NULL)
    return ERROR;
  parse_archive_init(&r->par, r->ib);

  r->req_handler = request_handle_request_line;
  return OK;
}

/* when connection keep-alive, reuse request_t in connection_t */
int request_reset(request_t *r) {
  buffer_t *ib = r->ib;
  buffer_t *ob = r->ob;
  connection_t *c = r->c;

  memset(r, 0, sizeof(request_t));
  r->ib = ib;
  r->ob = ob;
  r->c = c;
  buffer_clear(ib);
  buffer_clear(ob);
  parse_archive_init(&r->par, r->ib);

  r->req_handler = request_handle_request_line;
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
        lotos_log(LOG_ERR, "recv: %s", strerror(errno));
        return ERROR;
      } else
        return AGAIN; /* does not have data now */
    }
    buffer_cat(r->ib, buf, len); /* append new data to buffer */
#ifndef NDEBUG
    printf("recv %d bytes:\n", len);
    buffer_print(r->ib);
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
   *
   * if status == AGAIN, then exit `request_handle` and try to recv in the next
   * loop.
   * if status == ERROR, then exit `request_handle` and expire this connection
   *
   */
  do {
    status = r->req_handler(r);
  } while (r->req_handler != NULL && status == OK);

  return status;
}

static int request_handle_request_line(request_t *r) {
  int status;
  status = parse_request_line(r->ib, &r->par);
  if (status == AGAIN) // not a complete request line
    return AGAIN;
  if (status != OK) { // INVALID_REQUEST
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
  relative_path = ar->request_path.len == 1 && ar->request_path.str[0] == '/'
                      ? "./"
                      : ar->request_path.str + 1;

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
    ssstr_set(&ar->mime_extention, "html");
  }
  r->resource_fd = fd;
  r->resource_size = st.st_size;
  r->req_handler = request_handle_headers;
  return OK;
}

static int request_handle_headers(request_t *r) {
  int status;
  buffer_t *b = r->ib;
  parse_archive *ar = &r->par;
  while (TRUE) {
    status = parse_header_line(b, ar);
    switch (status) {
    /* not a complete header */
    case AGAIN:
      return AGAIN;
    /* header invalid */
    case INVALID_REQUEST:
      // TODO: send error response to client
      return ERROR;
    /* all headers completed */
    case CRLF_LINE:
      goto header_done;
    /* a header completed */
    case OK:
#ifndef NDEBUG
      printf("recv header:\n");
      ssstr_print(&r->par.header[0]);
      ssstr_print(&r->par.header[1]);
#endif
      // TODO: handle header individually
      break;
    }
  }
header_done:;
  r->req_handler = request_handle_body;
  return OK;
}

static int request_handle_body(request_t *r) {
// TODO: parse body in parse.c and test
#ifndef NDEBUG
  printf("%s done\n", __FUNCTION__);
#endif

  connection_disable_in(epoll_fd, r->c);
  connection_enable_out(epoll_fd, r->c);

  r->req_handler = NULL; // body parse done !!! no more handlers
  return OK;
}
