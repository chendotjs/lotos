#ifndef _LOTOS_EPOLL_H__
#define _LOTOS_EPOLL_H__
#include "connection.h"
#include <sys/epoll.h>

#define MAX_EVENTS (10240)
#define FILL_EPOLL_EVENT(pev, pconn, e_events)                                 \
  do {                                                                         \
    struct epoll_event *ev = pev;                                              \
    ev->data.ptr = pconn;                                                      \
    ev->events = e_events;                                                     \
  } while (0)

extern struct epoll_event lotos_events[MAX_EVENTS]; // global

extern int lotos_epoll_create(int flags);
extern int lotos_epoll_add(int epoll_fd, connection_t *pconn, uint32_t events,
                           struct epoll_event *pev);
extern int lotos_epoll_mod(int epoll_fd, connection_t *pconn, uint32_t events,
                           struct epoll_event *pev);
extern int lotos_epoll_del(int epoll_fd, connection_t *pconn, uint32_t events,
                           struct epoll_event *pev);
extern int lotos_epoll_wait(int epoll_fd, struct epoll_event *events,
                            int max_events, int timeout);

#endif
