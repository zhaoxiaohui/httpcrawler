/*****
** webgraph.c
** - implements the methods declared in webgraph.h
** - 构造、维护和存储整个web graph的顶点和边数据,分析web graph的链接关系,确保多线程安全访问
*/
#include "webgrap.h"

extern int haveCalInOut;
extern int haveCalPr;

typedef struct PR_TOP10 {
    char url[256];
    float pagerank;
    int numInLink;
} prTop10;

void web_init(webGraph * wg) {
    wg->vertexNum = 0;
    wg->vr = (vertexRow *)malloc(sizeof(vertexRow) * NODENUM);
    if(wg->vr == NULL) {
        fprintf(stderr,"initialize web error.malloc row\n");
        exit(0);
    }
    wg->vd = (vertexDict *)malloc(sizeof(vertexDict) * URLHASH);
    if(wg->vd == NULL) {
        fprintf(stderr,"initialize web error.malloc dict\n");
    }

    int i;
    for(i=0; i<URLHASH; i++) {
        wg->vd[i].vn = NULL;
    }
    //fprintf(stderr,"initialize web success\n");
}

void web_insertVertex(char *url, webGraph *wg) {
    unsigned int hres = sax_hash(url) % URLHASH;
    vertexNode *tempNode;
    vertexNode *curNode = wg->vd[hres].vn;
    tempNode = (vertexNode *)malloc(sizeof(vertexNode));
    if(tempNode == NULL) {
        fprintf(stderr,"generate node failed.malloc vn\n");
        exit(0);
    }
    tempNode->num = 0;//undone
    strcpy(tempNode->url,url);
    tempNode->next = NULL;
    if(curNode == NULL) {
        wg->vd[hres].vn = tempNode;
    } else {
        while(curNode->next != NULL)curNode = curNode->next;
        curNode->next = tempNode;
    }
    //fprintf(stderr,"insert one node success\n");
}

int  web_getNum(char *url, webGraph *wg) {
    unsigned int hres = sax_hash(url) % URLHASH;
    vertexNode *curNode = wg->vd[hres].vn;
    int flag = -3;
    while(curNode != NULL) {
        if(strcmp(curNode->url,url) == 0) {
            flag = curNode->num;
            break;
        }
        curNode = curNode->next;
    }
    return flag;
}

int web_setNum(char *url, webGraph *wg, int flag) {
    unsigned int hres = sax_hash(url) % URLHASH;
    vertexNode *curNode = wg->vd[hres].vn;
    int num = -3;
    while(curNode) {
        if(strcmp(curNode->url,url) == 0) {
            if(flag == 0 || flag == -1 || flag == -2) {
                curNode->num = flag;
                num = flag;
            } else {
                curNode->num = ++wg->vertexNum;
                num = curNode->num;
                wg->vr[num].num = num;
                strcpy(wg->vr[num].url,url);
                wg->vr[num].vc = NULL;
            }
            break;
        }
        curNode = curNode->next;
    }
    return num;
}

vertexNode *web_getNodeAddr(char *url, webGraph *wg) {
    unsigned int hres = sax_hash(url) % URLHASH;
    vertexNode *curNode = wg->vd[hres].vn;
    while(curNode) {
        if(strcmp(curNode->url,url) == 0) {
            break;
        }
        curNode = curNode->next;
    }
    return curNode;
}

int web_getSize(webGraph *wg) {
    return wg->vertexNum;
}

int web_insertEdge(char *src, char *dest, webGraph *wg) {
    int srcNum = web_getNum(src,wg);
    if(srcNum < 0) {
        fprintf(stderr,"failed to insert.get num error");
        return -1;
    }
    vertexCol *curColNode;
    vertexCol *tempColNode = (vertexCol *)malloc(sizeof(vertexCol));
    if(tempColNode == NULL) {
        fprintf(stderr,"failed to malloc new col\n");
        return -1;
    }

    tempColNode->vn = web_getNodeAddr(dest,wg);
    if(tempColNode->vn == NULL) {
        fprintf(stderr,"failed to insert,%s hasn't been insert to web\n",dest);
        free(tempColNode);
        return -1;
    }
    tempColNode->next = NULL;

    curColNode = wg->vr[srcNum].vc;
    if(curColNode == NULL)
        wg->vr[srcNum].vc = tempColNode;
    else {
        while(curColNode->next != NULL)
            curColNode = curColNode->next;
        curColNode->next = tempColNode;
    }
    return 1;
}

