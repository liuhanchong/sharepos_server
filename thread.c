#include "thread.h"
#include "util.h"
#include <unistd.h>
#include <stdlib.h>

void cleanup(void *data)
{
	pthread_mutex_t *mutex = (pthread_mutex_t *)data;
	unlock(*mutex);
}

#define resumethread(thread) \
		while (1) \
		{ \
			/*当线程异常退出时候，需要执行的清理函数*/ \
			pthread_cleanup_push(cleanup, (&(thread->thmutex))); \
			/*等待线程可以执行*/ \
			lock(((thread->thmutex))); \
			while (thread->run == 0) \
			{ \
				pthread_cond_wait((&(thread->thcondition)), (&(thread->thmutex))); \
			}

#define suspendthread(thread) \
			/*解锁资源 并将线程挂起*/ \
	 		unlock(((thread->thmutex))); \
	 		enablethread(thread, 0); \
	 		pthread_cleanup_pop(0); \
		} 

#define beginthread()  \
		while (1) \
		{

#define loopthread(second)  \
			if (second > 0) \
			{ \
				sleep(second); \
			} \
		} 

static int setcancelmode(int mode)
{
	if (mode)
	{
        //线程允许设置取消点
		if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) == 0 &&
			pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL) == 0)
		{
			return 1;
		}
	}
	else
	{
        //线程不允许设置取消点
		if (pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL) == 0)
		{
			return 1;
		}
	}
	return 0;
}

static void *execute(void *data)
{
	struct thread *thread = (struct thread *)data;

	if (setcancelmode(1) == 0)
	{
		return NULL;
	}

	if (thread->mode == 1)
	{
		beginthread();
		thread->fun(thread->data);
		pthread_testcancel();
		loopthread(thread->loop);
	}
	else 
	{
		resumethread(thread);
		thread->fun(thread->data);
		pthread_testcancel();
		suspendthread(thread);
	}

	return NULL;
}

struct thread *createthread(thfun fun, void *data, int loopsecond)
{
    struct thread *thread = cnew(struct thread);
	if (!thread)
	{
		return NULL;
	}

	pthread_attr_t thattr;
	pthread_mutexattr_t mutexattr;
	if (pthread_mutexattr_init(&mutexattr) == 0 &&
		pthread_mutex_init(&thread->thmutex, &mutexattr) == 0 &&
		pthread_cond_init(&thread->thcondition, NULL) == 0 &&
		pthread_attr_init(&thattr) == 0)
	{
		thread->mode = (loopsecond >= 0) ? 1 : 2;
		thread->run  = (loopsecond >= 0) ? 1 : 0;
		thread->fun  = fun;
		thread->data = data;
		thread->loop = loopsecond;
		if (pthread_create(&thread->thid, &thattr, execute, (void *)thread) == 0)
		{
			return thread;
		}
	}

	cfree(thread);

	return NULL;
}

int destroythread(struct thread *thread)
{
	if (!thread)
	{
		return 0;
	}

	if (pthread_cancel(thread->thid) == 0 &&
		pthread_join(thread->thid, NULL) == 0 &&
		pthread_cond_destroy(&thread->thcondition) == 0 &&
		pthread_mutex_destroy(&thread->thmutex) == 0)
	{
		cfree(thread);
		thread = NULL;
		return 1;
	}

	cfree(thread);
	thread = NULL;

	return 0;
}

int enablethread(struct thread *thread, int enable)
{
	if (lock(thread->thmutex) == 0)
	{
		if (enable)
		{
			thread->run = 1;
			pthread_cond_signal(&thread->thcondition);
		}
		else
		{
			thread->run = 0;
		}

		if (unlock(thread->thmutex) == 0)
		{
			return 1;
		}
	}
	
	return 0;
}

int isresume(struct thread *thread)
{
	return thread->run;
}

void setthreadexecute(struct thread *thread, thfun fun, void *data)
{
	thread->fun = fun;
	thread->data = data;
}

