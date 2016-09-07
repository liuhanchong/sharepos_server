#ifndef REQUEST_H
#define REQUEST_H

#define json ("application/json")

typedef struct req
{
    void *ctl;//保存控制器
} req;

#ifdef __cplusplus
extern "C"
{
#endif
    
#include "http.h"
#include "pyinter.h"
    
struct req *createrequest(struct sysc *sysc);
    
struct evbuffer *request(struct req *req, struct evbuffer *inbuf);
    
cbool destroyrequest(struct req *req);
    
#ifdef __cplusplus
}
#endif

#endif
