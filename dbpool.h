#ifndef DBPOOL_H
#define DBPOOL_H

#include "db.h"
#include "mydb.h"
extern "C"
{
#include "list.h"
}

typedef struct dbnode
{
	struct dbconn *conn;
	db *dbi;/*DB实例*/
	int use;/*使用标志 0-未使用 1-正在使用*/
} dbnode;

typedef struct dbpool
{
	list *dblist;
	int maxdbnum;/*最大db数*/
	int coredbnum;/*核心的db数*/
	int dbtype;/*1-mysql 2-oracle 3-sqlserver 4-...*/
} dbpool;

dbpool *createdbpool(int dbtype, int maxdbnum, int coredbnum, dbconn *conn);
cbool destroydbpool(dbpool *dbpool);
cbool adddb(dbpool *dbpool, int adddbnum, struct dbconn *conn);/*添加db*/
dbnode *getdb(dbpool *dbpool);/*获取一个未使用的连接*/
void reldb(dbpool *dbpool, dbnode *dbnode);/*释放不使用的db*/

#endif
