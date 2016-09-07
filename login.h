#ifndef LOGIN_H
#define LOGIN_H

#include "user.h"
#include "obj.h"

class login : public obj
{
public:
    login();
    virtual ~login();
    
    virtual struct evbuffer *handle(const char *url, cJSON *jsons);
    
private:
    user user;
};

#endif
