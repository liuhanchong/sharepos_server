#ifndef RES_H
#define RES_H

#include "dbpool.h"
#include "tpool.h"

class res
{
private:
    res();
    ~res();
    
public:
    static res *getinstance();
    static void exitinstance();
    
    struct dbnode *getdbinst();
    void reldbinst(struct dbnode *dbnode);
    int addttaskinst(thfun fun, void *data);

private:
    int create();
    int destroy();
    
private:
    struct dbpool *dbp;
    struct tpool *tp;
    static res *resource;
};

#endif 
