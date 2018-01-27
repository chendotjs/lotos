/**
 *  simple static c-style string, used with buffer_t or constant c string,
 *  no need to copy memory, just save time
 */
#ifndef _SSSTR_H__
#define _SSSTR_H__

#include "misc.h"
#include <string.h>
#include <strings.h>

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

static inline void ssstr_set(ssstr_t *s, const char *cstr) {
  s->str = (char *)cstr;
  s->len = strlen(cstr);
}

extern void ssstr_print(const ssstr_t *s);
extern void ssstr_tolower(ssstr_t *s);
extern int ssstr_cmp(const ssstr_t *l, const ssstr_t *r);
extern bool ssstr_equal(const ssstr_t *s, const char *cstr);

static inline bool ssstr_caseequal(ssstr_t *s, const char *cstr) {
  return strncasecmp(s->str, cstr, strlen(cstr)) == 0 ? TRUE : FALSE;
}

#endif
