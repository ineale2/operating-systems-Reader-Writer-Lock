/*	releaseall.c - releaseall	*/
#include <xinu.h>
#include <stdarg.h>

status release_lock(int32, pid32);
local void reset_maxprio(int32 ldes);
local void prinh_release(pid32 pid);
local void unblock(pid32 pid, int32 ldes);
local void tieBreaker(int32 ldes);
void printQueue(qid16, int);

syscall releaseall (int32 numlocks, ...) {

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
		temp  = release_lock(va_arg(valist, int32), currpid);
		if(temp == SYSERR) retVal = SYSERR;
	}
	prinh_release(currpid);
	/* Restart rescheduling */
	resched_cntl(DEFER_STOP);
	va_end(valist);
	restore(mask);
	return retVal;
}
/* rel_pid is the pid that is releasing this lock */
status release_lock(int32 ldes, pid32 rel_pid){
	struct lockent* lptr = &locktab[ldes];
	struct procent* prptr;
	pid32 pid;

	/* Only allow a process to release a lock that it holds */
	prptr = &proctab[rel_pid];
	if( (prptr->lockarr[ldes] != HELD) || (isbadlock(ldes)) || (locktab[ldes].lstate != L_USED) ) {
		return SYSERR;
	}

	/* Decrement reader count */
	if(lptr->ltype == READ){
		lptr->numReaders--;
	}
	/* This process no longer holds this lock */
	prptr->lockarr[ldes] = NOT_HELD;
	XDEBUG_KPRINTF("release_lock: ldes = %d\n", ldes);
	/*Check if there are any processes waiting on this lock */
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
			/* Mark all readers that are higher priority than writers as ready 	*/
			/* Note that rescheduling is deferred in the calling function 		*/
			/* Continue dequeueing processes until empty queue or found writer  */ 
			while(nonempty(lptr->lqueue) &&  proctab[firstid(lptr->lqueue)].prstate != PR_LWAIT_W){
				XDEBUG_KPRINTF("release_lock: PID = %d reader waiting\n", pid);
				pid = dequeue(lptr->lqueue);
				unblock(pid, ldes);
				lptr->numReaders++;
			}
			if(nonempty(lptr->lqueue) && proctab[firstid(lptr->lqueue)].prstate == PR_LWAIT_W){
				//Stopped dequeueing because there was a writer
				tieBreaker(ldes);
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
				unblock(pid, ldes);
			}
			else{
				XDEBUG_KPRINTF("release_lock: readers>0, writer still blocked\n");
			}
		}
	}
	/* Now that processes have been dequeued and marked ready, reset the locks max prio */
	reset_maxprio(ldes);
	return OK;
}

void tieBreaker(int32 ldes){
	//Release any readers whose priorities are equal to the first reader. 
	struct lockent* lptr = &locktab[ldes];
	qid16 q      = lptr->lqueue;
	int32 wlprio = firstkey(q);
	pid32 writer = firstid(q);
	pid32 next   = queuetab[writer].qnext;
	while(next != queuetail(q) && proctab[next].prstate == PR_LWAIT_R && wlprio == queuetab[next].qkey){
		//Remove from queue and unblock
		getitem(next);
		unblock(next, ldes);
		lptr->numReaders++;
		//getitem has changed writers next
		next = queuetab[writer].qnext;
	} 
}
//TODO: Does prinh_release take care of transitivity? Does it need to look at lockid?

void reset_maxprio(int32 ldes){
	struct lockent * lptr = &locktab[ldes];
	pri16 lock_maxpri = 0;
	qid16 curr;
	
	/* Loop over all processes in this locks queue */
	curr = firstid(lptr->lqueue);
	while(curr != queuetail(lptr->lqueue)){
		if(proctab[curr].prinh > lock_maxpri){
			lock_maxpri = proctab[curr].prinh;
		}
		curr = queuetab[curr].qnext;
	}
	/*Assign new lock maxprio */
	lptr->maxprio = lock_maxpri;
}

void prinh_release(pid32 pid){
	struct procent * prptr = &proctab[pid];
	struct lockent * lptr;
	int ldes;
	/* Reset to initial priority */
	prptr->prinh = prptr->prprio;

	/* Update processes priority to the maximum of processes in the wait queues of locks it holds */	
	for(ldes = 0; ldes < NLOCKS; ldes++){
		lptr = &locktab[ldes];
		if(prptr->lockarr[ldes] == HELD && prptr->prinh < lptr->maxprio){
				prptr->prinh = lptr->maxprio;
		}
	}

}

void unblock(pid32 pid, int32 ldes){
	ready(pid);
	proctab[pid].lockid = NO_LOCK;
	proctab[pid].lockarr[ldes] = HELD;
}

void printQueue(qid16 q, int print){
	pid32 curr = queuehead(q);
	if(print){
		kprintf("\nLock Queue:\n");
		kprintf("curr =  %d prev = %d key = %d next = %d HEAD\n", curr, queuetab[curr].qprev, 
		queuetab[curr].qkey, queuetab[curr].qnext);
		while(queuetab[curr].qnext != queuetail(q)){
			curr = queuetab[curr].qnext;
			kprintf("curr =  %d prev = %d key = %d next = %d\n", curr, queuetab[curr].qprev, 
			queuetab[curr].qkey, queuetab[curr].qnext);
		}
		curr = queuetab[curr].qnext;
		kprintf("curr =  %d prev = %d key = %d next = %d TAIL\n", curr, queuetab[curr].qprev, 
		queuetab[curr].qkey, queuetab[curr].qnext);
		kprintf("\n");
	}

}
