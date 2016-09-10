#ifndef HTTP_H
#define HTTP_H

#include "evself.h"
#include "evbuffer.h"
#include "tpool.h"
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

struct http
{
    struct reactor *reactor;
    int httpsock;/*服务器套接字*/
    struct tpool *tp;/*线程池*/
};

struct http *createhttp(struct eventtop *etlist, char *ip, int port);

int dispatchhttp(struct http *server);

int destroyhttp(struct http *server);
    
#ifdef __cplusplus
}
#endif

#endif
