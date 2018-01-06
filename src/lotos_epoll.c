#include "lotos_epoll.h"
#include "misc.h"
#include <sys/epoll.h>

struct epoll_event lotos_events[MAX_EVENTS];

#define FILL_EPOLL_EVENT(ev, fildes, e_events)                                                        \
  do {                                                                                             \
    ev->data.fd = fildes;                                                                          \
    ev->events = e_events;                                                                         \
  } while (0)

int lotos_epoll_add(int epoll_fd, int fd, uint32_t events, struct epoll_event *ev) {
  FILL_EPOLL_EVENT(ev, fd, events);
  return epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, ev);
}

int lotos_epoll_mod(int epoll_fd, int fd, uint32_t events, struct epoll_event *ev) {
  FILL_EPOLL_EVENT(ev, fd, events);
  return epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, ev);
}

int lotos_epoll_del(int epoll_fd, int fd, uint32_t events, struct epoll_event *ev) {
  FILL_EPOLL_EVENT(ev, fd, events);
  return epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, ev);
}

inline int lotos_epoll_wait(int epoll_fd, struct epoll_event *events, int max_events, int timeout) {
  return epoll_wait(epoll_fd, events, max_events, timeout);
}
