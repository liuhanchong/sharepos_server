#ifndef DB_H
#define DB_H

#ifdef __cplusplus
extern "C" {
#endif
    
struct dbconn
{
	char *host;
	char *user;
	char *pass;
	char *dbname;
	char *unixsock;
	unsigned long cliflag;
	unsigned int port;
};
    
#ifdef __cplusplus
}
#endif

class db
{
public:
	db();
	virtual ~db();

public:
	virtual int opendb(struct dbconn *conn) = 0;/*打开数据库*/
	virtual int querysql(char *sql) = 0;/*查询数据*/
	virtual int modifysql(char *sql) = 0;/*修改数据*/
	virtual int modifysqlex(char **sqlarray, int size) = 0;/*拓展的修改数据*/
	virtual int getrecordresult() = 0;/*获取结果集*/
	virtual int nextrow() = 0;/*获取下一行*/
	virtual void releaserecordresult() = 0;/*释放结果集*/
	virtual unsigned long getrecordcount() = 0;/*获取结果数量*/
	virtual char *getstring(char *field) = 0;/*获取字符串值*/
	virtual int getint(char *field) = 0;/*获取int值*/
	virtual float getfloat(char *field) = 0;/*获取float值*/
	virtual double getdouble(char *field) = 0;/*获取double值*/
	virtual int iseof() = 0;/*结果集是否到了末尾*/
	virtual int offrecordresult(int off) = 0;/*偏移结果集*/
	virtual int closedb() = 0;/*关闭数据库*/
    
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
