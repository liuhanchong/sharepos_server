#include "tpool.h"
#include "util.h"
#include "log.h"
#include <stdlib.h>

static void *defaulttfun(void *data)
{
	return NULL;
}

static int insert(struct tpool *tpool)
{
    struct tnode *newtnode = cnew(struct tnode);
	if (!newtnode)
	{
		return 0;
	}

	newtnode->thread = createthread(defaulttfun, newtnode, -1);
	if (!newtnode->thread)
	{
		return 0;
	}

	newtnode->accesstime = time(NULL);
	newtnode->exetime = newtnode->accesstime;

	if (push(tpool->tlist, (void *)newtnode, 0) == 0)
	{
		return 0;
	}

	return 1;
}

static struct tnode *getfreet(struct tpool *tpool)
{
	struct tnode *tnode = NULL;
	forlist(tpool->tlist)
	{
		tnode = (struct tnode *)headlistnode->data;
		if (tnode && (isresume(tnode->thread) == 0))
		{
			break;
		}
		tnode = NULL;
	}

	return tnode;
}

static int delthread(struct tpool *tpool, struct tnode *tnode)
{
    int ret = 0;
    lock(tpool->tlist->thmutex);
    forlist(tpool->tlist)
    {
        struct tnode *ftnode = (struct tnode *)headlistnode->data;
        if (ftnode == tnode)
        {
            ret = destroythread(tnode->thread);
            cfree(tnode);
            delnode(tpool->tlist, headlistnode);
            break;
        }
    }
    unlock(tpool->tlist->thmutex);
    return ret;
}

static int addthread(struct tpool *tpool, int addtnum)
{
    if (tpool->maxtnum - addtnum < getcurlistlen(tpool->tlist))
    {
        return 0;
    }
    
    int ret = 1;
    lock(tpool->tlist->thmutex);
    for (int i = 1; i <= addtnum; i++)
    {
        if (insert(tpool) == 0)
        {
            ret = 0;
            ploginfo(LERROR, "insert new thread failed seq=%d, total=%d", i, addtnum);
            break;
        }
    }
    unlock(tpool->tlist->thmutex);
    
    return ret;
}

struct tpool *createtpool(int maxtnum, int coretnum)
{
	struct tpool *newtpool = cnew(struct tpool);
	if (!newtpool)
	{
		return NULL;
	}

	/*默认核心5个 最大10个*/
	maxtnum = (coretnum > maxtnum) ? coretnum : maxtnum;
	newtpool->coretnum = (coretnum > 0) ? coretnum : 5;
	newtpool->maxtnum = (maxtnum > 0) ? maxtnum : 10; 

	if ((newtpool->tlist = createlist(maxtnum, 0, NULL)) == NULL ||
		addthread(newtpool, coretnum) == 0)
	{
		cfree(newtpool);
		return NULL;
	}

	return newtpool;
}

int destroytpool(struct tpool *tpool)
{
	if (!tpool)
	{
		return 0;
	}

	forlist(tpool->tlist)
	{
		struct tnode *tnode = (struct tnode *)headlistnode->data;
		if (destroythread(tnode->thread) == 0)
		{
            ploginfo(LERROR, "destroytpool->destroythread failed");
		}
		cfree(tnode);
	}

	destroylist(tpool->tlist);

	cfree(tpool);

	return 1;
}

int addttask(struct tpool *tpool, thfun fun, void *data)
{
	if (!tpool)
	{
		return 0;
	}

	int ret = 0;
	lock(tpool->tlist->thmutex);
	struct tnode *tnode = getfreet(tpool);
	if (tnode)
	{
		setthreadexecute(tnode->thread, fun, data);

		if (enablethread(tnode->thread, 1) == 1)
		{
			tnode->accesstime = time(NULL);
			tnode->exetime = tnode->accesstime;

			ret = 1;
		}
	}
	unlock(tpool->tlist->thmutex);

	return ret;
}

