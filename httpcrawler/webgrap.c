/*****
** webgraph.c
** - implements the methods declared in webgraph.h
** - 构造、维护和存储整个web graph的顶点和边数据,分析web graph的链接关系,确保多线程安全访问
*/
#include "webgrap.h"

extern int haveCalInOut;
extern int haveCalPr;

void web_init(webGraph *wg, char *site) {
    wg->vertexNum = 0;
    wg->vr = (vertexRow *)malloc(sizeof(vertexRow) * NODENUM);
    if(wg->vr == NULL) {
        fprintf(stderr,"initialize web error.malloc row\n");
        exit(0);
    }
    wg->vd = (vertexDict *)malloc(sizeof(vertexDict) * URLHASH);
    if(wg->vd == NULL) {
        fprintf(stderr,"initialize web error.malloc dict\n");
        exit(0);
    }
    int i;
    for(i=0; i<URLHASH; i++) {
        wg->vd[i].vn = NULL;
    }
    //fprintf(stderr,"initialize web success\n");
    wg->prt = (prTop10 *)malloc(sizeof(prTop10) * 10);
    if(wg->prt == NULL){
        fprintf(stderr,"initialize web error.malloc prt\n");
        exit(0);
    }
    for(i=0; i<10; i++){
        wg->prt[i].pagerank = 0.0;
    }
    
    wg->website = (char *)malloc(sizeof(char) * 256);
    if(wg->website == NULL){
        fprintf(stderr,"initialize web error.malloc website\n");
        exit(0);
    }
    strcpy(wg->website, site);
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
    FILE *find = fopen(filename,"w");
    if(find == NULL) {
        fprintf(stderr,"failed to open file %s\n",filename);
        return;
    }
    int *CDF = (int *)malloc(sizeof(int) * (wg->vertexNum+1));
    if(CDF == NULL) {
        fprintf(stderr,"failed to malloc CDF\n");
        return;
    }
    int i, maxIn = 0;
    for(i=0; i<=wg->vertexNum; i++)
        CDF[i] = 0;
    for(i=1; i<=wg->vertexNum; i++) {
        CDF[wg->vr[i].numInLink]++;
        if(maxIn < wg->vr[i].numInLink)maxIn = wg->vr[i].numInLink;
    }
    int last = 0;
    for(i=0; i<= maxIn; i++) {
        if(CDF[i] > 0) {
            last += CDF[i];
            fprintf(find,"%d\t\t%lf\n", i, 1.0*last / wg->vertexNum);
        }
    }
    //fprintf(find, "ALL:%d NUM:%d", last, wg->vertexNum);
    fclose(find);
    free(CDF);
    CDF = NULL;
    printf("@@@@@ Calculate CDF success  @@@@@\n");
}


void web_generateMap(webGraph *wg, char *filename) {
    FILE *fmap = fopen(filename,"w");
    if(fmap == NULL) {
        fprintf(stderr,"failed to open file %s\n",filename);
        return;
    }
    fprintf(fmap,"digraph URL_MAP{\n");
    int i, state;

    for(i=1; i<=wg->vertexNum; i++)
        fprintf(fmap,"%d[label = \"%s\"];\n",wg->vr[i].num,wg->vr[i].url);
    fprintf(fmap,"\n");

    vertexCol *curNode;
    for(i=1; i<=wg->vertexNum; i++) {
        curNode = wg->vr[i].vc;
        while(curNode) {
            state = curNode->vn->num;
            if(state > 0) {
                fprintf(fmap,"%d -> %d;\n",wg->vr[i].num,state);
            }
            curNode = curNode->next;
        }
    }
    fprintf(fmap,"}");
    fclose(fmap);
    printf("@@@@@ Generate Map success  @@@@@\n");
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

    float label, sum, father;
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
            father = lastMap[i]/outLink;
            while(curNode != NULL) {
                state = curNode->vn->num;
                if(state>0 && state!=i) {
                    tmpMap[state] += father;
                }
                curNode = curNode->next;
            }
        }
        sum = 0.0;
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
    lastMap = NULL;
    curMap = NULL;
    tmpMap = NULL;
}


void web_printAllPagerank(webGraph *wg, char *filename) {
    if(!haveCalPr) {
        web_calculatePagerank(wg);
        haveCalPr = 1;
    }
    FILE *fpra = fopen(filename,"w");
    if(fpra == NULL) {
        fprintf(stderr,"failed to open file %s\n",filename);
        return;
    }
    int i;
    double to = 0.0;
    for(i=1; i<=wg->vertexNum; i++){
        fprintf(fpra,"%s%s\t%lf\n",wg->website, wg->vr[i].url, wg->vr[i].pagerank);
        //to += wg->vr[i].pagerank;
    }
    //fprintf(fpra,"total pr:%lf\n",to);
    fclose(fpra);
    printf("@@@@@ print all pageranke success  @@@@@\n");
}

void insertSort(prTop10 *prt, vertexRow curNode, int up) {
    int i = up-1, split;
    while(i>=0 && curNode.pagerank > prt[i].pagerank)i--;

    if(i < up-1){
        split = i+1;
        for(i=up-1; i>split; i--)
            prt[i] = prt[i-1];
        strcpy(prt[split].url,curNode.url);
        prt[split].pagerank = curNode.pagerank;
        prt[split].numInLink = curNode.numInLink;
    }
}

void web_printTop10Pagerank(webGraph *wg, char *filename) {
    if(!haveCalPr) {
        web_calculatePagerank(wg);
        haveCalPr = 1;
    }
    int i, up;
    up = wg->vertexNum > 10 ? 10:wg->vertexNum;
    for(i=1; i<=wg->vertexNum; i++) {
        insertSort(wg->prt, wg->vr[i], up);
    }
    FILE *fprt = fopen(filename, "w");
    
    for(i=0; i<up; i++)
        fprintf(fprt,"%s%s\t%d\n",wg->website, wg->prt[i].url, wg->prt[i].numInLink);
    for(i=0; i<up; i++)
        fprintf(fprt,"%s%s\t%lf\n",wg->website, wg->prt[i].url, wg->prt[i].pagerank);

    fclose(fprt);
    printf("@@@@@ print top10 pagerank success  @@@@@\n");
}

char *web_getDir(char *site, char website[], char dir[]) {
    int i, j;
    for(i=0; (*(site+i)!='\0'); i++) {
        if(((*(site+i))=='/')&&(*(site+(++i))=='/')) {
            for(++i; *(site+i)!='/'; i++)
                website[i-7] = *(site+i);
            website[i-7] = '\0';
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

    char curUrl[256], dir[256], website[256];
    unsigned int hres = 0;
    int flag=0;
    vertexNode *curNode;
    while(fgets(curUrl,256,fin)) {
        flag = 0;
        curUrl[strlen(curUrl)-1]='\0';
        web_getDir(curUrl, website, dir);
        if(strcmp(website, wg->website) == 0){
            hres = sax_hash(dir) % URLHASH;
            curNode = wg->vd[hres].vn;
            while(curNode) {
                if(curNode->num>0 && strcmp(curNode->url,dir) == 0) {
                    fprintf(fout,"%s\t%d\t%lf\n", curUrl, wg->vr[curNode->num].numInLink, wg->vr[curNode->num].pagerank);
                    flag = 1;
                    break;
                }
                curNode = curNode->next;
            }
        }
        if(!flag)fprintf(fout,"%s\t%s\n",curUrl,"NOT FOUND");
    }
    fclose(fin);
    fclose(fout);
    printf("@@@@@ check url success  @@@@@\n");
}









