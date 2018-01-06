#ifndef _CONNECTION_H__
#define _CONNECTION_H__
#include <sys/epoll.h>

#define MAX_CONNECTION (10240)

// TODO: some other memebers, have not consider yet
typedef struct {
  int fd;                   /* connection fildes */
  struct epoll_event event; /* epoll event */
} connection_t;

extern int set_fd_nonblocking(int fd);

#endif