void web_calculateInAndOut(webGraph *wg) {
    int i, state;
    for(i=1; i<=wg->vertexNum; i++) {
        wg->vr[i].numInLink = 0;
        wg->vr[i].numOutLink = 0;
    }
    vertexCol *curNode;
    for(i=1; i<=wg->vertexNum; i++) {
        curNode = wg->vr[i].vc;
        while(curNode != NULL) {
            state = curNode->vn->num;
            if(state > 0) {
                wg->vr[i].numOutLink++;
                wg->vr[state].numInLink++;
            }
            curNode = curNode->next;
        }
    }
}

void web_calculateIndCDF(webGraph *wg, char *filename) {
    if(!haveCalInOut) {
        web_calculateInAndOut(wg);
        haveCalInOut = 1;
    }
    FILE *fres = fopen(filename,"w");
    if(fres == NULL) {
        fprintf(stderr,"failed to open file %s\n",filename);
        return;
    }
    int *CDF = (int *)malloc(sizeof(int) * (wg->vertexNum+1));
    if(CDF == NULL) {
        fprintf(stderr,"failed to malloc CDF\n");
        return;
    }
    int i, maxIn = 0;
    for(i=1; i<=wg->vertexNum; i++)
        CDF[wg->vr[i].numInLink] = 0;
    for(i=1; i<=wg->vertexNum; i++) {
        CDF[wg->vr[i].numInLink]++;
        maxIn = maxIn < CDF[wg->vr[i].numInLink] ?CDF[wg->vr[i].numInLink]:maxIn;
    }
    int last = 0;
    for(i=0; i<= maxIn; i++) {
        if(CDF[i] > 0) {
            last += CDF[i];
            fprintf(fres,"%d\t\t%lf\n", i, 1.0*last / wg->vertexNum);
        }
    }
    fclose(fres);
    free(CDF);
    fprintf(stderr,"Calculate CDF success\n");
}


void web_generateMap(webGraph *wg, char *filename) {
    FILE *fres = fopen(filename,"w");
    if(fres == NULL) {
        fprintf(stderr,"failed to open file %s\n",filename);
        return;
    }
    fprintf(fres,"digraph URL_MAP{\n");
    int i, state;

    for(i=1; i<=wg->vertexNum; i++)
        fprintf(fres,"%d[label = \"%s\"];\n",wg->vr[i].num,wg->vr[i].url);
    fprintf(fres,"\n");

    vertexCol *curNode;
    for(i=1; i<=wg->vertexNum; i++) {
        curNode = wg->vr[i].vc;
        while(curNode) {
            state = curNode->vn->num;
            if(state > 0) {
                fprintf(fres,"%d -> %d;\n",wg->vr[i].num,state);
            }
            curNode = curNode->next;
        }
    }
    fprintf(fres,"}");
    fclose(fres);
    fprintf(stderr,"Generate Map success\n");
}


void web_calculatePagerank(webGraph *wg) {
    if(!haveCalInOut) {
        web_calculateInAndOut(wg);
        haveCalInOut = 1;
    }
    int numVertex = wg->vertexNum;

    float *lastMap, *curMap, *tmpMap;
    if( (lastMap = (float *)malloc(sizeof(float)*(numVertex+1))) == NULL) {
        fprintf(stderr ,"failed to malloc last map.\n");
        exit(1);
    }
    if( (curMap = (float *)malloc(sizeof(float)*(numVertex+1))) == NULL) {
        fprintf(stderr ,"failed to malloc cur map.\n");
        exit(1);
    }
    if( (tmpMap = (float *)malloc(sizeof(float)*(numVertex+1))) == NULL) {
        fprintf(stderr ,"failed to malloc tmp map.\n");
        exit(1);
    }

    int i;
    for(i=1; i<=numVertex; i++)
        lastMap[i] = 1.0;

    float label, sum;
    int outLink, flag=0, state;
    vertexCol *curNode;
    while(1) {
        label = 0.0;
        for(i=1; i <=numVertex; i++) {
            label += lastMap[i];
            tmpMap[i] = 0.0;
        }
        for(i=1; i <=numVertex; i++) {
            curNode = wg->vr[i].vc;
            outLink = wg->vr[i].numOutLink;
            while(curNode != NULL) {
                state = curNode->vn->num;
                if(state > 0) {
                    tmpMap[state] += lastMap[i]/outLink;
                }
                curNode = curNode->next;
            }
        }
        sum = 0;
        for(i=1; i<=numVertex; i++) {
            curMap[i] = ((1.0 - DRATION) * tmpMap[i]) + ((DRATION/numVertex) * label);
            sum += curMap[i];
        }
        for(i=1; i<=numVertex; i++) {
            curMap[i] = curMap[i]/sum;
        }
        for(i=1; i <=numVertex; i++) {
            if(fabs(lastMap[i] - curMap[i]) > RANGE) {
                flag = 0;
                break;
            }
            flag = 1;
        }
        if(flag == 1)
            break;
        else {
            for(i=1; i<=numVertex; i++)
                lastMap[i] = curMap[i];
        }
    }
    for(i=1; i<=numVertex; i++)
        wg->vr[i].pagerank = curMap[i];

    free(lastMap);
    free(curMap);
    free(tmpMap);

}


