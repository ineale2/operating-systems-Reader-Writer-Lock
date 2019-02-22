/*	lock.c - lock */
#include <xinu.h>

/* Lab 2: Complete this function */

local status insertReader(pid32 pid, qid16 q, int32 key);
local status insertWriter(pid32 pid, qid16 q, int32 key);

syscall lock(int32 ldes, int32 type, int32 lpriority) {


	//TODO: Update kill to deal with state of deleetion

	intmask mask;
	struct lockent * lptr;
	mask = disable();

	/* Check that arguments are valid */
	if( (type != READ && type !=WRITE) || (isbadlock(ldes)) ){
		restore(mask);
		return SYSERR;
	}

	lptr = &locktab[ldes];
	/* If the lock is not held, process gets lock */
	if(lptr->ltype == FREE){
		lptr->ltype = type; 
		if(type == READ){
			lptr->numReaders++;
		}
		restore(mask);
		return OK;
	}
	
	/* Lock is held */

	/* If the lock is held by reader and read is requested, then give lock if priority allows */	
	if(lptr->ltype == READ && type == READ){
		//If the queue is empty, give lock
		if(isempty(lptr->lqueue)){
			lptr->numReaders++;
			restore(mask);
			return OK;
		}
		/* Queue is not empty, there must be a writer at the front of the queue. */
		else if( lpriority >= firstid(lptr->lqueue) ){ 			
			lptr->numReaders++;
			restore(mask);
			return OK;
		}
		//This call should block. Let execution continue. 
	}
	
	/* This call should block */
	if(type == READ){
		insertReader(currpid, lptr->lqueue, lpriority);
		lptr->numReaders++;
		proctab[currpid].prstate = PR_LWAIT_R;
	}
	else{
		insertWriter(currpid, lptr->lqueue, lpriority);
		proctab[currpid].prstate = PR_LWAIT_W;
	}
	resched();

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
	return OK;

}
