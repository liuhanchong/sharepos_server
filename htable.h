#ifndef HTABLE_H
#define HTABLE_H

#include "util.h"
#include <pthread.h>

#define hkey char*
#define htitem void*

typedef struct htnode
{
	hkey key;/*保存key*/
	int ksize;/*保存key size*/
	unsigned long hkid;/*保存每个key的hash值*/
	htitem item;
	int isize;/*保存item size*/
	struct htnode *next;
} htnode;

typedef struct hashtable
{
	htnode **hashtable;/*hash列表*/
	pthread_mutex_t tablemutex;/*互斥锁*/
	pthread_mutex_t *tmslot;/*每个互斥锁槽加锁*/
	int tablelen;/*散列表长度*/
} hashtable; 

hashtable *createhashtable(int tlen);
cbool destroyhashtable(hashtable *htable);
cbool setitem(hashtable *htable, hkey key, htitem item);
cbool delitem(hashtable *htable, hkey key);
cbool setitembyid(hashtable *htable, int key, htitem item);
cbool delitembyid(hashtable *htable, int key);
htitem getitemvalue(hashtable *htable, hkey key);
htitem getitemvaluebyid(hashtable *htable, int key);
cbool excap(hashtable *htable, int tlen);/*hashtable扩容*/

#endif