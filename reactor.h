#ifndef REACTOR_H
#define REACTOR_H

#include "evself.h"

/*创建反应堆*/
struct reactor *createreactor(struct eventtop *etlist, int selevmode);

/*获取事件*/
struct event *getevent(int fd, struct reactor *reactor);

/*设置事件*/
struct event *setevent(struct reactor *reactor, int fd, int evtype, callback call, void *arg);

/*设置信号*/
struct event *setsignal(struct reactor *reactor, int fd, int evtype, callback call, void *arg);

/*设置定时器*/
struct event *settimer(struct reactor *reactor, int evtype, callback call, void *arg);

/*添加事件*/
cbool addevent(struct event *uevent, int hbman);

/*添加信号事件*/
cbool addsignal(struct event *uevent);

/*添加计时器事件*/
cbool addtimer(struct event *uevent, struct timeval *timer);

/*删除事件*/
cbool delevent(struct event *uevent);

/*添加活动事件*/
cbool addactevent(int fd, struct reactor *reactor);

/*分发消息*/
cbool dispatchevent(struct reactor *reactor);

/*销毁反应堆*/
cbool destroyreactor(struct reactor *reactor);

#endif