/*	lock.c - lock */
#include <xinu.h>

/* Lab 2: Complete this function */

void printQueue(qid16, int);
local status insertReader(pid32 pid, qid16 q, int32 key);
local status insertWriter(pid32 pid, qid16 q, int32 key);

syscall lock(int32 ldes, int32 type, int32 lpriority) {


	//TODO: Update kill to deal with state of deleetion

	intmask mask;
	struct lockent * lptr;
	struct procent * prptr;
	mask = disable();
	prptr = &proctab[currpid];
	/* Check that arguments are valid */
	if( (type != READ && type !=WRITE) || (isbadlock(ldes)) || (locktab[ldes].lstate != L_USED)  ){
		restore(mask);
		return SYSERR;
	}

	lptr = &locktab[ldes];
	/* If the lock is not held, process gets lock */
	if(lptr->ltype == FREE){
		XDEBUG_KPRINTF("lock: ldes %d was free\n", ldes);
		lptr->ltype = type; 
		if(lptr->ltype == READ){
			 lptr->numReaders++;
		}
		prptr->lockarr[ldes] = HELD;
		restore(mask);
		return OK;
	}
	
	/* Lock is held */

	/* If the lock is held by reader and read is requested, then give lock if priority allows */	
	if(lptr->ltype == READ && type == READ){
		XDEBUG_KPRINTF("lock: ldes %d was held\n", ldes);
		//If the queue is empty, give lock
		if(isempty(lptr->lqueue)){
			XDEBUG_KPRINTF("lock: ldes %d readers only. granting lock\n", ldes);
			lptr->numReaders++;
			prptr->lockarr[ldes] = HELD;
			restore(mask);
			return OK;
		}
		/* Queue is not empty, there must be a writer at the front of the queue. */
		else if( lpriority >= firstkey(lptr->lqueue) ){ 			
			XDEBUG_KPRINTF("lock: ldes %d reader higher prio than writer. granting lock\n", ldes);
			lptr->numReaders++;
			prptr->lockarr[ldes] = HELD;
			restore(mask);
			return OK;
		}
		//This call should block. Let execution continue. 
	}
	
	/* This call should block */
	if(type == READ){
		insertReader(currpid, lptr->lqueue, lpriority);
		proctab[currpid].prstate = PR_LWAIT_R;
		XDEBUG_KPRINTF("lock: ldes %d blocking and placing in read state\n", ldes);
	}
	else{
		insertWriter(currpid, lptr->lqueue, lpriority);
		proctab[currpid].prstate = PR_LWAIT_W;
		XDEBUG_KPRINTF("lock: ldes %d blocking and placing in write state\n", ldes);
	}
	resched();
	XDEBUG_KPRINTF("lock: granted after blocking!\n\n");
	/* Check if the lock was deleted */
	if(proctab[currpid].lck_del == 1){
		/* Reset flag and return error */
		proctab[currpid].lck_del = 0;	
		restore(mask);
		return DELETED;
	}
	/* Someone else has released the lock, and it is now held */	
	prptr->lockarr[ldes] = HELD;
	restore(mask);
	return OK;
}

status insertReader(pid32 pid, qid16 q, int32 key){
	/* Insert reader according to priority into queue */
	/* Break ties by putting reader at the end 		  */
	return insert(pid, q, key);
}

status insertWriter(pid32 pid, qid16 q, int32 key){
	/* Insert writer according to priority into queue */
	/* Break ties by putting writer at the front	  */
	/* NOTE: This call is just insert() with while loop modified to break ties differently */

	int16	curr;			/* Runs through items in a queue*/
	int16	prev;			/* Holds previous node index	*/

	if (isbadqid(q) || isbadpid(pid)) {
		return SYSERR;
	}

	curr = firstid(q);
	while (queuetab[curr].qkey > key) {
		curr = queuetab[curr].qnext;
	}

	/* Insert process between curr node and previous node */

	prev = queuetab[curr].qprev;	/* Get index of previous node	*/
	queuetab[pid].qnext = curr;
	queuetab[pid].qprev = prev;
	queuetab[pid].qkey = key;
	queuetab[prev].qnext = pid;
	queuetab[curr].qprev = pid;
	XDEBUG_KPRINTF("insertWriter: printing queue\n");
	printQueue(q, XDEBUG);
	return OK;

}
