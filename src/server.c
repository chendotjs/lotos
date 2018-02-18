#define _GNU_SOURCE
#include "connection.h"
#include "lotos_epoll.h"
#include "misc.h"
#include "response.h"
#include "server.h"
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

config_t server_config = {
    .port = 8888,
    .debug = FALSE,
    .timeout = 60,
    .worker = 4,
    .rootdir = NULL,
    .rootdir_fd = -1,
};

int epoll_fd = -1;
int listen_fd = -1;

#if USE_MEM_POOL
mem_pool_t connection_pool;
#endif

static void sigint_handler(int signum);
static int make_server_socket(uint16_t port, int backlog);
static int add_listen_fd();

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
      if (server_config.worker > sysconf(_SC_NPROCESSORS_ONLN)) {
        fprintf(stderr,
                "Config ERROR: worker num greater than cpu available cores.\n");
        return ERROR;
      }
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
    server_config.rootdir_fd = open(server_config.rootdir, O_RDONLY);
    ERR_ON(server_config.rootdir_fd == ERROR, server_config.rootdir);
    return OK;
  } else {
    perror(server_config.rootdir);
    return ERROR;
  }
}

static void sigint_handler(int signum) {
  if (signum == SIGINT) {
    mime_dict_free();
    header_handler_dict_free();
    err_page_free();
#if USE_MEM_POOL
    pool_destroy(&connection_pool);
#endif
    lotos_log(LOG_INFO, "lotos(PID: %u) exit...", getpid());
    kill(-getpid(), SIGINT);
    exit(0);
  }
}

int server_setup(uint16_t port) {
  signal(SIGINT, sigint_handler);
  signal(SIGPIPE, SIG_IGN); // client close, server write will recv sigpipe

  mime_dict_init();
  header_handler_dict_init();
  status_table_init();
  err_page_init();
#if USE_MEM_POOL
  pool_create(&connection_pool, 1024, sizeof(connection_t));
#endif

  listen_fd = make_server_socket(port, 1024);
  ABORT_ON(listen_fd == ERROR, "make_server_socket");

  epoll_fd = lotos_epoll_create(0);
  ABORT_ON(epoll_fd == ERROR, "lotos_epoll_create");

  ABORT_ON(add_listen_fd() == ERROR, "add_listen_fd");
  return OK;
}

int server_shutdown() { return close(listen_fd); }

int server_accept(int listen_fd) {
  int conn_fd;
  static struct sockaddr_in saddr;
  socklen_t saddrlen = sizeof(struct sockaddr_in);
  while ((conn_fd = accept(listen_fd, &saddr, &saddrlen)) != ERROR) {
    connection_accept(conn_fd, &saddr);
  }
  return 0;
}

static int make_server_socket(uint16_t port, int backlog) {
  int listen_fd;
  struct sockaddr_in saddr;

  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd == ERROR)
    return ERROR;

  int enable = 1;
  setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
  if (server_config.worker > 1) {
    // since linux 3.9
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(enable));
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

static int add_listen_fd() {
  set_fd_nonblocking(listen_fd);
  struct epoll_event ev;
  ev.data.ptr = &listen_fd;
  ev.events = EPOLLIN;
  return epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev);
}

int get_internet_address(char *host, int len, uint16_t *pport,
                         struct sockaddr_in *paddr) {
  strncpy(host, inet_ntoa(paddr->sin_addr), len);
  *pport = ntohs(paddr->sin_port);
  return 0;
}
