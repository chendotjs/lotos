#include "buffer.h"
#include "misc.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

buffer_t *buffer_init() { return buffer_new(BUFSIZ); }

buffer_t *buffer_new(size_t initlen) {
  buffer_t *pb = malloc(sizeof(buffer_t) + initlen + 1);
  assert(pb != NULL);
  if (pb == NULL) {
    return NULL;
  }
  pb->len = 0;
  pb->free = initlen;
  pb->buf[initlen] = '\0';
  return pb;
}

void buffer_free(buffer_t *pb) {
  if (pb != NULL) {
    free(pb);
  }
}
