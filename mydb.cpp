#include "mydb.h"
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

cbool mydb::opendb(struct dbconn *conn)
{
	mysql = mysql_init(NULL);
	if (!mysql)
	{
        ploginfo(LERROR, "%s", geterror());
		return FAILED;
	}

	if (!mysql_real_connect(mysql, conn->host, conn->user,
							 conn->pass, conn->dbname, conn->port, conn->unixsock, conn->cliflag))
	{
        ploginfo(LERROR, "%s", geterror());
		closedb();
		return FAILED;
	}

	if (mysql_autocommit(mysql, 0) != 0)
	{
		ploginfo(LERROR, "%s", geterror());
		closedb();
		return FAILED;
	}

	return SUCCESS;
}

cbool mydb::querysql(char *sql)
{
	if (!sql)
	{
		return FAILED;
	}

	if (mysql_query(mysql, sql) != 0)
	{
        ploginfo(LERROR, "mydb::querysql exe query failed, sql=%s", sql);
		return FAILED;
	}

	return SUCCESS;
}

cbool mydb::modifysql(char *sql)
{
	if (!sql)
	{
		return FAILED;
	}

	if (mysql_real_query(mysql, sql, strlen(sql)) != 0)
	{
        ploginfo(LERROR, "mydb::modifysql mysql_real_query exe query failed, sql=%s, err=%s", sql, geterror());
		return FAILED;
	}

	if (mysql_commit(mysql) != 0)
	{
        ploginfo(LERROR, "mydb::modifysql mysql_commit failed, err=%s", sql, geterror());
		return FAILED;
	}

	return SUCCESS;
}

cbool mydb::modifysqlex(char **sqlarray, int size)
{
	if (!sqlarray || size <= 0)
	{
		return FAILED;
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
			return FAILED;
		}
	}

	if (mysql_commit(mysql) != 0)
	{
		return FAILED;
	}

	return SUCCESS;
}

cbool mydb::getrecordresult()
{
	result = mysql_store_result(mysql); 
	return (result) ? SUCCESS : FAILED;
}

cbool mydb::nextrow()
{
	return SUCCESS;
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

cbool mydb::iseof()
{
	MYSQL_ROW row = mysql_fetch_row(result);
	return (row == NULL) ? 0 : 1;
}

cbool mydb::offrecordresult(int off)
{
	mysql_data_seek(result, off);
	return SUCCESS;
}

cbool mydb::closedb()
{
	mysql_close(mysql);
	return SUCCESS;
}

unsigned long mydb::getaffectrow()
{
	return mysql_affected_rows(mysql);
}

cbool mydb::isactive()
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

