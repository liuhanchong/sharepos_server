#ifndef HTTP_H
#define HTTP_H

#include "util.h"
#include "evself.h"
#include "sock.h"
#include "reactor.h"
#include "evbuffer.h"
#include "klhttp-internal.h"
#include <stdlib.h>
#include <string.h>

typedef struct httpserver
{
    struct reactor *reactor;
    int httpsock;/*服务器套接字*/
} httpserver;

struct httpserver *createhttp(struct eventtop *etlist, char *ip, int port);

cbool dispatchhttp(struct httpserver *server);

cbool destroyhttp(struct httpserver *server);

#endif
