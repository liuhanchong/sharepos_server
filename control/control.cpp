#include "control.h"
#include "log.h"
#include "res.h"
#include "model.h"
#include <string.h>
#include <stdlib.h>

control::control()
{
}

control::~control()
{
}

void control::addhttphead(struct evbuffer *outbuf, const struct http_request *request, char *contype)
{
    /* http信息 */
    evbuffer_add_printf(outbuf, "HTTP/%d.%d %d %s\r\n", request->ver.major,
                        request->ver.minor, HTTP_OK, CODE_STR(HTTP_OK));
    
    /* 添加服务器信息 */
    evbuffer_add_printf(outbuf, "Server: sharepos/0.1.0\r\n");
    
    /* 添加时间信息 */
    add_time_header(outbuf);
    
    /* 添加文本类型 */
    evbuffer_add_printf(outbuf, "Content-Type: %s\r\n", contype);
}

void control::addhttpresponse(struct evbuffer *outbuf, struct evbuffer *resbuf)
{
    /* 添加响应的数据长度 */
    evbuffer_add_printf(outbuf, "Content-Length: %lu\r\n", EVBUFFER_LENGTH(resbuf));
    
    /* 添加头结束*/
    evbuffer_add(outbuf, "\r\n", 2);
    
    /* 添加发送的数据 */
    evbuffer_add_printf(outbuf, "%s", resbuf->buffer);
}

int control::response(struct evbuffer *resbuf, struct http_request *request)
{
    const char *url = request->uri;
    const char *data = http_get_header_value(request->headers, "data");
    const int len = atoi(http_get_header_value(request->headers, "Content-Length"));
    
//    ploginfo(LDEBUG, "url=%s dlen=%d data=\r\n%s",  url, len, data);
    
    return (data == NULL || strlen(data) < 1) ? 0 : handle(resbuf, url, data, len);
}

int control::request(evbuffer *inbuf, evbuffer *outbuf)
{
    struct http_request *request = http_request_parse(inbuf);
    
    /*检查请求的有效性*/
    if(check_request_valid(outbuf, request) == -1)
    {
        return 0;
    }
    
    //处理请求
    struct evbuffer *resbuf = evbuffer_new();
    if (response(resbuf, request) == 0)
    {
        ploginfo(LERROR, "control::request->response failed");
    }
    
    //添加http头
    addhttphead(outbuf, request, (char *)json);
    
    //添加数据
    addhttpresponse(outbuf, resbuf);
    
    evbuffer_free(resbuf);
    http_request_free(request);
    
    return 1;
}

int control::handle(struct evbuffer *resbuf, const char *url, const char *rdata, int len)
{
    obj *obj = NULL;
    
    if (strcmp(url, mlogin) == 0)
    {
        obj = new login();
    }
    else
    {
        return 0;
    }
    
    return obj->handle(resbuf, url, rdata, len);
}

void control::createres()
{
    res::getinstance();
}

void control::destroyres()
{
    res::exitinstance();
}

