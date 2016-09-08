#include "reactor.h"
#include "util.h"
#include "log.h"

/*全局reactor 用于信号注册*/
static struct reactor *glreactor = NULL;

static void *stopdispatch(void *event, void *arg)
{
    ((struct reactor *)arg)->listen = 0;
    
    ploginfo(LDEBUG, "stop reactor");
    
    return NULL;
}

static struct event *cpevent(struct event *sevent)
{
    struct event *devent = cnew(struct event);
    if (!devent)
    {
        return NULL;
    }
    
    memcpy(devent, sevent, sizeof(struct event));
    
    return devent;
}

static void frevent(struct event *event)
{
    cfree(event);
}

static inline int comparetime(struct heapnode *srchenode, struct heapnode *deshenode)
{
    /*0-相等 1-源<目的 -1源>目的*/
    struct timeval *src = &((struct event *)srchenode->data)->endtimer;
    struct timeval *dest = &((struct event *)deshenode->data)->endtimer;
    
    return timevalcompare(src, dest);
}

static int legsig(int sigid)
{
    return (sigid < EV_SIGNUM && sigid > 0) ? 1 : 0;
}

static void setsig(int sigid, siginfo_t *siginfo, void *data)
{
    if (glreactor == NULL)
    {
        ploginfo(LERROR, "%s->%s null", "setsig", "sigreactor");
        return;
    }
    
    //判断信号值是否合法
    if (legsig(sigid) == 0)
    {
        ploginfo(LERROR, "%s->%s is illeagal", "setsig", "legsig");
        return;
    }
    
    glreactor->usigevelist.sigid[sigid] = sigid;
    glreactor->usigevelist.sigstate = 1;
    
    if (send(glreactor->usigevelist.sockpair[0], "a", 1, 0) != 1)
    {
        //输出错误日志
        ploginfo(LERROR, "%s->%s failed", "setsig", "write");
    }
}

static void *clearsig(void *event, void *arg)
{
    assert((event != NULL));
    
    struct event *uevent = (struct event *)event;
    
    //对于来到的信号事件
    if (uevent->fd == uevent->reactor->usigevelist.sockpair[1])
    {
        //需要清空缓冲区
        const int size = 2;
        char array[size];
        recv(uevent->fd, array, size, 0);
    }
    
    return sustr;
}

/*获取最小超时时间*/
static int getminouttimer(struct reactor *reactor, struct timeval *mintime)
{
    assert((reactor != NULL && mintime != NULL));
    
    struct timeval tmintime = *mintime;
    
    //获取堆最小时间
    struct event *hemintimer = getminvalue(reactor->utimersevelist);
    if (hemintimer == NULL)
    {
        return 0;
    }
    
    time_t curtime = time(NULL);
    struct timeval endtime = hemintimer->endtimer;
    if (endtime.tv_usec > 0)
    {
        mintime->tv_usec = (pow(10, 6) - endtime.tv_usec);
        mintime->tv_sec = endtime.tv_sec - curtime - 1;
    }
    else
    {
        mintime->tv_sec = endtime.tv_sec - curtime;
    }
    
    //当最小计时器超时
    if (mintime->tv_sec < 0)
    {
        mintime->tv_sec = 0;
        mintime->tv_usec = 0;
    }
    
    if (timevalcompare(&tmintime, mintime) == 1)
    {
        memcpy(mintime, &tmintime, sizeof(struct timeval));
    }
    
    return 1;
}

/*遍历信号事件*/
static int loopsignal(struct reactor *reactor)
{
    assert ((reactor != NULL));
    
    if (reactor->usigevelist.sigstate != 1)
    {
        return 1;
    }
    
    /*重置信号状态*/
    reactor->usigevelist.sigstate = 0;
    
    int sigid = 0;
    for (int i = 1; i < EV_SIGNUM; ++i)
    {
        if ((sigid = reactor->usigevelist.sigid[i]) == 0)
        {
            continue;
        }
        
        /*恢复信号*/
        reactor->usigevelist.sigid[i] = 0;
        
        struct event *sigevent = NULL;
        forlist (reactor->usigevelist.usignalevelist)
        {
            sigevent = (struct event *)headlistnode->data;
            if (sigid == sigevent->fd)
            {
                push(reactor->uactevelist, sigevent, 0);
            }
        }
    }
    
    return 1;
}

