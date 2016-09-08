#ifndef HTTP_H
#define HTTP_H

#include "evself.h"
#include "reactor.h"
#include "evbuffer.h"
#include "klhttp-internal.h"
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

struct httpserver
{
    struct reactor *reactor;
    int httpsock;/*服务器套接字*/
};

struct httpserver *createhttp(struct eventtop *etlist, char *ip, int port);

int dispatchhttp(struct httpserver *server);

int destroyhttp(struct httpserver *server);
    
#ifdef __cplusplus
}
#endif

#endif
