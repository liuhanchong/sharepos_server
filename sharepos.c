#include "util.h"
#include "pyinter.h"
#include "event.h"
#include "http.h"
#include <stdio.h>

typedef struct spserver
{
    struct sysc sysc;
    struct slog *log;
    struct httpserver *http;
    struct eventtop etlist[SYSEVMODENUM]; /*按照优先级保存模型*/
} spserver;

/*全局服务器配置*/
struct spserver server;

void *closesys(void *event, void *arg)
{
    ploginfo(LDEBUG, "closesys");
    
    struct reactor *reactor = (struct reactor *)arg;
    reactor->listen = 0;
    
    return NULL;
}

int main(int argc, const char *argv[])
{
    memset(&server, 0, sizeof(spserver));
    
    //关闭服务器命令
    if (argc == 2 && (strcmp(argv[1], "stop") == 0))
    {
        //读取进程信息
        int pid = getpidfromfile();
        if (pid == -1)
        {
            printf("%s\n", "close pro failed, invalid pid!");
            return 0;
        }
        
        //向进程发送信号
        if (kill(pid, SIGINT) == -1)
        {
            printf("close pro failed, pid=%d!\n", pid);
            return 0;
        }
    
        printf("close pro success, pid=%d!\n", pid);
        
        return 0;
    }
    
    //保存当前进程信息
    printf("the process id is %d!\n", getpid());
    setpidtofile();
    
    /*创建事件模型*/
    createeventtop(server.etlist);

    //打开日志
    if (!(server.log = createlog()))
    {
        printf("create log failed\n");
        return 1;
    }

    //获取系统配置
    if (getsyscon("./server.ini", &server.sysc) == SUCCESS)
    {
        ploginfo(LDEBUG, "ip=%s port=%d", server.sysc.ip, server.sysc.port);
    }
    
    //创建http服务器模块
    server.http = createhttp(server.etlist, (char *)server.sysc.ip, server.sysc.port);
    if (server.http == NULL)
    {
        ploginfo(LERROR, "main->createhttp failed");
        return 1;
    }

    //添加关闭服务器信号处理
    struct event *uevent = setsignal(server.http->reactor, SIGINT, EV_SIGNAL, closesys, server.http->reactor);
    if (addsignal(uevent) == SUCCESS)
    {
        ploginfo(LDEBUG, "%s", "addsignal ok");
    }
    
    //服务器主体逻辑
    if (dispatchhttp(server.http) == FAILED)
    {
        ploginfo(LDEBUG, "main->dispatchevent failed");
        return 1;
    }
    
    if (!destroyhttp(server.http))
    {
        ploginfo(LDEBUG, "main->destroyhttp failed");
        return 1;
    }
    
    ploginfo(LDEBUG, "main->destroyhttp succeess");
    
    if (!destroylog(server.log))
    {
        printf("destroy log failed\n");
        return 1;
    }
    
    printf("\r\n main run stop\n");
    
    return 0;
}
