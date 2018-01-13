#ifndef _BUFFER_H__
#define _BUFFER_H__
#include "misc.h"
#include <stdio.h>

typedef char *BUFFER;

/* usually 8M, a request buffer cannot be larger than this, while a response
 can recycle using buffer space.
*/
#define BUFFER_LIMIT (BUFSIZ * 1000)

typedef struct {
  int len;    /* used space length in buf */
  int free;   /* free space length in buf */
  char buf[]; /* store data */
} buffer_t;

extern buffer_t *buffer_init();
extern buffer_t *buffer_new(size_t initlen);
extern void buffer_free(buffer_t *pb);

static inline size_t buffer_len(const buffer_t *pb) { return pb->len; }

static inline size_t buffer_avail(const buffer_t *pb) { return pb->free; }

static inline size_t buffer_len_(const BUFFER buf) {
  buffer_t *pb = (void *)(buf - (sizeof(buffer_t)));
  return pb->len;
}

static inline size_t buffer_avail_(const BUFFER buf) {
  buffer_t *pb = (void *)(buf - (sizeof(buffer_t)));
  return pb->free;
}

#endif
