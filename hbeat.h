#ifndef HBEAT_H
#define HBEAT_H

#include "htable.h"
#include "thread.h"

#ifdef __cplusplus
extern "C" {
#endif

struct hbnode
{
    int fd;
    int failed;//失败次数
    int state;//0-无效 1-有效
    time_t time;//最后发送心跳时间
};

struct heartbeat
{
    struct hashtable *hbtable;/*保存心跳信息*/
	struct thread *hbthread;/*监听fd并标记失败数*/
	int outtime;/*超时时间 单位:s*/
	int outcount;/*最多超时次数*/
	struct reactor *reactor;/*保存其对应的反应堆*/
};

struct heartbeat *createheartbeat(int connnum, int outtime);/*创建心跳监听*/
int delheartbeat(struct heartbeat *hebeat, int fd);/*删除一个心跳*/
int addheartbeat(struct heartbeat *hebeat, int fd);/*添加一个心跳*/
int upheartbeat(struct heartbeat *hebeat, int fd);/*更新心跳*/
int destroyheartbeat(struct heartbeat *hebeat);/*释放心跳监听*/
    
#ifdef __cplusplus
}
#endif

#endif