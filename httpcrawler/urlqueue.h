#ifndef _INCLUDE_STDLIB_
#define _INVLUDE_STDLIB_
#include <stdlib.h>
#endif

#ifndef _INCLUDE_STRING_
#define _INCLUDE_STRING_
#include <string.h>
#endif

#ifndef _URL_QUEUE_H
#define _URL_QUEUE_H


typedef struct _queue {
    int size;    /*队列可以存放的总个数*/
    int curNum;  /*当前已经存放的个数*/
    char **data;
    int head;
    int tail;
} urlq_t;

/*****
** initialize an empty urlq_t
** must be called first after a new urlq_t is declared
*/
urlq_t * queue_init(urlq_t *q);


/*****
** push a new element to the end of the urlq_t
** it's up to the client code to allocate and maintain memory of "element"
*/
int queue_push(urlq_t *q, char *element);

/*****
** return the first element in the urlq_t, or NULL when the urlq_t is empty
*/
char *queue_front(urlq_t *q);

/*****
** remove the first element (pointer) from the urlq_t
** set "release" to non-zero if memory deallocation is desired
*/
void queue_pop(urlq_t *q);

/*****
** remove all elements (pointers) from the urlq_t
** set "release" to non-zero if memory deallocation is desired
*/
void queue_clear(urlq_t *q);


#endif /* _RZ_C_QUEUE_ */
