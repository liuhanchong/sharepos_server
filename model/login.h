#ifndef LOGIN_H
#define LOGIN_H

#include "obj.h"
#include <string>
using namespace std;

class login : public obj
{
public:
    login();
    virtual ~login();
    
    virtual int handle(struct evbuffer *resbuf, const char *url, const char *rdata, int len);
    
    void setnick(char *nick);
    void setpaw(char *paw);
    void setphone(char *phone);
    
private:
    int save();
    
private:
    string nick;
    string paw;
    string phone;
    
};

#endif
