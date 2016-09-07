#include "task.h"

task::task()
{

}

task::~task()
{
    
}

cbool task::createtask(struct sysc *sysc)
{
    //初始化线程池
    int core = getcpucorenum();
    tp = createtpool(core * 2, core);
    if (tp == NULL)
    {
        return FAILED;
    }
    
    //初始化数据库连接池
    dbconn *dbconn = db::createdbconn((char *)sysc->host, (char *)sysc->user,
                                      (char *)sysc->paw, (char *)sysc->db, (char *)"", 0, sysc->dbport);
    if (dbconn == NULL)
    {
        return FAILED;
    }
    
    dbp = createdbpool(1, 100, (100 / 2), dbconn);
    if (dbp == NULL)
    {
        return FAILED;
    }
    
    return SUCCESS;
}

dbnode *task::getdbinst()
{
    return getdb(dbp);
}

void task::reldbinst(dbnode *dbnode)
{
    return reldb(dbp, dbnode);
}

cbool task::addttaskinst(thfun fun, void *data)
{
    return addttask(tp, fun, data);
}

cbool task::destroytask()
{
    if (destroytpool(tp) == FAILED)
    {
        ploginfo(LERROR, "destroytask->destroytpool failed");
        return FAILED;
    }
    
    if (destroydbpool(dbp) == FAILED)
    {
        ploginfo(LERROR, "destroytask->destroydbpool failed");
        return FAILED;
    }
    
    return SUCCESS;
}