/*遍历计时器事件*/
static int looptimer(struct reactor *reactor)
{
    assert((reactor != NULL));
    
    struct timeval curtime = {.tv_sec = time(NULL), .tv_usec = 0};
    
    int fcount = 0;
    for (int i = 0; i < getheapsize(reactor->utimersevelist); i++)
    {
        struct event *headuevent = (struct event *)getvaluebyindex(reactor->utimersevelist, i);
        if (timevalcompare(&curtime, &headuevent->endtimer) == 1)
        {
            //找到第二次大于 时候终止
            if (fcount > 1)
            {
                break;
            }
            
            fcount++;
        }
        else
        {
            push(reactor->uactevelist, headuevent, 0);
        }
    }
    
    return 1;
}

static void handle(struct reactor *reactor)
{
    //处理事件
    forlist(reactor->uactevelist)
    {
        struct event *uevent = (struct event *)headlistnode->data;
        
        if (!(uevent->evtype & EV_PERSIST))
        {
            struct event *bkevent = cpevent(uevent);
            
            //从用户中删除
            if (delevent(uevent) == 0)
            {
                ploginfo(LERROR, "handle->delevent failed");
            }
            
            frevent(uevent);
            
            uevent = bkevent;
            
            ploginfo(LDEBUG, "del event success, this event not is EV_PERSIST event");
        }
        
        //调用接口函数
        if (uevent->call(uevent, uevent->arg) == fastr)
        {
            //调用失败
            if (uevent->evtype & EV_PERSIST)
            {
                //从用户中删除
                if (delevent(uevent) == 0)
                {
                    ploginfo(LERROR, "handle->call->delevent failed");
                }
                
                if (uevent->evtype & EV_READ || uevent->evtype & EV_WRITE)
                {
                    close(uevent->fd);
                }
                
                frevent(uevent);
                
                continue;
            }
        }
        
        //定时器事件
        if (uevent->evtype & EV_TIMER)
        {
            //更新计时器
            if (uevent->evtype & EV_PERSIST)
            {
                uevent->endtimer.tv_sec = time(NULL) + uevent->watimer.tv_sec;
            }
        }
        //信号事件
        else if (uevent->evtype & EV_SIGNAL)
        {
        }
        //读事件
        else if (uevent->evtype & EV_READ || uevent->evtype & EV_WRITE)
        {
            //更新心跳
            if (uevent->evtype & EV_PERSIST)
            {
                if (upheartbeat(uevent->reactor->hbeat, uevent->fd) == 1)
                {
                    ploginfo(LDEBUG, "%s->%s success clientsock=%d", "handleevent", "upheartbeat", uevent->fd);
                }
            }
        }
        
        //对于非持久事件 需要释放copy事件
        if (!(uevent->evtype & EV_PERSIST))
        {
            frevent(uevent);
        }
    }
}

/*创建反应堆*/
struct reactor *createreactor(struct eventtop *etlist, int selevmode)
{
    assert((etlist != NULL && selevmode >= 0));
    
    struct reactor *newreactor = cnew(struct reactor);
    if (newreactor == NULL)
    {
        return NULL;
    }
    
    /*使用进程最大打开文件数目*/
    int evenumber = getmaxfilenumber();
    ploginfo(LDEBUG, "%s->%s maxfileno=%d", "createreactor", "getmaxfilenumber", evenumber);
    if (evenumber <= 0)
    {
        evenumber = 256;
    }
    
    /*保存最大连接数*/
    newreactor->maxconnnum = evenumber;
    
