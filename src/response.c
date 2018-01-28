#include "buffer.h"
#include "connection.h"
#include "dict.h"
#include "request.h"
#include "response.h"
#include "ssstr.h"
#include <string.h>
#include <time.h>

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
#define XX(num, name, string) status_table[num] = #string;
  HTTP_STATUS_MAP(XX);
#undef XX
}

void response_append_status_line(struct request *r) {
  buffer_t *b = r->ob;
  if (r->par.version.http_minor == 1) {
    buffer_cat_cstr(b, "HTTP/1.1 ");
  } else {
    buffer_cat_cstr(b, "HTTP/1.0 ");
  }
  // status
  const char *status_str = status_table[r->status_code];
  if (status_str != NULL)
    buffer_cat_cstr(b, status_str);
  buffer_cat_cstr(b, CRLF);
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
