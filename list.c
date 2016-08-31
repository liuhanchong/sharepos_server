#include "list.h"
#include <stdlib.h>
#include <string.h>

static cbool delete(list *list, listnode *node)
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

		free(node);
		node = NULL;

		return SUCCESS;
	}

	return FAILED;
}

list *createlist(list_t maxlen, int openprio, listsort sortfun)
{
    list *list = (struct list *)malloc(sizeof(struct list));
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

cbool destroylist(list *list)
{
	if (clearlist(list) == FAILED)
	{
		return FAILED;
	}

	if (pthread_mutex_destroy(&list->thmutex) != 0)
	{
		return FAILED;
	}

	return SUCCESS;
}

list_t getcurlistlen(list *list)
{
	return list->curlistlen;
}

list_t getmaxlistlen(list *list)
{
	return list->maxlistlen;
}

void setmaxlistlen(list *list, list_t maxlen)
{
	list->maxlistlen = maxlen;
}

cbool empty(list *list)
{
	return ((list->curlistlen == 0) ? 1 : 0);
}

cbool push(list *list, void *data, int prio)
{
	if (full(list))
	{
		return FAILED;
	}

	listnode *node = (listnode *)malloc(sizeof(listnode));
	if (!node)
	{
		return FAILED;
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

		return SUCCESS;
	}

	//找到头指针
	listnode *head = list->head;

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

				return SUCCESS;
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

		return SUCCESS;
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

	return SUCCESS;
}

listnode *gethead(list *list)
{
	return list->head;
}

cbool full(list *list)
{
	return (((list->curlistlen < list->maxlistlen) || list->maxlistlen <= 0) ? 0 : 1);
}

cbool clearlist(list *list)
{
	while (!empty(list))
	{
		delnode(list, gethead(list));
	}
    
    list->curlistlen = 0;

	return SUCCESS;
}

cbool delnode(list *list, listnode *node)
{
	return delete(list, node);
}

