#ifndef LOG_H
#define LOG_H

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOGTYPENUM (LDUMP + 1) /*日志的分类 debug error dump等*/
#define LOGSIZE 512 /*每条日志信息最大长度*/
#define LOGNAMELEN 256 /*日志路径最大长度*/
#define LOGFILEMAXSIZE 5 /*每个日志文件最大的长度（MB）*/

/*增加新类型时候，从LDEBUG和LDUMP中间位置增加*/
typedef enum slogtype
{
    LDEBUG = 0,/*调试日志*/
    LERROR,/*错误日志*/
    LOTHER,/*其他日志*/
    LDUMP/*崩溃日志*/
} slogtype;

/*日志模块的配置信息结构体*/
struct slog
{
    char *logdir;/*保存日志目录*/
    int fdarray[LOGTYPENUM]; /*保存日志文件类型的文件描述符 0-debug 1-error 2-other 3-dump*/
    char *fnamearray[LOGTYPENUM];/*保存当前的文件名*/
    int run; /*运行状态*/
    pthread_mutex_t logmutex;/*日志锁*/
};

/*初始化log*/
struct slog *createlog();

/*释放log*/
int destroylog(struct slog *log);

/*打印日志信息*/
void ploginfo(slogtype logtype, const char *format, ...);

/*打印系统错误信息*/
void ploginfoerrno(const char *fun, int errorno);
    
#ifdef __cplusplus
}
#endif

#endif