    /*保存当前选择的模型和系统的模型列表*/
    newreactor->selmode.selevmode = selevmode;/*选择的事件模型*/
    newreactor->selmode.selevtop = &etlist[selevmode];
    newreactor->selmode.etlist = etlist;
    ploginfo(LDEBUG, "select event model is %s", newreactor->selmode.selevtop->name);
    
    /*初始化心跳管理*/
    newreactor->hbeat = createheartbeat(evenumber, 3);
    if (!newreactor->hbeat)
    {
        return NULL;
    }
    newreactor->hbeat->reactor = newreactor;
    
    if (newreactor->selmode.selevtop->create(newreactor, &evenumber) == 0)
    {
        return NULL;
    }
    
    /*初始化hash表 链地址法处理冲突*/
    newreactor->uevelist = createhashtable(evenumber);
    if (!newreactor->uevelist)
    {
        return NULL;
    }
    
    pthread_mutexattr_t mutexattr;
    if (pthread_mutexattr_init(&mutexattr) != 0 ||
        pthread_mutex_init(&newreactor->reactormutex, &mutexattr) != 0)
    {
        return NULL;
    }
    
    /*活动链表*/
    if ((newreactor->uactevelist = createlist(0, 0, NULL)) == NULL)
    {
        return NULL;
    }
    
    /*计时器链表*/
    newreactor->utimersevelist = createminheap(100, comparetime);
    if (newreactor->utimersevelist == NULL)
    {
        return NULL;
    }
    
    /*信号链表*/
    if ((newreactor->usigevelist.usignalevelist = createlist(0, 0, NULL)) == NULL)
    {
        return NULL;
    }
    
    /*开启监听默认时间*/
    newreactor->defaulttime.tv_sec = 1;
    newreactor->defaulttime.tv_usec = 0;
    
    /*初始化信号*/
    newreactor->usigevelist.sigstate = 0;
    memset(newreactor->usigevelist.sigid, 0, sizeof(newreactor->usigevelist.sigid));
    
    //创建sock对 并注册读事件
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, newreactor->usigevelist.sockpair) == -1)
    {
        return NULL;
    }
    
    struct event *uevent = setevent(newreactor, newreactor->usigevelist.sockpair[1],
                             EV_READ | EV_PERSIST, clearsig, NULL);
    if (uevent == NULL)
    {
        return NULL;
    }
    
    if (addevent(uevent, 0) == 0)
    {
        return NULL;
    }
    
    //添加关闭服务器信号处理
    uevent = setsignal(newreactor, SIGINT, EV_SIGNAL, stopdispatch, newreactor);
    if (uevent == NULL)
    {
        return NULL;
    }
    
    if (addsignal(uevent) == 0)
    {
        return NULL;
    }
    
    /*开启监听事件*/
    newreactor->listen = 1;
    
    return newreactor;
}

/*获取事件*/
struct event *getevent(int fd, struct reactor *reactor)
{
    assert((reactor != NULL && fd >= 0));
    
    //获取事件
    return getitemvaluebyid(reactor->uevelist, fd);
}

/*设置事件*/
struct event *setevent(struct reactor *reactor, int fd, int evtype, callback call, void *arg)
{
    assert((reactor != NULL));
    
    //创建一个事件
    struct event *newevent = cnew(struct event);
    if (newevent == NULL)
    {
        return NULL;
    }
    memset(newevent, 0, sizeof(struct event));
    
    newevent->fd = fd;
    newevent->evtype = evtype;
    newevent->call = call;
    newevent->arg = arg;
    newevent->reactor = reactor;
    newevent->buf = NULL;
    
    return newevent;
}

/*设置信号*/
struct event *setsignal(struct reactor *reactor, int fd, int evtype, callback call, void *arg)
{
    return setevent(reactor, fd, evtype, call, arg);
}

/*设置定时器*/
struct event *settimer(struct reactor *reactor, int evtype, callback call, void *arg)
{
    return setevent(reactor, -1, evtype, call, arg);
}

