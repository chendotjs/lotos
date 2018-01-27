/**
 * ./slow_client 8888 [1-9]
 */

#define _GNU_SOURCE
#include "../misc.h"
#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

const char *requests[] = {
#define SLOW_CLIENT_GET 0
    "GET / HTTP/1.1\r\n"
    "Host:127.0.0.1:8888\r\n"
    "User-Agent:SLOW_CLIENT\r\n"
    "\r\n",

#define CURL_GET 1
    "GET /test HTTP/1.1\r\n"
    "User-Agent: curl/7.18.0 (i486-pc-linux-gnu) "
    "libcurl/7.18.0 OpenSSL/0.9.8g zlib/1.2.3.3 "
    "libidn/1.1\r\n"
    "Host: 0.0.0.0=5000\r\n"
    "Accept: */*\r\n"
    "\r\n",

#define FIREFOX_GET 2
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
    "\r\n",

#define POST_IDENTITY_BODY_WORLD 3
    "POST /post_identity_body_world?q=search#hey HTTP/1.1\r\n"
    "Accept: */*\r\n"
    "Transfer-Encoding: identity\r\n"
    "Content-Length: 5\r\n"
    "\r\n"
    "World",

#define GET_FUNKY_CONTENT_LENGTH 4
    "GET /get_funky_content_length_body_hello HTTP/1.0\r\n"
    "conTENT-Length: 5\r\n"
    "\r\n"
    "HELLO",

#define POST_CHUNKED_ALL_YOUR_BASE 5
    "POST /post_chunked_all_your_base HTTP/1.1\r\n"
    "Transfer-Encoding: chunked\r\n"
    "\r\n"
    "1e\r\nall your base are belong to us\r\n"
    "0\r\n"
    "\r\n",

};

int connect_to_server(uint16_t port) {
  static const char *host = "127.0.0.1";
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  ABORT_ON(fd == -1, "socket");

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  int status = inet_pton(AF_INET, host, &addr.sin_addr);
  ABORT_ON(status <= 0, "inet_pton");

  status = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
  ABORT_ON(status != 0, "connect");

  return fd;
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    return 0;
  }
  int fd = connect_to_server(atoi(argv[1]));
  int opt = atoi(argv[2]);

  if (opt >= sizeof(requests) / sizeof(requests[0]))
    return 1;

  /* send begin */
  for (size_t i = 0; i < strlen(requests[opt]); i++) {
    int ch = requests[opt][i];
    assert(1 == send(fd, &ch, 1, 0));
    printf("%c", ch);
    fflush(stdout);
    usleep(30 * 1000);
  }
  /* send end */

  /* recv begin */
  while (1) {
    int ch;
    int len = recv(fd, &ch, 1, 0);
    if (len != 1) {
      break;
    }
    printf("%c", ch);
    fflush(stdout);
  }
  /* recv end */

  close(fd);
  return 0;
}
