#ifndef TPOOL_H
#define TPOOL_H

#include "thread.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

struct tnode
{
	struct thread *thread;
	time_t accesstime;
	time_t exetime;
};

struct tpool
{
	struct list *tlist;
	int maxtnum;/*最大线程数*/
	int coretnum;/*核心的线程数*/	
};

struct tpool *createtpool(int maxtnum, int coretnum);
int destroytpool(struct tpool *tpool);
int addttask(struct tpool *tpool, thfun fun, void *data);/*为线程池添加任务*/
    
#ifdef __cplusplus
}
#endif

#endif /* TPOOL_H */
