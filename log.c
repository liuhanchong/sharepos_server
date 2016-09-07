#include "io.h"
#include "log.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static slog *serverlog = NULL;

/*根据类型标示获取类型字符串*/
static char *getlogtype(slogtype type)
{
    char *TYPE = NULL;
    switch (type)
    {
        case LDEBUG:
            TYPE = "debug";
            break;
            
        case LERROR:
            TYPE = "error";
            break;
            
        case LDUMP:
            TYPE = "dump";
            break;
            
        case LOTHER:
            TYPE = "other";
            break;
            
        default:
            TYPE = "error";
            break;
    }

    unsigned long slen = strlen(TYPE);
    char *stype = (char *)malloc(slen + 1);
    memcpy(stype, TYPE, slen);
    stype[slen] = '\0';
    
    return stype;
}

static void freelogtype(char *logtype)
{
    free(logtype);
}

static char *timetostr(char *format)
{
    int slen = 21;
    char *stime = malloc(slen);
    
	time_t curtime = time(NULL);
	struct tm *strutm = localtime(&curtime);
	strftime(stime, slen, format, strutm);
    
    return stime;
}

static void freetimestr(char *stime)
{
    free(stime);
}

/*生成文件名*/
static cbool genfilename(slog *log, char *name, int size, slogtype type)
{
	memset(name, 0, size);

    char *logtype = getlogtype(type);
	char *stime = timetostr("%Y-%m-%d %H:%M:%S");

	//组合文件名
    snprintf(name, size, "%s/%s-%s.log", log->logdir, logtype, stime);
    
    freelogtype(logtype);
    freetimestr(stime);
    
    return SUCCESS;
}

/*生成日志文件*/
static int genlogfile(slog *log, slogtype type, int fnameindex)
{
    /*生成文件名*/
    char *name = (char *)malloc(LOGNAMELEN);
    genfilename(log, name, LOGNAMELEN, type);
    
	/*保存最新生成的文件名*/
    if (log->fnamearray[fnameindex])
    {
        free(log->fnamearray[fnameindex]);
        closefile(log->fdarray[fnameindex]);
    }
    log->fnamearray[fnameindex] = name;

    /*此处必须为创建新的不存在文件*/
	return openfile(name, O_WRONLY | O_CREAT | O_EXCL | O_APPEND, S_IRUSR | S_IWUSR);
}

/*合并日志信息*/
static int combinelog(slogtype type, char *log, int logsize,
                      va_list arg_list, const char *format)
{
    char *stime = timetostr("%H:%M:%S");
    char *logtype = getlogtype(type);
    
    int loglen = 0;
    char info[logsize];
    if (vsnprintf(info, logsize, format, arg_list) > 0)
    {
        loglen = snprintf(log, logsize, "%s(%s):%s\r\n", logtype, stime, info);
    }
    
    freelogtype(logtype);
    freetimestr(stime);
    
    return loglen;
}

/*判断文件是否删除*/
static int isdelfile(slog *log, slogtype type)
{
    int fnameindex = type;
    return (!existfile(log->fnamearray[fnameindex]));
}

static void gendir(const char *logdir)
{
    //判断日志目录是都存在
    DIR *dir = opendir(logdir);
    (dir == NULL) ? mkdir(logdir, S_IRUSR | S_IWUSR | S_IXUSR) :
    closedir(dir);
}

/*生成删除的日志文件*/
static cbool gendelfile(slog *log, slogtype type)
{
    if (isdelfile(log, type))
    {
        //判断日志目录是都存在
        gendir(log->logdir);
        
        int fdindex = type;
        
        //生成新的文件
        int tempfileno = -1;
        if ((tempfileno = genlogfile(log, type, fdindex)) == -1)
        {
            return FAILED;
        }
        
        //成功后才将最新的文件描述符保存
        log->fdarray[fdindex] = tempfileno;
    }
    
    return SUCCESS;
}

