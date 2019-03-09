/* chprio.c - chprio */

#include <xinu.h>
#include <lqueue.h>

/*------------------------------------------------------------------------
 *  chprio  -  Change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
void reset_maxprio(int32 ldes);
void prinh_release(pid32 pid);
void prinh_chprio(int32 ldes);


pri16	chprio(
	  pid32		pid,		/* ID of process to change	*/
	  pri16		newprio		/* New priority			*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process' table entry	*/
	pri16	oldprio;		/* Priority to return		*/

	mask = disable();
	if (isbadpid(pid)) {
		restore(mask);
		return (pri16) SYSERR;
	}
	prptr = &proctab[pid];
	oldprio = prptr->prprio;
	prptr->prprio = newprio;
	
	/* Lab 2 added code */
	if(prptr->prinh < newprio){
		prptr->prinh = newprio;
		/* Remove from ready list and reinsert */
		if(prptr->prstate == PR_READY){
			getitem(pid);
			insert(pid, readylist, prptr->prinh);	
		}
	}
	//For equal case... do nothing
	if(prptr->prinh > newprio){
		/* Reset its inherited priority */
		prinh_release(pid);
	}
	/* Is this process blocked? If so, update priority of other processes*/
	if(prptr->lockid != NO_LOCK){
		prinh_chprio(prptr->lockid);
	}
	resched();
	restore(mask);
	return oldprio;
}

void prinh_chprio(int32 ldes){
	pid32 pid;
	lqueue_t lq;
	int32 lock;
	int32 lockid;
	XDEBUG_KPRINTF("prinh_chprio: pid = %d ldes = %d\n", currpid, ldes);

	reset_maxprio(ldes);
	lq_init(&lq);
	status a = lq_enqueue(&lq, ldes);	
	if(a == SYSERR) XDEBUG_KPRINTF("enqueue err\n");
	while(!lq_empty(&lq)){
		lock = lq_dequeue(&lq);
		XDEBUG_KPRINTF("processing ldes = %d\n", lock);
		for(pid = 0; pid < NPROC; pid++){
			// For each process that holds the lock //
			if(proctab[pid].lockarr[lock] == HELD){
				XDEBUG_KPRINTF("prinh_chprio: updating pid = %d priority\n", pid);
				// Update its priority if necessary //
				prinh_release(pid);
				// Enqueue any lock that this process is blocked on that has maxprio less than newly blocked process //
				
				lockid = proctab[pid].lockid;
 				if(lockid != NO_LOCK){
					lq_enqueue(&lq, lockid);
					//Update max prio of next lock if necessary
					reset_maxprio(lockid);
				} 
				
				
			}
		}

	}
}
