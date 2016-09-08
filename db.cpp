#include "db.h"
#include "mydb.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

db::db()
{
}

db::~db()
{
}

char *db::copystr(char *str)
{
    size_t size = strlen(str);
    char *nstr = (char *)cmalloc(size + 1);
    memcpy(nstr, str, size);
    nstr[size] = '\0';
    
    return nstr;
}

struct dbconn *db::copydbconn(struct dbconn *conn)
{
    struct dbconn *dbconn = cnew(struct dbconn);
    if (!dbconn)
    {
        return NULL;
    }
    
    dbconn->host = copystr(conn->host);
    dbconn->user = copystr(conn->user);
    dbconn->pass = copystr(conn->pass);
    dbconn->dbname = copystr(conn->dbname);
    dbconn->unixsock = copystr(conn->unixsock);
    dbconn->cliflag = conn->cliflag;
    dbconn->port = conn->port;
    
    return dbconn;
}

struct dbconn *db::createdbconn(char *host,
                                   char *user,
                                   char *pass,
                                   char *dbname,
                                   char *unixsock,
                                   unsigned long cliflag,
                                   unsigned int port)
{
    struct dbconn *dbconn = cnew(struct dbconn);
    if (!dbconn)
    {
        return NULL;
    }
    
    dbconn->host = copystr(host);
    dbconn->user = copystr(user);
    dbconn->pass = copystr(pass);
    dbconn->dbname = copystr(dbname);
    dbconn->unixsock = copystr(unixsock);
    dbconn->cliflag = cliflag;
    dbconn->port = port;

    return dbconn;
}

void db::destroydbconn(struct dbconn *conn)
{
    cfree(conn->host);
    cfree(conn->user);
    cfree(conn->pass);
    cfree(conn->dbname);
    cfree(conn->unixsock);
    cfree(conn);
}

/*根据数据库类型实例化数据库*/
db *db::insdbbytype(int dbtype)
{
    db *dbi = NULL;
    switch (dbtype)
    {
        case 1:
            dbi = new mydb();
            break;
            
        case 2:
            break;
            
        case 3:
            break;
            
        default:
            dbi = new mydb();
            break;
    }
    return dbi;
}
