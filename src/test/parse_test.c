#include "../buffer.h"
#include "../http_parser.h"
#include "../misc.h"
#include "minctest/minctest.h"
#include <stdio.h>
#include <string.h>

void test_method1() {
  buffer_t *buffer = buffer_init();
  parse_archive ar;
  parse_archive_init(&ar, buffer);

  buffer_cat(buffer, "POST / HTTP/1.0", 10);
  parse_request_line(buffer, &ar);
  lok(ar.method_begin == buffer->buf);
  lok(ar.next_parse_pos == buffer_end(buffer));
  lequal(HTTP_POST, ar.method);
  lsequal(ar.request_url, "/");
  lsequal(ar.request_path, "/");
  lsequal(ar.query_string, "");
}

/* so parse_request_line can be called many times when recv new data */
void test_method2() {
  buffer_t *buffer = buffer_init();
  parse_archive ar;
  parse_archive_init(&ar, buffer);

  int status = -1;
  buffer_cat(buffer, "GE", 2);
  status = parse_request_line(buffer, &ar);
  lequal(AGAIN, status);
  lok(ar.method_begin == buffer->buf);
  lok(ar.next_parse_pos == buffer_end(buffer));

  char next_buf[] = "T /s?wd=hello%20world";
  buffer_cat(buffer, next_buf, strlen(next_buf));
  status = parse_request_line(buffer, &ar);
  lequal(HTTP_GET, ar.method);
  lequal(AGAIN, status);

  buffer_cat(buffer, " ", 1);
  status = parse_request_line(buffer, &ar);
  lsequal(ar.request_path, "/s");
  lsequal(ar.query_string, "wd=hello%20world");
}

/* valid request line */
void test_method3() {
  buffer_t *buffer = buffer_init();
  parse_archive ar;
  parse_archive_init(&ar, buffer);

  int status = -1;
  char req_line[] = "GET /api/set/?wd=123abc HTTP/1.1\r\nHost:localhost:8888";
  buffer_cat(buffer, req_line, strlen(req_line));
  status = parse_request_line(buffer, &ar);

  lequal(HTTP_GET, ar.method);
  lequal(ar.version.http_major, 1);
  lequal(ar.version.http_minor, 1);
  lsequal(ar.request_url, "/api/set/?wd=123abc");
  lsequal(ar.request_path, "/api/set/");
  lsequal(ar.query_string, "wd=123abc");
  lequal(OK, status);
  lok(ar.next_parse_pos[0] = 'H');
  lok(ar.next_parse_pos[1] = 'o');
}

/* invalid request line */
void test_method4() {
  buffer_t *buffer = buffer_init();
  parse_archive ar;
  parse_archive_init(&ar, buffer);

  int status = -1;
  char req_line[] =
      "POST /api/set/?wd=123abc HTTP/01.10\r\nHost:localhost:8888";
  buffer_cat(buffer, req_line, strlen(req_line));
  status = parse_request_line(buffer, &ar);

  lequal(HTTP_POST, ar.method);
  lequal(ar.version.http_major, 1);
  lequal(ar.version.http_minor, 10);
  lsequal(ar.request_url, "/api/set/?wd=123abc");
  lequal(ERROR, status);
}

/* curl GET */
void test_method5() {
  buffer_t *buffer = buffer_init();
  parse_archive ar;
  parse_archive_init(&ar, buffer);
  ar.state = S_HD_BEGIN;

  int status = -1;
  char req_line[] = "User-Agent: curl/7.18.0 (i486-pc-linux-gnu) "
                    "libcurl/7.18.0 OpenSSL/0.9.8g zlib/1.2.3.3 libidn/1.1\r\n"
                    "Host: 0.0.0.0=5000\r\n"
                    "Accept: */*\r\n"
                    "\r\n";
  buffer_cat(buffer, req_line, strlen(req_line));

  status = parse_header_line(buffer, &ar);
  lsequal(ar.header[0], "User-Agent");
  lsequal(ar.header[1], "curl/7.18.0 (i486-pc-linux-gnu) libcurl/7.18.0 "
                        "OpenSSL/0.9.8g zlib/1.2.3.3 libidn/1.1");
  lequal(OK, status);

  status = parse_header_line(buffer, &ar);
  lsequal(ar.header[0], "Host");
  lsequal(ar.header[1], "0.0.0.0=5000");
  lequal(OK, status);

  status = parse_header_line(buffer, &ar);
  lsequal(ar.header[0], "Accept");
  lsequal(ar.header[1], "*/*");
  lequal(OK, status);

  status = parse_header_line(buffer, &ar);
  lequal(CRLF_LINE, status);
}

/* curl GET */
void test_method6() {
  buffer_t *buffer = buffer_init();
  parse_archive ar;
  parse_archive_init(&ar, buffer);

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

  status = parse_request_line(buffer, &ar);

  lequal(HTTP_GET, ar.method);
  lequal(ar.version.http_major, 1);
  lequal(ar.version.http_minor, 1);
  lsequal(ar.request_url, "/favicon.ico");
  lsequal(ar.request_path, "/favicon.ico");
  lsequal(ar.query_string, "");
  lequal(OK, status);

  status = parse_header_line(buffer, &ar);
  lsequal(ar.header[0], "Host");
  lsequal(ar.header[1], "0.0.0.0=5000");
  lequal(OK, status);

  status = parse_header_line(buffer, &ar);
  lsequal(ar.header[0], "User-Agent");
  lsequal(ar.header[1], "Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9) "
                        "Gecko/2008061015 Firefox/3.0");
  lequal(OK, status);

  status = parse_header_line(buffer, &ar);
  lsequal(ar.header[0], "Accept");
  lsequal(ar.header[1],
          "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
  lequal(OK, status);

  status = parse_header_line(buffer, &ar);
  lsequal(ar.header[0], "Accept-Language");
  lsequal(ar.header[1], "en-us,en;q=0.5");
  lequal(OK, status);

  status = parse_header_line(buffer, &ar);
  lsequal(ar.header[0], "Accept-Encoding");
  lsequal(ar.header[1], "gzip,deflate");
  lequal(OK, status);

  status = parse_header_line(buffer, &ar);
  lsequal(ar.header[0], "Accept-Charset");
  lsequal(ar.header[1], "ISO-8859-1,utf-8;q=0.7,*;q=0.7");
  lequal(OK, status);

  status = parse_header_line(buffer, &ar);
  lsequal(ar.header[0], "Keep-Alive");
  lsequal(ar.header[1], "300");
  lequal(OK, status);

  status = parse_header_line(buffer, &ar);
  lsequal(ar.header[0], "Connection");
  lsequal(ar.header[1], "keep-alive");
  lequal(OK, status);

  status = parse_header_line(buffer, &ar);
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
