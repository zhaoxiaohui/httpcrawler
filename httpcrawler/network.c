/*****
** network.c
** - implements the methods declared in network.h
** - 对网络socket的收发进行封装
*/

#include "network.h"

int create_tcp_socket() {
    int sock;
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr,"Can't create TCP socket\n");
        return -1;
    }
    return sock;
}

char *get_ip(char *host) {
    struct hostent *hent;
    int iplen = 15; /*XXX.XXX.XXX.XXX*/
    char *ip = (char *)malloc(iplen+1);
    if(ip == NULL) {
        fprintf(stderr,"malloc ip error in get_ip\n");
        return NULL;
    }
    memset(ip, 0, iplen+1);
    if((hent = gethostbyname(host)) == NULL) {
        fprintf(stderr,"Can't get IP\n");
        return NULL;
    }
    if(inet_ntop(AF_INET, (void *)hent->h_addr_list[0], ip, iplen) == NULL) {
        fprintf(stderr,"Can't resolve host\n");
        return NULL;
    }
    return ip;
}

int open_tcp(char *host,int port) {
    struct sockaddr_in *remote;
    int sock = 0;
    int tmpres;
    char *ip;
    sock = create_tcp_socket();
    if(sock == -1) {
        fprintf(stderr,"create socket error in open_tcp\n");
        return -1;
    }
    ip = get_ip(host);
    if(ip == NULL) {
        fprintf(stderr,"get ip error in open_tcp\n");
        return -1;
    }
    remote = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
    if(remote == NULL) {
        fprintf(stderr,"malloc remote error in open_tcp\n");
        return -1;
    }
    remote->sin_family = AF_INET;
    tmpres = inet_pton(AF_INET, ip, (void *)(&(remote->sin_addr.s_addr)));
    if( tmpres < 0) {
        fprintf(stderr,"Can't set remote->sin_addr.s_addr\n");
        return -1;
    } else if(tmpres == 0) {
        fprintf(stderr,"%s is not a valid IP address\n", ip);
        return -1;
    }
    remote->sin_port = htons(port);

    if(connect(sock, (struct sockaddr *)remote, sizeof(struct sockaddr)) < 0) {
        fprintf(stderr,"Could not connect\n");
        return -1;
    }
    return sock;
}
int open_tcp2(char *host, int port) {

    int sock;
    struct sockaddr_in serveraddr;
	struct hostent *hp, hostbuf;
	struct timeval nTimeout = {10,0};
	int res,ret,re;
	char buffer[1024];

	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr,"failed to create socket.\n");
		return -1;
	}
	if((re = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&nTimeout, sizeof(struct timeval)))!=0){
		fprintf(stderr,"failed to set time,error : %d\n",re);
		return -1;
	}
    res = gethostbyname_r(host, &hostbuf, buffer, sizeof(buffer), &hp, &ret);
	if(res || hp == NULL){
		fprintf(stderr,"failed to open tcp\n");
		return -1;
	}
	bzero((char*) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	bcopy((char*)hp->h_addr_list[0],(char *)&serveraddr.sin_addr.s_addr,hp->h_length);
	serveraddr.sin_port = htons(port);

	if(connect(sock, (struct sockaddr*) &serveraddr,sizeof(serveraddr)) < 0) {
		fprintf(stderr, "failed to connect.\n");
		return -1;
	}

    return sock;
}
void closeSocket(int sock) {
    close(sock);
    //shutdown(sock,SHUT_RDWR);
}
