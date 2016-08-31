#include "util.h"
#include "pyinter.h"
#include "event.h"
#include <stdio.h>

typedef  struct share
{
    void *server;
} share;

typedef struct spserver
{
    struct sysc sysc;
    struct slog *log;
    struct reactor *reactor;
    struct eventtop etlist[SYSEVMODENUM]; /*按照优先级保存模型*/
    int serverfd;/*服务器套接字*/
    int shareid;/*共享内存id*/
    void *shmem;/*共享的内存*/
} spserver;

/*全局服务器配置*/
struct spserver server;

void *readwrite(void *event, void *arg)
{
    struct event *uevent = (struct event *)event;
    if (uevent == NULL)
    {
        return NULL;
    }
    
    //读取数据
    if (uevent->evtype & EV_READ)
    {
        /*接收信息*/
        int recvlen = (int)recv(uevent->fd, uevent->readbuf, EV_READBUF - 1, 0);
        if (recvlen <= 0)
        {
            if (errno == EINTR)
            {
            }
            
            ploginfo(LERROR, "%s->%s failed sockid=%d errno=%d", "readwrite", "recv", uevent->fd, errno);
            
            return NULL;
        }
        
        uevent->readbufsize = recvlen;
        uevent->readbuf[recvlen] = '\0';
        
        ploginfo(LDEBUG, "----------rdata=%s", uevent->readbuf);
        
        //将客户端套接字注册事件
        struct event *urevent = setevent(uevent->reactor, uevent->fd, EV_WRITE,
                                         readwrite, NULL);
        if (urevent == NULL)
        {
            ploginfo(LERROR, "%s->%s failed clientsock=%d", "readwrite", "setevent", uevent->fd);
            return NULL;
        }
        
        strncpy((char *)urevent->writebuf, "wo shi server", 13);
        urevent->writebufsize = 13;
        urevent->writebuf[urevent->writebufsize] = '\0';
        
        if (addevent(urevent) == FAILED)
        {
            ploginfo(LERROR, "%s->%s failed clientsock=%d", "readwrite", "addevent", urevent->fd);
            return NULL;
        }
    }
    else if (uevent->evtype & EV_WRITE)
    {
        /*发送信息*/
        int sendlen = (int)send(uevent->fd, uevent->writebuf, uevent->writebufsize, 0);
        if (sendlen <= 0)
        {
            if (errno == EINTR)
            {
            }
            
            ploginfo(LERROR, "%s->%s failed sockid=%d errno=%d", "readwrite", "send", uevent->fd, errno);
            
            return NULL;
        }
        
        ploginfo(LDEBUG, "----------sdata=%s", uevent->writebuf);
        
        //将客户端套接字注册事件
        struct event *uwevent = setevent(uevent->reactor, uevent->fd, EV_READ, readwrite, NULL);
        if (uwevent == NULL)
        {
            ploginfo(LERROR, "%s->%s failed clientsock=%d", "readwrite", "setevent_2", uevent->fd);
            return NULL;
        }
        
        if (addevent(uwevent) == FAILED)
        {
            ploginfo(LERROR, "%s->%s failed clientsock=%d", "readwrite", "addevent_2", uevent->fd);
            return NULL;
        }
    }
    
    return NULL;
}

void *acceptconn(void *uev, void *data)
{
    struct spserver *server = (struct spserver *)data;
    
    struct sockaddr_in clisockaddr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    int clientsock = acceptsock(server->serverfd, &clisockaddr, &addrlen);
    if (clientsock < 0)
    {
        ploginfo(LERROR, "%s->%s failed clientsock=%d, errno=%d", "acceptconn", "accept", clientsock, errno);
        return NULL;
    }
    
    //将客户端socket设置为non_blocked
    setnoblock(clientsock);
    
    //将客户端套接字注册事件
    struct event *uevent = setevent(server->reactor, clientsock, EV_READ, readwrite, NULL);
    if (uevent == NULL)
    {
        ploginfo(LERROR, "%s->%s failed clientsock=%d", "acceptconn", "setevent", clientsock);
        return NULL;
    }
    
    if (addevent(uevent) == FAILED)
    {
        ploginfo(LERROR, "%s->%s failed clientsock=%d", "acceptconn", "addevent", clientsock);
        return NULL;
    }
    
    ploginfo(LDEBUG, "%s->%s sockid=%d success", "acceptconn", "accept", clientsock);
    
    return NULL;
}

int openserver(const char *ip, const int port, const int backlog)
{
    int domain = AF_INET;//AF_INET6 目前没有使用IPV6
    int socktype = SOCK_STREAM;
    int protocol = 0;
    
    int sockid = sock(domain, socktype, protocol);
    if (sockid < 0)
    {
        ploginfo(LERROR, "openserver->socket failed");
        return -1;
    }
    
    int flags = 1;
    struct linger linger = {0, 0};
    setsocketopt(sockid, SOL_SOCKET, SO_KEEPALIVE, (void *)&flags, sizeof(flags));
    setsocketopt(sockid, SOL_SOCKET, SO_LINGER, (void *)&linger, sizeof(linger));
    setsocketopt(sockid, SOL_SOCKET, SO_REUSEADDR, (void *)&flags, sizeof(flags));
//	setsocketopt(sockid, IPPROTO_IP, TCP_NODELAY, (void *)&flags, sizeof(flags));
    
    struct sockaddr_in sockaddr;
    memset(&sockaddr, 0, sizeof(struct sockaddr_in));
    setsockaddrin(&sockaddr, domain, port, ip);
    if (bindsock(sockid, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr_in)) != 0)
    {
        ploginfo(LERROR, "openserver->bindsock failed %d", errno);
        return -1;
    }
    
    if (listensock(sockid, backlog) == -1)
    {
        ploginfo(LERROR, "openserver->listensock failed");
        return -1;
    }
    
    return sockid;
}

