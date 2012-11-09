#ifndef _INCLUDE_STRING_
#define _INCLUDE_STRING_
#include <string.h>
#endif

#ifndef _INCLUDE_STDLIB_
#define _INVLUDE_STDLIB_
#include <stdlib.h>
#endif

#ifndef _INCLUDE_URLQUEUE_
#define _INCLUDE_URLQUEUE_
#include "urlqueue.h"
#endif

#ifndef _INCLUDE_WEBGRAP_
#define _INCLUDE_WEBGRAP_
#include "webgrap.h"
#endif

#ifndef _INCLUDE_LINKPARSER_
#define _INCLUDE_LINKPARSER_
#include "link_parser.h"
#endif

#ifndef _INCLUDE_HTTPCLIENT_
#define _INCLUDE_HTTPCLIENT_
#include "http_client.h"
#endif

#ifndef _INCLUDE_NETWORK_
#define _INCLUDE_NETWORK_
#include "network.h"
#endif

#ifndef _CRAWLER_H_
#define _CRAWLER_H_


#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <assert.h>
#include <semaphore.h>

/**
 * 线程池结构
 *
 */
typedef struct ThreadPool{
	pthread_mutex_t queue_lock;
	pthread_cond_t queue_have;

	/*这个线程池访问的站点*/
	char *website;
	/*线程数组*/
	pthread_t *threads;
	/*线程个数*/
	int threadNum;
	/*正在执行个数*/
    int cur_running;
	int *working;
    /*已经处理的个数*/
    int worked;
	/*是否销毁*/
	int destroy;
    /*有几个还活着*/
    int alive;	
}threadPool;

/*销毁线程池*/
int pool_destroy();
/*初始化线程池*/
void pool_init(int threadnum, char *);
/*线程执行函数*/
void *threadCrawling(void *arg);

#endif
