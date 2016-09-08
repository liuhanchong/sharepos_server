#include "dbpool.h"
#include "util.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>

static int insert(struct dbpool *dbpool, struct dbconn *conn)
{
	struct dbnode *dbnode = cnew(struct dbnode);
	if (!dbnode)
	{
		return 0;
	}

    dbnode->conn = db::copydbconn(conn);
	if (!dbnode->conn)
	{
		return 0;
	}
    
	if (!(dbnode->dbi = db::insdbbytype(dbpool->dbtype)))
	{
		return 0;
	}

	if (dbnode->dbi->opendb(dbnode->conn) == 0)
	{
		return 0;
	}

	dbnode->use = 0;

    return push(dbpool->dblist, dbnode, 0);
}

static int deldb(struct dbpool *dbpool, struct dbnode *dbnode)
{
    int ret = 0;
    lock(dbpool->dblist->thmutex);
    forlist(dbpool->dblist)
    {
        struct dbnode *fdbnode = (struct dbnode *)headlistnode->data;
        if (fdbnode == dbnode)
        {
            ret = dbnode->dbi->closedb();
            cfree(dbnode);
            delnode(dbpool->dblist, headlistnode);
            break;
        }
    }
    unlock(dbpool->dblist->thmutex);
    return ret;
}

static int adddb(dbpool *dbpool, int adddbnum, struct dbconn *conn)
{
    lock(dbpool->dblist->thmutex);
    for (int i = 1; i <= adddbnum; i++)
    {
        if (insert(dbpool, conn) == 0)
        {
            ploginfo(LERROR, "adddb failed, seq=%d", i);
        }
    }
    unlock(dbpool->dblist->thmutex);
    return 1;
}

dbpool *createdbpool(int dbtype, int maxdbnum, int coredbnum, dbconn *conn)
{
	dbpool *newdbpool = cnew(struct dbpool);
	if (!newdbpool)
	{
		return NULL;
	}

	/*默认核心5个 最大10个*/
	maxdbnum = (coredbnum > maxdbnum) ? coredbnum : maxdbnum;
	newdbpool->coredbnum = (coredbnum > 0) ? coredbnum : 5;
	newdbpool->maxdbnum = (maxdbnum > 0) ? maxdbnum : 10; 
	newdbpool->dbtype = dbtype;

	if ((newdbpool->dblist = createlist(maxdbnum, 0, NULL)) == NULL ||
		adddb(newdbpool, coredbnum, conn) == 0)
	{
		cfree(newdbpool);
		return NULL;
	}

	return newdbpool;
}

int destroydbpool(struct dbpool *dbpool)
{
	forlist(dbpool->dblist)
	{
		struct dbnode *dbnode = (struct dbnode *)headlistnode->data;
        
        dbnode->use = 0;
        
		if (dbnode->dbi->closedb() == 0)
		{
			ploginfo(LERROR, "destroydbpool->closedb failed");
		}
		delete dbnode->dbi;
        
        db::destroydbconn(dbnode->conn);
        
		cfree(dbnode);
	}

	destroylist(dbpool->dblist);

	cfree(dbpool);

	return 1;
}

dbnode *getdb(dbpool *dbpool)
{
	struct dbnode *dbnode = NULL;
	lock(dbpool->dblist->thmutex);
	forlist(dbpool->dblist)
	{
		struct dbnode *fdbnode = (struct dbnode *)headlistnode->data;
		if (fdbnode->use == 0)
		{
            fdbnode->use = 1;
			dbnode = fdbnode;
			break;
		}
	}
	unlock(dbpool->dblist->thmutex);

	return dbnode;
}

void reldb(dbpool *dbpool, dbnode *dbnode)
{
	lock(dbpool->dblist->thmutex);
	dbnode->use = 0;
	unlock(dbpool->dblist->thmutex);
}
