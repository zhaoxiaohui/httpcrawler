/*****
** link_parser.c
** - implements the methods declared in link_parser.h
** - 处理http响应字节流，提取网络链接
*/

#include "link_parser.h"

int check(char res[][256], int num, char link[]) {
    int i;
    for(i=0; i<num; i++)
        if(strcmp(res[i],link)==0)return 0;
    return 1;
}
int extract_link(char *directory, char *htmltxt, char res[][256]) {
    /*get the real directory*/
    int len = strlen(directory)-1, i;
    while(*(directory + len)!='/')len--;
    char dir[1024];
    for(i=0; i<= len; i++) {
        dir[i] = directory[i];
    }
    dir[i] = '\0';
    int num = 0;
    if(res == NULL)return 0;
    char *p=NULL, link[256], temp[256], *t2 = NULL, *t3 = NULL;
    int link_id = 0;
    /*printf("%s\n",htmltxt);*/
    //p = strstr(htmltxt,"<a");
    p = strstr(htmltxt,LINKTOFIND);
    /*printf("%s\n",p);*/
    while(p != NULL) {
        p += 4;
        while((*p) != '=') {
            if((*p) !=' ')break;
            p++;
        }
        if((*p) == '=') {

            for(i=0; i<= len; i++) {
                dir[i] = directory[i];
            }
            dir[i] = '\0';
            link_id = 0;
            while(p && (*p)!='\"' && (*p)!='\'') {
                /*printf("%c\n",*p);*/
                p++;
            }
            /*printf("%s\n",p);*/
            p++;
            while(p && (*p)!='\"' && (*p)!='\'') {
                /*printf("%c\n",*p);*/
                link[link_id++] = *p;
                p++;
            }
            link[link_id] = '\0';
            //printf("%s\n",link);
            //t = strstr(link,CATEGORY1);
            int len = strlen(link);
            int html = link[len-1]=='l' && link[len-2]=='m' && link[len-3]=='t' && link[len-4]=='h';
            int htm = link[len-1]=='m' && link[len-2]=='t' && link[len-3]=='h';
            if(link[0]!='/' && (html || htm)) {
                t2 = strstr(link,HTTPCATE);
                if(!t2) { /*is not a outside web*/
                    t3 = strstr(link,POTPOT);
                    if(!t3) {
                        strcpy(temp,dir);
                        strcat(temp,link);
                        if(check(res,num,temp))
                            strcpy(res[num++],temp);
                    } else {
                        int link_len = strlen(link), pLen, po = 0, poDir, numLink = 0, numDir = 0, dirLen;
                        pLen = link_len;
                        while(1) {
                            if(pLen >2 && link[po]=='.' && link[po+1]=='.' && link[po+2]=='/') {
                                pLen -= 3;
                                po += 3;
                                numLink++;
                                dirLen = strlen(dir);
                                if(dirLen > 1) {
                                    poDir = dirLen-2;
                                    while(dir[poDir]!='/')poDir--;
                                    dir[poDir+1]='\0';
                                    numDir++;
                                } else {
                                    break;
                                }
                            } else break;
                        }
                        if(numLink == numDir) {
                            strcpy(temp,dir);
                            strcat(temp,link+po);
                            if(check(res,num,temp))
                                strcpy(res[num++],temp);
                        }
                    }
                }
            }
        }

        //printf("%d\n",num);
        //p = strstr(p,LINKNODEA);
        p = strstr(p,LINKTOFIND);
        /*printf("%s\n",p);*/
    }
    /*return the number of url found*/
    return num;
}
