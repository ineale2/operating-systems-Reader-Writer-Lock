/*	lock.c - lock */
#include <xinu.h>

/* Lab 2: Complete this function */

syscall lock(int32 ldes, int32 type, int32 lpriority) {


	//TODO: Update kill to deal with state of deleetion
	//TODO: Make sure all return paths restoremask

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
	//TODO: FINISH THIS	 decide how to store reader or writer (in  process table or as a state ) and then update the code below to write that correctly
	if(type == READ){
		insertReader(lptr->lqueue);
		lptr->numReaders++;
	}
	else{
		insertWriter(lptr->lqueue);
	}
	proctab[currpid].prstate = PR_LWAIT;
	resched();

	restore(mask);
	return OK;
}
