#ifndef EVBASE_H
#define EVBASE_H

#include "util.h"
#include "list.h"
#include "htable.h"
#include "minheap.h"
#include "hbeat.h"
#include <time.h>
#include <signal.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <stdio.h>

/*反应堆支持的事件类型*/
#define EV_TIMER 0x01 //定时器事件
#define EV_READ 0x02    //读取事件
#define EV_WRITE 0x04   //写入事件
#define EV_SIGNAL 0x08  //信号事件
#define EV_PERSIST 0x10 //持久事件
#define EV_ET 0x20 //边缘触发模式
#define EV_CTL_ADD 0x40 //添加读取事件
#define EV_CTL_MOD 0x80 //修改读取事件
#define EV_CTL_DEL 0x100 //删除读取事件
#define EV_RWERROR 0x200   //读写事件错误
#define EV_READBUF  1024 /*网络读字节缓冲区*/
#define EV_WRITEBUF 1024 /*网络写字节缓冲区*/
#define EV_SIGNUM 256 /*系统信号数量*/

typedef void *(*callback)(void *, void *);

typedef struct signalevent
{
    int sigid[EV_SIGNUM];/*注册信号类型*/
    int sigstate;/*注册事件状态 0-未触发 1-触发*/
    list *usignalevelist;/*用户注册的信号事件列表*/
    int sockpair[2];/*sockpair对，用于信号触发*/
} signalevent;

typedef struct selmode
{
    int selevmode;/*选择的事件模型*/
    struct eventtop *selevtop;/*保存选择的eventtop*/
    struct eventtop *etlist;/*保存eventtop数组*/
} selmode;

typedef struct selset
{
    void *readfds;
    void *writefds;
    void *errorfds;
} selset;

typedef struct multiplex
{
    int kerevid;/*保存内核事件id kqueue epoll_create 返回值*/
    void *evelist;/*系统内核事件列表 xx_event_t类型*/
    int evelistlen;/*系统内核事件列表长度*/
} multiplex;

typedef struct pol
{
    void *evelist;/*系统内核事件列表 xx_event_t类型*/
    int evelistlen;/*系统内核事件列表长度*/
} pol;

typedef struct reactor
{
    pthread_mutex_t reactormutex;/*反应堆锁*/
    hashtable *uevelist;/*用户注册读写事件hash列表*/
    list *uactevelist;/*用户注册的活动事件列表*/
    minheap *utimersevelist;/*用户注册的计时器列表*/
    signalevent usigevelist;/*用户注册的信号事件列表*/
    int listen;/*是否监听事件*/
    int maxconnnum;/*保存最大的连接数*/
    heartbeat *hbeat;/*心跳管理*/
    struct timeval defaulttime;/*默认监听超时*/
    struct selmode selmode;/*保存模型信息*/
    struct selset selset;/*保存select中需要的结构体信息*/
    struct multiplex multiplex;/*复用技术 epoll kqueue*/
    struct pol pol;/*poll信息*/
} reactor;

typedef struct event
{
    int fd;/*计时器:-1 信号:信号值 IO:文件描述符*/
    int evtype;/*读写、计时器、信号*/
    callback call;
    void *arg;
    reactor *reactor;/*所属反应器*/
    struct timeval endtimer;/*保存定时器结束时间*/
    struct timeval watimer;/*保存定时间隔*/
    struct sigaction *oldsiga;/*保存设置前信号处理*/
    struct event *next;
} event;

typedef struct eventtop
{
    int recreate;/*重新创建*/
    const char *name;/*模型名称*/
    cbool (*create)(struct reactor *, void *);
    cbool (*add)(struct event *, void *);
    cbool (*del)(struct event *, void *);
    cbool (*dispatch)(struct reactor *, struct timeval *, void *);
    cbool (*destroy)(struct reactor *, void *);
} eventtop;

#endif
