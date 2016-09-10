#include "util.h"
#include "io.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>

int getmaxfilenumber()
{
	struct rlimit rlt;
	if (getrlimit(RLIMIT_NOFILE, &rlt) == 0)
	{
		return (int)rlt.rlim_cur;
	}

	return 0;
}

int setmaxfilenumber(int filenumber)
{
	struct rlimit rlt;
	if (getrlimit(RLIMIT_NOFILE, &rlt) == 0)
	{
		rlt.rlim_cur = (filenumber >= rlt.rlim_max) ? rlt.rlim_max : filenumber;
		if (setrlimit(RLIMIT_NOFILE, &rlt) == 0)
		{
			return 1;
		}
	}

	return 0;
}

int setcorefilesize(int filesize)
{
	struct rlimit rlt;
	if (getrlimit(RLIMIT_CORE, &rlt) == 0)
	{
		rlt.rlim_cur = (filesize >= rlt.rlim_max) ? rlt.rlim_max : filesize;
		if (setrlimit(RLIMIT_CORE, &rlt) == 0)
		{
			return 1;
		}
	}

	return 0;
}

int getcorefilesize()
{
	struct rlimit rlt;
	if (getrlimit(RLIMIT_CORE, &rlt) == 0)
	{
		return (int)rlt.rlim_cur;
	}

	return 0;
}

int getpidfromfile()
{
	int pid = -1;
	int fileno = -1;
	int flag = O_RDONLY;
	mode_t mode = S_IRUSR | S_IWUSR;
	if ((fileno = open("pid", flag, mode)) != -1)
	{
		char cpid[20];
		if (readline(fileno, cpid, 20) > 0)
		{
			pid = atoi(cpid);
		}

		closefile(fileno);
	}

	return pid;
}

int setpidtofile()
{
	int pid = getpid();
	int fileno = -1;
	int flag = O_WRONLY | O_CREAT;
	mode_t mode = S_IRUSR | S_IWUSR;
	if ((fileno = open("pid", flag, mode)) != -1)
	{
		/*清空文件*/
		clearfile(fileno);

		char cpid[20] = {0};
		sprintf(cpid, "%d", pid);
		if (writefile(fileno, cpid, (int)strlen(cpid)) <= 0)
		{
			printf("%s\n", "write pid to file failed!");
			closefile(fileno);
			return 0;
		}

		closefile(fileno);
	}
    
    printf("the process id is %d!\n", pid);

	return 1;
}

int getcpucorenum()
{
	#if defined(_WIN32)
		#error no support operate system
	#elif defined(__APPLE__) && defined(__MACH__)
		return (int)sysconf(_SC_NPROCESSORS_ONLN);
	#elif defined(__linux__) || defined(_AIX) || defined(__FreeBSD__)  
		return sysconf(_SC_NPROCESSORS_ONLN);
	#else
		#error no support operate system
	#endif
}

int timevalcompare(struct timeval *src, struct timeval *dest)
{
	/*0-相等 1-源<目的 -1源>目的*/
	return (src->tv_sec < dest->tv_sec) ? 1 :
			(src->tv_sec > dest->tv_sec) ? -1 : 
			(src->tv_usec < dest->tv_usec) ? 1 :
			(src->tv_usec > dest->tv_usec) ? -1 : 0;
}

char *ntos(int num)
{
    int len = 1;
    int tnum = num;
    while ((tnum = tnum / 10) > 0)
    {
        len++;
    }
    
    char *snum = (char *)cmalloc(len + 1);//+1保证负数
    sprintf(snum, "%d", num);
    
    return snum;
}

void freebyntos(char *str)
{
    cfree(str);
}

unsigned long strhash(char *str)
{
    if (str == NULL)
    {
        return 0;
    }
    
    unsigned long ret = 0;
    int l = (int)((strlen(str) + 1) / 2);
    unsigned short *s = (unsigned short *)str;
    for (int i = 0; i < l; i++)
    {
        ret ^= (s[i] << (i & 0x0f));
    }
    
    return ret; 
}

void *createshare(key_t key, size_t size, int *shid)
{
    int nFlag = 0644 | IPC_CREAT;
    
    *shid = shmget(key, size, nFlag);
    if (*shid == -1)
    {
        return NULL;
    }
    
    void *mem = shmat(*shid, NULL, 0);
    return (mem == (void *)-1) ? NULL : mem;
}

int destroyshare(int shareid, void *mem)
{
    if (shmdt(mem) != 0 || shmctl(shareid, IPC_RMID, 0) != 0)
    {
        return 0;
    }
    
    return 1;
}

char *jsonstringform(cJSON *jsons, char *name)
{
    char *value = cJSON_Print(cJSON_GetObjectItem(jsons, name));
    if (value == NULL)
    {
        return NULL;
    }
    
    long len = strlen(value);
    
    //判断字符串合法性
    if (value[0] != '\"' || value[len - 1] != '\"')
    {
        return value;
    }
    
    int index = 0;
    while (index < len - 1)
    {
        value[index] = value[index + 1];
        index++;
    }
    
    value[index - 1] = '\0';
    
    return value;
}
