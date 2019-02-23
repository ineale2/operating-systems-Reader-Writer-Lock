/* kill.c - kill */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  kill  -  Kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
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
		/* Fall through */

	case PR_READY:
		getitem(pid);		/* Remove from queue */
		/* Fall through */

	case PR_LWAIT_W:
		//Which locks is this process waiting on? update numreaders if necessary. maybe type too
		//Need to freeALL the locks this process has. Could just abuse releaseall() call and call with all locks. 
		getitem(pid); 		/* Remove from queue */
		//stop deferring rescheduing, break; and then dont fall through. Make sure process state is set before break;
		/* Fall through */
	case PR_LWAIT_R:
		//Which lock is it waiting on? 
		//Need to free ALL the locks this process has
		getitem(pid); 		/* Remove from queue */
		/* Fall through */
	default:
		prptr->prstate = PR_FREE;
	}

	restore(mask);
	return OK;
}
