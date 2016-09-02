#ifndef SOCKET_H
#define SOCKET_H

#include "util.h"
#include <netdb.h>
#include <stdint.h>

/*将本机字节序转换为网络*/
uint32_t htonlv(uint32_t host);

/*将网络转本机*/
uint32_t ntohlv(uint32_t net);

/*将本机字节序转换为网络*/
uint16_t htonsv(uint16_t host);

/*将网络转本机*/
uint16_t ntohsv(uint16_t net);

char *inetntoa(struct in_addr in);

/*将ip转换为网路字节序列*/
int iptonet(int domain, const char *ip, void *addr);

/*将网路序列转换为ip*/
const char *nettoip(int domain, const void *addr, char *desaddr, size_t len);

/*设置addr*/
cbool setsockaddrin(struct sockaddr_in *sockaddr, sa_family_t family, uint16_t port, const char *ip);

/*获取主机名和服务名得IP信息*/
int getipaddrinfo(const char *host, const char *server,
					 const struct addrinfo *hint, struct addrinfo **result);

/*释放获取的主机信息*/
void freeipaddrinfo(struct addrinfo *result);

/*获取addinfo返回的错误信息*/
const char *gaierrorinfo(int errcode);

/*获取主机名和服务名*/
int gethostinfo(const struct sockaddr *addr, socklen_t socklen, char *host,
				 size_t hostlen, char *service, size_t serlen, int flag);

/*通过套接字获取IP和端口信息*/
int getsocketname(int fd, struct sockaddr *addr, socklen_t *addrlen);

/*获取对端的套接字地址信息*/
int getpeersname(int peerfd, struct sockaddr *addr, socklen_t *addrlen);

/*关闭一端通信*/
int shutdownsock(int fd, int how);

/*设置tcp的选项*/
int setsocketopt(int fd, int level, int opt,
                 const void *optval, socklen_t optlen);

/*获取tcp的选项*/
int getsocketopt(int fd, int level, int opt,
                 void *optval, socklen_t *optlen);

/*设置非阻塞IO*/
int setnoblock(int fd);

/*创建socket*/
int sock(int domain, int socktype, int protocol);

/*绑定ip*/
int bindsock(int fd, struct sockaddr *addr, socklen_t addrlen);

/*监听链接*/
int listensock(int fd, int backlog);

/*接收*/
int acceptsock(int servfd, struct sockaddr_in *cliaddr, socklen_t *addrlen);

/*连接*/
int connectsock(int fd, struct sockaddr *addr, socklen_t addrlen);

/*关闭*/
int closesock(int fd);

#endif