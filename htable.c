#include "htable.h"
#include <stdlib.h>
#include <string.h>

static void freekey(htnode *htnode)
{
	free(htnode->key);
}

static cbool equalkey(hkey skey, hkey dkey)
{
	return (strcmp(skey, dkey) == 0) ? 1 : 0;
}

static cbool equalkeybyid(unsigned long skid, unsigned long dkid)
{
    return (skid == dkid) ? 1 : 0;
}

hashtable *createhashtable(int tlen)
{
	hashtable *newhtable = (struct hashtable *)malloc(sizeof(struct hashtable));
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
    int psize = sizeof(pthread_mutex_t) * tlen;
	newhtable->tmslot = (pthread_mutex_t *)malloc(psize);
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
	newhtable->hashtable = (struct htnode **)malloc(tsize);
	if (!newhtable->hashtable)
	{
		return NULL;
	}
    
    //初始化所有的信息后清空
	memset(newhtable->hashtable, 0, tsize);

	return newhtable;
}

cbool destroyhashtable(hashtable *htable)
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
			free(htnode);
            
			htnode = htnextnode;
		}
	}
	free(htable->hashtable);
    
    //释放所有卡槽
    for (int i = 0; i < htable->tablelen; i++)
    {
        pthread_mutex_destroy(&htable->tmslot[i]);
    }
    free(htable->tmslot);


	pthread_mutex_destroy(&htable->tablemutex);

	free(htable);

	return SUCCESS;
}

cbool setitem(hashtable *htable, hkey key, htitem item)
{
	htnode *newhtnode = (struct htnode *)malloc(sizeof(struct htnode));
	if (!newhtnode)
	{
		return FAILED;
	}

	/*复制key值*/
    newhtnode->hkid = 0;
	newhtnode->ksize = (int)strlen(key);
	newhtnode->key = malloc(newhtnode->ksize + 1);
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

	return SUCCESS;
}

cbool delitem(hashtable *htable, hkey key)
{
	int index = (int)strhash(key);
	index = index % htable->tablelen;

	int ret = FAILED;
	htnode *prehtnode = NULL;
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
			free(fnode);

			ret = SUCCESS;
			break;
		}

		prehtnode = fnode; 
		fnode = fnode->next;
	}

	return ret;
}

cbool setitembyid(hashtable *htable, int key, htitem item)
{
    char *skey = ntos(key);
    int ret = setitem(htable, skey, item);
    freebyntos(skey);
    
    return ret;
}

cbool delitembyid(hashtable *htable, int key)
{
    char *skey = ntos(key);
    int ret = delitem(htable, skey);
    freebyntos(skey);
    
    return ret;
}

htitem getitemvalue(hashtable *htable, hkey key)
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

htitem getitemvaluebyid(hashtable *htable, int key)
{
    char *skey = ntos(key);
    htitem item = getitemvalue(htable, skey);
    freebyntos(skey);
    
    return item;
}

cbool excap(hashtable *htable, int tlen)
{
	if (tlen <= htable->tablelen)
	{
		return SUCCESS;
	}

	int tsize = sizeof(htnode *) * tlen;
	htnode **echashtable = (struct htnode **)malloc(tsize);
	if (!echashtable)
	{
		return FAILED;
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
	free(htable->hashtable);

	htable->hashtable = echashtable;
	htable->tablelen = tlen;

	return SUCCESS;
}