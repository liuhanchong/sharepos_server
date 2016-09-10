#ifndef OBJ_H
#define OBJ_H

class obj
{
public:
    obj();
    virtual ~obj();
    
    virtual int handle(struct evbuffer *resbuf, const char *url, const char *rdata, int len) = 0;
    
private:
    
};

#endif
