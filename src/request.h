#ifndef _REQUEST_H__
#define _REQUEST_H__

#include "buffer.h"
#include "connection.h"
#include "http_parser.h"
#include "misc.h"
#include <assert.h>



struct request {
  struct connection *c; /* belonged connection */
  buffer_t *b;          /* request buffer */
};
typedef struct request request_t;

extern int request_init(request_t *r, struct connection *c);
extern int request_handle(struct connection *c);

#endif
