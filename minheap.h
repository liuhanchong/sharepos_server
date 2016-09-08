#ifndef MINHEAP_H
#define MINHEAP_H

#ifdef __cplusplus
extern "C" {
#endif

struct heapnode
{
	void *data;
	int pos;/*从1开始*/
};

typedef int (*comparehenode)(struct heapnode *srchenode, struct heapnode *deshenode);

struct minheap
{
	struct heapnode **head;
	int size;/*保存整个数组大小*/
	int cursize;/*当前保存的节点数*/
	int addsize;/*每次自动增加的节点数*/
    comparehenode compare;/*传递比较函数*/
};

struct minheap *createminheap(int size, comparehenode compare);
int addhn(struct minheap *heap, void *data);
int delhn(struct minheap *heap, void *data);
void *getminvalue(struct minheap *heap);
int getheapsize(struct minheap *heap);
int heapempty(struct minheap *heap);
int reverseminheap(struct minheap *heap, int size);
void *getvaluebyindex(struct minheap *heap, int index);
int destroyminheap(struct minheap *heap);
    
#ifdef __cplusplus
}
#endif

#endif