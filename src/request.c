#include "buffer.h"
#include "connection.h"
#include "misc.h"
#include <assert.h>
#include <string.h>

void request_init(request_t *r, connection_t *c) {
  assert(r != NULL);
  memset(r, 0, sizeof(request_t));
  r->c = c;
  r->b = buffer_init();
}
