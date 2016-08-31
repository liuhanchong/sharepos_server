#ifndef SELECT_H
#define SELECT_H

#include "evself.h"
#include "reactor.h"
#include <sys/select.h>

typedef struct fd_set se_event_t;

cbool createse(struct reactor *reactor, void *data)
{
    reactor->selset.readfds = (void *)malloc(sizeof(se_event_t));
    if (reactor->selset.readfds)
    {
        return FAILED;
    }

    reactor->selset.writefds = (void *)malloc(sizeof(se_event_t));
    if (reactor->selset.readfds)
    {
        return FAILED;
    }

    reactor->selset.errorfds = (void *)malloc(sizeof(se_event_t));
    if (reactor->selset.readfds)
    {
        return FAILED;
    }
    
    FD_ZERO(reactor->selset.readfds);
    FD_ZERO(reactor->selset.writefds);
    FD_ZERO(reactor->selset.errorfds);

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
    FD_COPY(reactor->selset.readfds, &rset);
    FD_COPY(reactor->selset.writefds, &wset);
    FD_COPY(reactor->selset.errorfds, &eset);
    
    int actnum = select(reactor->maxconnnum + 1, &rset, &wset, &eset, tv);
    if (actnum == -1)
    {
        return FAILED;
    }
    
    //没有活动的描述符
    if (actnum == 0)
    {
        return SUCCESS;
    }
    
    for (int i = 0; i <= reactor->maxconnnum; ++i)
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
