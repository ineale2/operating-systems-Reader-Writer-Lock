/* kill.c - kill */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  kill  -  Kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
local void kill_release(pid32, int32);
status release_lock(int32, pid32);
void prinh_chprio(int32 ldes);

syscall	kill(
	  pid32		pid		/* ID of process to kill	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process' table entry	*/
	int32	i;			/* Index into descriptors	*/

	mask = disable();
	if (isbadpid(pid) || (pid == NULLPROC)
	    || ((prptr = &proctab[pid])->prstate) == PR_FREE) {
		restore(mask);
		return SYSERR;
	}

	if (--prcount <= 1) {		/* Last user process completes	*/
		xdone();
	}

	send(prptr->prparent, pid);
	for (i=0; i<3; i++) {
		close(prptr->prdesc[i]);
	}
	freestk(prptr->prstkbase, prptr->prstklen);

	/* Release all locks this process holds 				*/
	/* Avoid calling resched() while interrupts are disabled */
	resched_cntl(DEFER_START);
	for(i = 0; i<NLOCKS; i++){
		kill_release(pid, i);
	}	

	switch (prptr->prstate) {
	case PR_CURR:
		prptr->prstate = PR_FREE;	/* Suicide */
		resched();

	case PR_SLEEP:
	case PR_RECTIM:
		unsleep(pid);
		prptr->prstate = PR_FREE;
		break;

	case PR_WAIT:
		semtab[prptr->prsem].scount++;
		prptr->prstate = PR_FREE;
		break;

	case PR_READY:
		getitem(pid);		/* Remove from queue */
		prptr->prstate = PR_FREE;
		break;

	case PR_LWAIT_W:
		getitem(pid); 		/* Remove from queue */
		if(prptr->lockid != NO_LOCK){
			prinh_chprio(prptr->lockid);
		}
		prptr->prstate = PR_FREE;
		break;

	case PR_LWAIT_R:
		//Which lock is it waiting on? 
		//Need to free ALL the locks this process has
		getitem(pid); 		/* Remove from queue */
		if(prptr->lockid != NO_LOCK){
			prinh_chprio(prptr->lockid);
		}
		prptr->prstate = PR_FREE;
		break;

	default:
		prptr->prstate = PR_FREE;
	}
	resched_cntl(DEFER_STOP);
	restore(mask);
	return OK;
}

void kill_release(pid32 pid, int32 ldes){
	
	if(proctab[pid].lockarr[ldes] == HELD){
		release_lock(ldes, pid);
	}

}
