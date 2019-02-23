/*	ldelete.c - ldelete 	*/
#include <xinu.h>

/* Lab 2: Complete this function */

syscall ldelete( 
		int32 ldes	/* lock descriptor */
	)
{
	intmask mask;
	struct lockent* lptr;
	pid32 pid;
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

	/* Mark all processes as ready */
	resched_cntl(DEFER_START);
	while(nonempty(lptr->lqueue)){
		pid = dequeue(lptr->lqueue);
		ready(pid);
		/* Set flag to check on return from lock()*/
		proctab[pid].lck_del = 1;
	}

	resched_cntl(DEFER_STOP);
	restore(mask);
	return OK;
}
