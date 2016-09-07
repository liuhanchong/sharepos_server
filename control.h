#ifndef CONTROL_H
#define CONTROL_H

#include "obj.h"

#define mlogin ("/login")

class control : public obj
{
public:
    control(struct sysc *sysc);
    virtual ~control();
    
    virtual struct evbuffer *handle(const char *url, cJSON *jsons);
    
private:
};

#endif
