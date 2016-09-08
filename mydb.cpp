#include "mydb.h"
#include "log.h"
#include <string.h>
#include <stdlib.h>

mydb::mydb()
{
	mysql = NULL;
	result = NULL;
}

mydb::~mydb()
{
}

int mydb::opendb(struct dbconn *conn)
{
	mysql = mysql_init(NULL);
	if (!mysql)
	{
        ploginfo(LERROR, "%s", geterror());
		return 0;
	}

	if (!mysql_real_connect(mysql, conn->host, conn->user,
							 conn->pass, conn->dbname, conn->port, conn->unixsock, conn->cliflag))
	{
        ploginfo(LERROR, "%s", geterror());
		closedb();
		return 0;
	}

	if (mysql_autocommit(mysql, 0) != 0)
	{
		ploginfo(LERROR, "%s", geterror());
		closedb();
		return 0;
	}

	return 1;
}

int mydb::querysql(char *sql)
{
	if (!sql)
	{
		return 0;
	}

	if (mysql_query(mysql, sql) != 0)
	{
        ploginfo(LERROR, "mydb::querysql exe query failed, sql=%s", sql);
		return 0;
	}

	return 1;
}

int mydb::modifysql(char *sql)
{
	if (!sql)
	{
		return 0;
	}

	if (mysql_real_query(mysql, sql, strlen(sql)) != 0)
	{
        ploginfo(LERROR, "mydb::modifysql mysql_real_query exe query failed, sql=%s, err=%s", sql, geterror());
		return 0;
	}

	if (mysql_commit(mysql) != 0)
	{
        ploginfo(LERROR, "mydb::modifysql mysql_commit failed, err=%s", sql, geterror());
		return 0;
	}

	return 1;
}

int mydb::modifysqlex(char **sqlarray, int size)
{
	if (!sqlarray || size <= 0)
	{
		return 0;
	}

	for (int i = 0; i < size; i++)
	{
		if (mysql_query(mysql, sqlarray[i]) != 0)
		{
            ploginfo(LERROR, "mydb::modifysqlex mysql_query exe query failed, sql=%s, err=%s", sqlarray[i], geterror());

			if (mysql_rollback(mysql) != 0)
			{
				ploginfo(LERROR, "mydb::modifysqlex mysql_rollback failed, err=%s", geterror());
			}
			return 0;
		}
	}

	if (mysql_commit(mysql) != 0)
	{
		return 0;
	}

	return 1;
}

int mydb::getrecordresult()
{
	result = mysql_store_result(mysql); 
	return (result) ? 1 : 0;
}

int mydb::nextrow()
{
	return 1;
}

void mydb::releaserecordresult()
{
	mysql_free_result(result);
}

unsigned long mydb::getrecordcount()
{
	return mysql_num_rows(result);
}

char *mydb::getstring(char *field)
{
	if (!field)
	{
		return NULL;
	}

	unsigned int cols = mysql_num_fields(result);
	MYSQL_FIELD *fields = mysql_fetch_fields(result);
	MYSQL_ROW rowresult = result->current_row;
	for (unsigned int i = 0; i < cols; i++)
	{
		if (strcmp(fields[i].name, field) == 0)
		{
			return rowresult[i];
		}
	}

	return NULL;
}

int mydb::getint(char *field)
{
	return atoi(getstring(field));
}

float mydb::getfloat(char *field)
{
	return (float)atof(getstring(field));
}

double mydb::getdouble(char *field)
{
	return atoi(getstring(field));
}

int mydb::iseof()
{
	MYSQL_ROW row = mysql_fetch_row(result);
	return (row == NULL) ? 0 : 1;
}

int mydb::offrecordresult(int off)
{
	mysql_data_seek(result, off);
	return 1;
}

int mydb::closedb()
{
	mysql_close(mysql);
	return 1;
}

unsigned long mydb::getaffectrow()
{
	return mysql_affected_rows(mysql);
}

int mydb::isactive()
{
	return (mysql_ping(mysql) == 0) ? 1 : 0;
}

const char *mydb::getexecuteresult()
{
	return mysql_info(mysql);
}

char *mydb::geterror()
{
	return (char *)mysql_error(mysql);
}

