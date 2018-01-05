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

  FILE *file = fopen(argv[2], "r");
  if (file == NULL) {
    fprintf(stderr, "open file: %s failed.", argv[2]);
    return -1;
  }

  /* send begin */
  while (1) {
    int ch = getc(file);
    if (ch == EOF) {
      break;
    }
    assert(1 == send(fd, &ch, 1, 0));
    printf("%c", ch);
    fflush(stdout);
    usleep(30 * 1000);
  }
  assert(2 == send(fd, CRLF, 2, 0));
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

  fclose(file);
  return 0;
}
