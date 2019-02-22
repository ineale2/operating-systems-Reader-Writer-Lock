/*	ldelete.c - ldelete 	*/
#include <xinu.h>

/* Lab 2: Complete this function */

syscall ldelete( 
		int32 ldes	/* lock descriptor */
	)
{
	intmask mask;
	struct lockent* lptr;
	mask = disable();

	if(isbadlock(ldes)){
		restore(mask);
		return SYSERR;
	}
	lptr = &locktab[ldes];

	if(lptr->lstate == L_FREE){
		restore(mask);
		return SYSERR;
	}

	/* Set state to free */
	lptr->lstate = L_FREE;
	resched_cntl(DEFER_START);
	while(nonempty(lptr->lqueue)){
		ready(dequeue(lptr->lqueue));
		//For part 2: mark each of these in process tables and check for this value in lock.c
	}

	resched_cntl(DEFER_STOP);
	restore(mask);
	return OK;
}
