#ifndef OBJ_H
#define OBJ_H

#include "task.h"

#ifdef __cplusplus
extern "C"
{
#endif
    
#include "util.h"
#include "cjson.h"
#include "evbuffer.h"
    
#ifdef __cplusplus
}
#endif

class obj
{
public:
    obj();
    virtual ~obj();
    
    virtual struct evbuffer *handle(const char *url, cJSON *jsons) = 0;
    
    char *jsonstringform(cJSON *jsons, char *name);//格式化jason字符 "123" 变为 123
    
public:
    static task task;
};

#endif
