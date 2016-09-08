#include "list.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

static int delete(struct list *list, struct listnode *node)
{
	if (node->next != NULL && node->pre != NULL)
	{
		node->next->pre = node->pre;
		node->pre->next = node->next;

		//删除第一个元素改变头指针
		if (node == list->head)
		{
			list->head = node->next;
		}

		//当链表为空将链表头置为空
		if ((--list->curlistlen) == 0)
		{
			list->head = NULL;
		}

		cfree(node);
		node = NULL;

		return 1;
	}

	return 0;
}

struct list *createlist(list_t maxlen, int openprio, listsort sortfun)
{
    struct list *list = cnew(struct list);
    if (!list)
    {
        return NULL;
    }
    
	pthread_mutexattr_t mutexattr;
	if (pthread_mutexattr_init(&mutexattr) == 0 &&
		pthread_mutex_init(&list->thmutex, &mutexattr) == 0)
	{
        pthread_mutexattr_destroy(&mutexattr);
        
		list->curlistlen = 0;
		list->head = NULL;
		list->maxlistlen = maxlen;
		list->openprio = openprio;
		list->sortfun = sortfun;
        
		return list;
	}

	return NULL;
}

int destroylist(struct list *list)
{
	if (clearlist(list) == 0)
	{
		return 0;
	}

	if (pthread_mutex_destroy(&list->thmutex) != 0)
	{
		return 0;
	}

	return 1;
}

list_t getcurlistlen(struct list *list)
{
	return list->curlistlen;
}

list_t getmaxlistlen(struct list *list)
{
	return list->maxlistlen;
}

void setmaxlistlen(struct list *list, list_t maxlen)
{
	list->maxlistlen = maxlen;
}

int empty(struct list *list)
{
	return ((list->curlistlen == 0) ? 1 : 0);
}

int push(struct list *list, void *data, int prio)
{
	if (full(list))
	{
		return 0;
	}

	struct listnode *node = cnew(struct listnode);
	if (!node)
	{
		return 0;
	}

	/*填充结构体*/
	node->data = data;
	node->next = NULL;
	node->pre = NULL;
	node->prio = prio;

	/*第一次插入元素*/
	if (empty(list))
	{
		list->head = node;
		node->next = node;
		node->pre = node;

		list->curlistlen++;

		return 1;
	}

	//找到头指针
	struct listnode *head = list->head;

	//优先级队列
	if (list->openprio == 1 || list->openprio == 2)
	{
		while (head)
		{
			//找到合适的节点
			if ((list->openprio == 1 && node->prio < head->prio)
				|| (list->openprio == 2 && list->sortfun != NULL && list->sortfun(head, node) == -1))
			{
				node->next = head;
				node->pre = head->pre;
				head->pre->next = node;
				head->pre = node;

				//改变头结点
				if (head == list->head)
				{
					list->head = node;
				}

				list->curlistlen++;

				return 1;
			}

			//遍历到最后一个元素
			if (head->next == list->head)
			{
				break;
			}

			head = head->next;
		}

		//没找到合适的节点
		node->pre = head;
		node->next = head->next;
		head->next->pre = node;
		head->next = node;

		list->curlistlen++;

		return 1;
	}
	//非优先级队列
	else if (list->openprio == 0)
	{
		node->pre = head->pre;
		head->pre->next = node;
		head->pre = node;
		node->next = head;

		list->curlistlen++;
	}

	return 1;
}

struct listnode *gethead(struct list *list)
{
	return list->head;
}

int full(struct list *list)
{
	return (((list->curlistlen < list->maxlistlen) || list->maxlistlen <= 0) ? 0 : 1);
}

int clearlist(struct list *list)
{
	while (!empty(list))
	{
		delnode(list, gethead(list));
	}
    
    list->curlistlen = 0;

	return 1;
}

int delnode(struct list *list, struct listnode *node)
{
	return delete(list, node);
}

