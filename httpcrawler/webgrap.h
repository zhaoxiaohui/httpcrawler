#ifndef _INCLUDE_STDLIB_
#define _INVLUDE_STDLIB_
#include <stdlib.h>
#endif

#ifndef _INCLUDE_STDIO_
#define _INCLUDE_STDIO_
#include <stdio.h>
#endif

#ifndef _INCLUDE_STRING_
#define _INCLUDE_STRING_
#include <string.h>
#endif

#ifndef _WEBGRAPH_H_
#define _WEBGRAPH_H_

#include "urlhash.h"
#include <math.h>

#define RANGE 0.0001
#define DRATION 0.15
#define URLHASH 400001
#define NODENUM 400000

typedef struct VertexNode {
    int               num;   //0:undone -1:being worked
    char              url[256];
    struct VertexNode *next;
} vertexNode;

typedef struct VertexCol {
    vertexNode       *vn;
    struct VertexCol *next;
} vertexCol;

typedef struct VertexDict {
    vertexNode *vn;
} vertexDict;

typedef struct PR_TOP10 {
     char url[256];
     float pagerank;
     int numInLink;
 } prTop10;

typedef struct VertexRow {
    int        num;
    char       url[256];
    float     pagerank;
    int        numInLink;
    int        numOutLink;
    vertexCol  *vc;
} vertexRow;

typedef struct WEB {
    int        vertexNum; //num of nodes
    vertexRow  *vr;
    vertexDict *vd;
    prTop10 *prt;    
} webGraph;

void web_init(webGraph * wg);
void web_insertVertex(char *url, webGraph *wg);
int  web_getNum(char *url, webGraph *wg);
int web_setNum(char *url, webGraph *wg, int flag);
vertexNode *web_getNodeAddr(char *url, webGraph *wg);
int web_getSize(webGraph *wg);
int web_insertEdge(char *src, char *dest, webGraph *wg);
void web_calculateIndCDF(webGraph *wg, char *filename);
void web_generateMap(webGraph *wg, char *filename);
void web_calculatePagerank(webGraph *wg);
void web_printAllPagerank(webGraph *wg, char *fielname);
void web_printTop10Pagerank(webGraph *wg, char *filename);
void web_checkUrlPagerank(webGraph *wg, char *infile, char *outfile);
#endif

