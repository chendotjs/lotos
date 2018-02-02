#define _GNU_SOURCE
#include "response.h"
#include "buffer.h"
#include "connection.h"
#include "dict.h"
#include "misc.h"
#include "request.h"
#include "server.h"
#include "ssstr.h"
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define ERR_HEADER_MAX_LEN (BUFSIZ)

static const char *status_table[512];

static ssstr_t mime_list[][2] = {
    {SSSTR("word"), SSSTR("application/msword")},
    {SSSTR("pdf"), SSSTR("application/pdf")},
    {SSSTR("zip"), SSSTR("application/zip")},
    {SSSTR("js"), SSSTR("application/javascript")},
    {SSSTR("gif"), SSSTR("image/gif")},
    {SSSTR("jpeg"), SSSTR("image/jpeg")},
    {SSSTR("jpg"), SSSTR("image/jpeg")},
    {SSSTR("png"), SSSTR("image/png")},
    {SSSTR("css"), SSSTR("text/css")},
    {SSSTR("html"), SSSTR("text/html")},
    {SSSTR("htm"), SSSTR("text/html")},
    {SSSTR("txt"), SSSTR("text/plain")},
    {SSSTR("xml"), SSSTR("text/xml")},
    {SSSTR("svg"), SSSTR("image/svg+xml")},
    {SSSTR("mp4"), SSSTR("video/mp4")},
};

static dict_t mime_dict;

void mime_dict_init() {
  size_t nsize = sizeof(mime_list) / sizeof(mime_list[0]);
  int i;
  dict_init(&mime_dict);
  for (i = 0; i < nsize; i++) {
    dict_put(&mime_dict, &mime_list[i][0], &mime_list[i][1]);
  }
}

void mime_dict_free() { dict_free(&mime_dict); }

void status_table_init() {
  memset(status_table, 0, sizeof(status_table));
#define XX(num, name, string) status_table[num] = #num " " #string;
  HTTP_STATUS_MAP(XX);
#undef XX
}

static err_page_t err_page;

int err_page_init() {
  err_page_t *ep = &err_page;
  // open error.html
  ep->err_page_fd = openat(server_config.rootdir_fd, "error.html", O_RDONLY);
  ABORT_ON(ep->err_page_fd == ERROR, "openat");
  struct stat st;
  fstat(ep->err_page_fd, &st);
  ep->raw_page_size = st.st_size;

  ep->rendered_err_page = buffer_init(ep->raw_page_size + ERR_HEADER_MAX_LEN);
  ABORT_ON(ep->rendered_err_page == NULL, "buffer_init");

  // mmap file to memory
  ep->raw_err_page =
      mmap(NULL, ep->raw_page_size, PROT_READ, MAP_SHARED, ep->err_page_fd, 0);
  ABORT_ON(ep->raw_err_page == NULL, "mmap");
  return OK;
}

void err_page_free() {
  err_page_t *ep = &err_page;
  buffer_free(ep->rendered_err_page);
  // munmap
  munmap((void *)ep->raw_err_page, ep->raw_page_size);
  close(ep->err_page_fd);
}

inline char *err_page_render_buf() { return err_page.rendered_err_page->buf; }

void response_append_status_line(struct request *r) {
  buffer_t *b = r->ob;
  if (r->par.version.http_minor == 1) {
    r->ob = buffer_cat_cstr(b, "HTTP/1.1 ");
  } else {
    r->ob = buffer_cat_cstr(b, "HTTP/1.0 ");
  }
  // status
  const char *status_str = status_table[r->status_code];
  if (status_str != NULL)
    r->ob = buffer_cat_cstr(b, status_str);
  r->ob = buffer_cat_cstr(b, CRLF);
}

void response_append_date(struct request *r) {
  buffer_t *b = r->ob;
  time_t now = time(NULL);
  struct tm *tm = gmtime(&now);
  size_t len = strftime(b->buf + buffer_len(b), b->free,
                        "Date: %a, %d %b %Y %H:%M:%S GMT" CRLF, tm);
  b->len += len;
  b->free -= len;
}

void response_append_server(struct request *r) {
  buffer_t *b = r->ob;
  r->ob = buffer_cat_cstr(b, "Server: ");
  r->ob = buffer_cat_cstr(b, SERVER_NAME CRLF);
}

void response_append_content_type(struct request *r) {
  buffer_t *b = r->ob;
  parse_archive *ar = &r->par;

  ssstr_t content_type;
  if (ar->err_req) {
    content_type = SSSTR("text/html");
    goto done;
  }
  ssstr_t *v = dict_get(&mime_dict, &ar->url.mime_extension, NULL);
  if (v != NULL) {
    content_type = *v;
  } else {
    content_type = SSSTR("text/html");
  }
done:;
  r->ob = buffer_cat_cstr(b, "Content-Type: ");
  r->ob = buffer_cat_cstr(b, content_type.str);
  r->ob = buffer_cat_cstr(b, CRLF);
}

void response_append_content_length(struct request *r) {
  buffer_t *b = r->ob;
  char cl[128];
  int len;
  buffer_t *rendered_err_page = err_page.rendered_err_page;
  // modify content_length when sending err page
  if (r->par.err_req) {
    len = snprintf(rendered_err_page->buf,
                   rendered_err_page->free + rendered_err_page->len,
                   err_page.raw_err_page, status_table[r->status_code]);
    err_page.rendered_page_size = len;
  } else {
    len = r->resource_size;
  }
  sprintf(cl, "Content-Length: %d" CRLF, len);
  r->ob = buffer_cat_cstr(b, cl);
}

void response_append_connection(struct request *r) {
  buffer_t *b = r->ob;
  ssstr_t connection;
  if (r->par.keep_alive) {
    connection = SSSTR("Connection: keep-alive");
  } else {
    connection = SSSTR("Connection: close");
  }
  r->ob = buffer_cat_cstr(b, connection.str);
  r->ob = buffer_cat_cstr(b, CRLF);
}

void response_append_crlf(struct request *r) {
  buffer_t *b = r->ob;
  r->ob = buffer_cat_cstr(b, CRLF);
}
