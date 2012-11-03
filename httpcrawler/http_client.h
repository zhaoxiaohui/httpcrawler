#ifndef _INCLUDE_STDLIB_
#define _INVLUDE_STDLIB_
#include <stdlib.h>
#endif

#ifndef _INCLUDE_STRING_
#define _INCLUDE_STRING_
#include <string.h>
#endif

#ifndef _INCLUDE_TYPES_
#define _INCLUDE_TYPES_
#include <sys/types.h>
#endif

#ifndef _INCLUDE_SOCKET_
#define _INCLUDE_SOCKET_
#include <sys/socket.h>
#endif

#ifndef _INCLUDE_STDIO_
#define _INCLUDE_STDIO_
#include <stdio.h>
#endif

#ifndef _INCLUDE_NETDB_
#define _INCLUDE_NETDB_
#include <netdb.h>
#endif

#ifndef _INCLUDE_INET_
#define _INCLUDE_INET_
#include <arpa/inet.h>
#endif

#ifndef _HTTPCLIENT_H_
#define _HTTPCLIENT_H_


int http_do_get(int socket,char *site, char *dirctory);/*用get命令请求远程服务器的网页*/
int http_response_len(char *);
int http_response_status(char *htmtext);/*远程WEB服务器的http响应代码，如404*/
char *http_response_body(int socket,int len, char body[]);/*获取http响应的消息体字节流*/
int http_response_content(int socket, char content[]);
char *http_response_body2(int socket, char []);/* */
#endif

