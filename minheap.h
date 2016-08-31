#ifndef MINHEAP_H
#define MINHEAP_H

#include "util.h"

typedef struct heapnode
{
	void *data;
	int pos;/*从1开始*/
} heapnode;

typedef int (*comparehenode)(struct heapnode *srchenode, struct heapnode *deshenode);

typedef struct minheap
{
	struct heapnode **head;
	int size;/*保存整个数组大小*/
	int cursize;/*当前保存的节点数*/
	int addsize;/*每次自动增加的节点数*/
    comparehenode compare;/*传递比较函数*/
} minheap;

minheap *createminheap(int size, comparehenode compare);
cbool addhn(minheap *heap, void *data);
cbool delhn(minheap *heap, void *data);
void *getminvalue(minheap *heap);
int getheapsize(minheap *heap);
cbool heapempty(minheap *heap);
cbool reverseminheap(minheap *heap, int size);
void *getvaluebyindex(minheap *heap, int index);
cbool destroyminheap(minheap *heap);

#endif