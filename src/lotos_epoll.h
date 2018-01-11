#ifndef _LOTOS_EPOLL_H__
#define _LOTOS_EPOLL_H__
#include "connection.h"
#include "misc.h"
#include <sys/epoll.h>

#define MAX_EVENTS (10240)
#define FILL_EPOLL_EVENT(pev, pconn, e_events)                                 \
  do {                                                                         \
    struct epoll_event *ev = pev;                                              \
    ev->data.ptr = pconn;                                                      \
    ev->events = e_events;                                                     \
  } while (0)
#define CONN_IS_IN(c) ((c)->event.events & EPOLLIN)
#define CONN_IS_OUT(c) ((c)->event.events & EPOLLOUT)

extern struct epoll_event lotos_events[MAX_EVENTS]; // global

extern int lotos_epoll_create(int flags);
extern int lotos_epoll_add(int epoll_fd, connection_t *restrict c,
                           uint32_t events, struct epoll_event *pev);
extern int lotos_epoll_mod(int epoll_fd, connection_t *restrict c,
                           uint32_t events, struct epoll_event *pev);
extern int lotos_epoll_del(int epoll_fd, connection_t *restrict c,
                           uint32_t events, struct epoll_event *pev);
extern int lotos_epoll_wait(int epoll_fd, struct epoll_event *events,
                            int max_events, int timeout);

static inline int connection_enable_in(int epoll_fd, connection_t *c) {
  if (CONN_IS_IN(c))
    return OK;
  c->event.events |= EPOLLIN;
  return epoll_ctl(epoll_fd, EPOLL_CTL_MOD, c->fd, &c->event);
}

static inline int connection_disable_in(int epoll_fd, connection_t *c) {
  if (!CONN_IS_IN(c))
    return OK;
  c->event.events &= ~EPOLLIN;
  return epoll_ctl(epoll_fd, EPOLL_CTL_MOD, c->fd, &c->event);
}

static inline int connection_enable_out(int epoll_fd, connection_t *c) {
  if (CONN_IS_OUT(c))
    return OK;
  c->event.events |= EPOLLOUT;
  return epoll_ctl(epoll_fd, EPOLL_CTL_MOD, c->fd, &c->event);
}

static inline int connection_disable_out(int epoll_fd, connection_t *c) {
  if (!CONN_IS_OUT(c))
    return OK;
  c->event.events &= ~EPOLLOUT;
  return epoll_ctl(epoll_fd, EPOLL_CTL_MOD, c->fd, &c->event);
}

#endif