void web_printAllPagerank(webGraph *wg, char *filename) {
    if(!haveCalPr) {
        web_calculatePagerank(wg);
        haveCalPr = 1;
    }
    FILE *fres = fopen(filename,"w");
    if(fres == NULL) {
        fprintf(stderr,"failed to open file %s\n",filename);
        return;
    }
    int i;

    for(i=1; i<=wg->vertexNum; i++)
        fprintf(fres,"%s\t%lf\n",wg->vr[i].url, wg->vr[i].pagerank);
    fclose(fres);
    fprintf(stderr,"print all pageranke success\n");
}

void insertSort(prTop10 *prt, vertexRow curNode, int top) {
    int i = top-1, split;
    while(i>=0 && curNode.pagerank > prt[i].pagerank)i--;

    split = i+1;
    for(i=top-1; i>split; i--)
        prt[i] = prt[i-1];
    strcpy(prt[split].url,curNode.url);
    prt[split].pagerank = curNode.pagerank;
    prt[split].numInLink = curNode.numInLink;
}

void web_printTop10Pagerank(webGraph *wg, char *filename) {
    if(!haveCalPr) {
        web_calculatePagerank(wg);
        haveCalPr = 1;
    }
    prTop10 *prt = (prTop10 *)malloc(sizeof(prTop10)*10);
    if(prt == NULL) {
        fprintf(stderr,"faile to malloc top10\n");
        return;
    }

    int i, top;
    top = wg->vertexNum>10?10:wg->vertexNum;
    for(i=0; i<top; i++)
        prt[i].pagerank = 0.0;

    //insert sort
    for(i=1; i<=wg->vertexNum; i++) {
        insertSort(prt, wg->vr[i], top);
    }

    FILE *fres = fopen(filename,"w");
    if(fres == NULL) {
        fprintf(stderr,"failed to open file %s\n",filename);
        return;
    }
    for(i=0; i<top; i++)
        fprintf(fres,"%s\t%d\n",prt[i].url,prt[i].numInLink);
    for(i=0; i<top; i++)
        fprintf(fres,"%s\t%lf\n",prt[i].url,prt[i].pagerank);
    fclose(fres);
    fprintf(stderr,"print top10 pagerank success\n");
}

char *web_getDir(char *site, char dir[]) {
    int i, j;
    for(i=0; (*(site+i)!='\0'); i++) {
        if(((*(site+i))=='/')&&(*(site+(++i))=='/')) {
            for(++i; *(site+i)!='/'; i++);
            for(j=0; *(site+i)!='\0'; i++,j++)
                dir[j]=*(site+i);
            dir[j]='\0';
        }
    }
    return dir;
}

void web_checkUrlPagerank(webGraph *wg, char *infile, char *outfile) {
    FILE *fin = fopen(infile,"r");
    if(fin == NULL) {
        fprintf(stderr,"open %s failed\n",infile);
        return;
    }
    FILE *fout = fopen(outfile,"w");
    if(fout == NULL) {
        fprintf(stderr,"open %s failed\n",outfile);
        return;
    }

    char curUrl[256], dir[256];
    unsigned int hres = 0;
    int flag=0;
    vertexNode *curNode;
    while(fgets(curUrl,256,fin)) {
        flag = 0;
        curUrl[strlen(curUrl)-1]='\0';
        web_getDir(curUrl,dir);
        hres = sax_hash(dir) % URLHASH;
        curNode = wg->vd[hres].vn;
        while(curNode) {
            if(curNode->num>0 && strcmp(curNode->url,dir) == 0) {
                fprintf(fout,"%s\t%d\t%lf\n", dir, wg->vr[curNode->num].numInLink, wg->vr[curNode->num].pagerank);
                flag = 1;
                break;
            }
            curNode = curNode->next;
        }
        if(!flag)fprintf(fout,"%s\t%s\n",dir,"NOT FOUND");
    }
    fclose(fin);
    fclose(fout);
    fprintf(stderr,"check url success\n");
}









