#ifndef UTIL_H
#define UTIL_H

typedef int cbool;

#include "log.h"
#include <sys/time.h>
#include <sys/shm.h>

/*函数返回值*/
#define SUCCESS 1
#define FAILED 0
#define SUCCESSSTR "success"
#define FAILEDSTR NULL

#define lock(thmutex) (pthread_mutex_lock(&thmutex))
#define unlock(thmutex) (pthread_mutex_unlock(&thmutex))

int getmaxfilenumber();

cbool setmaxfilenumber(int filenumber);

cbool setcorefilesize(int filesize);

int getcorefilesize();

int getpidfromfile();

cbool setpidtofile();

int getcpucorenum();

int timevalcompare(struct timeval *src, struct timeval *dest);

char *ntos(int num);

void freebyntos(char *str);

unsigned long strhash(char *str);

void *createshare(key_t key, size_t size, int *shid);

cbool destroyshare(int shareid, void *mem);

#endif