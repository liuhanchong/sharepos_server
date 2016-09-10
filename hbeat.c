#include "hbeat.h"
#include "evself.h"
#include "util.h"
#include "log.h"
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

static void *handlefd(void *event, void *data)
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

struct heartbeat *createheartbeat(struct reactor *reactor, int connnum, int outtime)
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
    newhb->reactor = reactor;

	/*创建hashtable*/
    if ((newhb->hbtable = createhashtable(connnum)) == NULL)
    {
        return NULL;
    }
    
    //创建定时器
    struct event *event = settimer(newhb->reactor, EV_TIMER | EV_PERSIST, handlefd, newhb);
    if (event == NULL)
    {
        return NULL;
    }
    
    struct timeval timer = {.tv_sec = outtime, .tv_usec = 0};
    if (addtimer(event, &timer) == 0)
    {
        return NULL;
    }

	return newhb;
}

int destroyheartbeat(struct heartbeat *hebeat)
{
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
    
    return ret;
}