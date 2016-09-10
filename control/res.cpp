#include "res.h"
#include "util.h"
#include "log.h"
#include "pyinter.h"

res *res::resource = NULL;

res::res()
{
    if (create() == 0)
    {
        ploginfo(LERROR, "res::res->create failed");
    }
}

res::~res()
{
    if (destroy() == 0)
    {
        ploginfo(LERROR, "res::~res->destroy failed");
    }
}

res *res::getinstance()
{
    if (!resource)
    {
        resource = new res();
    }
    
    return resource;
}

void res::exitinstance()
{
    if (resource)
    {
        delete resource;
        resource = NULL;
    }
}

int res::create()
{
    //初始化线程池
    int core = getcpucorenum();
    core = (core > 1) ? core : 1;
    tp = createtpool(core * 2, core);
    if (tp == NULL)
    {
        return 0;
    }
    
    //获取系统配置
    struct sysc sysc;
    if (getsyscon("./server.ini", &sysc) == 0)
    {
        ploginfo(LERROR, "res::create->getsyscon failed");
        return 0;
    }
    
    //初始化数据库连接池
    struct dbconn *dbconn = db::createdbconn((char *)sysc.host,
                                             (char *)sysc.user,
                                             (char *)sysc.paw,
                                             (char *)sysc.db,
                                             (char *)"",
                                             0,
                                             sysc.dbport);
    if (dbconn == NULL)
    {
        return 0;
    }
    
    core = 100;
    dbp = createdbpool(1, core, core / 2, dbconn);
    if (dbp == NULL)
    {
        return 0;
    }
    
    db::destroydbconn(dbconn);
    
    return 1;
}

dbnode *res::getdbinst()
{
    return getdb(dbp);
}

void res::reldbinst(dbnode *dbnode)
{
    return reldb(dbp, dbnode);
}

int res::addttaskinst(thfun fun, void *data)
{
    return addttask(tp, fun, data);
}

int res::destroy()
{
    if (destroytpool(tp) == 0)
    {
        ploginfo(LERROR, "destroy->destroytpool failed");
        return 0;
    }
    
    if (destroydbpool(dbp) == 0)
    {
        ploginfo(LERROR, "destroy->destroydbpool failed");
        return 0;
    }
    
    return 1;
}