/*添加事件*/
int addevent(struct event *uevent, int hbman)
{
    assert((uevent != NULL));
    
    ploginfo(LDEBUG, "add sock event sid=%d", uevent->fd);
    
    if (uevent->evtype & EV_READ || uevent->evtype & EV_WRITE)
    {
        /*删除之前重复注册过的事件*/
        struct event *oldevent = getevent(uevent->fd, uevent->reactor);
        if (oldevent)
        {
            if (delevent(oldevent) == 1)
            {
                ploginfo(LDEBUG, "del exist event success clientsock=%d", uevent->fd);
            }
        }
        
        //加入到系统列表
        if (uevent->reactor->selmode.selevtop->add(uevent, NULL) == 0)
        {
            return 0;
        }
        
        //当加入的事件需要心跳管理
        if (hbman > 0)
        {
            if (addheartbeat(uevent->reactor->hbeat, uevent->fd) != 1)
            {
                ploginfo(LERROR, "%s->%s failed clientsock=%d", "acceptconn", "addheartbeat", uevent->fd);
            }
        }
        
        //加入到用户事件列表
        return setitembyid(uevent->reactor->uevelist, uevent->fd, uevent);
    }
    
    return 0;
}

/*添加信号事件*/
int addsignal(struct event *uevent)
{
    assert((uevent != NULL));
    
    ploginfo(LDEBUG, "add sig event sid=%d", uevent->fd);
    
    /*保存此信号的来源*/
    glreactor = uevent->reactor;
    
    if (uevent->evtype & EV_SIGNAL)
    {
        /*注册信号*/
        struct sigaction usignal;
        sigemptyset(&usignal.sa_mask);
        usignal.sa_sigaction = setsig;
        usignal.sa_flags = SA_SIGINFO | SA_RESTART;
        
        uevent->oldsiga = cnew(struct sigaction);
        if (uevent->oldsiga == NULL)
        {
            return 0;
        }
        
        if (sigaction(uevent->fd, &usignal, uevent->oldsiga) == -1)
        {
            return 0;
        }
        
        //加入到用户事件列表
        return push(uevent->reactor->usigevelist.usignalevelist, uevent, 0);
    }

    return 0;
}

/*添加计时器事件*/
int addtimer(struct event *uevent, struct timeval *timer)
{
    assert((uevent != NULL));
    
    ploginfo(LDEBUG, "add timer event tid=-1");
    
    if (uevent->evtype & EV_TIMER)
    {
        assert((timer != NULL));
        
        //保存计时器等待时间
        memcpy(&uevent->watimer, timer, sizeof(struct timeval));
        
        //保存计时器结束时间
        memcpy(&uevent->endtimer, timer, sizeof(struct timeval));
        uevent->endtimer.tv_sec += time(NULL);
        
        //加入到最小堆
        return addhn(uevent->reactor->utimersevelist, uevent);
    }

    return 0;
}

/*删除事件*/
int delevent(struct event *uevent)
{
    assert((uevent != NULL));
    
    int ret = 0;
    
    if (uevent->evtype & EV_SIGNAL)
    {
        //设置回以前的信号处理
        if (sigaction(uevent->fd, uevent->oldsiga, NULL) == -1)
        {
            ploginfo(LERROR, "%s->%s sigid %d failed", "freeevent", "sigaction", uevent->fd);
        }
        
        cfree(uevent->oldsiga);
        uevent->oldsiga = NULL;
        
        forlist(uevent->reactor->usigevelist.usignalevelist)
        {
            if (headlistnode->data == uevent)
            {
                ret = delnode(uevent->reactor->usigevelist.usignalevelist, headlistnode);
                break;
            }
        }
    }
    else if (uevent->evtype & EV_TIMER)
    {
        ret = delhn(uevent->reactor->utimersevelist, uevent);
    }
    else if (uevent->evtype & EV_READ || uevent->evtype & EV_WRITE)
    {
        //非持久事件自动从系统中删除
        if (uevent->reactor->selmode.selevtop->del(uevent, NULL) == 0)
        {
            ploginfo(LERROR, "%s->%s success clientsock=%d", "delevent", "del", uevent->fd);
        }
        
        if (delheartbeat(uevent->reactor->hbeat, uevent->fd) == 1)
        {
            ploginfo(LDEBUG, "%s->%s success clientsock=%d", "delevent", "delheartbeat", uevent->fd);
        }

        ret = delitembyid(uevent->reactor->uevelist, uevent->fd);
    }
    
    return ret;
}

