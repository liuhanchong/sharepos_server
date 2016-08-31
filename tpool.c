#include "tpool.h"
#include <stdlib.h>

static void *defaulttfun(void *data)
{
	return NULL;
}

static cbool insert(tpool *tpool)
{
	tnode *newtnode = (tnode *)malloc(sizeof(tnode));
	if (!newtnode)
	{
		return FAILED;
	}

	newtnode->thread = createthread(defaulttfun, newtnode, -1);
	if (!newtnode->thread)
	{
		free(newtnode);
		return FAILED;
	}

	newtnode->accesstime = time(NULL);
	newtnode->exetime = newtnode->accesstime;

	if (push(tpool->tlist, (void *)newtnode, 0) == FAILED)
	{
        destroythread(newtnode->thread);
        free(newtnode);
		return FAILED;
	}

	return SUCCESS;
}

static tnode *getfreet(tpool *tpool)
{
	tnode *tnode = NULL;
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

static cbool delthread(tpool *tpool, tnode *tnode)
{
    int ret = FAILED;
    lock(tpool->tlist->thmutex);
    forlist(tpool->tlist)
    {
        struct tnode *ftnode = (struct tnode *)headlistnode->data;
        if (ftnode == tnode)
        {
            ret = destroythread(tnode->thread);
            free(tnode);
            delnode(tpool->tlist, headlistnode);
            break;
        }
    }
    unlock(tpool->tlist->thmutex);
    return ret;
}

tpool *createtpool(int maxtnum, int coretnum)
{
	tpool *newtpool = malloc(sizeof(tpool));
	if (!newtpool)
	{
		return NULL;
	}

	/*默认核心5个 最大10个*/
	maxtnum = (coretnum > maxtnum) ? coretnum : maxtnum;
	newtpool->coretnum = (coretnum > 0) ? coretnum : 5;
	newtpool->maxtnum = (maxtnum > 0) ? maxtnum : 10; 

	if ((newtpool->tlist = createlist(maxtnum, 0, NULL)) == NULL ||
		addthread(newtpool, coretnum) == FAILED)
	{
		free(newtpool);
		return NULL;
	}

	return newtpool;
}

cbool destroytpool(tpool *tpool)
{
	if (!tpool)
	{
		return FAILED;
	}

	forlist(tpool->tlist)
	{
		struct tnode *tnode = (struct tnode *)headlistnode->data;
		if (destroythread(tnode->thread) == FAILED)
		{
            ploginfo(LDEBUG, "destroytpool->destroythread failed");
		}
		free(tnode);
	}

	destroylist(tpool->tlist);

	free(tpool);

	return SUCCESS;
}

cbool addttask(tpool *tpool, void *(*fun)(void *), void *data)
{
	if (!tpool)
	{
		return FAILED;
	}

	int ret = FAILED;
	lock(tpool->tlist->thmutex);
	tnode *tnode = getfreet(tpool);
	if (tnode)
	{
		setthreadexecute(tnode->thread, fun, data);

		if (enablethread(tnode->thread, 1) == SUCCESS)
		{
			tnode->accesstime = time(NULL);
			tnode->exetime = tnode->accesstime;

			ret = SUCCESS;
		}
	}
	unlock(tpool->tlist->thmutex);

	return ret;
}

cbool addthread(tpool *tpool, int addtnum)
{
	if (tpool->maxtnum - addtnum < getcurlistlen(tpool->tlist))
	{
		return FAILED;
	}

	int ret = SUCCESS;
	lock(tpool->tlist->thmutex);
	for (int i = 1; i <= addtnum; i++)
	{
		if (insert(tpool) == FAILED)
		{
			ret = FAILED;
            ploginfo(LDEBUG, "insert new thread failed seq=%d, total=%d", i, addtnum);
			break;
		}
	}
	unlock(tpool->tlist->thmutex);

	return ret;
}

