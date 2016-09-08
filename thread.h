#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *(*thfun)(void *);

struct thread
{
	pthread_t thid;
	int mode;/*线程的执行模式 1-loop 2-work*/
	int run;/*线程是否正在执行*/
	void *data;/*传递的执行函数的参数*/
	void *(*fun)(void *);/*线程执行函数*/
	pthread_mutex_t thmutex;
	pthread_cond_t thcondition;
	int loop;/*每次循环时间*/
};

/*创建新线程*/
struct thread *createthread(thfun fun, void *data, int loopsecond);
/*销毁线程*/
int destroythread(struct thread *thread);
/*设置线程执行状态*/
int enablethread(struct thread *thread, int enable);
/*线程当前运行状态*/
int isresume(struct thread *thread);
/*设置线程执行的任务*/
void setthreadexecute(struct thread *thread, thfun fun, void *data);
    
#ifdef __cplusplus
}
#endif

#endif
