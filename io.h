#ifndef IO_H
#define IO_H

#include <fcntl.h>

/*打开文件*/
int openfile(const char *path, int flag, mode_t mode);

/*复制一个文件描述符，共享一个文件句柄*/
int dupfile(const int fileno);

/*删除文件*/
int rmfile(const char *path);

/*关闭描述符*/
int closefile(int fileno);

/*读取一行*/
unsigned long readline(int fileno, char *text, int size);

/*读数据*/
unsigned long readfile(int fileno, char *text, int size);

/*从头跳转指定位置*/
off_t offsethead(int fileno, off_t off);

/*从当前位置跳转*/
off_t offsetcur(int fileno, off_t off);

/*获取文件大小,计算单位为（M）*/
unsigned long filelen(int fileno);

/*写入文件*/
unsigned long writefile(int fileno, const char *text, int size);

/*获取文件选项*/
int getfcntl(int fileno);

/*判断文件是否存在*/
int existfile(char *path);

/*设置文件长度*/
int setfilelen(int fileno, int len);

/*清空文件*/
int clearfile(int fileno);

#endif
