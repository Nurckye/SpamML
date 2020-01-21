#ifndef EPOLL_HEADER
#define EPOLL_HEADER

#include <sys/epoll.h>

int as_epoll_create(void);
int as_epoll_add_fd(int, int);
int as_epoll_add_ptr(int, void*, int);
int as_epoll_update_ptr(int, void*, int);
int as_epoll_del_ptr(int, void*);
int as_epoll_wait_loop(struct epoll_event*);


#endif