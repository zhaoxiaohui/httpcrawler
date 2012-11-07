#include "crawler_http.h"

extern threadPool pool;

void poolInit(int threadNum, char *site){
	if(pool == NULL)
		pool = (threadPool *)malloc(sizeof(threadPool));
	/*初始化互斥锁*/
	pthread_mutex_init(&(pool->queue_lock), NULL);
	pthread_cond_init (&(pool->queue_have), NULL);
	
	pool->threadNum = threadNum;
	pool->working = 0;
	pool->destroy = 0;
	pool->website = (char *)malloc(sizeof(char) * 256);
	strcpy(pool->website, site);
	pool->threads = (pthread_t *)malloc(threadNum * sizeof(pthread_t));

	int i;
	for(i=0; i<threadNum; i++){
		pthread_creat(&(pool->threads[i]), NULL, threadCrawling, NULL);
	}	
}

int poolDestroy(){
	if(pool != NULL){
		int i;
		for(i=0; i<pool->threadNum; i++){
			free(pool->threads[i]);
			pool->threads[i] = NULL;
		}
		pool->threadNum = 0;
		pthread_mutex_destroy(&(pool->queue_lock));
		pthrad_cond_destroy(&(pool->queue_have));
		free(pool);
		pool = NULL;
	}
	return 0;
}

void *threadCrawling(void *arg){
	char *curUrl;
	int state;
	while(1){
		pthread_mutex_lock(&(pool->queue_lock));
		while(qurls->curNum==0 && !pool->destroy){
			pthread_cond_wait(&(pool->queue_have,&(pool->queue_lock)));
		}
		
		/*销毁*/
		if(pool->destroy){
			pthread_mutext_unlock(&(pool->queue_lock));
			pthread_exit(NULL);
		}

		curUrl = queue_front(qurls);
		queue_pop(qurls);
		pool->working++;
		pthread_mutex_unlock(&(pool->queue_lock));
		/*检查是否插入*/
		state = web_getNum(curUrl, wg);
		if(state == 0){
			pthread_mutex_lock(&(pool->queue_lock));
	                web_setNum(nextUrl,wg,-1);
        	        pthread_mutex_unlock(&(pool->queue_lock));

			/*beging crawling*/
			printf("@@@@@ SITE: %s DIR:%s @@@@@\n",pool->websit, curUrl);
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
			/*不能访问*/
			if(flag == -1){
				pthread_mutex_lock(&(pool->queue_lock));
				web_setNum(curUrl, wg, -2);
				pool->working--;
				pthread_mutex_unlock(&(pool->queue_lock));
				closeSocket(sock);
			}else{//可以访问
				char *content = http_response_body(sock, http_response_len(body), body);
				char linkUrls[1000][256];
				int found = extract_link(curUrl, content, linkUrls);
				closeSocket(sock);
				
				pthread_mutex_lock(&(pool->queue_lock));
				web_setNum(curUrl, wg, 1);
				free(content);
				content = NULL;
				pthread_mutex_unlock(&(pool->queue_lock));
				
				int i;
			        for(i=0; i<found; i++) {
            				state = web_getNum(linkUrls[i],wg);
            				pthread_mutex_lock(&(pool->queue_lock));
            				if(state == -3) {
                				web_insertVertex(linkUrls[i], wg);
                				queue_push(qurls,linkUrls[i]);
						
						/*信号两 加1*/
						pthread_cond_signal(&(pool_queue_have));
            				}
            				web_insertEdge(curUrl, linkUrls[i], wg);
            				pthread_mutex_unlock(&(pool->queue_lock));
        			}
        			pthread_mutex_lock(&(pool->queue_lock));
        			printf("Queue size:----------%d----------\n",qurls->curNum);
				pool->working--;
        			pthread_mutex_unlock(&(pool->queue_lock));

			}
		}	
	}				
}
