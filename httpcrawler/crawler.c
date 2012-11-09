/*****
** crawler.c
** - implements the methods declared in crawler.h
** -爬取线程的核心流程
*/
#include "crawler.h"


extern CThread_pool *pool;
extern webGraph *wg;
extern urlq_t *qurls;
extern pthread_mutex_t insertLock;

int worked = 0;

void pool_init (int max_thread_num) {
    pool = (CThread_pool *) malloc (sizeof (CThread_pool));

    pthread_mutex_init (&(pool->queue_lock), NULL);
    pthread_cond_init (&(pool->queue_ready), NULL);

    pool->queue_head = NULL;

    pool->max_thread_num = max_thread_num;
    pool->cur_queue_size = 0;
    pool->cur_running = 0;
    pool->shutdown = 0;

    pool->threadid = (pthread_t *) malloc (max_thread_num * sizeof (pthread_t));
    int i = 0;
    for (i = 0; i < max_thread_num; i++) {
        pthread_create(&(pool->threadid[i]), NULL, thread_routine, NULL);
    }
}
/*向线程池中加入任务*/
int pool_add_worker (void *(*process) (void *s, void *d), void *site, void *dir) {
    /*构造一个新任务*/
    CThread_worker *newworker = (CThread_worker *) malloc (sizeof (CThread_worker));
    newworker->site = site;
    newworker->dir = dir;
    newworker->process = process;
    newworker->next = NULL;/*别忘置空*/

    pthread_mutex_lock (&(pool->queue_lock));
    /*将任务加入到等待队列中*/
    CThread_worker *member = pool->queue_head;
    if (member != NULL) {
        while (member->next != NULL)
            member = member->next;
        member->next = newworker;
    } else {
        pool->queue_head = newworker;
    }

    assert(pool->queue_head != NULL);

    pool->cur_queue_size++;
    pthread_mutex_unlock (&(pool->queue_lock));
    /*好了，等待队列中有任务了，唤醒一个等待线程；
     注意如果所有线程都在忙碌，这句没有任何作用*/
    pthread_cond_signal (&(pool->queue_ready));
    return 0;
}

/*销毁线程池，等待队列中的任务不会再被执行，但是正在运行的线程会一直
把任务运行完后再退出*/
int pool_destroy () {
    if (pool->shutdown)
        return -1;/*防止两次调用*/
    pool->shutdown = 1;

    /*唤醒所有等待线程，线程池要销毁了*/
    pthread_cond_broadcast (&(pool->queue_ready));

    /*阻塞等待线程退出，否则就成僵尸了*/
    int i;
    for (i = 0; i < pool->max_thread_num; i++)
        pthread_join(pool->threadid[i], NULL);
    free (pool->threadid);

    /*销毁等待队列*/
    CThread_worker *head = NULL;
    while (pool->queue_head != NULL) {
        head = pool->queue_head;
        pool->queue_head = pool->queue_head->next;
        free (head);
    }
    /*条件变量和互斥量也别忘了销毁*/
    pthread_mutex_destroy(&(pool->queue_lock));
    pthread_cond_destroy(&(pool->queue_ready));

    free (pool);
    /*销毁后指针置空是个好习惯*/
    pool=NULL;
    return 0;
}

void * thread_routine (void *arg) {
    while (1) {
        pthread_mutex_lock (&(pool->queue_lock));
        /*如果等待队列为0并且不销毁线程池，则处于阻塞状态; 注意
         pthread_cond_wait是一个原子操作，等待前会解锁，唤醒后会加锁*/
        while (pool->cur_queue_size == 0 && !pool->shutdown) {
            pthread_cond_wait (&(pool->queue_ready), &(pool->queue_lock));
        }

        /*线程池要销毁了*/
        if (pool->shutdown) {
            /*遇到break,continue,return等跳转语句，千万不要忘记先解锁*/
            pthread_mutex_unlock (&(pool->queue_lock));
            pthread_exit (NULL);
        }

        /*assert是调试的好帮手*/
        assert (pool->cur_queue_size != 0);
        assert (pool->queue_head != NULL);

        /*等待队列长度减去1，并取出链表中的头元素*/
        pool->cur_queue_size--;
        pool->cur_running++;
        CThread_worker *worker = pool->queue_head;
        pool->queue_head = worker->next;
        pthread_mutex_unlock (&(pool->queue_lock));

        /*调用回调函数，执行任务*/
        (*(worker->process)) (worker->site, worker->dir);
        free (worker);
        pool->cur_running--;
        printf("Have Worked:---%d  @@  pool running:---%d  @@  pool queue---%d\n",++worked, pool->cur_running, pool->cur_queue_size);
        worker = NULL;
    }
    /*这一句应该是不可达的*/
    pthread_exit (NULL);
}
void * crawling (void *site, void *dir) {
    char *host = (char *)site;
    char *directory = (char *)dir;

    printf("@@@@@ SITE: %s DIR:%s @@@@@\n",host, dir);

    int flag = -1, count = 0;
    int sock, res;
    char body[300];
    while(flag == -1 && count < 3) {
        sock = open_tcp2(host,80);
        if(sock == -1) {
            flag = -1;
            count++;
            continue;
        }
        res = http_do_get(sock, host, directory);
        if(res ==0) {
            flag = -1;
            count++;
            closeSocket(sock);
            continue;
        }
        http_response_body2(sock,body);
        res = http_response_status(body);
        if(res/100 != 2) {
            flag = -1;
           count++;
            closeSocket(sock);
            continue;
        } else if(res/100 == 2) {
            flag = 1;
            break;
        }
    }
    if(flag == -1) {
        pthread_mutex_lock(&insertLock);
        web_setNum(directory,wg,-2);
        pthread_mutex_unlock(&insertLock);
        closeSocket(sock);
    } else {
        char content[500000];
        content[0] = '\0';
        strcat(content,body);
        //char *content = http_response_body(sock, http_response_len(body), body);
		//http_response_content(sock,content);
		if(http_response_content(sock,content) == -1){
			fprintf(stderr,"get content error\n");
			return NULL;
		}
        char linkUrls[1000][256];
        //printf("%s\n",content);
        int found = extract_link(directory, content,linkUrls);
        closeSocket(sock);

        pthread_mutex_lock(&insertLock);
        web_setNum(directory,wg,1);
       	//free(content);
	   	pthread_mutex_unlock(&insertLock);

        int i, state;
        for(i=0; i<found; i++) {
            state = web_getNum(linkUrls[i],wg);
            pthread_mutex_lock(&insertLock);
            if(state == -3) {
                web_insertVertex(linkUrls[i],wg);
                queue_push(qurls,linkUrls[i]);
            }
            web_insertEdge(directory,linkUrls[i],wg);
            pthread_mutex_unlock(&insertLock);
        }
        pthread_mutex_lock(&insertLock);
        printf("Queue size:----------%d----------\n",qurls->curNum);
        pthread_mutex_unlock(&insertLock);
    }

    return NULL;
}





