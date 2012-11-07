#include <stdio.h>
#include <stdlib.h>

#include "crawler.h"

#define INITURL "http://httpcrawler.sinaapp.com/hello.html"

#define INDCDF "indcdf.dat"
#define TOP10 "top10.dat"
#define SITEMAP "sitemap.dot"
#define CHECKRES "checkresult.dat"
//http://127.0.0.1/techqq/index.html check.dat

CThread_pool *pool;
webGraph *wg;
urlq_t *qurls;

pthread_mutex_t insertLock;

int haveCalInOut = 0;
int haveCalPr = 0;
int startCrawler(char *wsite, char *dir, int numOfThread) {

    /*initialize the queue of url*/
    queue_push(qurls,dir);
    /*insert the root node to the web*/
    web_insertVertex(dir,wg);
    /*initailize the thread pool*/
    pool_init(numOfThread);

    char *nextUrl;
    int state;
    /*if the url queue is not empty Or there is also a thread is running THEN continue*/
    while(1) {
        if(qurls->curNum > 0) {
            pthread_mutex_lock(&insertLock);
            nextUrl = queue_front(qurls);
            queue_pop(qurls);
            pthread_mutex_unlock(&insertLock);

            state = web_getNum(nextUrl,wg);

            //undone
            if(state == 0) {
                pthread_mutex_lock(&insertLock);
                web_setNum(nextUrl,wg,-1);
                pthread_mutex_unlock(&insertLock);
                pool_add_worker(crawling,wsite,nextUrl);
            }

        } else if(qurls->curNum==0 && pool->cur_running ==0)break;
        usleep(200);
    }
    /*done then destroy the pool*/
    pool_destroy();
    queue_clear(qurls);
    printf("\n////////////////// crawlering done ///////////////////////////\n");
    return 0;
}

int main(int argc, char *argv[]) {
    if(argc < 3) {
        fprintf(stderr,"argu error\n");
        exit(0);
    }
    char *hostname = argv[1];

    /*get the website and dire*/
    char website[1024], directory[1024];
    int i, j;
    for(i=0; (*(hostname+i)!='\0'); i++) {
        if(((*(hostname+i))=='/')&&(*(hostname+(++i))=='/')) {
            for(++i; *(hostname+i)!='/'; i++)
                website[i-7]=*(hostname+i);
            website[i-7]='\0';
            for(j=0; *(hostname+i)!='\0'; i++,j++)
                directory[j]=*(hostname+i);
            directory[j]='\0';
        }
    }

    //initialize the mutex
    pthread_mutex_init(&insertLock,NULL);

    //initialize web queue pool
    wg = (webGraph *)malloc(sizeof(webGraph));
    if(wg == NULL) {
        fprintf(stderr,"failed to malloc web\n");
        exit(0);
    }
    web_init(wg);

    qurls = malloc(sizeof(urlq_t));
    if(qurls == NULL) {
        fprintf(stderr,"failed to malloc queue\n");
        exit(0);
    }
    queue_init(qurls);

    pool = (CThread_pool *) malloc (sizeof (CThread_pool));
    if(pool == NULL) {
        fprintf(stderr,"failed to malloc pool\n");
        exit(0);

    }

    startCrawler(website,directory, 5);

    //cal cdf
    web_calculateIndCDF(wg, INDCDF);

    //gen map
    //web_generateMap(wg,SITEMAP);

    //print pr
    //web_printAllPagerank(wg,"allpagerank.dat");

    //print top10 pr
    web_printTop10Pagerank(wg, TOP10);

    //check print
    web_checkUrlPagerank(wg, argv[2], CHECKRES);
    return 0;
}
