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

/* valid request line */
void test_method3() {
  buffer_t *buffer = buffer_init();
  parse_settings st;
  parse_settings_init(&st, buffer);

  int status = -1;
  char req_line[] = "GET /api/set/?wd=123abc HTTP/1.1\r\nHost:localhost:8888";
  buffer_cat(buffer, req_line, strlen(req_line));
  status = parse_request_line(buffer, &st);

  lequal(HTTP_GET, st.method);
  lequal(st.version.http_major, 1);
  lequal(st.version.http_minor, 1);
  lsequal(st.request_url, "/api/set/?wd=123abc");
  lequal(OK, status);
  lok(st.next_parse_pos[0] = 'H');
  lok(st.next_parse_pos[1] = 'o');
}

/* invalid request line */
void test_method4() {
  buffer_t *buffer = buffer_init();
  parse_settings st;
  parse_settings_init(&st, buffer);

  int status = -1;
  char req_line[] =
      "POST /api/set/?wd=123abc HTTP/01.10\r\nHost:localhost:8888";
  buffer_cat(buffer, req_line, strlen(req_line));
  status = parse_request_line(buffer, &st);

  lequal(HTTP_POST, st.method);
  lequal(st.version.http_major, 1);
  lequal(st.version.http_minor, 10);
  lsequal(st.request_url, "/api/set/?wd=123abc");
  lequal(ERROR, status);
}

/* curl GET */
void test_method5() {
  buffer_t *buffer = buffer_init();
  parse_settings st;
  parse_settings_init(&st, buffer);
  st.state = S_HD_BEGIN;

  int status = -1;
  char req_line[] = "User-Agent: curl/7.18.0 (i486-pc-linux-gnu) "
                    "libcurl/7.18.0 OpenSSL/0.9.8g zlib/1.2.3.3 libidn/1.1\r\n"
                    "Host: 0.0.0.0=5000\r\n"
                    "Accept: */*\r\n"
                    "\r\n";
  buffer_cat(buffer, req_line, strlen(req_line));

  status = parse_header_line(buffer, &st);
  lsequal(st.header[0], "User-Agent");
  lsequal(st.header[1], "curl/7.18.0 (i486-pc-linux-gnu) libcurl/7.18.0 "
                        "OpenSSL/0.9.8g zlib/1.2.3.3 libidn/1.1");
  lequal(OK, status);

  status = parse_header_line(buffer, &st);
  lsequal(st.header[0], "Host");
  lsequal(st.header[1], "0.0.0.0=5000");
  lequal(OK, status);

  status = parse_header_line(buffer, &st);
  lsequal(st.header[0], "Accept");
  lsequal(st.header[1], "*/*");
  lequal(OK, status);

  status = parse_header_line(buffer, &st);
  lequal(CRLF_LINE, status);
}

/* curl GET */
void test_method6() {
  buffer_t *buffer = buffer_init();
  parse_settings st;
  parse_settings_init(&st, buffer);

  int status = -1;
  char req_line[] =
      "GET /favicon.ico HTTP/1.1\r\n"
      "Host: 0.0.0.0=5000\r\n"
      "User-Agent: Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9) "
      "Gecko/2008061015 Firefox/3.0\r\n"
      "Accept: "
      "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
      "Accept-Language: en-us,en;q=0.5\r\n"
      "Accept-Encoding: gzip,deflate\r\n"
      "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
      "Keep-Alive: 300\r\n"
      "Connection: keep-alive\r\n"
      "\r\n";
  buffer_cat(buffer, req_line, strlen(req_line));

  status = parse_request_line(buffer, &st);

  lequal(HTTP_GET, st.method);
  lequal(st.version.http_major, 1);
  lequal(st.version.http_minor, 1);
  lsequal(st.request_url, "/favicon.ico");
  lequal(OK, status);

  status = parse_header_line(buffer, &st);
  lsequal(st.header[0], "Host");
  lsequal(st.header[1], "0.0.0.0=5000");
  lequal(OK, status);

  status = parse_header_line(buffer, &st);
  lsequal(st.header[0], "User-Agent");
  lsequal(st.header[1], "Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9) "
                        "Gecko/2008061015 Firefox/3.0");
  lequal(OK, status);

  status = parse_header_line(buffer, &st);
  lsequal(st.header[0], "Accept");
  lsequal(st.header[1],
          "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
  lequal(OK, status);

  status = parse_header_line(buffer, &st);
  lsequal(st.header[0], "Accept-Language");
  lsequal(st.header[1], "en-us,en;q=0.5");
  lequal(OK, status);

  status = parse_header_line(buffer, &st);
  lsequal(st.header[0], "Accept-Encoding");
  lsequal(st.header[1], "gzip,deflate");
  lequal(OK, status);

  status = parse_header_line(buffer, &st);
  lsequal(st.header[0], "Accept-Charset");
  lsequal(st.header[1], "ISO-8859-1,utf-8;q=0.7,*;q=0.7");
  lequal(OK, status);

  status = parse_header_line(buffer, &st);
  lsequal(st.header[0], "Keep-Alive");
  lsequal(st.header[1], "300");
  lequal(OK, status);

  status = parse_header_line(buffer, &st);
  lsequal(st.header[0], "Connection");
  lsequal(st.header[1], "keep-alive");
  lequal(OK, status);

  status = parse_header_line(buffer, &st);
  lequal(CRLF_LINE, status);
}

int main(int argc, char const *argv[]) {
  lrun("test_method1", test_method1);
  lrun("test_method2", test_method2);
  lrun("test_method3", test_method3);
  lrun("test_method4", test_method4);
  lrun("test_method5", test_method5);
  lrun("test_method6", test_method6);

  lresults();
  printf("\n\n");
  return lfails != 0;
}
