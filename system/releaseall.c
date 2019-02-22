/*	releaseall.c - releaseall	*/
#include <xinu.h>
#include <stdarg.h>

local status release_lock(int32);
void printQueue(qid16, int);

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

	/* Decrement reader count */
	if(lptr->ltype == READ){
		lptr->numReaders--;
	}
	XDEBUG_KPRINTF("release_lock: ldes = %d\n", ldes);
	/*Check if there are any processes waiting on this lock */
	printQueue(lptr->lqueue, XDEBUG);
	if(isempty(lptr->lqueue)){
		XDEBUG_KPRINTF("release_lock: none waiting\n");
		lptr->ltype = FREE;
		
	}
	else{
		/* Processes are waiting on this lock */
		pid = firstid(lptr->lqueue);
		prptr = &proctab[pid];
		if(prptr->prstate == PR_LWAIT_R){
			lptr->ltype = READ;
			//ready(pid);
			//lptr->numReaders++;
			/* Mark all readers that are higher priority than writers as ready 	*/
			/* Note that rescheduling is deferred in the calling function 		*/
			/* Continue dequeueing processes until empty queue or found writer  */ 
			while(nonempty(lptr->lqueue) &&  proctab[firstid(lptr->lqueue)].prstate != PR_LWAIT_W){
				pid = dequeue(lptr->lqueue);
				XDEBUG_KPRINTF("release_lock: PID = %d reader waiting\n", pid);
				ready(pid);	
				lptr->numReaders++;
			}
		}
		else{
			XDEBUG_KPRINTF("release_lock: PID = %d writer waiting\n", pid);
			/* Only want last reader to release the writer */
			if(lptr->numReaders == 0){
				XDEBUG_KPRINTF("release_lock: no readers, releaseing writer\n");
				lptr->ltype = WRITE;
				/* Now actually dequeue the item */				
				pid = dequeue(lptr->lqueue);
				ready(pid);
			}
			else{
				XDEBUG_KPRINTF("release_lock: readers>0, writer still blocked\n");
			}
		}
	}
	return OK;
}

void printQueue(qid16 q, int print){
	pid32 curr = queuehead(q);
	if(print){
		kprintf("\nLock Queue:\n");
		kprintf("curr =  %d prev = %d next = %d HEAD\n", curr, queuetab[curr].qprev, queuetab[curr].qnext);
		while(queuetab[curr].qnext != queuetail(q)){
			curr = queuetab[curr].qnext;
			kprintf("curr =  %d prev = %d next = %d\n", curr, queuetab[curr].qprev, queuetab[curr].qnext);
		}
		curr = queuetab[curr].qnext;
		kprintf("curr =  %d prev = %d next = %d TAIL\n", curr, queuetab[curr].qprev, queuetab[curr].qnext);
		kprintf("\n");
	}

}
