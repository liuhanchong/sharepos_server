#include "minheap.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

static int moveup(struct minheap *heap, struct heapnode *node)
{
	if (node->pos == 1)
	{
		return 1;
	}

	struct heapnode *parnode = heap->head[(node->pos / 2) - 1];
	if (heap->compare(node, parnode) == 1)
	{
		void *data = node->data;
		node->data = parnode->data;
		parnode->data = data;

		return moveup(heap, parnode);
	}

	return 1;
}

static int movedown(struct minheap *heap, struct heapnode *node)
{
	if (node->pos > heap->cursize ||
		node->pos * 2 > heap->cursize || 
		node->pos * 2 + 1 > heap->cursize)
	{
		return 1;
	}

    //比较左右结点
	struct heapnode *comnode = (heap->compare(heap->head[(node->pos * 2) - 1],
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

	return 1;
}

struct minheap *createminheap(int size, comparehenode compare)
{
	struct minheap *heap = cnew(struct minheap);
	if (!heap)
	{
		return NULL;
	}

    int hsize = sizeof(struct heapnode *) * size;
	heap->head = (struct heapnode **)cmalloc(hsize);
	if (!heap->head)
	{
		cfree(heap);
		return NULL;
	}
    memset(heap->head, 0, hsize);

	heap->size = size;
	heap->cursize = 0;
	heap->addsize = 10;
    heap->compare = compare;

	return heap;
}

int addhn(struct minheap *heap, void *data)
{
	if (heap->size < heap->cursize + 1)
	{
		if (reverseminheap(heap, heap->size + heap->addsize) == 0)
		{
			return 0;
		}
	}

	struct heapnode *henode = cnew(struct heapnode);
	if (!henode)
	{
		return 0;
	}

	//将节点插入到完全二叉树末尾
	heap->head[heap->cursize++] = henode;
	henode->data = data;
	henode->pos = heap->cursize;

	/*将节点移动到合适的位置*/
	return moveup(heap, henode);
}

int delhn(struct minheap *heap, void *data)
{
	for (int i = 0; i < heap->cursize; i++)
	{
		if (heap->head[i]->data == data)
		{
			heap->head[i]->data = heap->head[--heap->cursize]->data;
			int ret = movedown(heap, heap->head[i]);
			cfree(heap->head[heap->cursize]);
			return ret;
		}
	}

	return 0;
}

void *getminvalue(struct minheap *heap)
{
	return (heap->head[0]) ? (heap->head[0]->data) : NULL;
}

int getheapsize(struct minheap *heap)
{
	return heap->cursize;
}

int heapempty(struct minheap *heap)
{
	return (heap->cursize == 0) ? 1 : 0;
}

int reverseminheap(struct minheap *heap, int size)
{
	if (size <= heap->size)
	{
		return 0;
	}

	struct heapnode **head = (struct heapnode **)cmalloc(sizeof(struct heapnode *) * size);
	if (!heap->head)
	{
		return 0;
	}

	for (int i = 0; i < heap->cursize; i++)
	{
		head[i] = heap->head[i];
	}
	cfree(heap->head);

	heap->head = head;
	heap->size = size;

	return 1;
}

void *getvaluebyindex(struct minheap *heap, int index)
{
	return heap->head[index]->data;
}

int destroyminheap(struct minheap *heap)
{
	for (int i = 0; i < heap->cursize; i++)
	{
		cfree(heap->head[i]);
	}

	cfree(heap->head);
	cfree(heap);

	return 1;
}