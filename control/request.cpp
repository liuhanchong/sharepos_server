#include "request.h"
#include "cjson.h"
#include "control.h"

static void addhttphead(struct evbuffer *outbuf, const struct http_request *request, char *contype)
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

static void addhttpresponse(struct evbuffer *outbuf, struct evbuffer *resbuf)
{
    /* 添加响应的数据长度 */
    evbuffer_add_printf(outbuf, "Content-Length: %lu\r\n", EVBUFFER_LENGTH(resbuf));
    
    /* 添加头结束*/
    evbuffer_add(outbuf, "\r\n", 2);
    
    /* 添加发送的数据 */
    evbuffer_add_printf(outbuf, "%s", resbuf->buffer);
}

static struct evbuffer *response(struct req *req, struct http_request *request)
{
    const char *url = request->uri;
    const char *data = http_get_header_value(request->headers, "data");
    const int dlen = atoi(http_get_header_value(request->headers, "Content-Length"));
    
    ploginfo(LDEBUG, "url=%s dlen=%d data=\r\n%s",  url, dlen, data);
    
    //调用控制器
    struct evbuffer *resbuf = NULL;
    cJSON *jsons = cJSON_Parse(data);
    resbuf = ((obj *)req->ctl)->handle(url, jsons);
    cJSON_Delete(jsons);
    
    return resbuf;
}

struct req *createrequest(struct sysc *sysc)
{
    struct req *req = (struct req *)malloc(sizeof(struct req));
    if (req == NULL)
    {
        return NULL;
    }
    
    obj *ctl = new control(sysc);
    req->ctl = ctl;
    
    return req;
}

struct evbuffer *request(struct req *req, struct evbuffer *inbuf)
{
    ploginfo(LDEBUG, "\r\n%s", inbuf->buffer);
    
    struct http_request *request = http_request_parse(inbuf);
    
    struct evbuffer *outbuf = evbuffer_new();
    
    /* 检查请求的有效性 */
    if(check_request_valid(outbuf, request) == -1)
    {
        return outbuf;
    }
    
    //处理请求
    struct evbuffer *resbuf = response(req, request);

    //添加http头
    addhttphead(outbuf, request, (char *)json);
    
    //处理请求并添加到回应数据
    addhttpresponse(outbuf, resbuf);
    
    evbuffer_free(resbuf);
    http_request_free(request);
    
    return outbuf;
}

cbool destroyrequest(struct req *req)
{
    delete (obj *)req->ctl;
    
    free(req);
    
    return SUCCESS;
}

