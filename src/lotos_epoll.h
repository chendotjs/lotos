#ifndef _LOTOS_EPOLL_H__
#define _LOTOS_EPOLL_H__
#include <sys/epoll.h>

#define MAX_EVENTS (10240)

extern struct epoll_event lotos_events[MAX_EVENTS]; // global

extern inline int lotos_epoll_create(int flags) { return epoll_create1(flags); }
extern int lotos_epoll_add(int epoll_fd, int fd, uint32_t events, struct epoll_event *ev);
extern int lotos_epoll_mod(int epoll_fd, int fd, uint32_t events, struct epoll_event *ev);
extern int lotos_epoll_del(int epoll_fd, int fd, uint32_t events, struct epoll_event *ev);
extern int lotos_epoll_wait(int epoll_fd, struct epoll_event *events, int max_events, int timeout);

#endif
