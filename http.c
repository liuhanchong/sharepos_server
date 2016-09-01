#include "http.h"

static void *httpread(void *event, void *data)
{
    assert(event != NULL);
    
    struct event *uevent = (struct event *)event;
    
    struct evbuffer *inbuf = evbuffer_new();
    
    /*读取数据*/
    int ret = evbuffer_read(inbuf, uevent->fd, EV_READBUF);
    if(ret <= 0)
    {
        ploginfo(LERROR, "%s->%s failed sockid=%d errno=%d", "httpread", "recv", uevent->fd, errno);
        return NULL;
    }
    
    printf("%s", inbuf->buffer);
    
    /*解析http请求*/
    struct http_request *request = http_request_parse(inbuf);
    
    /*处理请求*/
    

    http_request_free(request);
    evbuffer_free(inbuf);
    
    return NULL;
}

static void *httpwrite(void *event, void *data)
{
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
    
    ploginfo(LDEBUG, "%s->%s sockid=%d success", "acceptconn", "accept", clisock);
    
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
    int sockid = sock(AF_INET, SOCK_STREAM, 0);
    if (sockid < 0)
    {
        ploginfo(LERROR, "createhttp->socket failed");
        return NULL;
    }
    
    //设置sock选项
    int flags = 1;
    struct linger linger = {0, 0};
    setsocketopt(sockid, SOL_SOCKET, SO_KEEPALIVE, (void *)&flags, sizeof(flags));
    setsocketopt(sockid, SOL_SOCKET, SO_LINGER, (void *)&linger, sizeof(linger));
    setsocketopt(sockid, SOL_SOCKET, SO_REUSEADDR, (void *)&flags, sizeof(flags));
//	setsocketopt(sockid, IPPROTO_IP, TCP_NODELAY, (void *)&flags, sizeof(flags));
    
    struct sockaddr_in sockaddr;
    setsockaddrin(&sockaddr, AF_INET, port, ip);
    if (bindsock(sockid, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr_in)) != 0)
    {
        ploginfo(LERROR, "createhttp->bindsock failed %d", errno);
        return NULL;
    }
    
    if (listensock(sockid, 5) == -1)
    {
        ploginfo(LERROR, "createhttp->listensock failed");
        return NULL;
    }
    
    //保存创建成功sockid
    httpserver->httpsock = sockid;
    
    //添加接收客户端事件
    event *uevent = setevent(httpserver->reactor, sockid, EV_READ | EV_PERSIST, httpaccept, httpserver);
    if (uevent == NULL)
    {
        ploginfo(LERROR, "%s->%s failed sersock=%d", "createhttp", "setevent", sockid);
        return NULL;
    }
    
    if (addevent(uevent) == FAILED)
    {
        ploginfo(LDEBUG, "%s->%s failed sersock=%d", "createhttp", "addevent", sockid);
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

