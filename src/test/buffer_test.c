#include "../buffer.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char const *argv[]) {
  {
    buffer_t *buffer = buffer_new(10);
    assert(buffer->len == 0 && buffer->free == 10);

    buffer = buffer_cat(buffer, "abc", 3);
    assert(buffer->len == 3 && buffer->free == 7);

    buffer_print(buffer);
    free(buffer);
  }

  {
    buffer_t *buffer = buffer_new(10);
    assert(buffer->len == 0 && buffer->free == 10);

    buffer = buffer_cat(buffer, "aaaaabbbbb", 10);
    assert(buffer->len == 10 && buffer->free == 0);

    buffer_print(buffer);
    free(buffer);
  }

  {
    buffer_t *buffer = buffer_new(10);
    assert(buffer->len == 0 && buffer->free == 10);

    buffer = buffer_cat(buffer, "", 0);
    assert(buffer->len == 0 && buffer->free == 10);

    buffer_print(buffer);
    free(buffer);
  }

  {
    buffer_t *buffer = buffer_new(10);
    assert(buffer->len == 0 && buffer->free == 10);

    buffer = buffer_cat(buffer, "abc", 3);
    assert(buffer->len == 3 && buffer->free == 7);
    buffer = buffer_cat(buffer, "defgh", 5);
    assert(buffer->len == 8 && buffer->free == 2);

    buffer_print(buffer);
    free(buffer);
  }


  {
    buffer_t *buffer = buffer_new(10);
    assert(buffer->len == 0 && buffer->free == 10);

    buffer = buffer_cat(buffer, "abc", 3);
    assert(buffer->len == 3 && buffer->free == 7);
    buffer = buffer_cat(buffer, "defghijk", 8);
    assert(buffer->len == 11 && buffer->free == 11);

    buffer_print(buffer);
    free(buffer);
  }
  return 0;
}
