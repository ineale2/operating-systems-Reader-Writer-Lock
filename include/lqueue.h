/* lqueue.h - defines lock queue data structure */

typedef struct lqueue{
	int32 arr[NLOCKS];
	int front;
	int back;	
} lqueue_t;

int lq_empty(lqueue_t*);

void lq_init(lqueue_t*);

status lq_enqueue(lqueue_t*, int32);

int32 lq_dequeue(lqueue_t*);

status lq_sort(lqueue_t*);

void print_lqueue(lqueue_t* lq, int print);

