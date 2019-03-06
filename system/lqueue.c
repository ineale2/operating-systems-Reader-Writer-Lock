#include <xinu.h>
#include <lqueue.h>
int lq_empty(lqueue_t* lq){
	if(lq == NULL){
		return SYSERR;
	}
	return (lq->front == lq->back);
}

void lq_init(lqueue_t* lq){
	lq->front = 0;
	lq->back  = 0;
}

status lq_enqueue(lqueue_t* lq, int32 lock){
	if(lq == NULL || lq->back >= NLOCKS - 1){
		return SYSERR;
	}
	lq->arr[lq->back++] = lock; 

	return OK;
}

int32 lq_dequeue(lqueue_t* lq){
	if(lq == NULL || lq->front == lq->back){
		return SYSERR;
	}
	int32 lock = lq->arr[lq->front++];
	return lock;
}
