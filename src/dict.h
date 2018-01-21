#ifndef _DICT_H__
#define _DICT_H__

/**
 * dict used with a small amout of data, no need to rehash
 */

#include "ssstr.h"
#include "misc.h"

#define DICT_MASK_SIZE (0xFF)

typedef struct dict_node {
  ssstr_t k;
  void *v;
  struct dict_node *next;
} dict_node_t;

typedef struct {
  dict_node_t *table[DICT_MASK_SIZE + 1];
  unsigned int size_mask;
  unsigned int used;
} dict_t;

static inline void dict_init(dict_t *d) {
  memset(d, 0, sizeof(dict_t));
  d->size_mask = DICT_MASK_SIZE;
}

static inline void dict_node_init(dict_node_t *node, ssstr_t *s, void *v,
                                  dict_node_t *next) {
  node->k = *s;
  node->v = v;
  node->next = next;
}

extern void dict_put(dict_t *dict, const ssstr_t *key, void *val);
extern void *dict_get(dict_t *dict, const ssstr_t *key, bool *found_flag);
extern void dict_free(dict_t *d);


#endif
