#ifndef EPOLL_H
#define EPOLL_H

#include "evbase.h"
#include "reactor.h"
#include <sys/epoll.h>

typedef struct epoll_event ep_event_t;

static evset(ep_event_t *event, int fd, int filter)
{
    event->data.fd = fd;
    event->events = filter;
}

static cbool addepevent(int kerevid, int fd, int filter, int flags)
{
    //设置事件
    struct ep_event_t addevent;
    evset(&addevent, fd, filter);
    
    //加入到系统内核监听
    return (epoll_ctl(kerevid, flags, fd, &addevent) == -1) ? FAILED : SUCCESS;
}

cbool createep(struct reactor *reactor, void *data)
{
    reactor->multiplex.kerevid = epoll_create(10);
    if (reactor->multiplex.kerevid == -1)
    {
        return FAILED;
    }
    
    reactor->multiplex.evelistlen = *((int *)data);
    reactor->multiplex.evelist = (void *)malloc(sizeof(ep_event_t) * reactor->multiplex.evelistlen);
    return (reactor->multiplex.evelist == NULL) ? FAILED : SUCCESS;
}

cbool addep(struct event *event, void *data)
{
    //设置事件标记
    int filter = (event->evtype & EV_READ) ? EPOLLIN : EPOLLOUT;
    int filter |= (event->evtype & EV_PERSIST) ? 0 : EPOLLONESHOT;
    int flags = (event->evtype & EV_CTL_ADD) ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
    
    return addepevent(event->reactor->multiplex.kerevid, event->fd, filter, flags);
}

cbool delep(struct event *event, void *data)
{
    //设置事件标记
    int filter = (event->evtype & EV_READ) ? EPOLLIN : EPOLLOUT;
    int flags = (event->evtype & EV_CTL_DEL) ? EPOLL_CTL_DEL : EPOLL_CTL_DEL;
    
    return addepevent(event->reactor->multiplex.kerevid, event->fd, filter, flags);
}

cbool dispatchep(struct reactor *reactor, struct timeval *tv, void *data)
{
    int watime = (int)(tv->tv_sec * 1000 + tv->tv_usec / 1000);
    int actnum = epoll_wait(reactor->multiplex.kerevid, reactor->multiplex.evelist, reactor->multiplex.evelistlen, watime);
    if (actnum == -1)
    {
        return FAILED;
    }
    
    //获取活动的用户事件
    ep_event_t *evelist = (ep_event_t *)reactor->multiplex.evelist;
    for (int i = 0; i < actnum; i++)
    {
        addactevent((int)evelist[i].data.fd, reactor);
    }
    
    return SUCCESS;
}

cbool destroyep(struct reactor *reactor, void *data)
{
    free(reactor->multiplex.evelist);
    return (close(reactor->multiplex.kerevid) == -1) ? FAILED : SUCCESS;
}

void setevtopep(struct eventtop *evtop, int etindex)
{
    evtop[etindex].recreate = 0;
    evtop[etindex].name = "epoll";
    
    evtop[etindex].create = createep;
    evtop[etindex].add = addep;
    evtop[etindex].del = delep;
    evtop[etindex].dispatch = dispatchep;
    evtop[etindex].destroy = destroyep;
}

#endif
