#ifndef DB_H
#define DB_H

extern "C"
{
#include "util.h"
}
#include <stdlib.h>
#include <string.h>

typedef struct dbconn
{
	char *host;
	char *user;
	char *pass;
	char *dbname;
	char *unixsock;
	unsigned long cliflag;
	unsigned int port;
} dbconn;

class db
{
public:
	db();
	virtual ~db();

public:
	virtual cbool opendb(struct dbconn *conn) = 0;/*打开数据库*/
	virtual cbool querysql(char *sql) = 0;/*查询数据*/
	virtual cbool modifysql(char *sql) = 0;/*修改数据*/
	virtual cbool modifysqlex(char **sqlarray, int size) = 0;/*拓展的修改数据*/
	virtual cbool getrecordresult() = 0;/*获取结果集*/
	virtual cbool nextrow() = 0;/*获取下一行*/
	virtual void releaserecordresult() = 0;/*释放结果集*/
	virtual unsigned long getrecordcount() = 0;/*获取结果数量*/
	virtual char *getstring(char *field) = 0;/*获取字符串值*/
	virtual int getint(char *field) = 0;/*获取int值*/
	virtual float getfloat(char *field) = 0;/*获取float值*/
	virtual double getdouble(char *field) = 0;/*获取double值*/
	virtual cbool iseof() = 0;/*结果集是否到了末尾*/
	virtual cbool offrecordresult(int off) = 0;/*偏移结果集*/
	virtual cbool closedb() = 0;/*关闭数据库*/
    
    /*根据数据库类型实例化数据库*/
    static db *insdbbytype(int dbtype);
    static struct dbconn *createdbconn(char *host,
                                       char *user,
                                       char *pass,
                                       char *dbname,
                                       char *unixsock,
                                       unsigned long cliflag,
                                       unsigned int port);
    static struct dbconn *copydbconn(struct dbconn *conn);
    static void destroydbconn(struct dbconn *conn);

private:
    static char *copystr(char *str);

private:

};

#endif /* DB_H */
