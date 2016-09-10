#include "util.h"
#include "log.h"
#include "pyinter.h"
#include "event.h"
#include "http.h"
#include <stdio.h>

struct spserver
{
    struct sysc sysc;
    struct slog *log;
    struct http *http;
    struct eventtop etlist[SYSEVMODENUM]; /*按照优先级保存模型*/
};

/*全局服务器配置*/
struct spserver server;

int main(int argc, const char *argv[])
{
    memset(&server, 0, sizeof(struct spserver));
    
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
        if (kill(pid, SIGTERM) == -1)
        {
            printf("close pro failed, pid=%d!\n", pid);
            return 0;
        }
    
        printf("close pro success, pid=%d!\n", pid);
        
        return 0;
    }
    
    //保存当前进程信息
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
    if (getsyscon("./server.ini", &server.sysc) == 0)
    {
        ploginfo(LERROR, "main->getsyscon failed");
        return 1;
    }
    ploginfo(LDEBUG, "ip=%s port=%d", server.sysc.ip, server.sysc.port);
    
    //创建http服务器模块
    server.http = createhttp(server.etlist, (char *)server.sysc.ip, server.sysc.port);
    if (server.http == NULL)
    {
        ploginfo(LERROR, "main->createhttp failed");
        return 1;
    }

    //服务器主体逻辑
    if (dispatchhttp(server.http) == 0)
    {
        ploginfo(LERROR, "main->dispatchevent failed");
        return 1;
    }
    
    //关闭http服务器
    if (!destroyhttp(server.http))
    {
        ploginfo(LDEBUG, "main->destroyhttp failed");
        return 1;
    }
    
    ploginfo(LDEBUG, "main->destroyhttp succeess");
    
    //释放日志
    if (!destroylog(server.log))
    {
        printf("destroy log failed\n");
        return 1;
    }
    
    printf("\r\n main run stop\n");
    
    return 0;
}
