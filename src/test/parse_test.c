#include "../buffer.h"
#include "../http_parser.h"
#include "../misc.h"
#include "minctest/minctest.h"
#include <stdio.h>
#include <string.h>

void test_method1() {
  buffer_t *buffer = buffer_init();
  parse_settings st;
  parse_settings_init(&st, buffer);

  buffer_cat(buffer, "POST / HTTP/1.0", 10);
  parse_request_line(buffer, &st);
  lok(st.method_begin == buffer->buf);
  lok(st.next_parse_pos == buffer_end(buffer));
  lequal(HTTP_POST, st.method);
}

/* so parse_request_line can be called many times when recv new data */
void test_method2() {
  buffer_t *buffer = buffer_init();
  parse_settings st;
  parse_settings_init(&st, buffer);

  int status = -1;
  buffer_cat(buffer, "GE", 2);
  status = parse_request_line(buffer, &st);
  lequal(AGAIN, status);
  lok(st.method_begin == buffer->buf);
  lok(st.next_parse_pos == buffer_end(buffer));

  buffer_cat(buffer, "T ", 2);
  parse_request_line(buffer, &st);
  lequal(HTTP_GET, st.method);
}

//TODO: test
void test_method3() {
  buffer_t *buffer = buffer_init();
  parse_settings st;
  parse_settings_init(&st, buffer);

  int status = -1;
  char req_line[] = "GET /api/ HTTP/1.1\r\n";
  buffer_cat(buffer, req_line, strlen(req_line));
  status = parse_request_line(buffer, &st);
}

int main(int argc, char const *argv[]) {
  lrun("test_method1", test_method1);
  lrun("test_method2", test_method2);
  lrun("test_method3", test_method3);

  lresults();
  return lfails != 0;
  return 0;
}
