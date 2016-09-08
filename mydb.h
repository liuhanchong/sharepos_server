#ifndef MYDB_H
#define MYDB_H

#include "db.h"
#include <mysql.h>

class mydb : public db
{
public:
	mydb();
	~mydb();

public:
	int opendb(struct dbconn *conn);/*打开数据库*/
	int querysql(char *sql);/*查询数据*/
	int modifysql(char *sql);/*修改数据*/
	int modifysqlex(char **sqlarray, int size);/*拓展的修改数据*/
	int getrecordresult();/*获取结果集*/
	int nextrow();/*获取下一行*/
	void releaserecordresult();/*释放结果集*/
	unsigned long getrecordcount();/*获取结果数量*/
	char *getstring(char *field);/*获取字符串值*/
	int getint(char *field);/*获取int值*/
	float getfloat(char *field);/*获取float值*/
	double getdouble(char *field);/*获取double值*/
	int iseof();/*结果集是否到了末尾*/
	int offrecordresult(int off);/*偏移结果集*/
	int closedb();/*关闭数据库*/

public:
	int isactive();/*数据库通讯是否正常*/
	const char *getexecuteresult();/*获取执行一条sql的结果*/
	char *geterror();/*获取错误信息*/

private:
	unsigned long getaffectrow();/*sql操作影响的记录数*/

private:
	struct dbconn dbconn;
	MYSQL *mysql;
	MYSQL_RES *result;

};

#endif /* MYDB_H */
