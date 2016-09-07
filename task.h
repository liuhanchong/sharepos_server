#ifndef TASK_H
#define TASK_H

#include "dbpool.h"

#ifdef __cplusplus
extern "C"
{
#endif
    
#include "util.h"
#include "pyinter.h"
#include "tpool.h"

#ifdef __cplusplus
}
#endif

class task
{
public:
    task();
    ~task();
    
public:
    cbool createtask(struct sysc *sysc);
    dbnode *getdbinst();
    void reldbinst(dbnode *dbnode);
    cbool addttaskinst(thfun fun, void *data);
    cbool destroytask();
    
private:
    dbpool *dbp;
    tpool *tp;
};

#endif
