#ifndef CONTROL_H
#define CONTROL_H

#include "evbuffer.h"
#include "klhttp-internal.h"

class control
{
public:
    control();
    virtual ~control();
    
    static int request(evbuffer *inbuf , evbuffer *outbuf);
    
    static void createres();
    
    static void destroyres();
    
private:
    static void addhttphead(struct evbuffer *outbuf, const struct http_request *request, char *contype);
    
    static void addhttpresponse(struct evbuffer *outbuf, struct evbuffer *resbuf);
    
    static int response(struct evbuffer *resbuf, struct http_request *request);
    
    static int handle(struct evbuffer *resbuf, const char *url, const char *rdata, int len);
    
private:
};

#endif
