#ifndef _INCLUDE_STRING_
#define _INCLUDE_STRING_
#include <string.h>
#endif

#ifndef _INCLUDE_STDLIB_
#define _INVLUDE_STDLIB_
#include <stdlib.h>
#endif

#ifndef _INCLUDE_STDIO_
#define _INCLUDE_STDIO_
#include <stdio.h>
#endif

#ifndef _INCLUDE_UNISTD_
#define _INCLUDE_UNISTD_
#include <unistd.h>
#endif

#ifndef _INCLUDE_INET_
#define _INCLUDE_INET_
#include <arpa/inet.h>
#endif

#ifndef _INCLUDE_NETDB_
#define _INCLUDE_NETDB_
#include <netdb.h>
#endif

#ifndef _INCLUDE_TYPES_
#define _INCLUDE_TYPES_
#include <sys/types.h>
#endif

#ifndef _INCLUDE_SOCKET_
#define _INCLUDE_SOCKET_
#include <sys/socket.h>
#endif

#ifndef _NETWORK_H_
#define _NETWORK_H_


int open_tcp(char *host,int port); /*建立一个到远程服务器的socket连接*/

void closeSocket(int sock);/*关闭socket*/

#endif
