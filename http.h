#ifndef HTTP_H
#define HTTP_H

#include "util.h"
#include "evself.h"
#include "sock.h"
#include "reactor.h"
#include "evbuffer.h"
#include "klhttp-internal.h"
#include "request.h"
#include "pyinter.h"
#include <stdlib.h>
#include <string.h>

typedef struct httpserver
{
    struct reactor *reactor;
    int httpsock;/*服务器套接字*/
    struct req *req;/*请求服务*/
} httpserver;

typedef struct httpbuf
{
    int fd;
    struct evbuffer *inbuf;
    struct evbuffer *outbuf;
} httpbuf;

struct httpserver *createhttp(struct sysc *sysc, struct eventtop *etlist, char *ip, int port);

cbool dispatchhttp(struct httpserver *server);

cbool destroyhttp(struct httpserver *server);

struct httpbuf *createhttpbuf(int fd);

cbool destroyhttpbuf(struct httpbuf *buf);

#endif
