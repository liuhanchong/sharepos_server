#include "http.h"
#include "log.h"
#include "control.h"

static void *httpread(void *event, void *data);
static void *httpwrite(void *event, void *data);

static const char *getfdname(int fd)
{
    static char buf[128];
    struct sockaddr_in addr;
    int len = sizeof(addr);
    if(getpeersname(fd, (struct sockaddr *)&addr, (socklen_t *)&len) < 0)
    {
        return "0.0.0.0:0";
    }
    
    sprintf(buf, "%s:%d", inetntoa(addr.sin_addr), ntohsv(addr.sin_port));
    
    return buf;
}

static int packleg(const struct evbuffer *buf)
{
    ploginfo(LDEBUG, "%c %c %d", buf->buffer[0], buf->buffer[EVBUFFER_LENGTH(buf) - 1], EVBUFFER_LENGTH(buf));
    
    if (EVBUFFER_LENGTH(buf) <= 2 ||
        (buf->buffer[0] != 'P' && buf->buffer[0] != 'G') ||
        buf->buffer[EVBUFFER_LENGTH(buf) - 1] != '}')
    {
        return 0;
    }
    
    return 1;
}

static void *httpwrite(void *event, void *data)
{
    ploginfo(LDEBUG, "httpwrite");
    
    assert(event != NULL);
    
    struct event *wuevent = (struct event *)event;
    struct evbuffer *outbuf = (struct evbuffer *)data;
    
    if(EVBUFFER_LENGTH(outbuf) <= 0)
    {
        evbuffer_free(outbuf);
        return (void *)sustr;
    }
    
    if (evbuffer_write(outbuf, wuevent->fd) <= 0)
    {
        evbuffer_free(outbuf);
        ploginfo(LERROR, "%s->%s failed", "httpwrite", "evbuffer_write");
        return (void *)fastr;
    }
    evbuffer_free(outbuf);
    
    //注册接收事件
    struct event *ruevent = setevent(wuevent->reactor, wuevent->fd, EV_READ, httpread, NULL);
    if (ruevent == NULL)
    {
        ploginfo(LERROR, "%s->%s failed clientsock=%d", "acceptconn", "setevent", wuevent->fd);
        return (void *)fastr;
    }
    
    //添加接收事件
    if (addevent(ruevent, 1) == 0)
    {
        ploginfo(LERROR, "%s->%s failed clientsock=%d", "acceptconn", "addevent", wuevent->fd);
        return (void *)fastr;
    }
    
    return (void *)sustr;
}

static void *httpread(void *event, void *data)
{
    ploginfo(LDEBUG, "httpread");
    
    assert(event != NULL);
    
    struct event *ruevent = (struct event *)event;
    
    /*读取数据*/
    struct evbuffer *inbuf = evbuffer_new();
    if(evbuffer_read(inbuf, ruevent->fd, EV_READBUF) <= 0)
    {
        ploginfo(LERROR, "%s->%s failed sockid=%d errno=%d", "httpread", "evbuffer_read", ruevent->fd, errno);
        evbuffer_free(inbuf);
        return (void *)fastr;
    }
    
    //判断数据包的有效性
    if (packleg(inbuf) == 0)
    {
        ploginfo(LERROR, "no legal data package data is \r\n%s", inbuf->buffer);
        return (void *)sustr;
    }
    
    /*处理请求*/
    struct evbuffer *outbuf = evbuffer_new();
    int ret = control::request(inbuf, outbuf);
    evbuffer_free(inbuf);
    if (ret == 0)
    {
        evbuffer_free(outbuf);
        ploginfo(LERROR, "httpread->request failed");
        return (void *)fastr;
    }
    
    //判断是否需要回应的信息
    if(EVBUFFER_LENGTH(outbuf) <= 0)
    {
        evbuffer_free(outbuf);
        return (void *)sustr;
    }
    
    //将客户端套接字注册事件
    struct event *wuevent = setevent(ruevent->reactor, ruevent->fd, EV_WRITE, httpwrite, outbuf);
    if (wuevent == NULL)
    {
        evbuffer_free(outbuf);
        ploginfo(LERROR, "%s->%s failed clientsock=%d", "httpread", "setevent", ruevent->fd);
        return (void *)fastr;
    }
    
    //添加注册事件
    if (addevent(wuevent, 1) == 0)
    {
        evbuffer_free(outbuf);
        ploginfo(LERROR, "%s->%s failed clientsock=%d", "httpread", "addevent", ruevent->fd);
        return (void *)fastr;
    }
    
    return (void *)sustr;
}

