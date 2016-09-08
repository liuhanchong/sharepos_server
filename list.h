#ifndef LIST_H
#define LIST_H

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int list_t;

struct listnode
{
	void *data;
	int prio;
	struct listnode *next;
	struct listnode *pre;
};

typedef int (*listsort)(struct listnode *src, struct listnode *dest);

/*循环链表*/
struct list
{
	list_t curlistlen;
	list_t maxlistlen;
    struct listnode *head;
	pthread_mutex_t thmutex;
	int openprio;/*开启队列优先级 0-不开启 1-开启 2-开启可排序队列*/
	listsort sortfun;/*开启排序队列后，传递的排序函数 0-相等 1-src<dest -1-src>desc*/
};

/*遍历所有的节点*/
#define forlist(list) \
	struct listnode *headlistnode = NULL; \
	struct listnode *nextlistnode = NULL; \
	for (headlistnode = gethead((list)), nextlistnode = NULL;  \
		(headlistnode != NULL && nextlistnode != gethead((list))); \
		 nextlistnode = headlistnode = headlistnode->next)

struct list *createlist(list_t maxlen, int openprio, listsort sortfun);
int destroylist(struct list *list);
list_t getcurlistlen(struct list *list);
void setmaxlistlen(struct list *list, list_t maxlen);
list_t getmaxlistlen(struct list *list);
int empty(struct list *list);
int full(struct list *list);
int push(struct list *list, void *data, int prio);
struct listnode *gethead(struct list *list);
int clearlist(struct list *list);
int delnode(struct list *list, struct listnode *node);
    
#ifdef __cplusplus
}
#endif

#endif 