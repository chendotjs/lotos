#include "../buffer.h"
#include "minctest/minctest.h"
#include <stdio.h>
#include <string.h>

void test1() {
  buffer_t *buffer = buffer_new(10);
  lok(buffer->len == 0 && buffer->free == 10);

  buffer = buffer_cat(buffer, "abc", 3);
  lok(buffer->len == 3 && buffer->free == 7);

  lsequal(buffer->buf, "abc");
  free(buffer);
}

void test2() {
  buffer_t *buffer = buffer_new(10);
  lok(buffer->len == 0 && buffer->free == 10);

  buffer = buffer_cat(buffer, "aaaaabbbbb", 10);
  lok(buffer->len == 10 && buffer->free == 0);

  lsequal(buffer->buf, "aaaaabbbbb");
  free(buffer);
}

void test3()

{
  buffer_t *buffer = buffer_new(10);
  lok(buffer->len == 0 && buffer->free == 10);

  buffer = buffer_cat(buffer, "", 0);
  lok(buffer->len == 0 && buffer->free == 10);

  lsequal(buffer->buf, "");
  free(buffer);
}

void test4() {
  buffer_t *buffer = buffer_new(10);
  lok(buffer->len == 0 && buffer->free == 10);

  buffer = buffer_cat(buffer, "abc", 3);
  lok(buffer->len == 3 && buffer->free == 7);
  buffer = buffer_cat(buffer, "defgh", 5);
  lok(buffer->len == 8 && buffer->free == 2);

  lsequal(buffer->buf, "abcdefgh");
  free(buffer);
}

void test5() {
  buffer_t *buffer = buffer_new(10);
  lok(buffer->len == 0 && buffer->free == 10);

  buffer = buffer_cat(buffer, "abc", 3);
  lok(buffer->len == 3 && buffer->free == 7);
  buffer = buffer_cat(buffer, "defghijk", 8);
  lok(buffer->len == 11 && buffer->free == 11);

  lsequal(buffer->buf, "abcdefghijk");
  free(buffer);
}

void test6() {
  buffer_t *buffer = buffer_init();
  lok(buffer->len == 0 && buffer->free == BUFSIZ);

  static char buf[BUFFER_LIMIT];
  memset(buf, 'x', sizeof(buf));

  buffer = buffer_cat(buffer, "abc", 3);
  lok(buffer->len == 3 && buffer->free == BUFSIZ - 3);
  buffer = buffer_cat(buffer, buf, sizeof(buf));
  lequal(buffer->len, BUFFER_LIMIT + 3);
  lequal(buffer->free, BUFFER_LIMIT);

  free(buffer);
}

void test7() {
  buffer_t *buffer = buffer_new(10);
  lok(buffer->len == 0 && buffer->free == 10);
  lok(buffer->buf == buffer_end(buffer));

  buffer = buffer_cat(buffer, "abc", 3);
  lok(buffer->len == 3 && buffer->free == 7);

  lsequal(buffer->buf, "abc");
  lequal('c', *(buffer_end(buffer) - 1));
  lequal('b', *(buffer_end(buffer) - 2));
  free(buffer);
}

int main(int argc, char const *argv[]) {
  lrun("test1", test1);
  lrun("test2", test2);
  lrun("test3", test3);
  lrun("test4", test4);
  lrun("test5", test5);
  lrun("test6", test6);
  lrun("test7", test7);
  lresults();
  printf("\n\n");
  return lfails != 0;
}
