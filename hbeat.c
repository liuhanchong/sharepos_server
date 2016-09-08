#include "hbeat.h"
#include "evself.h"
#include "util.h"
#include "log.h"
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

static void *handlefd(void *data)
{
	struct heartbeat *hebeat = (struct heartbeat *)data;
    
    int rtconnnum = 0;/*计算实时连接数*/
    struct hbnode *hbnode = NULL;
    struct htnode *htnode = NULL;
    struct htnode *htnextnode = NULL;
    for (int i = 0; i < hebeat->hbtable->tablelen; i++)
    {
        htnode = hebeat->hbtable->hashtable[i];
        while (htnode)
        {
            hbnode = htnode->item;
            htnextnode = htnode->next;
            
            //超时的fd
            if (time(NULL) - hbnode->time > hebeat->outtime)
            {
                ploginfo(LOTHER, "outtime sid=%d", hbnode->fd);
                
                hbnode->failed++;
                hbnode->time = time(NULL);
                
                //超过最大超时次数
                if (hbnode->failed >= hebeat->outcount)
                {
                    hbnode->state = 0;
                }
            }
            
            //对于失效的的fd 删除
            if (hbnode->state == 0)
            {
                //删除事件
                struct event *event = getevent(hbnode->fd, hebeat->reactor);
                if (event)
                {
                    if (closeevent(event) == 0)
                    {
                        ploginfo(LERROR, "handlefd->closeevent failed");
                    }
                }
                
                ploginfo(LOTHER, "no effect sid=%d", hbnode->fd);
            }
            else
            {
                rtconnnum++;
            }
            
            htnode = htnextnode;
        }
    }

	ploginfo(LOTHER, "max conn num is %d, real time conn num is %d", hebeat->hbtable->tablelen, rtconnnum);

	return NULL;
}

static void freehbnode(struct hbnode *hbnode)
{
    cfree(hbnode);
}

struct heartbeat *createheartbeat(int connnum, int outtime)
{
	connnum = (connnum > 0) ? connnum : 1024;
	outtime = (outtime > 0) ? outtime : 1;

	struct heartbeat *newhb = cnew(struct heartbeat);
	if (!newhb)
	{
		return NULL;
	}

	newhb->outtime = outtime;
	newhb->outcount = 3;/*给定最多三次超时*/

	/*创建hashtable*/
    if ((newhb->hbtable = createhashtable(connnum)) == NULL)
    {
        return NULL;
    }
	
	newhb->hbthread = createthread(handlefd, newhb, outtime);
	if (!newhb->hbthread)
	{
		return NULL;
	}

	return newhb;
}

int destroyheartbeat(struct heartbeat *hebeat)
{
	destroythread(hebeat->hbthread);
    
    struct htnode *htnode = NULL;
    for (int i = 0; i < hebeat->hbtable->tablelen; i++)
    {
        htnode = hebeat->hbtable->hashtable[i];
        while (htnode)
        {
            freehbnode(htnode->item);
            htnode = htnode->next;
        }
    }
    destroyhashtable(hebeat->hbtable);
    
    cfree(hebeat);

	return 1;
}

int addheartbeat(struct heartbeat *hebeat, int fd)
{
    struct hbnode *newnode = cnew(struct hbnode);
    if (!newnode)
    {
        return 0;
    }
    
    newnode->fd = fd;
    newnode->failed = 0;
    newnode->state = 1;
    newnode->time = time(NULL);
    
    lock(hebeat->hbtable->tablemutex);
    int ret = setitembyid(hebeat->hbtable, fd, newnode);
    unlock(hebeat->hbtable->tablemutex);
    
    ploginfo(LOTHER, "addheartbeat sid=%d", fd);
    
    return ret;
}

int delheartbeat(struct heartbeat *hebeat, int fd)
{
    int ret = 0;
    lock(hebeat->hbtable->tablemutex);
    //删除节点值
    freehbnode(getitemvaluebyid(hebeat->hbtable, fd));
    //删除节点
    ret = delitembyid(hebeat->hbtable, fd);
    unlock(hebeat->hbtable->tablemutex);
    
    ploginfo(LOTHER, "delheartbeat sid=%d", fd);
   
	return ret;
}

int upheartbeat(struct heartbeat *hebeat, int fd)
{
    int ret = 0;
    lock(hebeat->hbtable->tablemutex);
    struct hbnode *hbnode = getitemvaluebyid(hebeat->hbtable, fd);

    if (hbnode)
    {
        hbnode->failed = 0;
        hbnode->time = time(NULL);
        
        ret = 1;
    }
    unlock(hebeat->hbtable->tablemutex);
    
    ploginfo(LOTHER, "upheartbeat sid=%d", fd);
    
    return ret;
}