static cbool writelog(slog *slog, slogtype type, const char *log, int size)
{
    /*如果日志文件被删除 生成新的日志文件*/
    gendelfile(slog, type);

    /*目前依据enum的值作为对应错误文件名和fd的索引*/
    int fdindex = type;
    int fileno = slog->fdarray[fdindex];
    
    if (writefile(fileno, log, size) != size)
    {
        return FAILED;
    }
    
    fsync(fileno);
    
    //日志文件过大
    if (filelen(fileno) >= LOGFILEMAXSIZE)
    {
        int tempfileno = -1;
        if ((tempfileno = genlogfile(slog, type, fdindex)) == -1)
        {
            return FAILED;
        }
        
        //成功后才将最新的文件描述符保存
        slog->fdarray[fdindex] = tempfileno;
    }
    
    return SUCCESS;
}

/*初始化log*/
slog *createlog()
{
    struct slog *log = (struct slog *)malloc(sizeof(slog));
    if (!log)
    {
        return NULL;
    }
    
    memset(log, 0, sizeof(slog));
    
    //保存全局的server日志
    serverlog = log;
    
    char *logdir = "slog";/*给定日志文件夹名*/
    unsigned long ldlen = strlen(logdir);
    log->logdir = malloc(ldlen + 1);
    if (!log->logdir)
    {
        return NULL;
    }
    memcpy(log->logdir, logdir, ldlen);
    log->logdir[ldlen] = '\0';

	//判断日志目录是都存在
    gendir(log->logdir);
    
    //生成不同类型的文件
	for (slogtype type = LDEBUG; type <= LDUMP; type++)
	{
		if ((log->fdarray[type] = genlogfile(log, type, type)) == -1)
		{
			return NULL;
		}
	}

	/*初始互斥量*/
	pthread_mutexattr_t mutexattr;
	if (pthread_mutexattr_init(&mutexattr) != 0 ||
        pthread_mutex_init(&log->logmutex, &mutexattr) != 0)
	{
		return NULL;
	}
	pthread_mutexattr_destroy(&mutexattr);
	
	/*日志模块运行*/
	log->run = 1;

	return log;
}

/*释放log*/
cbool destroylog(slog *log)
{
	log->run = 0;

	for (int i = 0; i < LOGTYPENUM; i++)
	{
        free(log->fnamearray[i]);
        if (closefile(log->fdarray[i]) != 0)
        {
            return FAILED;
        }
	}
    
    free(log->logdir);
    
	if (pthread_mutex_destroy(&log->logmutex) != 0)
	{
		return FAILED;
	}

	return SUCCESS;
}

/*打印日志信息*/
void ploginfo(slogtype logtype, const char *format, ...)
{
    //对于debug日志 如果未定义打印宏 不打印信息
    if (logtype == LDEBUG)
    {
        #ifndef PRINTDEBUG
            return;
        #endif
    }
    
    int loglen = 0;
    char log[LOGSIZE];
    va_list arg_list;
    va_start(arg_list, format);
    loglen = combinelog(logtype, log, LOGSIZE, arg_list, format);
    va_end(arg_list);
    
    if (writelog(serverlog, logtype, log, loglen) != SUCCESS)
    {
        printf("写入%d日志信息失败\n", logtype);
    }
    
    //对于崩溃日志 系统自动退出
    if (logtype == LDUMP)
    {
        exit(1);
    }
}

/*打印系统错误信息*/
void ploginfoerrno(const char *fun, errno_t errorno)
{
    slogtype logtype = LERROR;
    
    char *perror = strerror(errorno);
	if (!perror)
	{
		return;
	}

	char log[LOGSIZE];
    char *stime = timetostr("%H:%M:%S");
	int size = snprintf(log, LOGSIZE, "error_errno(%s):%s->%s\r\n", stime, fun, perror);
    freetimestr(stime);

	if (writelog(serverlog, logtype, log, size) != SUCCESS)
    {
        printf("写入%d日志信息失败\n", logtype);
    }
}
