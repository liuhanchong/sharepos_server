#ifndef TPOOL_H
#define TPOOL_H

#include "thread.h"
#include "list.h"
#include "util.h"

typedef struct tnode
{
	thread *thread;
	time_t accesstime;
	time_t exetime;
} tnode;

typedef struct tpool
{
	list *tlist;
	int maxtnum;/*最大线程数*/
	int coretnum;/*核心的线程数*/	
} tpool;

tpool *createtpool(int maxtnum, int coretnum);
cbool destroytpool(tpool *tpool);
cbool addttask(tpool *tpool, thfun fun, void *data);/*为线程池添加任务*/
cbool addthread(tpool *tpool, int addtnum);/*添加线程*/

#endif /* TPOOL_H */
