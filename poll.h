#ifndef POLL_H
#define POLL_H

#include "evself.h"
#include "reactor.h"
#include <poll.h>

typedef struct pollfd po_event_t;

cbool createpo(struct reactor *reactor, void *data)
{
    reactor->pol.evelistlen = reactor->maxconnnum + 1;
    reactor->pol.evelist = (void *)malloc(sizeof(po_event_t) * reactor->pol.evelistlen);
    return (reactor->pol.evelist == NULL) ? FAILED : SUCCESS;
}

static int findpolfd(struct event *event, po_event_t *evelist, int fd)
{
    int index = event->fd % event->reactor->pol.evelistlen;
    
    //以hash表的形式查找
    if (evelist[index].fd == fd)
    {
        return index;
    }
    
    //没找到 直接遍历所有
    for (int i = 0; i < event->reactor->pol.evelistlen; i++)
    {
        if (evelist[i].fd == fd)
        {
            return i;
        }
    }
    
    return -1;
}

cbool addpo(struct event *event, void *data)
{
    po_event_t *evelist = (po_event_t *)event->reactor->pol.evelist;
    int findex = findpolfd(event, evelist, -1);
    if (findex >= 0)
    {
        evelist[findex].fd = event->fd;
        evelist[findex].events =
        (event->evtype & EV_READ) ? POLLIN :
        (event->evtype & EV_WRITE) ? POLLOUT :
        (event->evtype & EV_RWERROR) ? POLLERR :
        POLLERR;
        
        return SUCCESS;
    }
    
    return FAILED;
}

cbool delpo(struct event *event, void *data)
{
    po_event_t *evelist = (po_event_t *)event->reactor->pol.evelist;
    int findex = findpolfd(event, evelist, event->fd);
    if (findex >= 0)
    {
        evelist[findex].fd = -1;
        return SUCCESS;
    }
    
    return FAILED;
}

cbool dispatchpo(struct reactor *reactor, struct timeval *tv, void *data)
{
    int watime = (int)(tv->tv_sec * 1000 + tv->tv_usec / 1000);
    int actnum = poll(reactor->pol.evelist, reactor->pol.evelistlen, watime);
    if (actnum == -1)
    {
        return FAILED;
    }
    
    if (actnum == 0)
    {
        return SUCCESS;
    }
    
    po_event_t *event = (po_event_t *)reactor->pol.evelist;
    for (int i = 0; i < reactor->pol.evelistlen; i++)
    {
        if (event[i].revents & POLLIN || event[i].revents & POLLOUT)
        {
            addactevent(event[i].fd, reactor);
        }
    }
    
    return SUCCESS;
}

cbool destroypo(struct reactor *reactor, void *data)
{
    free(reactor->pol.evelist);
    return SUCCESS;
}

void setevtoppo(struct eventtop *evtop, int etindex)
{
    evtop[etindex].recreate = 0;
    evtop[etindex].name = "poll";
    
    evtop[etindex].create = createpo;
    evtop[etindex].add = addpo;
    evtop[etindex].del = delpo;
    evtop[etindex].dispatch = dispatchpo;
    evtop[etindex].destroy = destroypo;
}

#endif 