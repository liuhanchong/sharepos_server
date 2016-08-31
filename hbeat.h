#ifndef HBEAT_H
#define HBEAT_H

#include "util.h"
#include "htable.h"
#include "thread.h"

typedef struct hbnode
{
    int fd;
    int failed;//失败次数
    int state;//0-无效 1-有效
    time_t time;//最后发送心跳时间
} hbnode;

typedef struct heartbeat
{
    hashtable *hbtable;/*保存心跳信息*/
	thread *hbthread;/*监听fd并标记失败数*/
	int outtime;/*超时时间 单位:s*/
	int outcount;/*最多超时次数*/
	struct reactor *reactor;/*保存其对应的反应堆*/
} heartbeat;

heartbeat *createheartbeat(int connnum, int outtime);/*创建心跳监听*/
cbool delheartbeat(heartbeat *hebeat, int fd);/*删除一个心跳*/
cbool addheartbeat(heartbeat *hebeat, int fd);/*添加一个心跳*/
cbool upheartbeat(heartbeat *hebeat, int fd);/*更新心跳*/
cbool destroyheartbeat(heartbeat *hebeat);/*释放心跳监听*/

#endif