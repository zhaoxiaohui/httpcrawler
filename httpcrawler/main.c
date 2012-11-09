#include <stdio.h>
#include <stdlib.h>

#include "crawler_http.h"

#define INITURL "http://httpcrawler.sinaapp.com/hello.html"

#define INDCDF "indcdf.dat"
#define TOP10 "top10.dat"
#define SITEMAP "sitemap.dot"
#define CHECKRES "checkresult.dat"
#define DEFAULTPORT "8080"
//http://127.0.0.1/techqq/index.html check.dat

threadPool *pool;
webGraph *wg;
urlq_t *qurls;

int socketPort;
int haveCalInOut = 0;
int haveCalPr = 0;
int badVertex = 0;

int startCrawler(char *wsite, char *dir, int numOfThread) {

    /*insert the root node to the web*/
    web_insertVertex(dir,wg);
    /*initailize the thread pool*/
    pool_init(numOfThread, wsite);

    /*initialize the queue of url*/
    pthread_mutex_lock(&(pool->queue_lock));
	queue_push(qurls,dir);
	pthread_mutex_unlock(&(pool->queue_lock));
	pthread_cond_signal(&(pool->queue_have));

    /*if the url queue is not empty Or there is also a thread is running THEN continue*/
    int st1 , st2;
	while(1) {
        pthread_mutex_lock(&(pool->queue_lock));
		st1 = qurls->curNum;
		st2 = pool->cur_running;
		pthread_mutex_unlock(&(pool->queue_lock));
		printf("\n@@@@@ In Main  @@@@@\n\n");
        if(st1==0 && st2 == 0)break;
        sleep(1);
    }
    /*done then destroy the pool*/
    pool_destroy();
    queue_clear(qurls);
    printf("\n////////////////// crawlering done ///////////////////////////\n\n");
    return 0;
}

int main(int argc, char *argv[]) {
    if(argc < 3) {
        fprintf(stderr,"argu error\n");
        exit(0);
    }
    char *hostname = argv[1];
    char urlPort[6];
    if(argc == 4 && strlen(argv[3])<=5){
        strcpy(urlPort, argv[3]);
        urlPort[strlen(argv[3])] = '\0';
    }else{
         strcpy(urlPort, DEFAULTPORT);
        urlPort[4] = '\0';
    }
    int i, j;
    int len = strlen(urlPort);
    socketPort = 0;
    for(i=0; i<len; i++)
        socketPort = socketPort*10 + (urlPort[i]-'0');
    /*get the website and dire*/
    char website[1024], directory[1024];
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

    pool = (threadPool *) malloc (sizeof (threadPool));
    if(pool == NULL) {
        fprintf(stderr,"failed to malloc pool\n");
        exit(0);

    }

    startCrawler(website,directory, 50);

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
    printf("\n////////////////// calculating done ///////////////////////////\n");
    return 0;
}
