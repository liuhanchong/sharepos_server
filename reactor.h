#ifndef REACTOR_H
#define REACTOR_H

#include "evself.h"

#ifdef __cplusplus
extern "C" {
#endif

/*创建反应堆*/
struct reactor *createreactor(struct eventtop *etlist, int selevmode);

/*设置事件*/
struct event *setevent(struct reactor *reactor, int fd, int evtype, callback call, void *arg);

/*设置信号*/
struct event *setsignal(struct reactor *reactor, int fd, int evtype, callback call, void *arg);

/*设置定时器*/
struct event *settimer(struct reactor *reactor, int evtype, callback call, void *arg);

/*添加事件*/
int addevent(struct event *uevent, int hbman);

/*添加信号事件*/
int addsignal(struct event *uevent);

/*添加计时器事件*/
int addtimer(struct event *uevent, struct timeval *timer);

/*分发消息*/
int dispatchevent(struct reactor *reactor);

/*销毁反应堆*/
int destroyreactor(struct reactor *reactor);
    
#ifdef __cplusplus
}
#endif

#endif