static void *httpaccept(void *event, void *data)
{
    struct http *server = (struct http *)data;
    
    struct sockaddr_in clisockaddr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    int clisock = acceptsock(server->httpsock, &clisockaddr, &addrlen);
    if (clisock < 0)
    {
        ploginfo(LERROR, "%s->%s failed clientsock=%d, errno=%d", "httpaccept", "accept", clisock, errno);
        return (void *)fastr;
    }
    
    //将socket设置为non_blocked
    setnoblock(clisock);
    
    //将客户端套接字注册事件
    struct event *uevent = setevent(server->reactor, clisock, EV_READ, httpread, NULL);
    if (uevent == NULL)
    {
        ploginfo(LERROR, "%s->%s failed clientsock=%d", "acceptconn", "setevent", clisock);
        return (void *)fastr;
    }
    
    //添加客户端事件
    if (addevent(uevent, 1) == 0)
    {
        ploginfo(LERROR, "%s->%s failed clientsock=%d", "acceptconn", "addevent", clisock);
        return (void *)fastr;
    }
    
    ploginfo(LDEBUG, "%s->%s sockid=%d, ip=%s success", "acceptconn", "accept", clisock, getfdname(clisock));
    
    return (void *)sustr;
}

struct http *createhttp(struct eventtop *etlist, char *ip, int port)
{
    struct http *httpserver = cnew(struct http);
    if (!httpserver)
    {
        return NULL;
    }
    
    //创建反应堆模型
    if ((httpserver->reactor = createreactor(etlist, 0)) == NULL)
    {
        ploginfo(LERROR, "createhttp->createreactor failed");
        return NULL;
    }
    
    //创建服务器
    httpserver->httpsock = sock(AF_INET, SOCK_STREAM, 0);
    if (httpserver->httpsock < 0)
    {
        ploginfo(LERROR, "createhttp->socket failed");
        return NULL;
    }
    
    //设置sock选项
    int flags = 1;
    struct linger linger = {0, 0};
    setsocketopt(httpserver->httpsock, SOL_SOCKET, SO_KEEPALIVE, (void *)&flags, sizeof(flags));
    setsocketopt(httpserver->httpsock, SOL_SOCKET, SO_LINGER, (void *)&linger, sizeof(linger));
    setsocketopt(httpserver->httpsock, SOL_SOCKET, SO_REUSEADDR, (void *)&flags, sizeof(flags));
    
    struct sockaddr_in sockaddr;
    setsockaddrin(&sockaddr, AF_INET, port, ip);
    if (bindsock(httpserver->httpsock, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr_in)) != 0)
    {
        ploginfo(LERROR, "createhttp->bindsock failed %d", errno);
        return NULL;
    }
    
    if (listensock(httpserver->httpsock, 5) == -1)
    {
        ploginfo(LERROR, "createhttp->listensock failed");
        return NULL;
    }
    
    //创建线程池
    int core = getcpucorenum();
    core = (core > 1) ? core : 1;//最少一个core
    httpserver->tp = createtpool(core * 2, core);
    if (httpserver->tp == NULL)
    {
        return NULL;
    }
    
    //添加接收客户端事件
    struct event *uevent = setevent(httpserver->reactor, httpserver->httpsock, EV_READ | EV_PERSIST, httpaccept, httpserver);
    if (uevent == NULL)
    {
        ploginfo(LERROR, "%s->%s failed sersock=%d", "createhttp", "setevent", httpserver->httpsock);
        return NULL;
    }
    
    if (addevent(uevent, 0) == 0)
    {
        ploginfo(LERROR, "%s->%s failed sersock=%d", "createhttp", "addevent", httpserver->httpsock);
        return NULL;
    }
    
    //创建资源
    control::createres();
    
    return httpserver;
}

int dispatchhttp(struct http *server)
{
    while (server->reactor->listen)
    {
        //获取活动的事件
        if (dispatchevent(server->reactor) == 0)
        {
            return 0;
        }
        
        //处理活动事件
        handle(server->reactor);
    }
    
    return 1;
}

int destroyhttp(struct http *server)
{
    //server->httpsock 已在reactor注册 会在destroyreactor 中自动关闭
    //close(server->httpsock);
    
    control::destroyres();
    
    if (destroytpool(server->tp) == 0)
    {
        ploginfo(LERROR, "destroyhttp->destroytpool failed");
        return 0;
    }
    
    if (destroyreactor(server->reactor) == 0)
    {
        ploginfo(LERROR, "destroyhttp->destroyreactor failed");
        return 0;
    }
    
    cfree(server);
    
    return 1;
}

