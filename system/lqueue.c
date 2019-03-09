#include <xinu.h>
#include <lqueue.h>

local void swap(int32* a, int32* b);

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

status lq_sort(lqueue_t* lq){
	if(lq == NULL){
		return SYSERR;
	}
	int i;
	int j;
	int len = lq->back;
	for(j = lq->front; j < len-1; j++){
		int iMin = j;
		for(i = j+1; i < len; i++){
			if( lq->arr[i] < lq->arr[iMin]){
				iMin = i;
			}
		}

		swap(&lq->arr[j], &lq->arr[iMin]);

	}	

	return OK;
}

void swap(int32 *a, int32 *b){
	int temp = *a;
	*a = *b;
	*b = temp;
}

void print_lqueue(lqueue_t* lq, int print){
	
	if(print){
		int i = lq->front;
		kprintf("Printing lqueue: ");
		if(lq_empty(lq)) {
			kprintf("EMPTY\n"); 
			return;
		}

		while(i < lq->back){
			kprintf("%d ", lq->arr[i]);
			i++;
		}
		kprintf("\n");		
	}
	

}
