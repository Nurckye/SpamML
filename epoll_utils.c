#include "epoll_utils.h"
#include <sys/epoll.h>


extern int epollfd;

int as_epoll_create(void)
{
	return epoll_create(1);
}


int as_epoll_add_fd(int fd, int flags)
{
    struct epoll_event ev;
    
    ev.events = flags;
	ev.data.fd = fd;
    return epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}


int as_epoll_add_ptr(int fd, void *ptr, int flags)
{
    struct epoll_event ev;

    ev.events = flags;
	ev.data.ptr = ptr;
    return epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}


int as_epoll_update_ptr(int fd, void *ptr, int flags)
{
    struct epoll_event ev;

    ev.events = flags;
	ev.data.ptr = ptr;
    return epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
}


int as_epoll_del_ptr(int fd, void *ptr)
{
    struct epoll_event ev;

    ev.events = EPOLLIN;
	ev.data.ptr = ptr;
    return epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
}


int as_epoll_wait_loop(struct epoll_event *ev)
{
	return epoll_wait(epollfd, ev, 1, -1);
}
