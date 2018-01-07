#ifndef _CONNECTION_H__
#define _CONNECTION_H__
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
} connection_t;

extern int connection_accept(int fd, struct sockaddr_in *paddr);
extern int connection_register(connection_t *c);

extern int set_fd_nonblocking(int fd);



#endif
