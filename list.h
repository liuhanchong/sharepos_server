#ifndef LIST_H
#define LIST_H

#include "util.h"
#include <pthread.h>

typedef int list_t;

typedef struct listnode
{
	void *data;
	int prio;
	struct listnode *next;
	struct listnode *pre;
} listnode;

typedef int (*listsort)(listnode *src, listnode *dest);

/*循环链表*/
typedef struct list
{
	list_t curlistlen;
	list_t maxlistlen;
	listnode *head;
	pthread_mutex_t thmutex;
	int openprio;/*开启队列优先级 0-不开启 1-开启 2-开启可排序队列*/
	listsort sortfun;/*开启排序队列后，传递的排序函数 0-相等 1-src<dest -1-src>desc*/
} list;

/*遍历所有的节点*/
#define forlist(list) \
	listnode *headlistnode = NULL; \
	listnode *nextlistnode = NULL; \
	for (headlistnode = gethead((list)), nextlistnode = NULL;  \
		(headlistnode != NULL && nextlistnode != gethead((list))); \
		 nextlistnode = headlistnode = headlistnode->next)

list *createlist(list_t maxlen, int openprio, listsort sortfun);
cbool destroylist(list *list);
list_t getcurlistlen(list *list);
void setmaxlistlen(list *list, list_t maxlen);
list_t getmaxlistlen(list *list);
cbool empty(list *list);
cbool full(list *list);
cbool push(list *list, void *data, int prio);
listnode *gethead(list *list);
cbool clearlist(list *list);
cbool delnode(list *list, listnode *node);

#endif 