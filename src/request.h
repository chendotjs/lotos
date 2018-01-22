#ifndef _REQUEST_H__
#define _REQUEST_H__

#include "buffer.h"
#include "connection.h"
#include "http_parser.h"
#include "misc.h"
#include <assert.h>

struct request {
  struct connection *c;                 /* belonged connection */
  buffer_t *ib;                         /* request buffer */
  buffer_t *ob;                         /* response buffer */
  parse_archive par;                    /* parse_archive */
  int resource_fd;                      /* resource fildes */
  int resource_size;                    /* resource size */
  int (*req_handler)(struct request *); /* request handler for rl, hd, bd */
};
typedef struct request request_t;

extern int request_init(request_t *r, struct connection *c);
extern int request_reset(request_t *r);
extern int request_handle(struct connection *c);

#endif
