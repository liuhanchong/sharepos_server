#ifndef KQUEUE_H
#define KQUEUE_H

#include "evself.h"
#include <sys/event.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kevent kq_event_t;

static int addkevent(int kerevid, int fd, int filter, int flags)
{
    //设置事件
    kq_event_t addkevent;
    EV_SET(&addkevent, fd, filter, flags, 0, 0, NULL);
    
    //添加事件
    return (kevent(kerevid, &addkevent, 1, NULL, 0, NULL) == -1) ? 0 : 1;
}

int createkq(struct reactor *reactor, void *data)
{
    reactor->multiplex.kerevid = kqueue();
    if (reactor->multiplex.kerevid == -1)
    {
        return 0;
    }
    
    reactor->multiplex.evelistlen = *((int *)data);
    reactor->multiplex.evelist = (void *)cmalloc(sizeof(kq_event_t) * reactor->multiplex.evelistlen);
    return (reactor->multiplex.evelist == NULL) ? 0 : 1;
}

int addkq(struct event *event, void *data)
{
    //设置事件标记
    int filter = (event->evtype & EV_READ) ? EVFILT_READ : EVFILT_WRITE;
    int flags = (event->evtype & EV_PERSIST) ? 0 : 0;//EV_ONESHOT 不使用 不使用系统自动删除 会在delevent删除时候操作
    flags |= (event->evtype & EV_CTL_ADD) ? EV_ADD : EV_ADD;
    
    return addkevent(event->reactor->multiplex.kerevid, event->fd, filter, flags);
}

int delkq(struct event *event, void *data)
{
    //设置事件标记
    int filter = (event->evtype & EV_READ) ? EVFILT_READ : EVFILT_WRITE;
    int flags = (event->evtype & EV_CTL_DEL) ? EV_DELETE : EV_DELETE;
    
    return addkevent(event->reactor->multiplex.kerevid, event->fd, filter, flags);
}

int dispatchkq(struct reactor *reactor, struct timeval *tv, void *data)
{
    struct timespec ts = {.tv_sec = (tv->tv_sec), .tv_nsec = (tv->tv_usec * 1000)};
    int actnum = kevent(reactor->multiplex.kerevid, NULL, 0, reactor->multiplex.evelist, reactor->multiplex.evelistlen, &ts);
    if (actnum == -1)
    {
        return 0;
    }
    
    //获取活动的用户事件
    kq_event_t *evelist = (kq_event_t *)reactor->multiplex.evelist;
    for (int i = 0; i < actnum; i++)
    {
        addactevent((int)evelist[i].ident, reactor);
    }
    
    return 1;
}

int destroykq(struct reactor *reactor, void *data)
{
    cfree(reactor->multiplex.evelist);
    return (close(reactor->multiplex.kerevid) == -1) ? 0 : 1;
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
    
#ifdef __cplusplus
}
#endif

#endif