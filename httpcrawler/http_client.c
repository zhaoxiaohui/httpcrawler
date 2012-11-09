/*****
** http_client.c
** - implements the methods declared in http_client.h
** - 封装处理http协议
*/

#include "http_client.h"

int http_do_get(int sock,char *site, char *directory) {
    char p[1024];
    memset(p,'0',1024);

    sprintf(p,"GET %s HTTP/1.1\r\n",directory);
    sprintf(p+strlen(p), "Host:%s\r\n",site);
    sprintf(p+strlen(p), "Accept: */*\r\n");
    //sprintf(p+strlen(p), "Content-Type: application/x-www-form-urlencoded\n");
    //sprintf(p+strlen(p), "Accept-Language: zh-cn\n");
    //sprintf(p+strlen(p), "User-Agent: Mozilla/4.0");
    //sprintf(p+strlen(p), "(compatible; MSIE 7.00; Windows 98)\r\n");
    sprintf(p+strlen(p), "Connection:Close\r\n");
    sprintf(p+strlen(p), "\r\n");

    if(send(sock,p,strlen(p),0) < strlen(p)) {
        fprintf(stderr,"get error\n");
        return 0;
    }
    return 1;
}
int http_response_content(int socket, char content[]){
    char temp[2000];
    int num, total = 0;
    while(1) {
        //memset(temp,'\0',sizeof(temp));
        num=(recv(socket,temp,2000,0));
        if(num < 1)
            break;
        total += num;
        if(total > 500000)return -1;
        temp[num] = '\0';
        strcat(content,temp);
    }
    if(total ==0)return -1;
    return 1;
}
char *http_response_body(int socket,int len, char body[]) {
    char *p;
    p = (char *)malloc(sizeof(char)*(len+500));
    if(p == NULL){
        fprintf(stderr,"failed to malloc content int httpresponse\n");
    }
    memset(p,'\0',len);
    strncat(p,body,strlen(body));
    char temp[2000];
    int num;
    while(1) {
        //memset(temp,'\0',sizeof(temp));
        num=(recv(socket,temp,2000,0));
        if(num < 1)
            break;
        temp[num] = '\0';
        strncat(p,temp,strlen(temp));
    }
    /*recv(socket,p,20000,0);*/
    return p;
}
int http_response_status(char *content) {
    int i, res=0;
    for(i=0; *(content+i)!=' '; i++) {
        if(*(content+i+1)==' ') {
            res += (*(content+i+2)-'0') * 100;
            res += (*(content+i+3)-'0') * 10;
            res += (*(content+i+4)-'0');
        }
    }
    return res;
}

char *http_response_body2(int socket, char p[]) {
    memset(p,'\0',300);
    recv(socket,p,300,0);
    return p;
}

int http_response_len(char *content) {
    char *p = strstr(content,"Content-Length");
    p+=16;
    int t = 0;
    while(*p>='0' && *p <='9') {
        t = 10*t + (*p-'0');
        p++;
    }
    return t;
}
