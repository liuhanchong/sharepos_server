#include "dbpool.h"
#include <stdlib.h>
#include <string.h>

static cbool insert(struct dbpool *dbpool, struct dbconn *conn)
{
	struct dbnode *dbnode = (struct dbnode *)malloc(sizeof(struct dbnode));
	if (!dbnode)
	{
		return FAILED;
	}

    dbnode->conn = db::createdbconn(conn);
	if (!dbnode->conn)
	{
		return FAILED;
	}
    
	if (!(dbnode->dbi = db::insdbbytype(dbpool->dbtype)))
	{
		return FAILED;
	}

	if (dbnode->dbi->opendb(dbnode->conn) == FAILED)
	{
		return FAILED;
	}

	dbnode->use = 0;

    return push(dbpool->dblist, dbnode, 0);
}

static cbool deldb(dbpool *dbpool, dbnode *dbnode)
{
    int ret = FAILED;
    lock(dbpool->dblist->thmutex);
    forlist(dbpool->dblist)
    {
        struct dbnode *fdbnode = (struct dbnode *)headlistnode->data;
        if (fdbnode == dbnode)
        {
            ret = dbnode->dbi->closedb();
            free(dbnode);
            delnode(dbpool->dblist, headlistnode);
            break;
        }
    }
    unlock(dbpool->dblist->thmutex);
    return ret;
}

dbpool *createdbpool(int dbtype, int maxdbnum, int coredbnum, dbconn *conn)
{
	dbpool *newdbpool = (struct dbpool *)malloc(sizeof(dbpool));
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
		adddb(newdbpool, coredbnum, conn) == FAILED)
	{
		free(newdbpool);
		return NULL;
	}

	return newdbpool;
}

cbool destroydbpool(dbpool *dbpool)
{
	forlist(dbpool->dblist)
	{
		struct dbnode *dbnode = (struct dbnode *)headlistnode->data;
        
        dbnode->use = 0;
        
		if (dbnode->dbi->closedb() == FAILED)
		{
			ploginfo(LERROR, "destroydbpool->closedb failed");
		}
		delete dbnode->dbi;
        
        db::destroydbconn(dbnode->conn);
        
		free(dbnode);
	}

	destroylist(dbpool->dblist);

	free(dbpool);

	return SUCCESS;
}

cbool adddb(dbpool *dbpool, int adddbnum, struct dbconn *conn)
{
    lock(dbpool->dblist->thmutex);
	for (int i = 1; i <= adddbnum; i++)
	{
		if (insert(dbpool, conn) == FAILED)
		{
			ploginfo(LERROR, "adddb failed, seq=%d", i);
		}
	}
    unlock(dbpool->dblist->thmutex);
	return SUCCESS;
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
            dbnode->use = 1;
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
