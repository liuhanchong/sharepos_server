#ifndef HTABLE_H
#define HTABLE_H

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define hkey char*
#define htitem void*

struct htnode
{
	hkey key;/*保存key*/
	int ksize;/*保存key size*/
	unsigned long hkid;/*保存每个key的hash值*/
	htitem item;
	int isize;/*保存item size*/
	struct htnode *next;
};

struct hashtable
{
	struct htnode **hashtable;/*hash列表*/
	pthread_mutex_t tablemutex;/*互斥锁*/
	pthread_mutex_t *tmslot;/*每个互斥锁槽加锁*/
	int tablelen;/*散列表长度*/
};

struct hashtable *createhashtable(int tlen);
int destroyhashtable(struct hashtable *htable);
int setitem(struct hashtable *htable, hkey key, htitem item);
int delitem(struct hashtable *htable, hkey key);
int setitembyid(struct hashtable *htable, int key, htitem item);
int delitembyid(struct hashtable *htable, int key);
htitem getitemvalue(struct hashtable *htable, hkey key);
htitem getitemvaluebyid(struct hashtable *htable, int key);
int excap(struct hashtable *htable, int tlen);/*hashtable扩容*/
    
#ifdef __cplusplus
}
#endif

#endif