#include "io.h"
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

#define KB 1024
#define MB ((1024) * KB)

/*打开文件*/
int openfile(const char *path, int flag, mode_t mode)
{
	return open(path, flag, mode);
}

/*复制一个文件描述符，共享一个文件句柄*/
int dupfile(const int fileno)
{
	return dup(fileno);
}

/*删除文件*/
int rmfile(const char *path)
{
	return unlink(path);
}

/*关闭描述符*/
int closefile(int fileno)
{
	return close(fileno);
}

/*读取一行*/
unsigned long readline(int fileno, char *text, int size)
{
	unsigned long readsize = readfile(fileno, text, size);
	if (readsize <= 0)
	{
		return readsize;
	}

	char *find = strchr(text, '\n');
	if (find)
	{
		readsize = find - text;
		*find = '\0';

		//读取得数据多于一行 将其指针返回
		lseek(fileno, (-(size - readsize - 1)), SEEK_CUR);
	}

	return readsize;
}

/*读数据*/
unsigned long readfile(int fileno, char *text, int size)
{
	return read(fileno, text, size);
}

/*从头跳转指定位置*/
off_t offsethead(int fileno, off_t off)
{
	return lseek(fileno, off, SEEK_SET);
}

/*从当前位置跳转*/
off_t offsetcur(int fileno, off_t off)
{
	return lseek(fileno, off, SEEK_CUR);
}

/*获取文件大小,计算单位为（M）*/
unsigned long filelen(int fileno)
{
	struct stat stats;
	if (fstat(fileno, &stats) == 0)
	{
		return (stats.st_size / MB);
	}
	return -1;
}

/*写入文件*/
unsigned long writefile(int fileno, const char *text, int size)
{
	return write(fileno, text, size);
}

/*获取文件选项*/
int getfcntl(int fileno)
{
	return fcntl(fileno, F_GETFL);
}

/*判断文件是否存在*/
int existfile(char *path)
{
	return (access(path, F_OK) == 0) ? 1 : 0;
}

/*设置文件长度*/
int setfilelen(int fileno, int len)
{
	return ftruncate(fileno, len);
}

/*清空文件*/
int clearfile(int fileno)
{
	return setfilelen(fileno, 0);
}