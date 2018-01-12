#ifndef _CONNECTION_H__
#define _CONNECTION_H__
#include "misc.h"
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <time.h>

#define MAX_CONNECTION (10240)

// TODO: some other memebers, have not consider yet
typedef struct {
  int fd;                   /* connection fildes */
  struct epoll_event event; /* epoll event */
  struct sockaddr_in saddr; /* IP socket address */
  time_t active_time;       /* connection accpet time */
  int heap_idx;             /* idx at lotos_connections */
} connection_t;

extern connection_t *connection_accept(int fd, struct sockaddr_in *paddr);
extern int connection_register(connection_t *c);
extern void connection_unregister(connection_t *c);
extern int connection_close(connection_t *c);
extern void connection_prune();
extern bool connecion_is_expired(connection_t *c);
extern void connecion_set_reactivated(connection_t *c);
extern void connecion_set_expired(connection_t *c);


extern int set_fd_nonblocking(int fd);

#endif
