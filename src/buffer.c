#include "buffer.h"
#include "misc.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

buffer_t *buffer_init() { return buffer_new(BUFSIZ); }

buffer_t *buffer_new(size_t initlen) {
  buffer_t *pb = malloc(sizeof(buffer_t) + initlen + 1); // reserve space for \0
  assert(pb != NULL);
  if (pb == NULL) {
    return NULL;
  }
  pb->len = 0;
  pb->free = initlen;
  pb->buf[pb->len] = '\0';
  return pb;
}

void buffer_free(buffer_t *pb) {
  if (pb != NULL) {
    free(pb);
  }
}

inline void buffer_clear(buffer_t *pb) {
  pb->free += pb->len;
  pb->len = 0;
}

/**
 *  newbuf = buffer_cat(oldbuf, "abc", 3);
 *  After the call, oldbuf is no longer valid, and must be substituted with
 * newbuf
 *
 * @param pb    [description]
 * @param buf   [description]
 * @param nbyte [description]
 */
buffer_t *buffer_cat(buffer_t *pb, const char *buf, size_t nbyte) {
  buffer_t *npb = NULL;

  if (nbyte <= buffer_avail(pb)) { // no need to realloc
    memcpy(pb->buf + pb->len, buf, nbyte);
    pb->len += nbyte;
    pb->free -= nbyte;
    pb->buf[pb->len] = '\0';
    return pb;
  }
  /* realloc */
  size_t cur_len = buffer_len(pb);
  size_t new_len = cur_len + nbyte;
  /* realloc strategy */
  if (new_len < BUFFER_LIMIT)
    new_len *= 2;
  else
    new_len += BUFFER_LIMIT;

  npb = realloc(pb, sizeof(buffer_t) + new_len + 1);
  if (npb == NULL)
    return NULL;
  memcpy(npb->buf + npb->len, buf, nbyte);
  npb->len += nbyte;
  npb->free = new_len - npb->len;
  npb->buf[npb->len] = '\0';
  return npb;
}

buffer_t *buffer_cat_cstr(buffer_t *pb, const char *cstr) {
  return buffer_cat(pb, cstr, strlen(cstr));
}

void buffer_print(buffer_t *pb) {
  char *p = pb->buf;
  for (; p != pb->buf + pb->len; p++) {
    printf("%c", *p);
    fflush(stdout);
  }
  printf("\n");
}
