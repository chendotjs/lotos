#ifndef _SERVER_H__
#define _SERVER_H__
#include "misc.h"
#include <netinet/in.h>
#include <stdint.h>
#include <sys/socket.h>

typedef struct {
  uint16_t port;   /* listen port */
  bool debug;      /* debug mode */
  int timeout;     /* connection expired time */
  uint32_t worker; /* worker num */
  char *rootdir;   /* html root directory */
  int rootdir_fd;  /* fildes of rootdir */
} config_t;

extern config_t server_config;
extern int epoll_fd;  /* epoll fd */
extern int listen_fd; /* server listen fd */

extern int config_parse(int argc,
                        char *argv[]);   /* parse command line options */
extern int server_setup(uint16_t port);  /* bind and listen */
extern int server_shutdown();            /* server shutdown */
extern int server_accept(int listen_fd); /* accpet all connections */

extern int
get_internet_address(char *host, int len, uint16_t *pport,
                     struct sockaddr_in *paddr); /* get ip/port info */

#endif
