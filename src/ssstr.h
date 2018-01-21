/**
 *  simple static c-style string, used with buffer_t or constant c string,
 *  no need to copy memory, just save time
 */
#ifndef _SSSTR_H__
#define _SSSTR_H__

#include "misc.h"
#include <string.h>

typedef struct {
  char *str;
  int len;
} ssstr_t;

/**
 * const char *p = "hello";
 * SSSTR("hello") ✅
 * SSSTR(p)❌
 */
#define SSSTR(cstr)                                                            \
  (ssstr_t) { cstr, sizeof(cstr) - 1 }

static inline void ssstr_init(ssstr_t *s) {
  s->str = NULL;
  s->len = 0;
}

extern void ssstr_print(const ssstr_t *s);
extern int ssstr_cmp(const ssstr_t *l, const ssstr_t *r);
extern bool ssstr_equal(const ssstr_t *s, const char *cstr);

#endif
