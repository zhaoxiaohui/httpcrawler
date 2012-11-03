/*****
** urlqueue.c
** - implements the methods declared in urlqueue.h
*/

#include "urlqueue.h"

urlq_t * queue_init(urlq_t *q) {
    q->head = 0;
    q->tail = 0;
    q->size = 100000;/*暂时一个循环存放1000个URL*/
    q->curNum = 0;
    q->data = (char **)malloc(sizeof(char *) * (q->size));
    int i;
    for(i=0; i< q->size; i++)
        q->data[i] = (char *)malloc(sizeof(char)*256);
    return q;
}

int queue_size(urlq_t *q) {
    return q->size;
}

int queue_push(urlq_t *q, char *element) {
    if(q->curNum >= q->size)return 0;/*队列已满 不能在存放*/
    strcpy(q->data[q->tail++],element);
    if(q->tail >= q->size)q->tail %= q->size;/*实现循环存放*/
    q->curNum++;
    return q->curNum;
}

char *queue_front(urlq_t *q) {
    if(q->curNum == 0)return NULL;
    return q->data[q->head];
}

void queue_pop(urlq_t *q) {
    if(q->curNum==0)return;/*没有数据直接返回*/
    q->head++;
    q->curNum--;
    if(q->head >= q->size)q->head %= q->size;
}

void queue_clear(urlq_t *q) {
    /*设置head和tail即可 不用判断是否有数据*/
    q->head = q->tail = 0;
    int i;
    for(i=0; i<q->size; i++)
        free(q->data[i]);
    free(q);
}
