#include "ssstr.h"
#include <stdio.h>
#include <string.h>

void ssstr_print(const ssstr_t *s) {
  if (s == NULL || s->str == NULL)
    return;
  for (int i = 0; i < s->len; i++) {
    printf("%c\n", s->str[0]);
    fflush(stdout);
  }
}

int ssstr_cmp(const ssstr_t *l, const ssstr_t *r) {
  if (l == r || (l->str == r->str && l->len == r->len))
    return 0;
  if (l->str == NULL)
    return -1;
  if (r->str == NULL)
    return 1;

  int llen = l->len;
  int rlen = r->len;
  int minlen = llen > rlen ? rlen : llen;

  int i;
  for (i = 0; i < minlen; i++) {
    if (l->str[i] < r->str[i])
      return -1;
    else if (l->str[i] > r->str[i])
      return 1;
  }

  return (rlen == llen) ? 0 : ((llen < rlen) ? -1 : 1);
}
