/*	releaseall.c - releaseall	*/
#include <xinu.h>
#include <stdarg.h>

local status release_lock(int32);

syscall releaseall (int32 numlocks, ...) {

	//TODO: Error check  
	va_list valist;
	intmask mask;
	int i;
	mask = disable();
	status retVal = OK; 
	status temp;
	va_start(valist, numlocks);
	/* Stop rescheduling during lock release process */
	resched_cntl(DEFER_START);
	for(i = 0; i< numlocks; i++){
		temp  = release_lock(va_arg(valist, int32));
		if(temp == SYSERR) retVal = SYSERR;
	}
	/* Restart rescheduling */
	resched_cntl(DEFER_STOP);
	va_end(valist);
	restore(mask);
	return retVal;
}

status release_lock(int32 ldes){
	struct lockent* lptr = &locktab[ldes];
	struct procent* prptr;
	pid32 pid;
	/*Check if there are any processes waiting on this lock */
	if(isempty(lptr->lqueue)){
		lptr->ltype = FREE;
	}
	else{
		/* Processes are waiting on this lock */
		pid = dequeue(lptr->lqueue);
		prptr = &proctab[pid];
		if(prptr->prstate == PR_LWAIT_R){
			lptr->ltype = READ;
			ready(pid);
			/* Mark all readers that are higher priority than writers as ready 	*/
			/* Note that rescheduling is deferred in the calling function 		*/
			/* Continue dequeueing processes until empty queue or found writer  */ 
			while(nonempty(lptr->lqueue) || proctab[firstid(lptr->lqueue)].prstate != PR_LWAIT_W){
				pid = dequeue(lptr->lqueue);
				ready(pid);	
			}
		}
		else{
			lptr->ltype = WRITE;
			ready(pid);
		}
	}
	return OK;
}
