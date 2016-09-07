#ifndef SELECT_H
#define SELECT_H

#include "evself.h"
#include "reactor.h"
#include <sys/select.h>

typedef struct fd_set se_event_t;

cbool createse(struct reactor *reactor, void *data)
{
    reactor->selset.readfds = (void *)malloc(sizeof(se_event_t));
    if (reactor->selset.readfds == NULL)
    {
        return FAILED;
    }

    reactor->selset.writefds = (void *)malloc(sizeof(se_event_t));
    if (reactor->selset.writefds == NULL)
    {
        return FAILED;
    }

    reactor->selset.errorfds = (void *)malloc(sizeof(se_event_t));
    if (reactor->selset.errorfds == NULL)
    {
        return FAILED;
    }
    
    FD_ZERO(reactor->selset.readfds);
    FD_ZERO(reactor->selset.writefds);
    FD_ZERO(reactor->selset.errorfds);
    
    reactor->selset.maxfd = -1;

    return SUCCESS;
}

cbool addse(struct event *event, void *data)
{
    se_event_t *fdset =
    (event->evtype & EV_READ) ? event->reactor->selset.readfds :
    (event->evtype & EV_WRITE) ? event->reactor->selset.writefds :
    (event->evtype & EV_RWERROR) ? event->reactor->selset.errorfds :
                                        event->reactor->selset.errorfds;
    
    if (FD_ISSET(event->fd, fdset) == 0)
    {
        FD_SET(event->fd, fdset);
    }
    
    //保存最大的连接描述符
    if (event->fd > event->reactor->selset.maxfd)
    {
        event->reactor->selset.maxfd = event->fd;
    }
    
    return SUCCESS;
}

cbool delse(struct event *event, void *data)
{
    se_event_t *fdset =
    (event->evtype & EV_READ) ? event->reactor->selset.readfds :
    (event->evtype & EV_WRITE) ? event->reactor->selset.writefds :
    (event->evtype & EV_RWERROR) ? event->reactor->selset.errorfds :
    event->reactor->selset.errorfds;
    
    FD_CLR(event->fd, fdset);
    
    return SUCCESS;
}

cbool dispatchse(struct reactor *reactor, struct timeval *tv, void *data)
{
    se_event_t rset;
    se_event_t wset;
    se_event_t eset;
    memcpy(&rset, reactor->selset.readfds, sizeof(se_event_t));
    memcpy(&wset, reactor->selset.writefds, sizeof(se_event_t));
    memcpy(&eset, reactor->selset.errorfds, sizeof(se_event_t));
    
    int actnum = select(reactor->selset.maxfd + 1, &rset, &wset, &eset, tv);
    if (actnum == -1)
    {
        return FAILED;
    }
    
    //没有活动的描述符
    if (actnum == 0)
    {
        return SUCCESS;
    }
    
    for (int i = 0; i <= reactor->selset.maxfd; ++i)
    {
        if (FD_ISSET(i, &rset) || FD_ISSET(i, &wset))
        {
            addactevent(i, reactor);
        }
    }
    
    return SUCCESS;
}

cbool destroyse(struct reactor *reactor, void *data)
{
    free(reactor->selset.readfds);
    free(reactor->selset.writefds);
    free(reactor->selset.errorfds);
    return SUCCESS;
}

void setevtopse(struct eventtop *evtop, int etindex)
{
    evtop[etindex].recreate = 0;
    evtop[etindex].name = "select";
    
    evtop[etindex].create = createse;
    evtop[etindex].add = addse;
    evtop[etindex].del = delse;
    evtop[etindex].dispatch = dispatchse;
    evtop[etindex].destroy = destroyse;
}

#endif
