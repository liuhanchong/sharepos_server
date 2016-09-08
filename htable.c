#include "htable.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

static void freekey(struct htnode *htnode)
{
	cfree(htnode->key);
}

static int equalkey(hkey skey, hkey dkey)
{
	return (strcmp(skey, dkey) == 0) ? 1 : 0;
}

static int equalkeybyid(unsigned long skid, unsigned long dkid)
{
    return (skid == dkid) ? 1 : 0;
}

struct hashtable *createhashtable(int tlen)
{
	struct hashtable *newhtable = cnew(struct hashtable);
	if (!newhtable)
	{
		return NULL;
	}

	pthread_mutexattr_t mutexattr;
	if (pthread_mutexattr_init(&mutexattr) != 0 ||
		pthread_mutex_init(&newhtable->tablemutex, &mutexattr) != 0)
	{
		return NULL;
	}
    
    tlen = (tlen > 0) ? tlen : 256;
    newhtable->tablelen = tlen;

    //初始化每个卡槽锁
	newhtable->tmslot = cnews(pthread_mutex_t, tlen);
    if (!newhtable->tmslot)
    {
        return NULL;
    }
    
    for (int i = 0; i < tlen; i++)
    {
        if (pthread_mutex_init(&newhtable->tmslot[i], &mutexattr) != 0)
        {
            return NULL;
        }
    }
    pthread_mutexattr_destroy(&mutexattr);

	int tsize = sizeof(struct htnode *) * tlen;
	newhtable->hashtable = (struct htnode **)cmalloc(tsize);
	if (!newhtable->hashtable)
	{
		return NULL;
	}
    
    //初始化所有的信息后清空
	memset(newhtable->hashtable, 0, tsize);

	return newhtable;
}

int destroyhashtable(struct hashtable *htable)
{
	//释放表结点
	struct htnode *htnode = NULL;
	struct htnode *htnextnode = NULL;
	for (int i = 0; i < htable->tablelen; i++)
	{
		htnode = htable->hashtable[i];
		while (htnode)
		{
			htnextnode = htnode->next;
            
			freekey(htnode);
			cfree(htnode);
            
			htnode = htnextnode;
		}
	}
	cfree(htable->hashtable);
    
    //释放所有卡槽
    for (int i = 0; i < htable->tablelen; i++)
    {
        pthread_mutex_destroy(&htable->tmslot[i]);
    }
    cfree(htable->tmslot);


	pthread_mutex_destroy(&htable->tablemutex);

	cfree(htable);

	return 1;
}

int setitem(struct hashtable *htable, hkey key, htitem item)
{
	struct htnode *newhtnode = cnew(struct htnode);
	if (!newhtnode)
	{
		return 0;
	}

	/*复制key值*/
    newhtnode->hkid = 0;
	newhtnode->ksize = (int)strlen(key);
	newhtnode->key = cmalloc(newhtnode->ksize + 1);
	memcpy(newhtnode->key, key, newhtnode->ksize);
	newhtnode->key[newhtnode->ksize] = '\0';

	/*value 直接使用用户的空间*/
	newhtnode->item = item;
    newhtnode->isize = 0;
	newhtnode->next = NULL;

	int index = strhash(key) % htable->tablelen;
	struct htnode *fnode = htable->hashtable[index];
	if (!fnode)
	{
		htable->hashtable[index] = newhtnode;
	}
	else
	{
		while (fnode->next)
		{
			fnode = fnode->next;
		}
		fnode->next = newhtnode;
	}

	return 1;
}

int delitem(struct hashtable *htable, hkey key)
{
	int index = (int)strhash(key);
	index = index % htable->tablelen;

	int ret = 0;
	struct htnode *prehtnode = NULL;
	struct htnode *fnode = htable->hashtable[index];
	while (fnode)
	{
		if (equalkey(fnode->key, key) == 1)
		{
			if (!prehtnode)
			{
				htable->hashtable[index] = fnode->next;
			}
			else
			{
				prehtnode->next = fnode->next;
			}

			freekey(fnode);
			cfree(fnode);

			ret = 1;
			break;
		}

		prehtnode = fnode; 
		fnode = fnode->next;
	}

	return ret;
}

int setitembyid(struct hashtable *htable, int key, htitem item)
{
    char *skey = ntos(key);
    int ret = setitem(htable, skey, item);
    freebyntos(skey);
    
    return ret;
}

int delitembyid(struct hashtable *htable, int key)
{
    char *skey = ntos(key);
    int ret = delitem(htable, skey);
    freebyntos(skey);
    
    return ret;
}

htitem getitemvalue(struct hashtable *htable, hkey key)
{
	int index = (int)strhash(key);
	index = index % htable->tablelen;

	htitem item = NULL;
	struct htnode *fnode = htable->hashtable[index];
	while (fnode)
	{
		if (equalkey(fnode->key, key) == 1)
		{
			item = fnode->item;
			break;
		}
		fnode = fnode->next;
	}

	return item;
}

htitem getitemvaluebyid(struct hashtable *htable, int key)
{
    char *skey = ntos(key);
    htitem item = getitemvalue(htable, skey);
    freebyntos(skey);
    
    return item;
}

int excap(struct hashtable *htable, int tlen)
{
	if (tlen <= htable->tablelen)
	{
		return 1;
	}

	int tsize = sizeof(struct htnode *) * tlen;
	struct htnode **echashtable = (struct htnode **)cmalloc(tsize);
	if (!echashtable)
	{
		return 0;
	}
	memset(echashtable, 0, tsize);

	struct htnode *htnode = NULL;
	for (int i = 0; i < htable->tablelen; i++)
	{
		htnode = htable->hashtable[i];
		if (htnode)
		{
			echashtable[i] = htnode;
		}
	}
	cfree(htable->hashtable);

	htable->hashtable = echashtable;
	htable->tablelen = tlen;

	return 1;
}