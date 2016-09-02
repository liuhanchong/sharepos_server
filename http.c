#include "http.h"

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

/** request handler */
static struct evbuffer *handlerequest(const struct http_request *request, void *arg)
{
    struct evbuffer *outbuf = evbuffer_new();
    if (http_handle_request(outbuf, request) == 0)
    {
        return outbuf;
    }
    
    evbuffer_free(outbuf);
    return NULL;
}

static void *httpwrite(void *event, void *data)
{
    struct event *uevent = (struct event *)event;
    struct httpbuf *buf = (struct httpbuf *)uevent->buf;
    
    if(EVBUFFER_LENGTH(buf->outbuf) <= 0)
    {
        return NULL;
    }
    
    printf("\r\n%s", buf->outbuf->buffer);
    
    if (evbuffer_write(buf->outbuf, buf->fd) <= 0)
    {
        ploginfo(LERROR, "%s->%s failed", "httpwrite", "evbuffer_write");
    }
    
    return NULL;
}

static void *httpread(void *event, void *data)
{
    assert(event != NULL);
    
    struct event *ruevent = (struct event *)event;
    struct httpbuf *buf = (struct httpbuf *)ruevent->buf;
    
    /*读取数据*/
    int ret = evbuffer_read(buf->inbuf, ruevent->fd, EV_READBUF);
    if(ret <= 0)
    {
        ploginfo(LERROR, "%s->%s failed sockid=%d errno=%d", "httpread", "recv", ruevent->fd, errno);
        return NULL;
    }
    
    printf("\r\n%s", buf->inbuf->buffer);
    
    /*处理请求*/
    struct http_request *request = http_request_parse(buf->inbuf);
    struct evbuffer *outbuf = handlerequest(request, NULL);
    http_request_free(request);
    if (outbuf == NULL)
    {
        ploginfo(LERROR, "%s->%s failed clientsock=%d", "httpread", "handlerequest", ruevent->fd);
        return NULL;
    }
    
    //将客户端套接字注册事件
    struct event *wuevent = setevent(ruevent->reactor, ruevent->fd, EV_WRITE, httpwrite, NULL);
    if (wuevent == NULL)
    {
        ploginfo(LERROR, "%s->%s failed clientsock=%d", "httpread", "setevent", ruevent->fd);
        return NULL;
    }
    
    //将回应的数据写入事件
    evbuffer_add_buffer(((struct httpbuf *)(wuevent->buf))->outbuf, outbuf);
    evbuffer_free(outbuf);
    
    //添加注册事件
    if (addevent(wuevent) == FAILED)
    {
        ploginfo(LERROR, "%s->%s failed clientsock=%d", "httpread", "addevent", ruevent->fd);
        return NULL;
    }
    
    return NULL;
}

static void *httpaccept(void *event, void *data)
{
    struct httpserver *server = (struct httpserver *)data;
    
    struct sockaddr_in clisockaddr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    int clisock = acceptsock(server->httpsock, &clisockaddr, &addrlen);
    if (clisock < 0)
    {
        ploginfo(LERROR, "%s->%s failed clientsock=%d, errno=%d", "httpaccept", "accept", clisock, errno);
        return NULL;
    }
    
    //将socket设置为non_blocked
    setnoblock(clisock);
    
    //将客户端套接字注册事件
    struct event *uevent = setevent(server->reactor, clisock, EV_READ, httpread, NULL);
    if (uevent == NULL)
    {
        ploginfo(LERROR, "%s->%s failed clientsock=%d", "acceptconn", "setevent", clisock);
        return NULL;
    }
    
    //添加注册事件
    if (addevent(uevent) == FAILED)
    {
        ploginfo(LERROR, "%s->%s failed clientsock=%d", "acceptconn", "addevent", clisock);
        return NULL;
    }
    
    ploginfo(LDEBUG, "%s->%s sockid=%d, ip=%s success", "acceptconn", "accept", clisock, getfdname(clisock));
    
    return NULL;
}

struct httpserver *createhttp(struct eventtop *etlist, char *ip, int port)
{
    struct httpserver *httpserver = (struct httpserver *)malloc(sizeof(struct httpserver));
    if (!httpserver)
    {
        return NULL;
    }
    
    //创建反应堆模型
    if ((httpserver->reactor = createreactor(etlist, 0)) == NULL)
    {
        ploginfo(LDEBUG, "createhttp->createreactor failed");
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
//	setsocketopt(sockid, IPPROTO_IP, TCP_NODELAY, (void *)&flags, sizeof(flags));
    
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
    
    //添加接收客户端事件
    event *uevent = setevent(httpserver->reactor, httpserver->httpsock, EV_READ | EV_PERSIST, httpaccept, httpserver);
    if (uevent == NULL)
    {
        ploginfo(LERROR, "%s->%s failed sersock=%d", "createhttp", "setevent", httpserver->httpsock);
        return NULL;
    }
    
    if (addevent(uevent) == FAILED)
    {
        ploginfo(LDEBUG, "%s->%s failed sersock=%d", "createhttp", "addevent", httpserver->httpsock);
        return NULL;
    }
    
    return httpserver;
}

cbool dispatchhttp(struct httpserver *server)
{
    return dispatchevent(server->reactor);
}

cbool destroyhttp(struct httpserver *server)
{
    //server->httpsock 已在reactor注册 会在destroyreactor 中自动关闭
    //close(server->httpsock);
    
    if (destroyreactor(server->reactor) == FAILED)
    {
        ploginfo(LDEBUG, "main->destroyreactor failed");
        return FAILED;
    }
    
    return SUCCESS;
}

struct httpbuf *createhttpbuf(int fd)
{
    struct httpbuf *buf = (struct httpbuf *)malloc(sizeof(struct httpbuf));
    buf->fd = fd;
    buf->inbuf = evbuffer_new();
    buf->outbuf = evbuffer_new();
    
    return buf;
}

cbool destroyhttpbuf(struct httpbuf *buf)
{
    evbuffer_free(buf->inbuf);
    evbuffer_free(buf->outbuf);
    free(buf);
    
    return SUCCESS;
}

