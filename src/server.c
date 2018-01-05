#define _GNU_SOURCE
#include "misc.h"
#include "server.h"
#include <dirent.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

config_t server_config = {
    .port = 8888,
    .debug = FALSE,
    .timeout = 60,
    .worker = 4,
    .rootdir = NULL,
};

extern int config_parse(int argc, char *argv[]);
extern int startup(uint16_t port);

static void sigint_handler(int signum);
static int make_server_socket(uint16_t port, int backlog);

int config_parse(int argc, char *argv[]) {
  int c;
  while ((c = getopt(argc, argv, "p:dt:w:r:")) != -1) {
    switch (c) {
    case 'p':
      server_config.port = atoi(optarg);
      break;
    case 'd':
      server_config.debug = TRUE;
      break;
    case 't':
      server_config.timeout = atoi(optarg);
      break;
    case 'w':
      server_config.worker = atoi(optarg);
      break;
    case 'r':
      server_config.rootdir = optarg;
      break;
    default:
      return ERROR;
    }
  }
  DIR *dirp = NULL;
  if (server_config.rootdir != NULL &&
      (dirp = opendir(server_config.rootdir)) != NULL) {
    closedir(dirp);
    return OK;
  } else {
    perror(server_config.rootdir);
    return ERROR;
  }
}

static void sigint_handler(int signum) {
  if (signum == SIGINT) {
    lotos_log(LOG_INFO, "lotos gracefully exit...");
    kill(-getpid(), SIGINT);
    exit(0);
  }
}

//TODO: add epoll and test slow_client whether will trigger
int startup(uint16_t port) {
  signal(SIGINT, sigint_handler);

  int listen_fd;
  listen_fd = make_server_socket(port, 1024);
  ABORT_ON(listen_fd == ERROR, "make_server_socket");
  return OK;
}

static int make_server_socket(uint16_t port, int backlog) {
  int listen_fd;
  struct sockaddr_in saddr;

  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd == ERROR)
    return ERROR;

  int on = 1;
  setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
  if (server_config.worker > 1) {
    // since linux 3.9
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
  }

  memset((void *)&saddr, 0, sizeof(saddr));

  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(port);
  saddr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(listen_fd, (struct sockaddr *)&saddr, sizeof(saddr)) != OK)
    return ERROR;
  if (listen(listen_fd, backlog) != OK)
    return ERROR;
  return listen_fd;
}