/*添加活动事件*/
int addactevent(int fd, struct reactor *reactor)
{
    struct event *hashuevent = getevent(fd, reactor);
    if (hashuevent != NULL)
    {
        return push(reactor->uactevelist, hashuevent, 0);
    }
    
    return 0;
}

/*分发消息*/
int dispatchevent(struct reactor *reactor)
{
    assert ((reactor != NULL));
    
    while (reactor->listen)
    {
        //清空激活链表
        clearlist(reactor->uactevelist);
        
        //设置超时时间
        struct timeval mintime = reactor->defaulttime;
        getminouttimer(reactor, &mintime);
    
        //获取活动读写事件
        if (reactor->selmode.selevtop->dispatch(reactor, &mintime, NULL) == 0)
        {
            //对于信号中断不做处理
            if (errno != EINTR)
            {
                ploginfo(LERROR, "%s->%s failed errno:%d", "dispatchevent", "dispatch", errno);
                return 0;
            }
        }
        
        //获取定时器超时事件
        looptimer(reactor);
        
        //获取信号事件
        loopsignal(reactor);
        
        //处理活动事件
        handle(reactor);
    }
    
    return 1;
}

/*销毁反应堆*/
int destroyreactor(struct reactor *reactor)
{
    assert((reactor != NULL));
    
    //释放sock事件
    struct event *uevent = NULL;
    struct htnode *htnode = NULL;
    struct htnode *htnextnode = NULL;
    for (int i = 0; i < reactor->uevelist->tablelen; i++)
    {
        htnode = reactor->uevelist->hashtable[i];
        while (htnode)
        {
            uevent = (struct event *)htnode->item;
            htnextnode = htnode->next;
            if (close(uevent->fd) == -1)
            {
                ploginfo(LERROR, "%s->%s sock=%d failed", "destroyreactor", "close", uevent->fd);
            }
            
            if (delevent(uevent) == 0)
            {
                ploginfo(LERROR, "%s->%s sock=%d failed", "destroyreactor", "delevent", uevent->fd);
            }
            
            frevent(uevent);
            
            htnode = htnextnode;
        }
    }
    if (!destroyhashtable(reactor->uevelist))
    {
        ploginfo(LERROR, "%s->%s failed", "destroyreactor", "destroyhashtable");
    }
    
    //释放计时器事件
    for (int i = 0; i < getheapsize(reactor->utimersevelist); i++)
    {
        uevent = getvaluebyindex(reactor->utimersevelist, i);
        if (delevent(uevent) == 0)
        {
            ploginfo(LERROR, "%s->%s sock=%d failed", "destroyreactor", "delevent", uevent->fd);
        }
        
        frevent(uevent);
    }
    if (!destroyminheap(reactor->utimersevelist))
    {
        ploginfo(LERROR, "%s->%s failed", "destroyreactor", "destroyminheap");
    }
    
    //释放信号事件
    while (!empty(reactor->usigevelist.usignalevelist))
    {
        uevent = gethead(reactor->usigevelist.usignalevelist)->data;
        if (delevent(uevent) == 0)
        {
            ploginfo(LERROR, "%s->%s sock=%d failed", "destroyreactor", "delevent", uevent->fd);
        }
        
        frevent(uevent);
    }
    destroylist(reactor->usigevelist.usignalevelist);
    
    destroyheartbeat(reactor->hbeat);
    destroylist(reactor->uactevelist);
    
    //关闭sock对
    close(reactor->usigevelist.sockpair[0]);
    
    pthread_mutex_destroy(&reactor->reactormutex);
    
    reactor->selmode.selevtop->destroy(reactor, NULL);
    
    cfree(reactor);
    
    return 1;
}