void *eventcallback(void *event, void *arg)
{
    ploginfo(LDEBUG, "eventcallback %d", *((int *)arg));
    
    return NULL;
}

void *timercallback(void *event, void *arg)
{
    ploginfo(LDEBUG, "timercallback %d", *((int *)arg));
    
    return NULL;
}

void *signalalam(void *event, void *arg)
{
    ploginfo(LDEBUG, "signalalam");
    
    return NULL;
}

void *closesys(void *event, void *arg)
{
    ploginfo(LDEBUG, "closesys");
    
    struct reactor *reactor = (struct reactor *)arg;
    reactor->listen = 0;
    
    return NULL;
}

int main(int argc, const char *argv[])
{
    memset(&server, 0, sizeof(spserver));
    
    if (argc == 2 && (strcmp(argv[1], "stop") == 0))
    {
        int pid = getpidfromfile();
        if (pid == -1)
        {
            printf("%s\n", "close pro failed, invalid pid!");
            return 0;
        }
        
        if (kill(pid, SIGINT) == -1)
        {
            printf("close pro failed, pid=%d!\n", pid);
            return 0;
        }
        
        printf("close pro success, pid=%d!\n", pid);
        
        return 0;
    }
    
    printf("the process id is %d!\n", getpid());
    
    setpidtofile();

    if (!(server.log = createlog()))
    {
        printf("create log failed\n");
        return 1;
    }

    if (getsyscon("./server.ini", &server.sysc) == SUCCESS)
    {
        ploginfo(LDEBUG, "ip=%s port=%d", server.sysc.ip, server.sysc.port);
    }
    
    /*创建事件模型*/
    createeventtop(server.etlist);
    
    if ((server.reactor = createreactor(server.etlist, 0)) == NULL)
    {
        ploginfo(LDEBUG, "main->createreactor failed");
        return 1;
    }
    
    ploginfo(LDEBUG, "main->createreactor succeess");
    
    server.serverfd = openserver((char *)server.sysc.ip, server.sysc.port, 10);
    if (server.serverfd < 0)
    {
        ploginfo(LERROR, "main->cretcpser failed");
        return 1;
    }
    ploginfo(LDEBUG, "main->cretcpser success serid=%d", server.serverfd);
    
    //添加事件
    event *uevent = setevent(server.reactor, server.serverfd, EV_READ | EV_PERSIST, acceptconn, &server);
    if (uevent == NULL)
    {
        ploginfo(LERROR, "%s->%s failed sersock=%d", "main", "setevent", server.serverfd);
        return 1;
    }

    if (addevent(uevent) == FAILED)
    {
        ploginfo(LDEBUG, "%s->%s failed sersock=%d", "main", "addevent", server.serverfd);
    }

    int i= 100;
    struct timeval timer = {1, 0};
    
    for (int k = 1; k <= 100; k++)
    {
        uevent = settimer(server.reactor, EV_TIMER, timercallback, &i);
        if (addtimer(uevent, &timer) == SUCCESS)
        {
            ploginfo(LDEBUG, "%s", "addtimer ok");
        }
    }
    
    int jarray[200] = {0};
    for (int k = 1; k <= 200; k++)
    {
        jarray[k - 1] = k;
        struct timeval timer2 = {1 + k / 50, 10000 * k};
        uevent = settimer(server.reactor, EV_TIMER | EV_PERSIST, timercallback, &jarray[k - 1]);
        if (addtimer(uevent, &timer2) == SUCCESS)
        {
            ploginfo(LDEBUG, "%s", "addtimer ok");
        }
    }

    uevent = setsignal(server.reactor, SIGALRM, EV_SIGNAL | EV_PERSIST, signalalam, NULL);
    if (addsignal(uevent) == SUCCESS)
    {
        ploginfo(LDEBUG, "%s", "addsignal ok");
    }
    
    //添加关闭服务器信号处理
    uevent = setsignal(server.reactor, SIGINT, EV_SIGNAL, closesys, server.reactor);
    if (addsignal(uevent) == SUCCESS)
    {
        ploginfo(LDEBUG, "%s", "addsignal ok");
    }
    
    //服务器主体逻辑
    if (dispatchevent(server.reactor) == FAILED)
    {
        ploginfo(LDEBUG, "main->dispatchevent failed");
        return 1;
    }
    
    ploginfo(LDEBUG, "main->dispatchevent succeess");
    
    if (!destroyreactor(server.reactor))
    {
        ploginfo(LDEBUG, "main->destroyreactor failed");
        return 1;
    }
    
    ploginfo(LDEBUG, "main->destroyreactor succeess");
    
    if (!destroylog(server.log))
    {
        printf("destroy log failed\n");
        return 1;
    }
    
    printf("main run stop\n");
    
    return 0;
}
