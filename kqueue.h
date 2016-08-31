#ifndef KQUEUE_H
#define KQUEUE_H

#include "evself.h"
#include "reactor.h"
#include <sys/event.h>

typedef struct kevent kq_event_t;

static cbool addkevent(int kerevid, int fd, int filter, int flags)
{
    //设置事件
    kq_event_t addkevent;
    EV_SET(&addkevent, fd, filter, flags, 0, 0, NULL);
    
    //添加事件
    return (kevent(kerevid, &addkevent, 1, NULL, 0, NULL) == -1) ? FAILED : SUCCESS;
}

cbool createkq(struct reactor *reactor, void *data)
{
    reactor->multiplex.kerevid = kqueue();
    if (reactor->multiplex.kerevid == -1)
    {
        return FAILED;
    }
    
    reactor->multiplex.evelistlen = *((int *)data);
    reactor->multiplex.evelist = (void *)malloc(sizeof(kq_event_t) * reactor->multiplex.evelistlen);
    return (reactor->multiplex.evelist == NULL) ? FAILED : SUCCESS;
}

cbool addkq(struct event *event, void *data)
{
    //设置事件标记
    int filter = (event->evtype & EV_READ) ? EVFILT_READ : EVFILT_WRITE;
    int flags = (event->evtype & EV_PERSIST) ? 0 : EV_ONESHOT;
    flags |= (event->evtype & EV_CTL_ADD) ? EV_ADD : EV_ADD;
    
    return addkevent(event->reactor->multiplex.kerevid, event->fd, filter, flags);
}

cbool delkq(struct event *event, void *data)
{
    //设置事件标记
    int filter = (event->evtype & EV_READ) ? EVFILT_READ : EVFILT_WRITE;
    int flags = (event->evtype & EV_CTL_DEL) ? EV_DELETE : EV_DELETE;
    
    return addkevent(event->reactor->multiplex.kerevid, event->fd, filter, flags);
}

cbool dispatchkq(struct reactor *reactor, struct timeval *tv, void *data)
{
    struct timespec ts = {.tv_sec = (tv->tv_sec), .tv_nsec = (tv->tv_usec * 1000)};
    int actnum = kevent(reactor->multiplex.kerevid, NULL, 0, reactor->multiplex.evelist, reactor->multiplex.evelistlen, &ts);
    if (actnum == -1)
    {
        return FAILED;
    }
    
    //获取活动的用户事件
    kq_event_t *evelist = (kq_event_t *)reactor->multiplex.evelist;
    for (int i = 0; i < actnum; i++)
    {
        addactevent((int)evelist[i].ident, reactor);
    }
    
    return SUCCESS;
}

cbool destroykq(struct reactor *reactor, void *data)
{
    free(reactor->multiplex.evelist);
    return (close(reactor->multiplex.kerevid) == -1) ? FAILED : SUCCESS;
}

void setevtopkq(struct eventtop *evtop, int etindex)
{
    evtop[etindex].recreate = 0;
    evtop[etindex].name = "kqueue";
    
    evtop[etindex].create = createkq;
    evtop[etindex].add = addkq;
    evtop[etindex].del = delkq;
    evtop[etindex].dispatch = dispatchkq;
    evtop[etindex].destroy = destroykq;
}

#endif