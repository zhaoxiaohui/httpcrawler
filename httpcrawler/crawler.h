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
/*
*线程池里所有运行和等待的任务都是一个CThread_worker
*由于所有任务都在链表里，所以是一个链表结构
*/
typedef struct worker {
    /*回调函数，任务运行时会调用此函数，注意也可声明成其它形式*/
    void *(*process) (void *site, void *dir);
    void *site;
    void *dir;
    struct worker *next;

} CThread_worker;

/*线程池结构*/
typedef struct {
    pthread_mutex_t queue_lock;
    pthread_cond_t queue_ready;

    /*链表结构，线程池中所有等待任务*/
    CThread_worker *queue_head;

    /*是否销毁线程池*/
    int shutdown;
    pthread_t *threadid;
    /*线程池中允许的活动线程数目*/
    int max_thread_num;
    /*当前等待队列的任务数目*/
    int cur_queue_size;
    int cur_running;

} CThread_pool;

int pool_add_worker (void *(*process) (void *site, void *dir), void *s, void *d);
void *thread_routine (void *arg);
void * crawling (void *site, void *dir);
void pool_init (int max_thread_num);
int pool_destroy ();
#endif

