#include "minheap.h"
#include <stdlib.h>
#include <string.h>

static cbool moveup(minheap *heap, heapnode *node)
{
	if (node->pos == 1)
	{
		return SUCCESS;
	}

	heapnode *parnode = heap->head[(node->pos / 2) - 1];
	if (heap->compare(node, parnode) == 1)
	{
		void *data = node->data;
		node->data = parnode->data;
		parnode->data = data;

		return moveup(heap, parnode);
	}

	return SUCCESS;
}

static cbool movedown(minheap *heap, heapnode *node)
{
	if (node->pos > heap->cursize ||
		node->pos * 2 > heap->cursize || 
		node->pos * 2 + 1 > heap->cursize)
	{
		return SUCCESS;
	}

    //比较左右结点
	heapnode *comnode = (heap->compare(heap->head[(node->pos * 2) - 1],
						 heap->head[node->pos * 2]) != -1) ?
	 			heap->head[(node->pos * 2) - 1] : heap->head[node->pos * 2];
    
    //比较当前结点和左右中最小节点
	if (heap->compare(node, comnode) == -1)
	{
		void *data = node->data;
		node->data = comnode->data;
		comnode->data = data;

		return movedown(heap, comnode);
	}

	return SUCCESS;
}

minheap *createminheap(int size, comparehenode compare)
{
	struct minheap *heap = (struct minheap *)malloc(sizeof(struct minheap));
	if (!heap)
	{
		return NULL;
	}

    int hsize = sizeof(struct heapnode *) * size;
	heap->head = (struct heapnode **)malloc(hsize);
	if (!heap->head)
	{
		free(heap);
		return NULL;
	}
    memset(heap->head, 0, hsize);

	heap->size = size;
	heap->cursize = 0;
	heap->addsize = 10;
    heap->compare = compare;

	return heap;
}

cbool addhn(minheap *heap, void *data)
{
	if (heap->size < heap->cursize + 1)
	{
		if (reverseminheap(heap, heap->size + heap->addsize) == FAILED)
		{
			return FAILED;
		}
	}

	struct heapnode *henode = (struct heapnode *)malloc(sizeof(struct heapnode));
	if (!henode)
	{
		return FAILED;
	}

	//将节点插入到完全二叉树末尾
	heap->head[heap->cursize++] = henode;
	henode->data = data;
	henode->pos = heap->cursize;

	/*将节点移动到合适的位置*/
	return moveup(heap, henode);
}

cbool delhn(minheap *heap, void *data)
{
	for (int i = 0; i < heap->cursize; i++)
	{
		if (heap->head[i]->data == data)
		{
			heap->head[i]->data = heap->head[--heap->cursize]->data;
			int ret = movedown(heap, heap->head[i]);
			free(heap->head[heap->cursize]);
			return ret;
		}
	}

	return FAILED;
}

void *getminvalue(minheap *heap)
{
	return (heap->head[0]) ? (heap->head[0]->data) : NULL;
}

int getheapsize(minheap *heap)
{
	return heap->cursize;
}

cbool heapempty(minheap *heap)
{
	return (heap->cursize == 0) ? 1 : 0;
}

cbool reverseminheap(minheap *heap, int size)
{
	if (size <= heap->size)
	{
		return FAILED;
	}

	struct heapnode **head = (struct heapnode **)malloc(sizeof(struct heapnode *) * size);
	if (!heap->head)
	{
		return FAILED;
	}

	for (int i = 0; i < heap->cursize; i++)
	{
		head[i] = heap->head[i];
	}
	free(heap->head);

	heap->head = head;
	heap->size = size;

	return SUCCESS;
}

void *getvaluebyindex(minheap *heap, int index)
{
	return heap->head[index]->data;
}

cbool destroyminheap(minheap *heap)
{
	for (int i = 0; i < heap->cursize; i++)
	{
		free(heap->head[i]);
	}

	free(heap->head);
	free(heap);

	return SUCCESS;
}