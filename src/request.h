#ifndef _REQUEST_H__
#define _REQUEST_H__

#include "buffer.h"
#include "connection.h"
#include "http_parser.h"
#include "misc.h"
#include <assert.h>

#define MAX_HEADERS (20)
#define MAX_ELEMENT_SIZE (2048)

struct request {
  http_method method;
  http_version version;
  bool keep_alive;
  int num_headers;

  char query_string[MAX_ELEMENT_SIZE];
  char request_path[MAX_ELEMENT_SIZE];
  char request_url[MAX_ELEMENT_SIZE];
  char headers[MAX_HEADERS][2][MAX_ELEMENT_SIZE];

  struct connection *c; /* belonged connection */
  buffer_t *b;          /* requset buffer */
  int parse_pos;        /* parser position in buffer_t */
  int state;            /* parser state */
};
typedef struct request request_t;

extern int request_init(request_t *r, struct connection *c);
extern int request_handle(struct connection *c);

#endif
