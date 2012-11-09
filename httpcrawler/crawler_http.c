#include "crawler_http.h"

extern threadPool *pool;
extern webGraph *wg;
extern urlq_t *qurls;
extern int badVertex;
extern int socketPort;
void pool_init(int threadNum, char *site){
	if(pool == NULL)
		pool = (threadPool *)malloc(sizeof(threadPool));
	/*初始化互斥锁*/
	pthread_mutex_init(&(pool->queue_lock), NULL);
	pthread_cond_init (&(pool->queue_have), NULL);
	
    int i;
    pool->threadNum = threadNum;
    pool->cur_running = 0;
    pool->alive = threadNum;
	pool->working = (int *)malloc(sizeof(int) * threadNum);
    if(pool->working == NULL){
        fprintf(stderr,"failed to malloc pool working\n");
        exit(0);
    }
    for(i=0; i<threadNum; i++)
        pool->working[i] = i+1;
    //pool->worked = 0;
	pool->destroy = 0;
	pool->website = (char *)malloc(sizeof(char) * 256);
	strcpy(pool->website, site);
	pool->threads = (pthread_t *)malloc(threadNum * sizeof(pthread_t));
    if(pool->threads == NULL){
        fprintf(stderr,"failed to malloc pool threads\n");
        exit(0);
    }
	for(i=0; i<threadNum; i++){
		pthread_create(&(pool->threads[i]), NULL, threadCrawling, &pool->working[i]);
	}	
}

int pool_destroy(){
	if(pool != NULL){
        pool->destroy = 1;
        printf("@@@@@ start to destroy pool  @@@@@\n");
        while(pool->alive){
            pthread_cond_signal(&(pool->queue_have));
        }
		free(pool->threads);
        free(pool->working);
		pthread_mutex_destroy(&(pool->queue_lock));
		pthread_cond_destroy(&(pool->queue_have));
		free(pool);
		pool = NULL;
        printf("@@@@@ destroy pool done  @@@@@\n");
	}
	return 0;
}

void *threadCrawling(void *arg){
	char *curUrl, host[256];
	int state;
    int *threadid = (int *)arg;
    char content[500000];
	while(1){
		pthread_mutex_lock(&(pool->queue_lock));
		while(qurls->curNum==0 && !pool->destroy){
			pthread_cond_wait(&(pool->queue_have), &(pool->queue_lock));
		}
		
		/*销毁*/
		if(pool->destroy){
            //printf("@@@@@ %d thread down\n",*threadid);
            pool->alive--;
			pthread_mutex_unlock(&(pool->queue_lock));
			pthread_exit(NULL);
		}

		curUrl = queue_front(qurls);
		queue_pop(qurls);
		strncpy(host, pool->website, strlen(pool->website));
        pool->cur_running++;
        printf("@@@@@ Running:%d\n", pool->cur_running);
		pthread_mutex_unlock(&(pool->queue_lock));
		/*检查是否插入*/
		state = web_getNum(curUrl, wg);
		if(state == 0){
			pthread_mutex_lock(&(pool->queue_lock));
	        web_setNum(curUrl,wg,-1);
        	pthread_mutex_unlock(&(pool->queue_lock));

			/*beging crawling*/
			printf("@@@@@ SITE: %s DIR:%s @@@@@\n",pool->website, curUrl);
			int flag = -1;
			int sock, res;
    		char body[300];
    		while(1) {//执行一次 避免if嵌套
        		sock = open_tcp2(pool->website,socketPort);
        		if(sock == -1) {
        			flag = -1;
            		break;
       			}
        		res = http_do_get(sock, host, curUrl);
        		if(res == 0) {
            		flag = -1;
     				closeSocket(sock);
            		break;
        		}
        		http_response_body2(sock,body);
        		res = http_response_status(body);
        		if(res/100 != 2) {
            		flag = -1;
            		closeSocket(sock);
            		break;
        		} else if(res/100 == 2) {
            		flag = 1;
           			 break;
        		}
    		}
			/*不能访问*/
			if(flag == -1){
				pthread_mutex_lock(&(pool->queue_lock));
				web_setNum(curUrl, wg, -2);
                printf("@@@@@ QueueSize: %d Working:%d  @@@@@\n",qurls->curNum, *threadid);
                printf("@@@@@ Worked:%d     Bad:%d\n\n",wg->vertexNum, ++badVertex);
                pthread_mutex_unlock(&(pool->queue_lock));
			}else{//可以访问
				//char *content = http_response_body(sock, http_response_len(body), body);
				//char content[500000];
                content[0] = '\0';
                strncat(content,body,strlen(body));
                int t = http_response_content(sock, content);
                if(t == -1)content[0] = '\0';
                char linkUrls[1000][256];
				int found = extract_link(curUrl, content, linkUrls);
				closeSocket(sock);
			
				pthread_mutex_lock(&(pool->queue_lock));
				web_setNum(curUrl, wg, 1);
				//content = NULL;
				pthread_mutex_unlock(&(pool->queue_lock));
				int i;
			    for(i=0; i<found; i++) {
            		state = web_getNum(linkUrls[i],wg);
            		pthread_mutex_lock(&(pool->queue_lock));
            		if(state == -3) {
                		web_insertVertex(linkUrls[i], wg);
                		queue_push(qurls,linkUrls[i]);
					
					    /*信号两 加1*/
					    pthread_cond_signal(&(pool->queue_have));
            		}
            		web_insertEdge(curUrl, linkUrls[i], wg);
            		pthread_mutex_unlock(&(pool->queue_lock));
        		}
        		pthread_mutex_lock(&(pool->queue_lock));
        		printf("@@@@@ QueueSize: %d Working:%d  @@@@@\n",qurls->curNum, *threadid);
                printf("@@@@@ Worked:%d       Bad:%d\n\n",wg->vertexNum, badVertex);
                pthread_mutex_unlock(&(pool->queue_lock));
			}
		}
        pthread_mutex_lock(&(pool->queue_lock));
        pool->cur_running--;
        pthread_mutex_unlock(&(pool->queue_lock));
	}				
}
