#ifndef DBPOOL_H
#define DBPOOL_H

#include "db.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

struct dbnode
{
	struct dbconn *conn;
	db *dbi;/*DB实例*/
	int use;/*使用标志 0-未使用 1-正在使用*/
};

struct dbpool
{
	list *dblist;
	int maxdbnum;/*最大db数*/
	int coredbnum;/*核心的db数*/
	int dbtype;/*1-mysql 2-oracle 3-sqlserver 4-...*/
};

struct dbpool *createdbpool(int dbtype, int maxdbnum, int coredbnum, struct dbconn *conn);
int destroydbpool(struct dbpool *dbpool);
struct dbnode *getdb(struct dbpool *dbpool);/*获取一个未使用的连接*/
void reldb(struct dbpool *dbpool, struct dbnode *dbnode);/*释放不使用的db*/
    
#ifdef __cplusplus
}
#endif

#endif
