#include "login.h"
#include "control.h"
#include "task.h"

control::control(struct sysc *sysc)
{
    if (task.createtask(sysc) == FAILED)
    {
        ploginfo(LERROR, "control::control->createtask failed");
    }
}

control::~control()
{
    task.destroytask();
}

struct evbuffer *control::handle(const char *url, cJSON *jsons)
{
    obj *object = NULL;
    
    if (strcmp(url, mlogin) == 0)
    {
        object = new login();
    }
    else
    {
        return evbuffer_new();
    }
    
    struct evbuffer *resbuf = object->handle(url, jsons);
    delete object;
    
    return resbuf;
}
