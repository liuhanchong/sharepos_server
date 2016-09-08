#ifndef UTIL_H
#define UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/time.h>
#include <sys/shm.h>

/*函数返回值*/
#define sustr "ok"
#define fastr "no"

/*内存分配*/
#define cmalloc malloc
#define cfree free
#define cnew(type) (type *)(cmalloc)(sizeof(type))
#define cnews(type, n) (type *)(cmalloc)(sizeof(type) * (n))

/*互斥锁操作*/
#define lock(thmutex) (pthread_mutex_lock(&thmutex))
#define unlock(thmutex) (pthread_mutex_unlock(&thmutex))

int getmaxfilenumber();

int setmaxfilenumber(int filenumber);

int setcorefilesize(int filesize);

int getcorefilesize();

int getpidfromfile();

int setpidtofile();

int getcpucorenum();

int timevalcompare(struct timeval *src, struct timeval *dest);

char *ntos(int num);

void freebyntos(char *str);

unsigned long strhash(char *str);

void *createshare(key_t key, size_t size, int *shid);

int destroyshare(int shareid, void *mem);
    
#ifdef __cplusplus
}
#endif

#endif