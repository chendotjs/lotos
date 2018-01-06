#ifndef _LOTOS_EPOLL_H__
#define _LOTOS_EPOLL_H__
#include <sys/epoll.h>

#define MAX_EVENTS (10240)
#define FILL_EPOLL_EVENT(pev, fildes, e_events)                                                    \
  do {                                                                                             \
    struct epoll_event *ev = pev;                                                                  \
    ev->data.fd = fildes;                                                                          \
    ev->events = e_events;                                                                         \
  } while (0)

extern struct epoll_event lotos_events[MAX_EVENTS]; // global

extern int lotos_epoll_create(int flags);
extern int lotos_epoll_add(int epoll_fd, int fd, uint32_t events, struct epoll_event *pev);
extern int lotos_epoll_mod(int epoll_fd, int fd, uint32_t events, struct epoll_event *pev);
extern int lotos_epoll_del(int epoll_fd, int fd, uint32_t events, struct epoll_event *pev);
extern int lotos_epoll_wait(int epoll_fd, struct epoll_event *events, int max_events, int timeout);

#endif
